#pragma once

#include "dvi.h"

extern struct dvi_inst dvi0;

#define DVI_WORDS_PER_CHANNEL (640/2)
#define DVI_APPLE2_XOFS       ((640/2-560/2)/2)

#define dvi_get_scanline(tmdsbuf)  \
	uint32_t* tmdsbuf;\
	queue_remove_blocking_u32(&dvi0.q_tmds_free, &tmdsbuf);

#define dvi_scanline_rgb(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue) \
        uint32_t *tmdsbuf_blue  = tmdsbuf+DVI_APPLE2_XOFS; \
        uint32_t* tmdsbuf_green = tmdsbuf_blue  + DVI_WORDS_PER_CHANNEL; \
        uint32_t* tmdsbuf_red   = tmdsbuf_green + DVI_WORDS_PER_CHANNEL;

#define dvi_send_scanline(tmdsbuf) \
	queue_add_blocking_u32(&dvi0.q_tmds_valid, &tmdsbuf);

extern void update_text_flasher();
extern void render_text();
