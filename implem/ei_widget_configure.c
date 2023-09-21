#include "ei_frame.h"
#include "ei_widgetclass.h"
#include "ei_implementation.h"
#include "ei_utils.h"


void			ei_frame_configure		(ei_widget_t		widget,
                                           ei_size_t*		requested_size,
                                           const ei_color_t*	color,
                                           int*			border_width,
                                           ei_relief_t*		relief,
                                           ei_string_t*		text,
                                           ei_font_t*		text_font,
                                           ei_color_t*		text_color,
                                           ei_anchor_t*		text_anchor,
                                           ei_surface_t*		img,
                                           ei_rect_ptr_t*		img_rect,
                                           ei_anchor_t*		img_anchor)

                                           {
    ei_frame_t* frame_widget = (ei_frame_t*) widget;
    if (color) {
        frame_widget->color = *color;

    }
    if (border_width) {
        frame_widget->border_width = *border_width;
    }
    if (relief) {
        frame_widget->relief = *relief;

    }
    if (text) {
        if (*text!=NULL)
        {
            frame_widget->txt.text = calloc(1,strlen(*text)+1);
            strcpy(frame_widget->txt.text, *text);
        }

        if (text_font) {
            frame_widget->txt.text_font = *text_font;
        }
        if (text_color) {
            frame_widget->txt.text_color = *text_color;
        }
        if (text_anchor) {
            frame_widget->txt.text_anchor = *text_anchor;
        }
        if (!requested_size) {
            int txt_width, txt_height;
            hw_text_compute_size(frame_widget->txt.text, frame_widget->txt.text_font, &txt_width, &txt_height);
            widget->requested_size = (ei_size_t){txt_width, txt_height};
        }
    }
    else if (img) {
        ei_size_t img_size = hw_surface_get_size(*img);
        ei_surface_t img_copy = hw_surface_create(*img, img_size, false);
        ei_copy_surface(img_copy, NULL, *img, NULL, false);
        frame_widget->img.image = img_copy;
        if (!requested_size) {
            widget->requested_size = img_size;
        }
        if (img_rect) {
            frame_widget->img.image_rect = malloc(sizeof(ei_rect_t));
            *frame_widget->img.image_rect = **img_rect;
            if (!requested_size) {
                widget->requested_size = frame_widget->img.image_rect->size;
            }
        }
        if (img_anchor) {
            frame_widget->img.img_anchor = *img_anchor;
        }
    }
    if (requested_size) {
        widget->requested_size = *requested_size;
        if (widget->placer_params)
        {
            ei_place(widget,&(widget->placer_params->anchor),&(widget->placer_params->x),&(widget->placer_params->y),&widget->requested_size.width,&widget->requested_size.height,&(widget->placer_params->rel_x),&(widget->placer_params->rel_y),&(widget->placer_params->rel_width),&(widget->placer_params->rel_height));
        }
    }
    else {
        if (!text && !img) {
            widget->requested_size = (ei_size_t){0, 0};
        }
    }

}

void  ei_button_configure (ei_widget_t       widget,
                          ei_size_t*		requested_size,
                          const ei_color_t*	color,
                          int*			    border_width,
                          int*			    corner_radius,
                          ei_relief_t*		relief,
                          ei_string_t*		text,
                          ei_font_t*		text_font,
                          ei_color_t*		text_color,
                          ei_anchor_t*		text_anchor,
                          ei_surface_t*		img,
                          ei_rect_ptr_t*	img_rect,
                          ei_anchor_t*		img_anchor,
                          ei_callback_t*	callback,
                          ei_user_param_t*	user_param) {

    ei_button_t* button_widget = (ei_button_t*) widget;
    if (color) {
        button_widget->color = *color;

    }
    if (border_width) {
        button_widget->border_width = *border_width;
    }
    if (relief) {
        button_widget->relief = *relief;

    }
    if (text) {
        button_widget->txt.text = malloc(strlen(*text)+1);
        strcpy(button_widget->txt.text, *text);
        if (text_font) {
            button_widget->txt.text_font = *text_font;
        }
        if (text_color) {
            button_widget->txt.text_color = *text_color;
        }
        if (text_anchor) {
            button_widget->txt.text_anchor = *text_anchor;
        }
        if (!requested_size) {
            int txt_width, txt_height;
            hw_text_compute_size(button_widget->txt.text, button_widget->txt.text_font, &txt_width, &txt_height);
            widget->requested_size = (ei_size_t){txt_width, txt_height};
        }
    }
    else if (img ) {
        if (*img==NULL){
            button_widget->img.image =NULL;
        }
        else{
            ei_size_t img_size = hw_surface_get_size(*img);
            ei_surface_t img_copy = hw_surface_create(*img, img_size, false);

            ei_copy_surface(img_copy, NULL, *img, NULL, false);
            button_widget->img.image = img_copy;
            if (!requested_size) {
                widget->requested_size = img_size;
            }
            if (img_rect) {
                button_widget->img.image_rect = malloc(sizeof(ei_rect_t));
                *button_widget->img.image_rect = **img_rect;
                if (!requested_size) {
                    widget->requested_size = button_widget->img.image_rect->size;
                }
        }

        }
        if (img_anchor) {
            button_widget->img.img_anchor = *img_anchor;
        }
    }
    if (requested_size) {
        widget->requested_size = *requested_size;

        if (widget->placer_params)
        {
            ei_place(widget,&(widget->placer_params->anchor),&(widget->placer_params->x),&(widget->placer_params->y),&widget->requested_size.width,&widget->requested_size.height,&(widget->placer_params->rel_x),&(widget->placer_params->rel_y),&(widget->placer_params->rel_width),&(widget->placer_params->rel_height));
        }
    }

    else {
        if (!text && !img) {
            widget->requested_size = (ei_size_t){0, 0};
        }
    }
    if (callback) {
        button_widget->callback = *callback;
    }
    if (user_param) {
        button_widget->user_param = *user_param;
    }
    if (corner_radius) {
        button_widget->corner_radius = *corner_radius;
    }


}

void			ei_toplevel_configure		(ei_widget_t		widget,
                                              ei_size_t*		requested_size,
                                              ei_color_t*		color,
                                              int*			border_width,
                                              ei_string_t*		title,
                                              bool*			closable,
                                              ei_axis_set_t*		resizable,
                                              ei_size_ptr_t*		min_size) {

    ei_toplevel_t *toplevel_widget = (ei_toplevel_t *) widget;
    if (requested_size) {
        widget->requested_size = *requested_size;
        if (widget->placer_params)
        {
            ei_place(widget,&(widget->placer_params->anchor),&(widget->placer_params->x),&(widget->placer_params->y),0,0,&(widget->placer_params->rel_x),&(widget->placer_params->rel_y),&(widget->placer_params->rel_width),&(widget->placer_params->rel_height));
        }
    }

    if (color) {
        toplevel_widget->color = *color;
    }
    if (border_width) {
        toplevel_widget->border_width = *border_width;
    }
    if (title) {
        toplevel_widget->title = malloc(strlen(*title)+1);
        strcpy(toplevel_widget->title, *title);
    }
    if (closable != NULL) {
        toplevel_widget->closable = closable;
    }
    if (resizable != NULL) {
        toplevel_widget->resizable = *resizable;
    }
    if (min_size != NULL) {
        toplevel_widget->min_size = *min_size;
    }
    else{

        toplevel_widget->min_size =malloc(sizeof(ei_size_t));
        *(toplevel_widget->min_size) = ei_size(160,120);
    }
}

ei_widgetclass_t*	ei_widget_get_class	(ei_widget_t widget) {
    return widget->wclass;
}

const ei_color_t*	ei_widget_get_pick_color (ei_widget_t widget) {
    return widget->pick_color;
}

ei_widget_t ei_widget_get_parent (ei_widget_t widget) {
    return widget->parent;
}

ei_widget_t ei_widget_get_first_child (ei_widget_t	widget) {
    return widget->children_head;
}

ei_widget_t ei_widget_get_last_child (ei_widget_t widget) {
    return widget->children_tail;
}

ei_widget_t ei_widget_get_next_sibling	(ei_widget_t widget) {
    return widget->next_sibling;
}

void* ei_widget_get_user_data (ei_widget_t widget) {
    return widget->user_data;
}

const ei_rect_t* ei_widget_get_screen_location (ei_widget_t	widget) {
    ei_rect_t* screen_location_ptr = malloc(sizeof(ei_rect_t));
    screen_location_ptr = &(widget->screen_location);
    return screen_location_ptr;
}

const ei_rect_t* ei_widget_get_content_rect	(ei_widget_t widget) {
    return widget->content_rect;
}

void ei_widget_set_content_rect	(ei_widget_t widget, const ei_rect_t* content_rect) {
    widget->content_rect = (ei_rect_t*) content_rect;
}


