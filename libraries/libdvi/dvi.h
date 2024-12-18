#ifndef _DVI_H
#define _DVI_H

#ifdef __cplusplus
extern "C" {
#endif

#define DELAYED_COPY_CODE(n) __noinline __attribute__((section(".delayed_code."))) n
#define DELAYED_COPY_DATA(n) __attribute__((section(".delayed_data."))) n

// Pull into RAM but apply unique section suffix to allow linker GC
#define __dvi_func(x)  DELAYED_COPY_CODE(x)
#define __dvi_const(x) DELAYED_COPY_DATA(x)

#define N_TMDS_LANES 3
#define TMDS_SYNC_LANE 0 // blue!

#include "pico/util/queue.h"

#include "dvi_config_defs.h"
#include "dvi_timing.h"
#include "dvi_serialiser.h"
#include "util_queue_u32_inline.h"

#if 0
typedef void (*dvi_callback_t)(void);
#endif

struct dvi_inst {
	// Config ---
	const struct dvi_timing *timing;
	struct dvi_lane_dma_cfg dma_cfg[N_TMDS_LANES];
	struct dvi_timing_state timing_state;
	struct dvi_serialiser_cfg* ser_cfg;
#if 0
	// Called in the DMA IRQ once per scanline -- careful with the run time!
	dvi_callback_t scanline_callback;
#endif

	// State ---
	struct dvi_scanline_dma_list dma_list_vblank_sync;
	struct dvi_scanline_dma_list dma_list_vblank_nosync;
	struct dvi_scanline_dma_list dma_list_active;
	struct dvi_scanline_dma_list dma_list_error;

	// After a TMDS buffer has been enqueue via a control block for the last
	// time, two IRQs must go by before freeing. The first indicates the control
	// block for this buf has been loaded, and the second occurs some time after
	// the actual data DMA transfer has completed.
	uint32_t *tmds_buf_release_next;
	uint32_t *tmds_buf_release;
	// Remember how far behind the source is on TMDS scanlines, so we can output
	// solid colour until they catch up (rather than dying spectacularly)
	uint32_t late_scanline_ctr;
	// count production errors (scanlines were not ready in time)
	uint32_t scanline_errors;
	// enable/disable scan line emulation (alternating blank lines)
	uint8_t scanline_emulation;

	// Encoded scanlines:
	queue_t q_tmds_valid;
	queue_t q_tmds_free;

#if 0
	// Either scanline buffers or frame buffers:
	queue_t q_colour_valid;
	queue_t q_colour_free;
#endif
};

// Set up data structures and hardware for DVI.
void dvi_init(struct dvi_inst *inst, uint spinlock_tmds_queue, uint spinlock_colour_queue);

void dvi_destroy(struct dvi_inst *inst, uint irq_num);

// Call this after calling dvi_init(). DVI DMA interrupts will be routed to
// whichever core called this function. Registers an exclusive IRQ handler.
void dvi_register_irqs_this_core(struct dvi_inst *inst, uint irq_num);

// Start actually wiggling TMDS pairs. Call this once you have initialised the
// DVI, have registered the IRQs, and are producing rendered scanlines.
void dvi_start(struct dvi_inst *inst);

// TMDS encode worker function: core enters and doesn't leave, but still
// responds to IRQs. Repeatedly pop a scanline buffer from q_colour_valid,
// TMDS encode it, and pass it to the tmds valid queue.
#if 0
void dvi_scanbuf_main_8bpp(struct dvi_inst *inst);
void dvi_scanbuf_main_16bpp(struct dvi_inst *inst);
#endif

// Same as above, but each q_colour_valid entry is a framebuffer
#if 0
void dvi_framebuf_main_8bpp(struct dvi_inst *inst);
void dvi_framebuf_main_16bpp(struct dvi_inst *inst);
#endif

#ifdef __cplusplus
}
#endif

#endif
