#include <stdio.h>
#include <inttypes.h>
#include <fbdraw.h>

#include "fonthandler.h"
#include "toolkit.h"
#include "render.h"

int ui_draw_button(widget* w, uint8_t active);
int ui_draw_label(widget* w, uint8_t active);
int ui_draw_option_button(widget* w, uint8_t active);
int ui_draw_check_button(widget* w, uint8_t active);

int ui_draw_widget(widget* w, uint8_t active) {
	switch(w->type) {
		case BUTTON:
			return ui_draw_button(w, active);
			break;
		case LABEL:
			return ui_draw_label(w, active);
			break;
		case CHECK_BOX:
			return ui_draw_check_button(w, active);
			break;
		case OPTION_BOX:
			fprintf(stderr, "OPTION_BOX not implemented\n");
			break;
		default:
			fprintf(stderr, "unknown widget type %d\n", w->type);
	}
	return 1;
}

int ui_draw_button(widget* w, uint8_t active) {
	uint32_t t, color, height, width;
	textfield txt = {
			.caption = w->caption,
			.x = w->xpos + 8,
			.y = w->ypos + 20
	};
	
	width = w->width ? w->width : 96;
	color = active ? R_ACTIVE_COLOR : R_INACTIVE_COLOR;
	height = w->height ? w->height : 24;

	txt.width = width - 10;
	txt.height = height - 6;
	txt.color = color;

	for(t = w->xpos + 1; t < w->xpos + width; t++) {
		fb_draw_pixel(t, w->ypos, color);
		fb_draw_pixel(t, w->ypos + height, color);
	}

	for(t = w->ypos + 1; t < w->ypos + height; t++) {
		fb_draw_pixel(w->xpos, t, color);
		fb_draw_pixel(w->xpos + width, t, color);
	}

	ft_print(&txt);
	return 0;
}

int ui_draw_label(widget* w, uint8_t active) {
	textfield txt = {
		.caption = w->caption,
		.x = w->xpos,
		.y = w->ypos + 18,
		.color = R_FONT_COLOR,
		.height = (w->height) ? w->height : 24,
		.width = (w->width) ? w->width : (320 - w->xpos)
	};
	ft_print(&txt);
	return 0;
}

int ui_draw_check_button(widget* w, uint8_t active) {
	uint32_t color, t;
	textfield txt = {
		.caption = w->caption,
		.x = w->xpos + 26,
		.y = w->ypos + 18,
		.height = (w->height) ? w->height : 24,
		.width = (w->width) ? w->width : (320 - w->xpos)
	};

	txt.color = color = (active) ? R_ACTIVE_COLOR : R_INACTIVE_COLOR;

	for(t = 1; t < 18; t++) {
		fb_draw_pixel(w->xpos + t, w->ypos, color);
		fb_draw_pixel(w->xpos + t, w->ypos + 18, color);
		fb_draw_pixel(w->xpos, w->ypos + t, color);
		fb_draw_pixel(w->xpos + 18, w->ypos + t, color);
	}
	
	color = w->reserved[31] ? color : 0x00000000;
	for(t = 0; t < 121; t++)
		fb_draw_pixel(w->xpos + 4 + t % 11, w->ypos + 4 + t/11, color);

	ft_print(&txt);

	return 0;
}

int ui_draw_option_button(widget* w, uint8_t active) {
	
	return 0;
}
