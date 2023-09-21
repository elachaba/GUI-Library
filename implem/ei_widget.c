#include <stdlib.h>
#include "ei_widgetclass.h"
#include "ei_widget.h"
#include "ei_implementation.h"
#include "draw_tools.h"
#include "ei_application.h"
#include "ei_frame.h"

static uint32_t pick_random = 1;

ei_color_t* from_id_to_color(uint32_t pick_id) {
    ei_color_t* color = (ei_color_t*)calloc(1, sizeof(ei_color_t));

    // Map pick_id to color components
    color->red = ((pick_id & 0x00FF0000) >> 16);      // Use the middle 8 bits for red component
    color->green = (pick_id & 0x0000FF00) >> 8;     // Use the next 8 bits for green component
    color->blue = pick_id & 0x000000FF;             // Use the last 8 bits for blue component
    color->alpha = 255;                             // Set alpha to maximum (fully opaque)

    return color;
}


ei_widget_t ei_widget_create (ei_const_string_t class_name,
                              ei_widget_t parent,
                              ei_user_param_t user_data,
                              ei_widget_destructor_t destructor)
{
    ei_widgetclass_t* widgetclass = ei_widgetclass_from_name(class_name);
    ei_widget_t widget;
    widget = (widgetclass->allocfunc)();
    widget->wclass = widgetclass;
    widget->wclass->setdefaultsfunc(widget);
    widget->parent = parent;
    widget->user_data = user_data;
    widget->destructor = destructor;
    widget->pick_id = pick_random++;
    widget->pick_color = from_id_to_color(widget->pick_id);
    if (parent)
    {
        if (parent->children_head == NULL)
    {
        parent->children_head = widget;
        parent->children_tail = widget;


    }
        else
    {

        parent->children_tail->next_sibling = widget;
        parent->children_tail = widget;

    }
    }
    return widget;
}
void ei_impl_widget_draw_children      (ei_widget_t		widget,
                                        ei_surface_t		surface,
                                        ei_surface_t		pick_surface,
                                        ei_rect_t*		clipper)
{
    ei_widget_t child = widget->children_head;
    while (child) {
        if (child->placer_params) {
            ei_rect_t* child_clipper = intersect_rect(&(child->screen_location),clipper);
            if (child_clipper) {

                   child->wclass->drawfunc(child,surface,pick_surface,clipper);
             }
        }
        child = child->next_sibling;

    }
}
void ei_widget_remove_child (ei_widget_t parent, ei_widget_t child) {
    if (parent && child) {
        if (!(parent->children_head)){
            return;
        }
        ei_widget_t* prev_widget = &(parent->children_head);
        if (*prev_widget == child) {
                (*prev_widget)= child->next_sibling;
                return;
            }
        while ((*prev_widget)->next_sibling){
            if ((*prev_widget)->next_sibling == child) {
                (*prev_widget)->next_sibling = child->next_sibling;
                return;
            }
            prev_widget = &((*prev_widget)->next_sibling);
        }
        *prev_widget = NULL;

    }
}

void ei_widget_destroy(ei_widget_t widget) {

    if (ei_widget_is_displayed(widget)) {
        if (widget->parent) {
            ei_widget_t w_parent  = widget->parent;
            ei_widget_t curr_widget = widget->children_head;
            while (curr_widget) {
                ei_widget_t next_widget = curr_widget->next_sibling;
                ei_widget_destroy(curr_widget);
                curr_widget = next_widget;
            }

            if (strcmp(widget->wclass->name,"toplevel")==0){
                   ei_toplevel_t* toplevel_widget =(ei_toplevel_t*)widget;
                   ei_point_t topleft_tplvl = {widget->screen_location.top_left.x ,
                                               widget->screen_location.top_left.y - 34};
                   ei_size_t topleft_tplvl_size = {widget->screen_location.size.width+2*toplevel_widget->border_width +3,
                                                   widget->screen_location.size.height + 1.25*34 };
                   ei_rect_t toplevel_rect = {topleft_tplvl,topleft_tplvl_size};
                   ei_app_invalidate_rect(&toplevel_rect);
           }
            else {
                ei_app_invalidate_rect(&widget->parent->screen_location);
            }

            ei_placer_forget(widget);
            ei_widget_remove_child (w_parent, widget);
            widget->wclass->releasefunc(widget);
            free(widget);
        }
    }

}

bool ei_widget_is_displayed	(ei_widget_t widget) {

    if (widget){
        if (widget->placer_params) {
            return true;
        }
    }
    return false;
}
