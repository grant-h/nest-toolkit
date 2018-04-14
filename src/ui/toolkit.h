/**
 * \file toolkit.h
 * \brief Toolkit to generate simple graphical user interfaces in the Nest
 * Learning Thermostat.
 */
#ifndef TOOLKIT_H__
#define TOOLKIT_H__

#include <inttypes.h>

/**
 * \brief Scrollable properties of a widget or window.
 *
 * The scrollable properties of a widget or window. Currently, this is not used
 * in the toolkit but may be supported in future versions.
 */
typedef enum scroll_type_ {
	NOT_SCROLLABLE,		/**< element can not be scrolled */
	SCROLLABLE,			/**< element can be scrolled */
} scroll_type;

/**
 * \brief Callback signals supported by the toolkit.
 *
 * The callback signals supported by the toolkit. Each signal can be attached
 * to one function. Please note that some widgets take a default action when an
 * event is raised. This behaviour can not be overriden, but a callback can be
 * used to extend it. At this point, the SCROLLED signal is unused and will not
 * trigger a callback.
 */
typedef enum event_ {
	BUTTON_PRESSED,		/**< if the button was pressed over this widget */
	SCROLLED,			/**< if the scroll was activated over this widget */
	GAINED_FOCUS,		/**< if the widget gained focus */
	LOST_FOCUS,			/**< if the widget lost focus */
} event;

/**
 * \brief Widget types supported by the toolkit.
 *
 * The widget types supported by the toolkit. The option box widget is
 * currently not implemented.
 * \sa widget* ui_create_widget(widget_type wt)
 */
typedef enum widget_type_ {
	BUTTON,				/**< button widget */
	LABEL,				/**< label widget */
	CHECK_BOX,			/**< check box widget */
	OPTION_BOX,			/**< option box widget */
} widget_type;

struct widget_;
struct window_;

/**
 * \brief Callback handler for widget signals.
 *
 * The user must provide a function with prototype that matches this function
 * pointer. The function will be given a pointer to the window and a pointer to
 * the widget that triggered the callback. As such, multiple callbacks can be
 * handled with the same function.
 *
 * \sa int ui_set_callback(event e, widget* wgt, callbackptr f)
 */
typedef void (*callbackptr)(struct window_*, struct widget_*);

/**
 * @brief Widget definition struct. All elements of this struct should not be
 * directly accessed.
 */
typedef struct widget_ {
	char* caption;					/* widget caption/text */
	widget_type type;				/* widget type */
	uint8_t multiline;				/* set to 1 if multiline is desired */
	uint8_t enabled;				/* set to 0 if disabled, 1 = default */
	uint8_t visible;				/* set to 0 if non-visible, 1 = default */
	uint8_t can_focus;				/* set to 1 if element can get focus */
	uint32_t tag;					/* widget tag for lookups */
	uint32_t xpos;					/* top corner widget x position */
	uint32_t ypos;					/* top corner widget y position */
	uint32_t height;				/* widget height, 0 = default height */
	uint32_t width;					/* widget width, 0 = default width */
	scroll_type can_scroll;			/* unused for now */
	callbackptr button_event;		/* function pointer to button event */
	callbackptr scroll_event;		/* function pointer to scroll event */
	callbackptr gain_focus_event;	/* function pointer to got focus */
	callbackptr lost_focus_event;	/* function pointer to lost focus */
	uint8_t reserved[32];			/* reserved for widget specific and future use */
} widget;

/**
 * \brief Widget list struct. Members of this struct should not be directly
 * accessed.
 */
typedef struct widgetlist_ {
	widget* w;
	struct widgetlist_* next;
	struct widgetlist_* prev;
} widgetlist;

/**
 * \brief Window list struct. Members of this struct should not be directly
 * accessed.
 */
typedef struct window_ {
	uint32_t y_pos;					/* vertical window position */
	scroll_type scrollable;			/* whether the window is scrollable */
	struct {
		widgetlist* head;
		widgetlist* tail;
	} widget_list;					/* linekd list with widgets */
	widgetlist* current_widget;		/* widget with focus */
} window;

/**
 * \brief Initialize the UI toolkit.
 *
 * This function initializes the UI toolkit by attempting to obtain a handle to
 * the framebuffer, font handling and input sources. This function must be
 * called before ui_main_loop() is called.
 *
 * \return If successful, 0 is returned, otherwise, a non-zero value is
 * returned errno is set accordingly.
 *
 * \sa int ui_main_loop(int* argc, char*** argv)
 */
int ui_init(void);

/**
 * \brief Creates a UI widget.
 *
 * This function creates a UI widget and sets its default properties. If
 * successful, a pointer to the created widget is returned.
 *
 * \param[in] wt 	Type of widget as described by enum widget_type.
 *
 * \return If successful, a pointer to a widget object is returned, otherwise
 * (widget*)NULL is returned instead and errno is set accordingly.
 *
 * \sa enum widget_type
 */
widget* ui_create_widget(widget_type wt);

/**
 * \brief Creates a UI window.
 *
 * This function creates a UI window with a determined scroll characteristic.
 * At this time, only non-scrolling windows are implemented, but this may change
 * in latter versions.
 *
 * \param[in] s		Scroll type as defined in enum scroll_type. At the time
 * only non-scrolling windows are implemented.
 *
 * \return If successfu, a pointer to a window object is returned, otehrwise
 * (window*)NULL is returned instead and errno is set accordingly.
 *
 * \sa enum scroll_type
 */
window* ui_create_window(scroll_type s);

/**
 * \brief Main UI loop.
 * 
 * This is the main UI loop function. Call this function after the UI has been
 * generated and the main window has been drawn. This function is responsible
 * for handing user input accordingly and delivering callbacks to widgets. It
 * is imperative to notice that the toolkit must have been previously
 * initialized and a window must have been drawn before this function is
 * called.
 *
 * \param[in] argc	Pointer to an integer containing the number of arguments
 * given in the command which invoked the program.
 * \param[in] argv	Pointer to a C strings array which contains the arguments
 * given in the command which invoked the program.
 *
 * \return This function always returns EXIT_SUCCESS.
 *
 * \sa void ui_init(void)
 * \sa int ui_show_window(window* w)
 */
int ui_main_loop(int* argc, char*** argv);

/**
 * \brief Terminates the main UI loop.
 *
 * This function terminates the main UI loop. It is meant to be attached as a
 * callback for a signal a widget received, although it can also be called from
 * within the callback function of another widget.
 *
 * \param[in] win	Not used.
 * \param[in] w		Not used.
 *
 * Example usage:
 * \code
 * widget* w;
 * if(!(w = ui_create_widget(BUTTON)))
 *     exit(1);
 * ui_set_caption(w, "Quit");
 * ui_set_callback(BUTTON_PRESSED, w, ui_quit_loop);
 * \endcode
 *
 * \sa int ui_main_loop(int* argc, char*** argv)
 * \sa int ui_set_callback(event e, widget* wgt, callbackptr f)
 */
void ui_quit_loop(window* win, widget* w);

/**
 * \brief Attach a callback function to a widget.
 *
 * This function attaches a callback function to be executed upon the widget
 * receiving a signal. Supported callback signals are described in enum event.
 * The callback function must take window* and widget* as arguments. These will
 * point to the window and widget that received the signal, respectively.
 *
 * \param[in] e		Signal event as described in enum event.
 * \param[in] wgt	Pointer to a widget to which the callback is to be attached
 * to.
 * \param[in] f		Callback function to be attached. The function must follow
 * the format described in typedef callbackptr.
 *
 * \return Upon completion, this function returns 0.
 *
 * Example usage:
 * Refer to void ui_quit_loop(window* win, widget* w) for an example.
 *
 * \sa enum event
 * \sa typedef callbackptr
 * \sa void ui_quit_loop(window* win, widget* w)
 */
int ui_set_callback(event e, widget* wgt, callbackptr f);

/**
 * \brief Set widget caption.
 *
 * This function sets the caption for a widget. The text will be rendered so
 * that it fits within the widget size. At the time, no smart word wrapping is
 * implemented when multiple lines are used.
 *
 * \param[in] wgt		Pointer to the widget object to be modified.
 * \param[in] caption	Pointer to a C string containing the caption.
 *
 * \return Upon success, this function returns 0, otherwise -1 is returned and
 * errno is set accordingly.
 *
 * \sa void ui_set_widget_size(widget* wgt, uint32_t height, uint32_t width)
 */
int ui_set_caption(widget* wgt, const char* caption);

/**
 * \brief Set widget position.
 *
 * This function sets the position of a widget as an absolute coordinate from
 * the top left corner of the screen. Please be aware that the Nest Learning
 * Thermostat has a round screen and your widget may not be completely
 * displayed. Notice that setting a widget position is not sufficient, it still
 * needs to be added to a window.
 *
 * \param[in] wgt		Pointer to a widget object.
 * \param[in] x			The horizontal coordinate of the widget.
 * \param[in] y			The vertical coordinate of the widget.
 *
 * \sa int ui_add_widget(window* w, widget* wgt)
 * \sa void ui_set_widget_size(widget* wgt, uint32_t height, uint32_t width)
 */
void ui_set_widget_position(widget* wgt, uint32_t x, uint32_t y);

/**
 * \brief Set widget size.
 *
 * This function sets the size of a widget. A parameter of 0 either in the
 * height or width will result in the default value for that specific
 * parameter when rendering. Default sizes are computed depending on the widget
 * type.
 *	- For a text box, the default size is 24 pixels in height and its width is
 *	the difference of its horizontal position and 320.
 *  - For a button, the default size is 24 pixels in height and its width is 96
 *  pixels.
 *  - For a checkbox, the rules are the same as the text box.
 *  - For an option box, the rules are the same as the text box. Notice that
 *  option boxes are not yet implemented.
 *
 * \param[in] wgt		Pointer to a widget object.
 * \param[in] height	The height of the widget. Set to 0 for default.
 * \param[in] width		The width of the widget. Set to 0 for default.
 *
 * \sa ui_set_widget_position(widget* wgt, uint32_t x, uint32_t y)
 */
void ui_set_widget_size(widget* wgt, uint32_t height, uint32_t width);

/**
 * \brief Set widget tag.
 *
 * This function sets the tag of a widget. This function is provided as a
 * compation to the ui_get_widget_by_tag() function. By default, the tag field
 * of a widget is set to 0. The tag field has no effect on how the widget is
 * rendered and just serves as an identifier. As such, tag values should be
 * unique per widget.
 *
 * \param[in] wgt		Pointer to a widget object.
 * \param[in] tag		The tag to be assigned to the widget.
 *
 * \sa widget* ui_get_widget_by_tag(window* win, uint32_t tag)
 */
void ui_set_widget_tag(widget* wgt, uint32_t tag);

/**
 * \brief Sets whether a checbox is checked or not.
 *
 * This function will tick a checkbox on the screen, the screen is not
 * automatically refreshed after this function is called. If the widget is not
 * of type CHECKBOX, the behaviour of this function is undefined.
 *
 * \param[in] wgt		Pointer to a widget object.
 * \param[in] value		Set to 0 if the checkbox is to be checked, 1 otherwise.
 *
 * \sa uint8_t ui_get_checkbox_value(widget* wgt)
 */
void ui_set_checkbox_value(widget* wgt, uint8_t value);

/**
 * \brief Gets whether a checkbox is checked or not.
 *
 * This function returns the state of a checkbox, that is, if the option in the
 * checkbox has been selected.
 *
 * \param[in] wgt		Pointer to a checkbox widget.
 *
 * \return If the option in the checkbox is selected, 1 is returned, if not,
 * 0 is returned. If the pointer passed to the function does not reflect a
 * checkbox, the return value of this function is undefined.
 *
 * \sa void ui_set_checkbox_value(widget* wgt, uint8_t value)
 */
uint8_t ui_get_checkbox_value(widget* wgt);

/**
 * \brief Searches a window for a widget.
 *
 * This function searches a window for a widget with the given tag. The first
 * widget with a matching tag found is the one returned.
 *
 * \param[in] win		Window to be searched.
 * \param[in] tag		The tag to search for.
 *
 * \return A pointer to the first widget with a matching tag is returned. If
 * one is not found, then (widget*)NULL is returned instead.
 *
 * \sa void ui_set_widget_tag(widget* wgt, uint32_t tag)
 */
widget* ui_get_widget_by_tag(window* win, uint32_t tag);

/**
 * \brief Adds a widget to a window.
 *
 * This function adds a widget to a window container. It is imperative to
 * notice that the order upon which the widgets are added to a window
 * determines the order of navigation, not the widget's horizontal or vertical
 * positions.
 *
 * \param[in] w			Pointer to a window where the widget is to be added.
 * \param[in] wgt		Pointer to the widget to be added.
 *
 * \return Upon success, this function returns 0, a non-zero value is returned
 * otherwise and errno is set accordingly.
 */
int ui_add_widget(window* w, widget* wgt);

/**
 * \brief Redraws a window.
 *
 * This function redraws the currently active window. If a widget is changed by
 * means of a callback function, this function must be manually called to
 * refresh the widget on the screen.
 *
 * \return This function always returns 0.
 */
int ui_redraw_window(void);

/**
 * \brief Shows a window and sets it as active.
 *
 * This function sets the active window and draws it on the screen. It must be
 * called at least once before ui_main_loop() is called. An implicit call to
 * ui_redraw_window() is made as part of this function.
 *
 * \param[in] w			Pointer to the window to be shown.
 *
 * \return This function always returns 0.
 *
 * \sa int ui_redraw_window(void)
 * \sa int ui_main_loop(int* argc, char*** argv)
 */
int ui_show_window(window* w);

/**
 * \brief Unimplemented, may be removed.
 *
 * This function is unimplemented and may be removed.
 */
int ui_hide_window(window* w);

/**
 * \brief Frees all allocations done with regards to a window.
 *
 * This function fully frees all allocations done with regards to a window.
 * That is, all widgets, their captions and container are freed from memory and
 * their allocation pointers are no longer valid.
 *
 * \param[in] w			Window to deallocate.
 *
 * \return Upon completion, this function returns (window*)NULL.
 */
window* ui_destroy_window(window* w);

#endif
