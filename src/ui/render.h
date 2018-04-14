#ifndef RENDER_H__
#define RENDER_H__

#include <inttypes.h>

#include "toolkit.h"

/* no theming support yet */
#define R_ACTIVE_COLOR		0x6fc2ee
#define R_FONT_COLOR 		0x6fc2ee
#define R_BG_COLOR 			0x192ce6
#define R_INACTIVE_COLOR 	0x42748f
#define R_DISABLED_COLOR 	0x213a47

int ui_draw_widget(widget* w, uint8_t active);

#endif
