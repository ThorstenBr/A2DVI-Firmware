#include <string.h>
#include <hardware/dma.h>

static int dmacopy_channel = -1;

void __noinline memcpy32(void *dst, const void *src, uint32_t size)
{
    dma_channel_config c;

    // Nothing to do!
    if(!size) return;

    // Cowardly avoid unaligned transfers, let memcpy() handle them.
    if((size < 64) || (size & 0x3) || (((uint32_t)dst) & 0x3) || (((uint32_t)src) & 0x3))
    {
        memcpy(dst, src, size);
        return;
    }

    // 32 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // No DREQ is selected, so the DMA transfers as fast as it can.

    // Get a free channel, panic() if there are none
    if(dmacopy_channel == -1)
        dmacopy_channel = dma_claim_unused_channel(true);

    c = dma_channel_get_default_config(dmacopy_channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(dmacopy_channel, &c, dst, src, (size >> 2), true);

    dma_channel_wait_for_finish_blocking(dmacopy_channel);

    dma_channel_abort(dmacopy_channel);

    // Deinit the DMA channel
    c = dma_channel_get_default_config(dmacopy_channel);
    dma_channel_configure(dmacopy_channel, &c, NULL, NULL, 0, false);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
}
