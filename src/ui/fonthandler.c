#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <fbdraw.h>

#include "fonthandler.h"
#define FONT_FACE "/root/font.ttf"
int ft_draw_char(uint32_t x, uint32_t y, FT_Bitmap* bmp, uint32_t color);

FT_Library library;
FT_Face face;

int ft_init(void) {
	int error;
	error = FT_Init_FreeType(&library);
	if(error) {
		perror("FT_Init_FreeType");
		return error;
	}

	error = FT_New_Face(library,
			FONT_FACE,
			0,
			&face);

	if(error == FT_Err_Unknown_File_Format) {
		fprintf(stderr, "Font format not supported.\n");
		return error;
	} else if(error) {
		perror("FT_NewFace");
		return error;
	}

	fprintf(stderr, "Got font file with characteristics\n"
			"num_faces %lu\n"
			"num_glyphs %lu\n"
			"family_name %s\n"
			"style_name %s\n"
			"num_charmaps %d\n",
			face->num_faces,
			face->num_glyphs,
			face->family_name,
			face->style_name,
			face->num_charmaps);

	error = FT_Set_Char_Size(face, 0, 16*64, 96, 96);
	if(error) {
		perror("FT_Set_Char_Size");
		return error;
	}
	return 0;
}

int ft_print(textfield* t) {
	int error;
	FT_GlyphSlot slot = face->glyph;
	uint32_t x, y;
	char* str;
	uint8_t printable = 1;

	str = t->caption;
	x = t->x;
	y = t->y;

	while(*str) {
		error = FT_Load_Char(face, *str++,
				FT_LOAD_RENDER |
				FT_LOAD_DEFAULT);
		if(error) {
			perror("FT_Load_Char");
			return error;
		}

		/* TODO: smart word breaking, handle newlines */
		if((x + (slot->advance.x >> 6)) > (t->x + t->width)) {
			x = t->x;
			y += 24;
			if(y >= (t->y + t->height)) printable = 0;
		}

		if(printable) {
			ft_draw_char(x + slot->bitmap_left,
				y - slot->bitmap_top , &slot->bitmap, t->color);
			x += slot->advance.x >> 6;
		}
	}
	return 0;
}

int ft_draw_char(uint32_t x, uint32_t y, FT_Bitmap* bmp, uint32_t color) {
	uint32_t x_i, y_i, colormask;
	uint8_t* buffer = (uint8_t*)bmp->buffer;
	for(y_i = 0; y_i < bmp->rows; y_i++) {
		for(x_i = 0; x_i < bmp->pitch; x_i++) {
			if(x_i < bmp->width) {
				colormask = (buffer[0] << 16) | 
						(buffer[0] << 8) |
						buffer[0];
				fb_draw_pixel(x + x_i, y + y_i, color & colormask);
			}
			buffer++;
		}
	}
	return 0;
}
