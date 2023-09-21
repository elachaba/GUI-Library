#include <stdbool.h>
#include "ei_types.h"
#include "hw_interface.h"

/**
 * \brief Giving a point as a center, a radius, and starting and ending angles
 * gives the points that belong to the arc between the two angles
 * @param center The center of the arc
 * @param radius The radius, the distance between the points of the arc and the center
 * @param start_angle Determines the starting point
 * @param end_angle Determines the ending point
 * @return The list of points to draw the arc
 */

ei_point_t* draw_arc    (
        ei_point_t  center,
        float       radius,
        float       start_angle,
        float       end_angle
);

/**
 * Returns the points to draw a rounded frame, a rectangle with curved edges.
 * @param rectangle The rectangle of the frame
 * @param radius The radius of the curves
 * @param only_top If True, only the upper part of the rectangle is drawn
 * @param only_bottom If True, only the bottom part of rectangle is drawn
 * @return The list of points to draw, and the size of it
 */

 struct table_size rounded_frame (
        ei_rect_t   rectangle,
        const float radius,
        bool        only_top,
        bool        only_bottom
        );

/**
 * \brief Draws a button on the surface using \ref ei_draw_polygon
 * @param surface The surface on which the drawing is done
 * @param rectangle The rectangle of the button
 * @param clipper If not NULL, only the part of the rectangle that is within the clipper
 * is drawn
 * @param color_frame If NULL, the frame is colored in black.
 * @param border_width
 * @param radius
 * @param push
 * @param relief
 */

void draw_button (
        ei_surface_t    surface ,
        ei_rect_t       rectangle,
        ei_rect_t*      clipper,
        ei_color_t      color_frame,
        int             border_width,
        float           radius,
        ei_relief_t     relief);


/**
 *\brief Function that performs the drawing of the top level
 */
void top_level(ei_surface_t surface ,ei_rect_t rectangle,ei_rect_t *clipper,ei_color_t color_frame, float radius);