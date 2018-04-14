#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <toolkit.h>

void patch_nest(window* win, widget* unused);

uint8_t flags;

int main(int argc, char** argv) {
	window* win;
	widget* w;
	int t;

	const char* cap[] = {	"Entries to hosts file",
							"Modify hidden setting",
							"Clear system logs"};
	const uint32_t y[] = {124, 148, 172};

	if(ui_init()) {
		fprintf(stderr, "can not initialise ui toolkit\n");
		return EXIT_FAILURE;
	}

	win = ui_create_window(NOT_SCROLLABLE);
	
	for(t = 0; t < 3; t++) {
		w = ui_create_widget(CHECK_BOX);
		ui_set_widget_position(w, 54, y[(size_t)t]);
		ui_set_caption(w, cap[t]);
		if(t < argc - 1)
			ui_set_checkbox_value(w, atoi(argv[t + 1]));
		ui_set_widget_tag(w, (uint32_t)(t + 1));
		ui_add_widget(win, w);
	}
	
	w = ui_create_widget(BUTTON);
	ui_set_widget_position(w, 112, 212);
	ui_set_caption(w, "Quit");
	ui_set_callback(BUTTON_PRESSED, w, ui_quit_loop);
	ui_add_widget(win, w);

	w = ui_create_widget(BUTTON);
	ui_set_caption(w, "Patch!");
	ui_set_widget_position(w, 112, 242);
	ui_set_callback(BUTTON_PRESSED, w, patch_nest);
	ui_add_widget(win, w);
	
	w = ui_create_widget(LABEL);
	ui_set_caption(w, "Select your configurationoptions:");
	ui_set_widget_position(w, 42, 72);
	ui_set_widget_size(w, 48, 236);
	ui_add_widget(win, w);
	
	ui_show_window(win);
	t = ui_main_loop(&argc, &argv);
	ui_destroy_window(win);
	return flags;
}

void patch_nest(window* win, widget* unused) {
	widget* w;
	uint32_t i;

	for(i = 1; i < 4; i++) {
		flags <<= 1;
		w = ui_get_widget_by_tag(win, i);
		flags |= ui_get_checkbox_value(w);
	}

	ui_quit_loop(win, unused);
}
