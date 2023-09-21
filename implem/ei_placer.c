#include <stdlib.h>
#include "ei_widget.h"
#include "ei_types.h"
#include "ei_placer.h"
#include "ei_implementation.h"
#include "ei_application.h"
#include "ei_frame.h"
#include "ei_utils.h"

void ei_place (ei_widget_t widget,
 ei_anchor_t* anchor,
 int* x,
 int* y,
 int* width,
 int* height,
 float* rel_x,
 float* rel_y,
 float* rel_width,
 float* rel_height)
{
    if (!(widget->placer_params)) {
        widget->placer_params = calloc(1,sizeof(ei_impl_placer_params_t));
    }
    widget->placer_params->anchor = anchor ? *anchor : ei_anc_northwest;
    widget->placer_params->x = x ? *x : 0;
    widget->placer_params->y = y ? *y : 0;
    widget->placer_params->width = width ? *width : widget->requested_size.width;
    widget->placer_params->height = height ? *height : widget->requested_size.height;
    widget->placer_params->rel_x = rel_x ? *rel_x : 0;
    widget->placer_params->rel_y = rel_y ? *rel_y : 0;
    widget->placer_params->rel_width = rel_width ? *rel_width : 0;
    widget->placer_params->rel_height = rel_height ? *rel_height : 0;
    ei_impl_placer_run(widget);
}

void ei_placer_forget(ei_widget_t widget) {
    if (widget->placer_params) {
        free(widget->placer_params);
    }
}

void ei_impl_placer_run(ei_widget_t widget) {


    ei_rect_t new_screenlocation = widget->screen_location;
    ei_point_t new_topleft = {(int)(widget->placer_params->rel_x * widget->parent->screen_location.size.width + widget->placer_params->x),(int)(widget->placer_params->rel_y * widget->parent->screen_location.size.height +widget->placer_params->y )};

    new_screenlocation.top_left = new_topleft;
    new_screenlocation.size.width = widget->placer_params->rel_width ? widget->placer_params->rel_width * widget->parent->screen_location.size.width :widget->placer_params->width ;
    new_screenlocation.size.height = widget->placer_params->rel_height ? widget->placer_params->rel_height * widget->parent->screen_location.size.height :widget->placer_params->height ;

    switch (widget->placer_params->anchor) {
        case (ei_anc_north):
            new_screenlocation.top_left.x -= new_screenlocation.size.width / 2;
            break;
        case (ei_anc_northeast):
            new_screenlocation.top_left.x -= new_screenlocation.size.width;
            break;
        case (ei_anc_center):
            new_screenlocation.top_left.x -= new_screenlocation.size.width / 2;
            new_screenlocation.top_left.y -= new_screenlocation.size.height / 2;
            break;
        case (ei_anc_east):
            new_screenlocation.top_left.x -= new_screenlocation.size.width;
            new_screenlocation.top_left.y -= new_screenlocation.size.height / 2;
            break;
        case (ei_anc_west):
            new_screenlocation.top_left.y -= new_screenlocation.size.height / 2;
            break;
        case (ei_anc_southeast):
            new_screenlocation.top_left.x -= new_screenlocation.size.width;
            new_screenlocation.top_left.y -= new_screenlocation.size.height;
            break;
        case (ei_anc_south):
            new_screenlocation.top_left.x -= new_screenlocation.size.width / 2;
            new_screenlocation.top_left.y -= new_screenlocation.size.height;
            break;
        case (ei_anc_southwest):
            new_screenlocation.top_left.y -= new_screenlocation.size.height;
            break;
        default:
            break;
    }


    if (widget->parent) {
        new_screenlocation.top_left.x +=widget->parent->content_rect->top_left.x;
        new_screenlocation.top_left.y +=widget->parent->content_rect->top_left.y;
    }



    if (strcmp(widget->wclass->name,"toplevel")==0){
        ei_toplevel_t* toplevel_widget =(ei_toplevel_t*)widget;
        ei_point_t topleft_tplvl = {widget->screen_location.top_left.x ,
                                    widget->screen_location.top_left.y - 34};
        ei_size_t topleft_tplvl_size = {widget->screen_location.size.width+2*toplevel_widget->border_width +3,
                                        widget->screen_location.size.height + 1.25*34 };
        ei_rect_t toplevel_rect = {topleft_tplvl,topleft_tplvl_size};
        ei_app_invalidate_rect(&toplevel_rect);

        if  ( (new_screenlocation.size.width!=0 &&new_screenlocation.size.height!=0) &&(new_screenlocation.top_left.x != widget->screen_location.top_left.x ||new_screenlocation.top_left.y != widget->screen_location.top_left.y || new_screenlocation.size.width !=widget->screen_location.size.width ||new_screenlocation.size.height !=widget->screen_location.size.height )) {
            if (widget->wclass->geomnotifyfunc !=NULL) {
                widget->wclass->geomnotifyfunc(widget);
            }
            widget->screen_location = new_screenlocation;
            topleft_tplvl = ei_point(widget->screen_location.top_left.x ,
                                        widget->screen_location.top_left.y - 34-3);
            topleft_tplvl_size = ei_size(widget->screen_location.size.width+2*toplevel_widget->border_width +3,
                                            widget->screen_location.size.height + 1.25*34 );
            toplevel_rect = ei_rect(topleft_tplvl,topleft_tplvl_size);
            ei_app_invalidate_rect(&toplevel_rect);
            ei_widget_t ptr = widget->children_head;
            while (ptr) {
               if(ptr->placer_params)
               {
                   ei_impl_placer_run(ptr);
               }
                ptr =ptr->next_sibling;

            }
        }
    }
    else{

        if  ( (new_screenlocation.size.width!=0 &&new_screenlocation.size.height!=0) || (new_screenlocation.top_left.x != widget->screen_location.top_left.x ||new_screenlocation.top_left.y != widget->screen_location.top_left.y || new_screenlocation.size.width !=widget->screen_location.size.width ||new_screenlocation.size.height !=widget->screen_location.size.height )) {
            if (widget->wclass->geomnotifyfunc !=NULL) {
                widget->wclass->geomnotifyfunc(widget);
            }
            if (widget->screen_location.size.width!=0 && widget->screen_location.size.height!=0){
                ei_app_invalidate_rect(&widget->screen_location);

           }
            widget->screen_location = new_screenlocation;
            ei_app_invalidate_rect(&(widget->screen_location));
            ei_widget_t ptr = widget->children_head;
            while (ptr) {
                ei_impl_placer_run(ptr);
                ptr =ptr->next_sibling;
            }
        }
    }

    }
