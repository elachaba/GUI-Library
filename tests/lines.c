#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "hw_interface.h"
#include "ei_utils.h"
#include "ei_draw.h"
#include "ei_types.h"
#include "ei_event.h"



/* test_line --
 *
 *	Draws a simple line in the canonical octant, that is, x1>x0 and y1>y0, with
 *	dx > dy. This can be used to test a first implementation of Bresenham
 *	algorithm, for instance.
 */
void test_line(ei_surface_t surface, ei_rect_t* clipper)
{
    ei_color_t		color		= { 255, 0, 255, 255 };
    ei_point_t		pts[2]		= { {200, 200}, {600, 400} };

    ei_draw_polyline(surface, pts, sizeof(pts) / sizeof(ei_point_t), color, clipper);
}



/* test_octogone --
 *
 *	Draws an octogone in the middle of the screen. This is meant to test the
 *	algorithm that draws a polyline in each of the possible octants, that is,
 *	in each quadrant with dx>dy (canonical) and dy>dx (steep).
 */
void test_octogone(ei_surface_t surface, ei_rect_t* clipper)
{
    ei_point_t		pts[9];
    int			i, xdiff, ydiff;

    /* Initialisation */
    pts[0].x		= 400;
    pts[0].y		= 90;

    /* Draw the octogone */
    for(i = 1; i <= 8; i++) {
        /*	Add or remove 70/140 pixels for next point
           The first term of this formula gives the sign + or - of the operation
           The second term is 2 or 1, according to which coordinate grows faster
           The third term is simply the amount of pixels to skip */
        xdiff = (int)(pow(-1, (i + 1) / 4) * pow(2, (i / 2) % 2 == 0) * 70);
        ydiff = (int)(pow(-1, (i - 1) / 4) * pow(2, (i / 2) % 2) * 70);

        pts[i].x	= pts[i-1].x + xdiff;
        pts[i].y	= pts[i-1].y + ydiff;
    }

    /* Draw the shape with a polygon */
    //ei_draw_polygon(surface, pts, sizeof(pts) / sizeof(ei_point_t), (ei_color_t){0, 0, 255, 10}, clipper);

    /* Draw the shape with polylines */
    ei_draw_polyline(surface, pts, sizeof(pts) / sizeof(ei_point_t), (ei_color_t){0, 255, 0, 255}, clipper);
}



/* test_square --
 *
 *	Draws a square in the middle of the screen. This is meant to test the
 *	algorithm for the special cases of horizontal and vertical lines, where
 *	dx or dy are zero
 */
void test_square(ei_surface_t surface, ei_rect_t* clipper)
{
    ei_color_t		color		= { 255, 0, 0, 255 };
    ei_point_t		pts[5];
    int			i, xdiff, ydiff;

    /* Initialisation */
    pts[0].x		= 300;
    pts[0].y		= 400;

    /* Draw the square */
    for(i = 1; i <= 4; i++) {
        /*	Add or remove 200 pixels or 0 for next point
            The first term of this formula gives the sign + or - of the operation
            The second term is 0 or 1, according to which coordinate grows
            The third term is simply the side of the square */
        xdiff = (int)(pow(-1, i / 2) * (i % 2) * 200);
        ydiff = (int)(pow(-1, i / 2) * (i % 2 == 0) * 200);

        pts[i].x	= pts[i-1].x + xdiff;
        pts[i].y	= pts[i-1].y + ydiff;
    }

    /* Draw the shape with polylines */
    ei_draw_polyline(surface, pts, sizeof(pts) / sizeof(ei_point_t), color, clipper);
}



/* test_dot --
 *
 *	Draws a dot in the middle of the screen. This is meant to test the special
 *	case when dx = dy = 0
 */
void test_dot(ei_surface_t surface, ei_rect_t* clipper)
{
    ei_color_t		color		= { 0, 0, 255, 255 };
    ei_point_t		pts[2]		= { {400, 300}, {400, 300} };

    /* Draw the shape with polylines */
    ei_draw_polyline(surface, pts, sizeof(pts) / sizeof(ei_point_t), color, clipper);
}



/*
 * ei_main --
 *
 *	Main function of the application.
 */
int main(int argc, char** argv)
{
    ei_size_t		win_size	= ei_size(800, 600);
    ei_surface_t		main_window	= NULL;
    ei_color_t		white		= { 0xff, 0xff, 0xff, 0xff };
    ei_rect_t*		clipper_ptr	= NULL;
//	ei_rect_t		clipper		= ei_rect(ei_point(200, 150), ei_size(400, 300));
//	clipper_ptr		= &clipper;
    ei_event_t		event;

    hw_init();

    main_window = hw_create_window(win_size, false);

    /* Lock the drawing surface, paint it white. */
    hw_surface_lock	(main_window);
    ei_fill		(main_window, &white, clipper_ptr);

    /* Draw polylines. */
    test_line	(main_window, clipper_ptr);
    test_octogone	(main_window, clipper_ptr);
    test_square	(main_window, clipper_ptr);
    test_dot	(main_window, clipper_ptr);

    /* Unlock and update the surface. */
    hw_surface_unlock(main_window);
    hw_surface_update_rects(main_window, NULL);

    /* Wait for a character on command line. */
    event.type = ei_ev_none;
    while ((event.type != ei_ev_close) && (event.type != ei_ev_keydown))
        hw_event_wait_next(&event);

    hw_quit();
    return (EXIT_SUCCESS);
}