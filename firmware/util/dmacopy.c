#include <string.h>
#include <hardware/dma.h>
#include <debug/debug.h>

static int                dmacopy_channel = -1;
static dma_channel_config dmacopy_config;

// disable the DMA for copying memory, always use normal memcpy instead
void dmacopy_disable_dma(void)
{
    if (dmacopy_channel >= 0)
    {
        // cleanup and free DMA channel
        dma_channel_cleanup(dmacopy_channel);
        dma_channel_unclaim(dmacopy_channel);
    }
    // prevent the DMA use
    dmacopy_channel = -2;
}

void __noinline memcpy32(void *dst, const void *src, uint32_t size)
{
    // Nothing to do!
    if(!size)
        return;

    bool no_dma = false;

    // Cowardly avoid unaligned transfers, let memcpy() handle them.
    if((size < 64) || (size & 0x3) || (((uint32_t)dst) & 0x3) || (((uint32_t)src) & 0x3))
    {
        no_dma = true;
        debug_error(7);
    }
    else
    if (dmacopy_channel == -2)
        no_dma = true;

    if (no_dma)
    {
        memcpy(dst, src, size);
        return;
    }

    // Get a free channel, panic() if there are none
    if(dmacopy_channel == -1)
    {
        dmacopy_channel = dma_claim_unused_channel(true);
        if (dmacopy_channel == -1)
        {
            debug_error(5);
            while (1);
        }

        dmacopy_config = dma_channel_get_default_config(dmacopy_channel);
        // 32 bit transfers. Both read and write address increment after each
        // transfer (each pointing to a location in src or dst respectively).
        // No DREQ is selected, so the DMA transfers as fast as it can.
        channel_config_set_transfer_data_size(&dmacopy_config, DMA_SIZE_32);
        channel_config_set_read_increment(&dmacopy_config, true);
        channel_config_set_write_increment(&dmacopy_config, true);
    }

    dma_channel_configure(dmacopy_channel, &dmacopy_config, dst, src, (size >> 2), true);
    dma_channel_wait_for_finish_blocking(dmacopy_channel);
}
