#include <stdint.h>
#include <stdlib.h>
#include "hw_interface.h"
#include "ei_types.h"

/**
 * \brief Compares two integers and returns the maximum value between them
 * @param val_1 An integer
 * @param val_2 An integer
 * @return The maximum value between @param val_1 and @param val_2
 */

int max_function(int val_1, int val_2);
/**
 * \brief Compares two integers and returns the minimum value between them
 * @param val_1 An integer
 * @param val_2 An integer
 * @return The minimum value between @param val_1 and @param val_2
 */

int min_function(int val_1, int val_2);

/**
 * \brief Converts a point of coordinates (x,y) into a memory address
 * @param point The point to convert
 * @param surface The reference surface
 * @return The memory address corresponding to the pixel representing the point on the surface.
 */

uint32_t* point_to_pixel(ei_point_t* point, ei_surface_t surface);

/**
 * \brief Returns the value to affect to a pixel in order to color a surface
 * @param surface The surface to fill
 * @param color The filling color. If NULL, the color to be used is black
 * @return The value to put in the memory addresses corresponding to the pixels
 */

uint32_t get_pixel_color_unif(ei_surface_t surface, const ei_color_t* color);

/**
 * \brief Change the value of the pixel in order to color it with the color passed in parameters
 * taking in consideration the transperancy factor.
 * @param pixel The pixel to be re-colored
 * @param color If NULL, the black color is used
 * @param id_r
 * @param id_g
 * @param id_b
 * @param id_a
 */

/* This function is special for loop calls, since it doesn't need
 * to call hw_surface_get_channel_indices everytime */
void get_pixel_color_var(uint32_t* pixel, const ei_color_t* color, int id_r, int id_g, int id_b, int id_a);

/**
 * \brief Checks if a point is within the surface delimited by the clipper
 * @param point The point to check
 * @param clipper The clipper, should not be NULL
 * @return True if the point is within the surface delimited by the clipper
 *         False if else
 */

/* Fonction qui vérifie si un pixel est dans le clipper (non null)*/
bool point_in_clipper(ei_point_t* point, const ei_rect_t* clipper);

/**
 * \brief Calculates the intersection between two rectangles
 * @param rect_1
 * @param rect_2
 * @return a pointer to the intersection between rect_1 and rect_2
 */

ei_rect_t* intersect_rect(const ei_rect_t* rect_1, const ei_rect_t* rect_2);

/**
 * \brief the final pixel value by calculation the combination of the
 * value of pixel source and a pixel destination weighted by the alpha of
 * the pixel destination.
 * @param pixel_s the pixel source, (to be copied)
 * @param pixel_d the pixel destination, (where to write)
 * @param ir the channel idx of red
 * @param ig the channel idx of green
 * @param ib the channel idx of blue
 * @param ia the channel idx of alpha
 * @return the combination between the two pixels.
 */

uint32_t combine_pixels(uint32_t pixel_s, uint32_t pixel_d, int ir, int ig, int ib, int ia);


double* nbre_of_points_angle_step(ei_point_t center, float radius, float start_angle, float end_angle);

ei_color_t* from_id_to_color(uint32_t pick_id);

void ei_fill_frame(ei_surface_t surface, ei_color_t* color, ei_rect_t* clipper) ;