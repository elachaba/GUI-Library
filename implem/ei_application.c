#include <stdbool.h>
#include <stdio.h>
#include "ei_draw.h"
#include "ei_frame.h"
#include "ei_implementation.h"
#include "draw_tools.h"
#include "ei_event.h"
#include "ei_types.h"



ei_widget_t root_widget;
ei_surface_t root_surface, picking_surface;
ei_size_t root_surface_size;
ei_event_t* wait_event;
ei_linked_rect_t* invalidate_rect = NULL;
bool quit_request = false;
ei_default_handle_func_t default_handle_func = NULL;

static ei_widget_t active_widget = NULL;
void print_invalidate_rects(ei_linked_rect_t* rect) {
    ei_linked_rect_t* current_rect = rect;
    int count = 1;

    while (current_rect != NULL) {
        ei_point_t top_left = current_rect->rect.top_left;
        ei_size_t size = current_rect->rect.size;

        printf("\nRectangle %d:\n", count);
        printf("-------------\n");
        printf("Top-Left Point: (X: %d, Y: %d)\n", top_left.x, top_left.y);
        printf("Size: (Width: %d, Height: %d)\n", size.width, size.height);
        printf("-------------\n");

        current_rect = current_rect->next;
        count++;
    }
}


void ei_app_create(ei_size_t main_window_size, bool fullscreen){
    // Initialising required resources to create an app
    hw_init();
    ei_buttonclass_register();
    ei_frameclass_register();
    ei_toplevelclass_register();
    ei_rootclass_register();
    /* create root window */
    root_surface = hw_create_window(main_window_size,fullscreen);
    root_surface_size = hw_surface_get_size(root_surface);
    root_widget = ei_widget_create("root",NULL,NULL,NULL);
    /* create offscreen surface */
    picking_surface = hw_surface_create(root_surface,root_surface_size,false);
    //picking_surface = hw_create_window(main_window_size,fullscreen);
    root_widget->requested_size = root_surface_size;
    root_widget->screen_location.size = root_surface_size;
    root_widget->content_rect = &(root_widget->screen_location);
}

void ei_app_free(void) {
    // App freeing function
    hw_surface_free(picking_surface);
    ei_widget_destroy(root_widget);
    hw_quit();
}
ei_rect_t new_intersect_rect(ei_rect_t rect1, ei_rect_t rect2) {
    int x_min = min_function(rect1.top_left.x, rect2.top_left.x);
    int y_min = min_function(rect1.top_left.y, rect2.top_left.y);
    int x_max = max_function(rect1.top_left.x + rect1.size.width, rect2.top_left.x + rect2.size.width);
    int y_max = max_function(rect1.top_left.y + rect1.size.height, rect2.top_left.y + rect2.size.height);

    ei_rect_t bounding_rect;
    bounding_rect.top_left.x = x_min;
    bounding_rect.top_left.y = y_min;
    bounding_rect.size.width = x_max - x_min;
    bounding_rect.size.height = y_max - y_min;

    return bounding_rect;
}

void merge_overlapping_rects(ei_linked_rect_t **rect_list) {
    for (ei_linked_rect_t *rect1 = *rect_list; rect1 != NULL; rect1 = rect1->next) {

        for (ei_linked_rect_t *rect2 = rect1->next; rect2 != NULL; rect2 = rect2->next) {
            ei_rect_t* intersection = intersect_rect((&rect1->rect), &(rect2->rect)) ;
            if (intersection) {
                ei_rect_t r = new_intersect_rect(rect1->rect, rect2->rect);
                ei_rect_t* intersection2=  intersect_rect(&r,&(root_widget->screen_location));
                if(intersection2)
                {
                    rect1->rect = *intersect_rect(&r,&(root_widget->screen_location)) ;
                    rect1->next = rect2->next;
                    free(rect2);
                    rect2 = rect1;
                }
                free(intersection2);

            }
            free(intersection);
        }
    }
}
void ei_app_invalidate_rect(const ei_rect_t* rect) {
   if(rect->size.width >0 && rect->size.height > 0)
   {
       ei_linked_rect_t* new_rect = malloc(sizeof(ei_linked_rect_t));
       new_rect->rect = *rect;
       new_rect->next = NULL;
       if (invalidate_rect == NULL) {
           invalidate_rect = new_rect;
       } else {
           new_rect->next = invalidate_rect;
           invalidate_rect = new_rect;
       }
   }

}
void remove_used_rects(ei_linked_rect_t **rect) {
    ei_linked_rect_t *current_rect = *rect;
    ei_linked_rect_t *next_rect;

    while(current_rect != NULL) {
        next_rect = current_rect->next;
        free(current_rect);
        current_rect = next_rect;
    }

    *rect = NULL;
}

void ei_app_run(void)
{
    ei_app_invalidate_rect(&(root_widget->screen_location));
    while (!quit_request) {
        wait_event = malloc(sizeof(ei_event_t));

       // print_invalidate_rects(invalidate_rect);

        merge_overlapping_rects(&invalidate_rect);


       print_invalidate_rects(invalidate_rect);
        hw_surface_lock(root_surface);
        hw_surface_lock(picking_surface);
        for (ei_linked_rect_t *rect = invalidate_rect; rect != NULL; rect = rect->next) {
            root_widget->wclass->drawfunc(root_widget, root_surface, picking_surface, &(rect->rect));

        }
        hw_surface_unlock(picking_surface);
        hw_surface_unlock(root_surface);
        hw_surface_update_rects(root_surface, invalidate_rect);
        //hw_surface_update_rects(picking_surface,invalidate_rect);
        remove_used_rects(&invalidate_rect);


        hw_event_wait_next(wait_event);
        ei_widget_t curr_pick_widget = ei_event_get_active_widget();
        if (!curr_pick_widget) {
            //events that require picking

                if (wait_event->type <= 8 && wait_event->type >=6 ) {
                    if (point_in_clipper(&(wait_event->param.mouse.where), &(root_widget->screen_location))) {

                        curr_pick_widget = ei_widget_pick(&wait_event->param.mouse.where);

                    }
                }
                if (ei_event_get_default_handle_func()) {
                    ei_event_get_default_handle_func()(wait_event);
                }

            }

        if (curr_pick_widget && !(curr_pick_widget->wclass->handlefunc(curr_pick_widget, wait_event))) {

            if (ei_event_get_default_handle_func()) {
                ei_event_get_default_handle_func()(wait_event);
            }
        }

    }

        free(wait_event);
            }





ei_linked_rect_t* ei_app_get_invalidate_rect()
{
    return invalidate_rect;
}
void ei_app_quit_request(void) {
    quit_request = true ;
}

ei_widget_t ei_app_root_widget(void) {
    return root_widget;

}

ei_surface_t ei_app_root_surface(void) {
    return root_surface;
}

static int ei_color_equal(ei_color_t* color1, ei_color_t* color2) {
    return (color1->red == color2->red &&
            color1->green == color2->green &&
            color1->blue == color2->blue &&
            color1->alpha == color2->alpha);
}


static ei_widget_t ei_widget_tree_browse(ei_widget_t widget, ei_color_t* curr_pick_color) {
    ei_widget_t curr_widget = widget;
    while (curr_widget) {
        if ((curr_widget)->pick_color && ei_color_equal(curr_widget->pick_color, curr_pick_color)) {
            return curr_widget; // Found the widget with matching pick color
        }
        ei_widget_t child = curr_widget->children_head;
        if (child) {
            ei_widget_t result = ei_widget_tree_browse(child, curr_pick_color);
            if (result) {
                return result; // Found the widget in the child subtree
            }
        }
        curr_widget = curr_widget->next_sibling; // Move to the next sibling
    }
    return NULL; // No widget with matching pick color found
}

ei_widget_t ei_widget_pick (ei_point_t* where) {
    // Returns the widget to be manipulated based on the picking approach
    hw_surface_lock(picking_surface);
    ei_color_t* curr_pick_color = (ei_color_t*) calloc(1, sizeof(ei_color_t));
    uint32_t* pick_pixel = point_to_pixel(where, picking_surface);
    int id_r, id_g, id_b, id_a;
    hw_surface_get_channel_indices(picking_surface, &id_r, &id_g, &id_b, &id_a);
    uint8_t* pixel_ptr = (uint8_t*) pick_pixel;
    curr_pick_color->red = pixel_ptr[id_r];
    curr_pick_color->green = pixel_ptr[id_g];
    curr_pick_color->blue = pixel_ptr[id_b];
    curr_pick_color->alpha = 255;
    ei_widget_t widget_pick = ei_widget_tree_browse(root_widget, curr_pick_color);
    hw_surface_unlock(picking_surface);
    return widget_pick;
}

void ei_event_set_default_handle_func(ei_default_handle_func_t func) {
    default_handle_func = func;
}

ei_default_handle_func_t ei_event_get_default_handle_func(void) {
    return default_handle_func;
}

void ei_event_set_active_widget(ei_widget_t widget) {
    active_widget = widget;
}

ei_widget_t ei_event_get_active_widget(void) {
    return active_widget;
}
