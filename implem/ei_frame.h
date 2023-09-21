#include "ei_widget.h"
#include "ei_widgetclass.h"
#include "../implem/ei_implementation.h"
#ifndef PROJETC_IG_EI_FRAME_H
#define PROJETC_IG_EI_FRAME_H

#endif //PROJETC_IG_EI_FRAME_H

/**
 *\brief Text structure that contains the string, font, color and anchor of a text
 */
typedef struct text {
    ei_string_t		text;
    ei_font_t		text_font;
    ei_color_t		text_color;
    ei_anchor_t	text_anchor;
} text;

/**
 *\brief Image structure that contains the surface, delimiting rectangleand anchor of an image
 */
typedef struct image {
    ei_surface_t		image;
    ei_rect_ptr_t  	image_rect;
    ei_anchor_t		img_anchor;
} image;


/**
 *\brief Frame structure that contains the widget, relief, color, border width, text and image of a frame
 */
typedef struct ei_frame_t{
    ei_impl_widget_t widget;
    ei_relief_t relief;
    ei_color_t	color;
    int			border_width;
    text txt;
    image img;


} ei_frame_t;

/**
 *\brief Button structure that contains the widget, relief, color, border width, text, image, corner radius, callback and user parameters and anchor of a text
 */
typedef struct ei_button_t{
    ei_impl_widget_t widget;
    ei_relief_t relief;
    ei_color_t color;
    int border_width;
    text txt;
    image img;
    int corner_radius;
    ei_callback_t callback;
    ei_user_param_t user_param;
} ei_button_t;


/**
 *\brief Button structure that contains the widget, relief, color, border width, text, image, corner radius, callback and user parameters and anchor of a text
 */
typedef struct ei_toplevel_t{
    ei_impl_widget_t widget;
    ei_size_t requested_size;
    ei_color_t color;
    int border_width;
    ei_string_t title;
    bool closable;
    ei_axis_set_t resizable;
    ei_size_ptr_t min_size;
} ei_toplevel_t;


/**
 *\brief Functions implemented to register the necessary classes of widgets
 */
void ei_frameclass_register();
void ei_rootclass_register ();
void ei_buttonclass_register();
void ei_toplevelclass_register();


/**
 *\brief Anchor placement and management function
 */
void anchor_placement(ei_rect_t drawing_rect , ei_anchor_t anchor ,ei_point_t* where, int height , int width,int corner_offset);
