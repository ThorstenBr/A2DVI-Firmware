#ifndef _DVI_PIN_CONFIG_H
#define _DVI_PIN_CONFIG_H

// This file defines the TMDS pair layouts on a handful of boards I have been
// developing on. It's not a particularly important file -- just saves some
// copy + paste.

#include "dvi_serialiser.h"

// ----------------------------------------------------------------------------
// PicoDVI boards

// Ralle Palaveev A2DVI
static struct dvi_serialiser_cfg DELAYED_COPY_DATA(pico_a2dvi_cfg) = {
	.pio = pio1,
	.sm_tmds = {0, 1, 2},
	.pins_tmds = {20, 18, 16},
	.pins_clk = 14,
	.invert_diffpairs = false
};

#endif
