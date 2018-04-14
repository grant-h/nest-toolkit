#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <fbdraw.h>
#include <nestinput.h>

#include "fonthandler.h"
#include "toolkit.h"
#include "render.h"

#define DEFAULT_HEIGHT 24
#define FBDEV "/dev/fb0"


uint8_t main_loop_end;
window* current_window;

int ui_init(void) {
	int i;
	i = ni_init();
	if(i) return i;
	i = ft_init();
	if(i) return i;
	i = fb_init(FBDEV);
	/* if we got a framebuffer, clear it */
	if(!i) fb_clear();
	return i;
}

widget* ui_create_widget(widget_type wt) {
	widget* t;
	t = (widget*)calloc(1, sizeof(*t));
	if(!t) {
		perror("failed to create widget");
		return (widget*)NULL;
	}
	t->enabled = 1;
	t->visible = 1;
	t->type = wt;
	/* by default labels can not get focus,
	 * this may change if they are scrollable
	 */
	t->can_focus = !(wt == LABEL);
	return t;
}

window* ui_create_window(scroll_type t) {
	window* w;
	w = (window*)calloc(1, sizeof(*w));
	if(!w) {
		perror("failed to create window");
		return (window*)NULL;
	}
	w->scrollable = t;
	return w;
}

int ui_main_loop(int* argc, char*** argv) {
	widget* w;
	widgetlist* current_widget, *t;
	int i;
	uint8_t redraw;
	/* TODO: finetune */
	while(!main_loop_end) {
		redraw = 0;
		w = current_window->current_widget->w;

		if(ni_read_button()) {	/* button was pressed */
			/* execute button event handler if it exists */
			if(w->button_event)
				w->button_event(current_window, w);
			/* handle check box */
			if(w->type == CHECK_BOX) {
				w->reserved[31] = !w->reserved[31];
				redraw = 1;
			}
		}

		/* read scroll */
		i = ni_read_scroll();
		if(i) {
			current_widget = current_window->current_widget;
			/* find previous/next focusable widget */
			for(	t = (i < 0) ?
						current_widget->prev : current_widget->next;
					t && !t->w->can_focus && t->w->enabled;
					t = (i < 0) ?
						t->prev : t->next);
			/* if one was found */
			if(t) {
				redraw = 1;
				/* execute the lost focus event handler if required */
				if(current_widget->w->lost_focus_event)
					current_widget->w->
							lost_focus_event(current_window, current_widget->w);
				/* execute the gain focus event handler if required */
				if(t->w->gain_focus_event)
					t->w->gain_focus_event(current_window, t->w);
				/* and set the new active widget */
				current_window->current_widget = t;
			}
		}
		
		if(redraw) ui_redraw_window();		/* and show new focus */
	}
	fb_clear();
	return EXIT_SUCCESS;
}

void ui_quit_loop(window* win, widget* w) {
	main_loop_end = 1;
}

int ui_set_callback(event e, widget* wgt, callbackptr f) {
	switch(e) {
		case BUTTON_PRESSED:
			wgt->button_event = f;
			break;
		case SCROLLED:
			wgt->scroll_event = f;
			break;
		case GAINED_FOCUS:
			wgt->gain_focus_event = f;
			break;
		case LOST_FOCUS:
			wgt->lost_focus_event = f;
			break;
		default:
			fprintf(stderr, "invalid callback signal: %d\n", e);
			return -1;
	}
	return 0;
}

int ui_set_caption(widget* wgt, const char* caption) {
	char* t;
	size_t l;
	/* obtain new allocation for label */
	l = strlen(caption);
	t = (char*)malloc((l + 1) * sizeof(char));
	/* check whether allocation failed */
	if(!t) {
		perror("failed to allocate new label");
		return -1;
	}
	/* copy label and store pointer to it on widget */
	strcpy(t, caption);
	free(wgt->caption);
	wgt->caption = t;
	return 0;
}

void ui_set_widget_position(widget* wgt, uint32_t x, uint32_t y) {
	wgt->xpos = x;
	wgt->ypos = y;
}

void ui_set_widget_size(widget* wgt, uint32_t height, uint32_t width) {
	wgt->height = height;
	wgt->width = width;
}

void ui_set_widget_tag(widget* wgt, uint32_t tag) {
	wgt->tag = tag;
}

void ui_set_checkbox_value(widget* wgt, uint8_t value) {
	wgt->reserved[31] = value;
}

uint8_t ui_get_checkbox_value(widget* w) {
	return w->reserved[31];
}

widget* ui_get_widget_by_tag(window* win, uint32_t tag) {
	widgetlist* t;
	/* search widget list in window for widget with appropriate tag */
	for(t = win->widget_list.head; t && !(t->w->tag == tag); t = t->next);
	/* if found, return pointer to it */
	if(t)
		return t->w;
	
	/* else, return NULL */
	return (widget*)NULL;
}

int ui_add_widget(window* w, widget* wgt) {
	widgetlist* t;
	/* create a new list node */
	t = (widgetlist*)calloc(1, sizeof(*t));
	if(!t) {
		perror("failed to add new widget");
		return -1;
	}
	/* and add the widget to the list node */
	t->w = wgt;
	if(w->widget_list.head == (widgetlist*)NULL) {
		/* if the head of the list is empty, this is the first element that is
		 * being added to the window.
		 */
		w->widget_list.head = w->widget_list.tail = t;
	} else {
		/* otherwise, just add at the end */
		w->widget_list.tail->next = t;
		t->prev = w->widget_list.tail;
		w->widget_list.tail = t;
	}
	return 0;
}

int ui_redraw_window(void) {
	widgetlist* wl;
	widget* w;
	uint8_t active;
	for(wl = current_window->widget_list.head; wl; wl = wl->next) {
		w = wl->w;
		active = (w == current_window->current_widget->w);
		ui_draw_widget(w, active);
	}
	return 0;
}

int ui_show_window(window* w) {
	/* if the current window is not being shown update set to new window and
	 * refresh screen, otherwise, this function is equivalent to calling
	 * ui_redraw_window()
	  */
	if(current_window != w) {
		current_window = w;
		current_window->current_widget = w->widget_list.head;
		fb_clear();
	}
	return ui_redraw_window();
}

int ui_hide_window(window* w) {
	/* TODO: stub */
	return 0;
}

window* ui_destroy_window(window* w) {
	widgetlist* t, * n;
	/* loop thru the linked list freeing everything */
	for(t = w->widget_list.head; t;) {
		n = t->next;
		free(t->w->caption);
		free(t->w);
		free(t);
		t = n;
	}
	/* then free the window */
	free(w);
	return (window*)NULL;	
}
