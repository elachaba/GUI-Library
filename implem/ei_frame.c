#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ei_types.h"
#include "ei_implementation.h"
#include "hw_interface.h"
#include "ei_frame.h"
#include "ei_draw.h"
#include "draw_button.h"
#include "draw_tools.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "ei_application.h"

#define text_sunken_offset 3

static ei_widget_t frame_allocfunc() {
    return  (ei_widget_t) calloc(1,sizeof(ei_frame_t));
}

void frame_releasefunc(ei_widget_t widget) {
    ei_frame_t* frame_widget = (ei_frame_t*) widget;
    if (frame_widget->txt.text) {
        free(frame_widget->txt.text);
    }
    else if (frame_widget->img.image) {
        hw_surface_free(frame_widget->img.image);
        if (frame_widget->img.image_rect) {
            free(frame_widget->img.image_rect);
        }
    }
}


void anchor_placement(ei_rect_t drawing_rect, ei_anchor_t anchor ,ei_point_t* where, int height , int width, int corner_offset) {
    // Text anchor management function
    where->x = drawing_rect.top_left.x;
    where->y = drawing_rect.top_left.y;

    switch (anchor) {
        case (ei_anc_north):
            where->x += drawing_rect.size.width/2 - width/2;
            break;
        case (ei_anc_northeast):
            where->x += drawing_rect.size.width- width-corner_offset/2;
            break;
        case (ei_anc_east):
            where->x += drawing_rect.size.width-width;
            where->y += drawing_rect.size.height/2-height/2;
            break;
        case (ei_anc_west):
            where->y += drawing_rect.size.height/2- height/2;
            break;
        case(ei_anc_southeast):
            where->x += drawing_rect.size.width-width-corner_offset/2;
            where->y += drawing_rect.size.height-height;
            break;
        case (ei_anc_south):
            where->x += drawing_rect.size.width/2 - width/2;
            where->y += drawing_rect.size.height-height;
            break;
        case (ei_anc_center):
            where->x += drawing_rect.size.width/2-width/2;
            where->y += drawing_rect.size.height/2-height/2;
            break;
        case(ei_anc_southwest):
            where->y += drawing_rect.size.height - height;
            where->x +=corner_offset/2;
            break;
        case (ei_anc_northwest):
            where->x +=corner_offset/2;
            break;
        default:
            break;
    }
}

static void ei_draw_image (ei_surface_t surface, ei_widget_t widget, ei_rect_t* clipper) {
    // image drawing function
    ei_rect_t image_area = widget->screen_location;
    ei_frame_t* frame_widget = (ei_frame_t*) widget;
    ei_surface_t image = frame_widget->img.image;
    image_area.top_left.x += frame_widget->border_width;
    image_area.top_left.y += frame_widget->border_width;
    image_area.size.width -= 2 * frame_widget->border_width;
    image_area.size.width -= 2 * frame_widget->border_width;
    if (frame_widget->relief == ei_relief_sunken) {
        image_area.top_left.x += text_sunken_offset;
        image_area.top_left.y += text_sunken_offset;
    }
    ei_size_t img_size;
    if (frame_widget->img.image_rect) {
        img_size = frame_widget->img.image_rect->size;
    }
    else {
        img_size = hw_surface_get_size(image);
    }
    if (image_area.size.width < img_size.width) {
        image_area.size.width = img_size.width;
    }
    if (image_area.size.height < img_size.height) {
        image_area.size.height = img_size.height;
    }
    ei_point_t where;
    anchor_placement(image_area, frame_widget->img.img_anchor, &where, image_area.size.height, image_area.size.width, 0);
    ei_rect_t* image_area_ptr = &image_area;
    if (clipper) {
        image_area_ptr = intersect_rect(&image_area, clipper);
    }
    if (image_area_ptr) {
        ei_rect_t dst_rect = {where, image_area.size};
        ei_rect_t src_rect;
        if (frame_widget->img.image_rect){
            src_rect = ei_rect(frame_widget->img.image_rect->top_left, image_area.size);
        }
        src_rect = ei_rect(ei_point_zero(), image_area.size);
        ei_copy_surface(surface, &dst_rect, image, &src_rect, false);
    }

}
static void frame_drawfunc(ei_widget_t widget, ei_surface_t surface , ei_surface_t pick_surface , ei_rect_t* clipper) {
    ei_frame_t* frame_widget = (ei_frame_t*)widget;
    ei_rect_t* drawing_rect ;

    if (widget->parent && widget->parent->content_rect) {
        drawing_rect = widget->parent->content_rect;
        if (clipper) {
            drawing_rect = intersect_rect(drawing_rect, clipper);
        }
    }
    if (!widget->parent) {
        ei_fill(surface, &(frame_widget->color), clipper);
        ei_fill(pick_surface,widget->pick_color,clipper);

    }
    else {

        draw_button(surface, widget->screen_location, drawing_rect, frame_widget->color,
                    frame_widget->border_width, 0, frame_widget->relief);
        ei_rect_t* filling_clipper = intersect_rect(&widget->screen_location,drawing_rect);
        ei_fill(pick_surface,widget->pick_color,filling_clipper);

    }



    if (frame_widget->txt.text) {
        text widget_text = frame_widget->txt;
        int text_width, text_height;
        hw_text_compute_size(widget_text.text, widget_text.text_font, &text_width, &text_height);
        ei_point_t where;
        ei_rect_t text_area;
        text_area.top_left.x =  widget->screen_location.top_left.x + frame_widget->border_width;
        text_area.top_left.y =  widget->screen_location.top_left.y + frame_widget->border_width;
        text_area.size.width =  widget->screen_location.size.width - 2*frame_widget->border_width;
        text_area.size.height =  widget->screen_location.size.height - 2*frame_widget->border_width;
        anchor_placement(text_area, frame_widget->txt.text_anchor, &where, text_height, text_width,0);
        if (frame_widget->relief != ei_relief_sunken) {
            ei_draw_text(surface, &where, widget_text.text, widget_text.text_font, widget_text.text_color, drawing_rect);
        }
        else {
            where.x += text_sunken_offset;
            where.y += text_sunken_offset;
            ei_draw_text(surface, &where, widget_text.text, widget_text.text_font, widget_text.text_color, drawing_rect);
        }

    }
    else if (frame_widget->img.image) {
        ei_draw_image(surface, widget, clipper);
    }

    ei_impl_widget_draw_children(widget, surface, pick_surface, clipper);
}

static void frame_setdefaultsfunc(ei_widget_t widget)
{
    // Fr
    widget->content_rect = &(widget->screen_location);
    ei_frame_t* frame_widget = (ei_frame_t*) widget;
    frame_widget->relief = ei_relief_none;
    frame_widget->color = ei_default_background_color;
    frame_widget->border_width = 0;
    frame_widget->txt.text = NULL;
    frame_widget->txt.text_font = ei_default_font;
    frame_widget->txt.text_color = ei_font_default_color;
    frame_widget->txt.text_anchor = ei_anc_center;
    frame_widget->img.image = NULL;
 /*   frame_widget->img.image_rect = NULL;*/
    frame_widget->img.img_anchor = ei_anc_center;

}

bool frame_handlefunc(ei_widget_t widget, ei_event_t* event) {

    return false;
}
void ei_frameclass_register()
{
    ei_widgetclass_t* frameclass = malloc (sizeof(ei_widgetclass_t));
    strncpy(frameclass->name,"frame",20);
    frameclass->allocfunc = &frame_allocfunc;
    frameclass->releasefunc = &frame_releasefunc;
    frameclass->drawfunc = &frame_drawfunc;
    frameclass->setdefaultsfunc = &frame_setdefaultsfunc;
    frameclass->geomnotifyfunc = NULL;
    frameclass->handlefunc = &frame_handlefunc;
    ei_widgetclass_register(frameclass);
}




void ei_rootclass_register ()
{
    ei_widgetclass_t* frameclass = malloc (sizeof(ei_widgetclass_t));
    strncpy(frameclass->name,"root",20);
    frameclass->allocfunc = &frame_allocfunc;
    frameclass->releasefunc = &frame_releasefunc;
    frameclass->drawfunc = &frame_drawfunc;
    frameclass->setdefaultsfunc = &frame_setdefaultsfunc;
    frameclass->geomnotifyfunc = NULL;
    frameclass->handlefunc = &frame_handlefunc;
    ei_widgetclass_register(frameclass);
}