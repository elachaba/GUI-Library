#include <stdlib.h>
#include <string.h>
#include "ei_implementation.h"
#include "hw_interface.h"
#include "ei_frame.h"
#include "draw_button.h"
#include "draw_tools.h"
#include "ei_event.h"
#include "ei_widget.h"
#include "ei_utils.h"
#include <math.h>
#include <ei_application.h>

static ei_point_t prev_pos_mouse;
static int action ; // resize=1,move =2 a ajouter dans toplevel
static int close_radius;
static int corner_radius = 20;

static int close_button_size = 20;
static int resize_button_size = 20;

#define header_height 34
#define shade_level 0.3

ei_widget_t toplevel_allocfunc()
{
    return (ei_widget_t) calloc(1, sizeof(ei_toplevel_t));
}

void toplevel_releasefunc(ei_widget_t widget) {
    ei_toplevel_t* toplevel_widget = (ei_toplevel_t*) widget;
    if (toplevel_widget->title) {
        free(toplevel_widget->title);
    }
/*if (toplevel_widget->min_size) {
 free(toplevel_widget->min_size);
 }*/
}


void toplevel_drawfunc(ei_widget_t widget, ei_surface_t surface , ei_surface_t pick_surface , ei_rect_t* clipper) {


    ei_toplevel_t* toplevel_widget = (ei_toplevel_t*) widget;
    ei_rect_t* toplevel_clipper = intersect_rect(clipper , widget->parent->content_rect);

    if (!toplevel_clipper)
   {
       return;
   }

    ei_point_t topleft_tplvl = {widget->screen_location.top_left.x ,
                                widget->screen_location.top_left.y - header_height};
    ei_size_t topleft_tplvl_size = {widget->screen_location.size.width ,
                                    widget->screen_location.size.height + header_height };
    ei_rect_t toplevel_rect = {topleft_tplvl,topleft_tplvl_size};

    top_level(surface,toplevel_rect, toplevel_clipper,toplevel_widget->color,corner_radius);
    top_level(pick_surface, toplevel_rect, toplevel_clipper,*(widget->pick_color),corner_radius);

    ei_color_t color_center = {toplevel_widget->color.red * (1 - shade_level),
                               toplevel_widget->color.green * (1 - shade_level),
                               toplevel_widget->color.blue * (1 - shade_level),
                               toplevel_widget->color.alpha};


    ei_rect_t center_rect = ei_rect(ei_point(widget->screen_location.top_left.x +toplevel_widget->border_width,widget->screen_location.top_left.y),
                                    ei_size(widget->screen_location.size.width-2*toplevel_widget->border_width,widget->screen_location.size.height-toplevel_widget->border_width));


    ei_rect_t* center_rect_clipper= intersect_rect(&center_rect, toplevel_clipper);
    if (!center_rect_clipper)
    {
        return;
    }
    ei_fill_frame(surface,&color_center,center_rect_clipper);


// Fixed button sizes in pixels.

// Close button.
    if (toplevel_widget->closable)
    {
        ei_color_t close_color = {0xff, 0, 0, 0xff};
        ei_point_t close_center = {topleft_tplvl.x + close_button_size / 1.25,
                                   topleft_tplvl.y + close_button_size / 1.25};
        close_radius = close_button_size / 2;
        ei_point_t* close_arc = draw_arc(close_center, close_radius, 0, 2* M_PI);

        double close_number = (nbre_of_points_angle_step(close_center, close_radius, 0, 2*M_PI))[0];
        ei_draw_polygon(surface, close_arc, close_number, close_color, toplevel_clipper);
    }

// Resize button.

    if (toplevel_widget->title) {
        ei_rect_t* title_rect = intersect_rect(&toplevel_rect,toplevel_clipper );
        if (!title_rect)
        {
            return;
        }
        ei_color_t title_color = {0xff,0xff,0xff,0xff};
        ei_point_t title_where ={topleft_tplvl.x+2*close_button_size,topleft_tplvl.y+3};
        ei_draw_text(surface,&title_where,toplevel_widget->title,ei_default_font,title_color, title_rect);

        //free(title_rect);

    }


    ei_impl_widget_draw_children(widget, surface, pick_surface, toplevel_clipper);
    if (toplevel_widget->resizable)
    {
        ei_color_t resize_color = toplevel_widget->color;
        ei_point_t top_left_resize = {topleft_tplvl.x + toplevel_rect.size.width-resize_button_size,
                                      topleft_tplvl.y + toplevel_rect.size.height - resize_button_size };
        ei_size_t resize_size = {resize_button_size, resize_button_size - toplevel_widget->border_width};
        ei_rect_t resize = {top_left_resize, resize_size};
        ei_rect_t* resize_rect = intersect_rect(&resize, toplevel_clipper);
        if (resize_rect) {
            ei_fill(surface, &resize_color, resize_rect);
            ei_fill(pick_surface,widget->pick_color,resize_rect);
            free(resize_rect);
        }


    }
}


bool mouse_on_close_button(ei_widget_t widget , ei_event_t* event)
{
    return ((event->param.mouse.where.x-widget->screen_location.top_left.x - close_button_size / 1.25)*(event->param.mouse.where.x-widget->screen_location.top_left.x - close_button_size / 1.25)+(event->param.mouse.where.y-widget->screen_location.top_left.y + header_height- close_button_size / 1.25)*(event->param.mouse.where.y-widget->screen_location.top_left.y + header_height-close_button_size / 1.25)) <=close_radius*close_radius;
}
bool mouse_in_toplevel_header(ei_widget_t widget, struct ei_event_t* event)
{

    return (widget->screen_location.top_left.x<= event->param.mouse.where.x && widget->screen_location.top_left.y>= event->param.mouse.where.y && widget->screen_location.top_left.x +widget->screen_location.size.width>=event->param.mouse.where.x && widget->screen_location.top_left.y-header_height<= event->param.mouse.where.y )&& (!mouse_on_close_button(widget,event));
}

bool mouse_on_resize(ei_widget_t widget, ei_event_t* event)
{
    return (widget->screen_location.top_left.x + widget->screen_location.size.width - resize_button_size<=event->param.mouse.where.x && widget->screen_location.top_left.y + widget->screen_location.size.height - resize_button_size <=event->param.mouse.where.y && widget->screen_location.top_left.x+ widget->screen_location.size.width >= event->param.mouse.where.x && widget->screen_location.top_left.y+ widget->screen_location.size.height >= event->param.mouse.where.y );
}



bool toplevel_handlefunc(ei_widget_t widget, struct ei_event_t* event)
{

    if (ei_event_get_active_widget()) {

        if (event->type == ei_ev_mouse_move) {
            if (action == 1) { //resize

                int new_w = widget->placer_params->width;
                int new_h = widget->placer_params->height;
                if (((ei_toplevel_t*)widget)->resizable == ei_axis_x) {
                    new_w+= event->param.mouse.where.x -prev_pos_mouse.x;
                }
                if (((ei_toplevel_t*)widget)->resizable == ei_axis_y) {
                    new_h+= event->param.mouse.where.y -prev_pos_mouse.y;
                }
                if (((ei_toplevel_t*)widget)->resizable == ei_axis_both) {
                    new_w+= event->param.mouse.where.x -prev_pos_mouse.x;
                    new_h+= event->param.mouse.where.y -prev_pos_mouse.y;
                }
                ei_place(widget,&(widget->placer_params->anchor),&(widget->placer_params->x),&(widget->placer_params->y),&new_w,&new_h,&(widget->placer_params->rel_x),&(widget->placer_params->rel_y),&(widget->placer_params->rel_width),&(widget->placer_params->rel_height));
                prev_pos_mouse = event->param.mouse.where;



                return true;
            }
            if (action == 2) { //move

                int new_x = widget->placer_params->x +(event->param.mouse.where.x -prev_pos_mouse.x) ;
                int new_y =widget->placer_params->y+(event->param.mouse.where.y -prev_pos_mouse.y) ;
                ei_place(widget,&(widget->placer_params->anchor),&new_x,&new_y,&(widget->placer_params->width),&(widget->placer_params->height),&(widget->placer_params->rel_x),&(widget->placer_params->rel_y),&(widget->placer_params->rel_width),&(widget->placer_params->rel_height));
                prev_pos_mouse = event->param.mouse.where;


                return true;
            }
        }
        if (event->type== ei_ev_mouse_buttonup) {

            ei_event_set_active_widget(NULL);
            return true;
        }

    }
    else if(event->type == ei_ev_mouse_buttondown) {

        if ( mouse_in_toplevel_header(widget,event)) {
            ei_event_set_active_widget(widget);

            action = 2;
            prev_pos_mouse = event->param.mouse.where;
            return true;

        }
        if (((ei_toplevel_t*)widget)->resizable && mouse_on_resize(widget,event)) {
            ei_event_set_active_widget(widget);
            action = 1;
            prev_pos_mouse = event->param.mouse.where;
            return true;

        }
        if (((ei_toplevel_t*)widget)->closable && mouse_on_close_button(widget,event)) {
            ei_event_set_active_widget(NULL);

            ei_widget_destroy(widget);
            return true;


        }
    }
    return false ;
}

void toplevel_geomnotifyfunc(ei_widget_t widget) {
    ei_toplevel_t *toplevel_widget = (ei_toplevel_t *) widget;

    ei_size_ptr_t minsize = toplevel_widget->min_size;



    if (minsize) {
        if (widget->placer_params->width < minsize->width) {
            ei_place(widget, &(widget->placer_params->anchor), &(widget->placer_params->x), &(widget->placer_params->y),
                     &(minsize->width), &(widget->placer_params->height), &(widget->placer_params->rel_x),
                     &(widget->placer_params->rel_y), &(widget->placer_params->rel_width),
                     &(widget->placer_params->rel_height));


        }
        if (widget->placer_params->height < minsize->height) {

            ei_place(widget, &(widget->placer_params->anchor), &(widget->placer_params->x), &(widget->placer_params->y),
                     &(widget->placer_params->width), &(minsize->height), &(widget->placer_params->rel_x),
                     &(widget->placer_params->rel_y), &(widget->placer_params->rel_width),
                     &(widget->placer_params->rel_height));


        }
    }
}
void toplevel_setdefaultsfunc(ei_widget_t widget)
{

    widget->content_rect = &widget->screen_location;
    ((ei_toplevel_t*)widget)->requested_size = ei_size(320,240);
    ((ei_toplevel_t*)widget)->color = ei_default_background_color;
    ((ei_toplevel_t*)widget)->border_width = 4;
    ((ei_toplevel_t*)widget)->title = "Toplevel";
    ((ei_toplevel_t*)widget)->closable =true;
    ((ei_toplevel_t*)widget)->resizable=ei_axis_both;

}

void ei_toplevelclass_register() {
    ei_widgetclass_t *toplevelclass = malloc(sizeof(ei_widgetclass_t));
    strncpy(toplevelclass->name, "toplevel", 20);
    toplevelclass->allocfunc = &toplevel_allocfunc;
    toplevelclass->releasefunc = &toplevel_releasefunc;
    toplevelclass->drawfunc = &toplevel_drawfunc;
    toplevelclass->setdefaultsfunc = &toplevel_setdefaultsfunc;
    toplevelclass->geomnotifyfunc = &toplevel_geomnotifyfunc;
    toplevelclass->handlefunc = &toplevel_handlefunc;
    ei_widgetclass_register(toplevelclass);
}