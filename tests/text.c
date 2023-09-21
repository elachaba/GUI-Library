#include <stdio.h>
#include <stdlib.h>

#include "hw_interface.h"
#include "ei_draw.h"
#include "ei_types.h"
#include "ei_event.h"
#include "ei_utils.h"


int main(int argc, char **argv) {
    ei_size_t win_size = ei_size(800, 600);
    ei_surface_t main_window = NULL;
    ei_color_t white = {0xff, 0xff, 0xff, 0xff};
    ei_event_t event;

    hw_init();

    main_window = hw_create_window(win_size, false);

    // Lock the drawing surface, paint it white
    hw_surface_lock(main_window);
    ei_fill(main_window, &white, NULL);

    //clipper
    ei_rect_t* clipper_ptr = NULL;
    //ei_rect_t clipper = ei_rect(ei_point(50, 50), ei_size(400, 300));
    //clipper_ptr = &clipper;

    // Draw the text
    ei_point_t text_pos = {200, 300};
    ei_font_t font = ei_default_font;
    ei_color_t text_color = {255, 100, 200, 0xff};
    ei_draw_text(main_window, &text_pos, "This is my second text! Yes science, yes bitch, hell yeaaaa, let's goo Let's goo!", font, text_color, clipper_ptr);

    // Unlock and update the surface
    hw_surface_unlock(main_window);
    hw_surface_update_rects(main_window, NULL);

    // Wait for a character on command line
    event.type = ei_ev_none;
    while ((event.type != ei_ev_close) && (event.type != ei_ev_keydown))
        hw_event_wait_next(&event);

    // Free the surfaces and quit
    hw_quit();

    return (EXIT_SUCCESS);
}
