#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "hw_interface.h"
#include "ei_utils.h"
#include "ei_draw.h"
#include "ei_types.h"
#include "ei_event.h"
#include "ei_application.h"
#include "ei_widget_configure.h"

void draw_polygon_tests(ei_surface_t surface, const ei_rect_t *clipper) {
    ei_color_t color = {0, 0, 250, 128};

    // Draw a triangle
    ei_point_t triangle_points[] = {
            {100, 100},
            {200, 100},
            {150, 200}
    };
    size_t triangle_points_size = sizeof(triangle_points) / sizeof(triangle_points[0]);
    ei_draw_polygon(surface, triangle_points, triangle_points_size, color, clipper);

    // Draw a square
    ei_point_t square_points[] = {
            {300, 100},
            {400, 100},
            {400, 200},
            {300, 200}
    };
    size_t square_points_size = sizeof(square_points) / sizeof(square_points[0]);
    ei_draw_polygon(surface, square_points, square_points_size, color, clipper);

    // Draw a pentagon
    ei_point_t pentagon_points[] = {
            {500, 100},
            {600, 100},
            {650, 175},
            {550, 250},
            {450, 175}
    };
    size_t pentagon_points_size = sizeof(pentagon_points) / sizeof(pentagon_points[0]);
    ei_draw_polygon(surface, pentagon_points, pentagon_points_size, color, clipper);

    // Draw a non-convex polygon
    ei_point_t non_convex_points[] = {
            {100, 300},
            {200, 300},
            {150, 400},
            {200, 500},
            {100, 500}
    };
    size_t non_convex_points_size = sizeof(non_convex_points) / sizeof(non_convex_points[0]);
    ei_draw_polygon(surface, non_convex_points, non_convex_points_size, color, clipper);
}
/*
int main(int argc, char** argv)
{
    ei_size_t win_size = ei_size(800, 600);
    ei_surface_t main_window = NULL;
    ei_color_t white = {255, 255, 255, 255};

    //ei_color_t red = {255,0,0,123};
    ei_rect_t* clipper_ptr = NULL;
    //ei_rect_t clipper = ei_rect(ei_point(200, 150), ei_size(400, 300));
    //clipper_ptr = &clipper;
    ei_event_t event;

    hw_init();

    main_window = hw_create_window(win_size, false);

    hw_surface_lock (main_window);

    ei_fill (main_window, &white, clipper_ptr);



     //---------------- Draw polygon -----------------
    draw_polygon_tests(main_window, clipper_ptr);


    hw_surface_unlock(main_window);

    hw_surface_update_rects(main_window, NULL);

    // Wait for a character on command line.
    event.type = ei_ev_none;
    while ((event.type != ei_ev_close) && (event.type != ei_ev_keydown))
        hw_event_wait_next(&event);

    hw_quit();
    return (EXIT_SUCCESS);
}*/
void button_press(ei_widget_t widget, ei_event_t* event, ei_user_param_t user_param)
{
    printf("Click !\n");
}

bool process_key(ei_event_t* event)
{
    if ( (event->type == ei_ev_close) ||
         ((event->type == ei_ev_keydown) && (event->param.key.key_code == SDLK_ESCAPE))) {
        ei_app_quit_request();
        return true;
    } else
        return false;
}

int main(int argc, char** argv)
{
    ei_widget_t	button1, button2;
    ei_widget_t	window1, window2;


// Create the application and change the color of the background.


    ei_app_create((ei_size_t){800, 600}, false);
    ei_frame_set_bg_color(ei_app_root_widget(), (ei_color_t){0x52, 0x7f, 0xb4, 0xff});
    ei_event_set_default_handle_func(process_key);
    ei_axis_set_t axis = ei_axis_both;

    bool close = true;


// Create, configure and place a toplevel window on screen.


    window1 = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
    ei_toplevel_configure(window1, &(ei_size_t){420,340}, &(ei_color_t){0xA0,0xA0,0xA0, 0xff},
                          &(int){2}, &(ei_string_t){"Hello World"}, &close, &axis, NULL);
    ei_place_xy(window1, 30, 90);


// Create, configure and place a button as a descendant of the first toplevel window.


    button1 = ei_widget_create("button", window1, NULL, NULL);
    ei_button_configure(button1, &(ei_size_t){80,40}, &(ei_color_t){0x88, 0x88, 0x88, 0xff},
                        &(int){2}, NULL, &(ei_relief_t){ei_relief_raised},
                        &(ei_string_t){"click1"}, NULL, &(ei_color_t){0x00, 0x00, 0x00, 0xff},
                        NULL, NULL, NULL, NULL, &(ei_callback_t){button_press}, NULL);
    ei_place(button1, &(ei_anchor_t){ei_anc_southeast}, &(int){-20}, &(int){-20},
             NULL, NULL, &(float){1.0f}, &(float){1.0f}, &(float){0.5f}, NULL);


// Create, configure and place a second toplevel window as a descendant of the first.


    window2 = ei_widget_create("toplevel", window1, NULL, NULL);
    ei_toplevel_configure(window2, &(ei_size_t){200,200}, &(ei_color_t){0xA0,0xA0,0xA0, 0xff},
                          &(int){2}, &(ei_string_t){"Hello Again"}, &close, &axis, NULL);

// testing if the window child resizes if we resize the parent

    ei_place(window2,NULL,&(int){10},&(int){60},NULL,NULL,NULL,NULL,&(float){0.5f},&(float){0.5f});

    //ei_place(window2,NULL,&(int){10},&(int){60},NULL,NULL,&(float){0.5f},&(float){0.25f},NULL,NULL);

// Create, configure and place a button as a descendant of the second toplevel window.


    button2 = ei_widget_create("button", window2, NULL, NULL);
    ei_button_configure(button2, &(ei_size_t){40,30}, &(ei_color_t){0x88, 0x88, 0x88, 0xff},
                        &(int){2}, NULL, &(ei_relief_t){ei_relief_raised},
                        &(ei_string_t){"click2"}, NULL, &(ei_color_t){0x00, 0x00, 0x00, 0xff},
                        NULL, NULL, NULL, NULL, &(ei_callback_t){button_press}, NULL);
    ei_place(button2, &(ei_anchor_t){ei_anc_southeast}, &(int){-10}, &(int){-10},
             NULL, NULL, &(float){1.0f}, &(float){1.0f}, &(float){0.5f}, NULL);


// Run the application's main loop.


    ei_app_run();
    ei_app_free();

    return (EXIT_SUCCESS);

}


/*
int main(int argc, char** argv)
{
    ei_size_t win_size = ei_size(900, 600);
    ei_surface_t main_window = NULL;
    ei_color_t white = {255, 255, 255, 255};
    ei_color_t blue = {15, 5, 107, 255};
    ei_color_t red = {255, 0, 0, 255};

    ei_rect_t clipper_right = ei_rect(ei_point(0, 0), ei_size(300, 600));
    ei_rect_t clipper_left = ei_rect(ei_point(600, 0), ei_size(300, 600));
    ei_rect_t clipper_center = ei_rect(ei_point(300, 0), ei_size(300, 600));

    ei_rect_t* clipper_ptr = NULL;

    clipper_ptr = &clipper_left;
    ei_event_t event;

    hw_init();

    main_window = hw_create_window(win_size, false);

    // Lock the drawing surface, paint it white.
    hw_surface_lock (main_window);

    ei_fill (main_window, &red, clipper_ptr);

    clipper_ptr = &clipper_center;

    ei_fill (main_window, &white, clipper_ptr);

    clipper_ptr = &clipper_right;

    ei_fill (main_window, &blue, clipper_ptr);





    hw_surface_unlock(main_window);

    hw_surface_update_rects(main_window, NULL);

    // Wait for a character on command line.
    event.type = ei_ev_none;
    while ((event.type != ei_ev_close) && (event.type != ei_ev_keydown))
        hw_event_wait_next(&event);

    hw_quit();
    return (EXIT_SUCCESS);
}

*/
