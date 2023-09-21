#include <stdlib.h>
#include <string.h>
#include "ei_implementation.h"
#include "hw_interface.h"
#include "ei_frame.h"
#include "draw_button.h"
#include "draw_tools.h"
#include "ei_event.h"
#include "ei_application.h"
#include "structures.h"
#include "ei_utils.h"
#include "ei_widget_configure.h"

#define text_sunken_offset 3


ei_widget_t button_allocfunc() {
    return (ei_widget_t) calloc(1, sizeof(ei_button_t));
}
void button_releasefunc(ei_widget_t widget) {
    ei_button_t * button_widget = (ei_button_t*) widget;
    if (button_widget->txt.text) {
        free(button_widget->txt.text);
    }
    else if (button_widget->img.image) {
        hw_surface_free(button_widget->img.image);
        if (button_widget->img.image_rect) {
            free(button_widget->img.image_rect);
        }
    }
}

static void ei_draw_image (ei_surface_t surface, ei_widget_t widget, ei_rect_t* clipper) {
    ei_rect_t content_area  = widget->screen_location;
    ei_button_t * button_widget = (ei_button_t*) widget;
    ei_surface_t image = button_widget->img.image;
    content_area.top_left.x += button_widget->border_width;
    content_area.top_left.y += button_widget->border_width;
    content_area.size.width -=  button_widget->border_width;
    content_area.size.width -= 2 * button_widget->border_width;

    ei_size_t image_copy_size;
    if (button_widget->img.image_rect) {
        image_copy_size = button_widget->img.image_rect->size;
    }
    else {
        image_copy_size = hw_surface_get_size(image);
    }
    image_copy_size.width = min_function(image_copy_size.width, content_area.size.width);
    image_copy_size.height = min_function(image_copy_size.height, content_area.size.height);
    ei_point_t where_on_button;
    anchor_placement(content_area, button_widget->img.img_anchor, &where_on_button, image_copy_size.height, image_copy_size.width, button_widget->corner_radius);

    ei_rect_t paste_area = {where_on_button, image_copy_size};
    ei_rect_t* clipped_area = &paste_area;
    if (clipper) {
        clipped_area = intersect_rect(&paste_area, clipper);
    }
    if (clipped_area) {

        ei_point_t src_start;
        if (button_widget->img.image_rect) {
            src_start = button_widget->img.image_rect->top_left;
        }
        else {
            src_start = ei_point_zero();
        }
        src_start.x += clipped_area->top_left.x - paste_area.top_left.x;
        src_start.y += clipped_area->top_left.y - paste_area.top_left.y;
        ei_rect_t src_rect = {src_start, clipped_area->size};
        ei_copy_surface(surface, clipped_area, image, &src_rect, false);
    }
  // free(image_area_ptr);

}

void button_drawfunc(ei_widget_t widget, ei_surface_t surface , ei_surface_t pick_surface , ei_rect_t* clipper) {
    ei_button_t* button_widget = (ei_button_t *) widget;


    draw_button(surface, widget->screen_location, clipper, button_widget->color,
                button_widget->border_width, button_widget->corner_radius,
                button_widget->relief);


    draw_button(pick_surface, widget->screen_location, clipper, *(widget->pick_color),
                button_widget->border_width, button_widget->corner_radius,
                button_widget->relief);



    if (button_widget->txt.text) {
        text widget_text = button_widget->txt;
        int text_width, text_height;
        hw_text_compute_size(widget_text.text, widget_text.text_font, &text_width, &text_height);
        ei_point_t where;
        ei_rect_t text_area;
        text_area.top_left.x = widget->screen_location.top_left.x + button_widget->border_width;
        text_area.top_left.y = widget->screen_location.top_left.y + button_widget->border_width;
        text_area.size.width = widget->screen_location.size.width - 2 * button_widget->border_width;
        text_area.size.height = widget->screen_location.size.height - 2 * button_widget->border_width;
        anchor_placement(text_area, button_widget->txt.text_anchor, &where, text_height, text_width, button_widget->corner_radius);
        if (button_widget->relief != ei_relief_sunken) {
            ei_draw_text(surface, &where, widget_text.text, widget_text.text_font, widget_text.text_color,
                         clipper);
        } else {
            where.x += text_sunken_offset;
            where.y += text_sunken_offset;
            ei_draw_text(surface, &where, widget_text.text, widget_text.text_font, widget_text.text_color,
                         clipper);
        }
    }

    else if (button_widget->img.image) {
        ei_draw_image(surface, widget, clipper);
    }

    //free(drawing_rect);
    ei_impl_widget_draw_children(widget, surface, pick_surface, clipper);
}

void button_setdefaultsfunc(ei_widget_t widget) {
    // Setting necessary parameters to default
    widget->content_rect = &(widget->screen_location);
    ei_button_t* button_widget = (ei_button_t*) widget;
    button_widget->relief = ei_relief_raised;
    button_widget->color = ei_default_background_color;
    button_widget->border_width = k_default_button_border_width;
    button_widget->txt.text = NULL;
    button_widget->txt.text_font = ei_default_font;
    button_widget->txt.text_color = ei_font_default_color;
    button_widget->txt.text_anchor = ei_anc_center;
    button_widget->img.image = NULL;
    button_widget->corner_radius =k_default_button_corner_radius;
    //button_widget->img.image_rect = NULL;
    button_widget->img.img_anchor = ei_anc_center;
    button_widget->callback = NULL;
    button_widget->user_param = NULL;
}

bool button_handlefunc(ei_widget_t widget, ei_event_t* event) {
    ei_widget_t active_widget = ei_event_get_active_widget();
    ei_button_t *button_widget = (ei_button_t *) widget;
    if (active_widget) {
        ei_point_t cursor_position = event->param.mouse.where;
        if (point_in_clipper(&(event->param.mouse.where), (widget->parent->content_rect))) {
            ei_widget_t pick_widget = ei_widget_pick(&cursor_position);
            if (button_widget->relief == ei_relief_sunken) {
                if (widget == pick_widget) {
                    if (event->type == ei_ev_mouse_buttonup) {
                        button_widget->relief = ei_relief_raised;
                        if (button_widget->callback) {
                            button_widget->callback(widget, event, button_widget->user_param);
                        }
                        ei_app_invalidate_rect(&(widget->screen_location));
                        ei_event_set_active_widget(NULL);
                        return true;
                    }
                } else {
                    button_widget->relief = ei_relief_raised;
                    ei_app_invalidate_rect(&(widget->screen_location));
                    ei_event_set_active_widget(NULL);
                    return true;
                }

            }
        }
        if (event->type == ei_ev_mouse_buttonup) {
            button_widget->relief = ei_relief_raised;
            if (button_widget->callback) {
                button_widget->callback(widget, event, button_widget->user_param);
            }
            return false;

        }
    }else if (event->type == ei_ev_mouse_buttondown) {
            button_widget->relief = ei_relief_sunken;
            ei_app_invalidate_rect(&(widget->screen_location));
            ei_event_set_active_widget(widget);
            return true;

        }
        return false;
    }

void ei_buttonclass_register() {
    // Function that registers the widget class : button
    ei_widgetclass_t* buttonclass = malloc (sizeof(ei_widgetclass_t));
    strncpy(buttonclass->name,"button",20);
    buttonclass->allocfunc = &button_allocfunc;
    buttonclass->releasefunc = &button_releasefunc;
    buttonclass->drawfunc = &button_drawfunc;
    buttonclass->setdefaultsfunc = &button_setdefaultsfunc;
    buttonclass->geomnotifyfunc = NULL;
    buttonclass->handlefunc = &button_handlefunc;
    ei_widgetclass_register(buttonclass);

}