#include <stdbool.h>
#include <stdlib.h>
#include "ei_utils.h"
#include "tca_tools.h"
#include "hw_interface.h"
#include "draw_tools.h"
#include "draw_button.h"


void ei_fill(
        ei_surface_t surface,
        const ei_color_t* color,
        const ei_rect_t* clipper)
{
    // Fill a given surface with the given color while respecting the clipper
    uint32_t pixel_color = get_pixel_color_unif(surface, color);
    uint32_t* pixel;
    ei_size_t surface_size = hw_surface_get_size(surface);
    if (clipper == NULL)
    {
        uint32_t* pixel_int = (uint32_t*) hw_surface_get_buffer(surface);
        uint32_t total_pixels = surface_size.width * surface_size.height;
        for (pixel = pixel_int; pixel < pixel_int+total_pixels; pixel++)
        {
            *pixel = pixel_color;
        }


    }
    else
    {
        ei_rect_t full_screen = hw_surface_get_rect(surface);
        ei_rect_t* fill_screen = (intersect_rect(&full_screen, clipper));
        if (fill_screen)
        {
            ei_point_t starting_point = fill_screen->top_left;
            ei_point_t point;
            for (int y=starting_point.y; y < starting_point.y + fill_screen->size.height ; y++) {
                for (int x=starting_point.x; x < starting_point.x + fill_screen->size.width; x++){
                    point.x = x;
                    point.y = y;
                    pixel = point_to_pixel(&point, surface);
                    *pixel = pixel_color;
                }
            }
            free(fill_screen);
        }


    }

}

void ei_draw_polyline(
        ei_surface_t surface,
        ei_point_t* point_array,
        size_t point_array_size,
        ei_color_t color,
        const ei_rect_t* clipper) {
    // Draws a polyline based on the Bresenham algorithm
    hw_surface_lock(surface);
    // Case where the size of the array is 0
    if (point_array_size == 0) {
        hw_surface_unlock(surface);
        return;
    }
    else {
/* Check of the RGBA convention used*/
        uint32_t pixel_color = get_pixel_color_unif(surface, &color);
        /* Dessin du premier point */
        ei_point_t* point_courant = point_array;
        if (point_in_clipper(point_courant, clipper)) {
            uint32_t* pixel_courant = point_to_pixel(point_courant, surface);
            *pixel_courant = pixel_color;
        }
/* The case of one point */
        if (point_array_size == 1) {
            hw_surface_unlock(surface);
            return;
        }
        ei_point_t* point_suivant;
        int delta_x;
        int delta_y;
        int std_dev_x;
        int std_dev_y;
        int error;
        int err_save;
        /* Traitement des points suivants grâce à l'algorithme de Bresenham */
        for (size_t i = 1; i < point_array_size; i++) {
            point_courant = point_array + i - 1;
            point_suivant = point_array + i;
            delta_x = abs(point_suivant->x - point_courant->x);
            delta_y = abs(point_suivant->y - point_courant->y);
            std_dev_x = point_courant->x < point_suivant->x ? 1 : -1;
            std_dev_y = point_courant->y < point_suivant->y ? 1 : -1;
            error = (delta_x > delta_y ? delta_x : -delta_y) / 2;
            while (true) {
                if (point_in_clipper(point_courant, clipper)) {
                    uint32_t* pixel_courant = point_to_pixel(point_courant, surface);
                    *pixel_courant = pixel_color;
                }
                if (point_courant->x == point_suivant->x && point_courant->y == point_suivant->y) {
                    break;
                }
                err_save = error;
                if (err_save > -delta_x) {
                    error -= delta_y;
                    point_courant->x += std_dev_x;
                }
                if (err_save < delta_y) {
                    error += delta_x;
                    point_courant->y += std_dev_y;
                }
            }
        }

        hw_surface_unlock(surface);
    }
}
void free_hashtable(struct hashtable* tableau)
{
    free(tableau->table);
    free(tableau);
}


/*------------------------------Draw Polygon------------------------------------*/

void ei_draw_polygon (ei_surface_t surface, ei_point_t* point_array,
                      size_t point_array_size, ei_color_t color, const ei_rect_t* clipper){
    // Draw polygon main function based on active edge tables algorithm
    size_t TC_SIZE = 2 * point_array_size; // Vous pouvez ajuster ce facteur en fonction de vos besoins
    struct hashtable* TC = initialise_hashtable((int)TC_SIZE);
    TC_construction(TC, point_array, point_array_size);

    /*Initialisation de la TCA et de la scanline*/
    struct edge *TCA = NULL;
    int y_scanline = get_first_scanline(TC);
    /*Condition d'arrêt de la boucle : existence d'élements dans TC/TCA*/

    while (!is_TC_empty(TC)||TCA!=NULL) {
        /*Traitement des côtés dont ymax=scanline*/
        add_to_TCA(&TCA, TC, y_scanline);
        /* Elimination des côtés non désirés (ymax=scanline)*/
        remove_from_TCA(&TCA, y_scanline);
        /* Tri des côtés de la TCA*/
        sort_TCA(&TCA);
        /* Fill entre chaque paire d'élts de TCA*/
        scanline(surface,TCA,y_scanline,color,clipper);
        /*Propagation de scanline*/
        y_scanline++;
        update_xymin(TCA);
    }
    free_hashtable(TC);
}

/* ----------------------- Draw Text --------------------------- */

int	ei_copy_surface (
        ei_surface_t destination,
        const ei_rect_t*	dst_rect,
        ei_surface_t source,
        const ei_rect_t*	src_rect,
        bool	alpha){
// Function that performs the copying of a source surface to a desired destination


    ei_rect_t src_full_rect = hw_surface_get_rect(source);
    ei_rect_t dst_full_rect = hw_surface_get_rect(destination);

    if (src_rect == NULL) {
        src_rect = &src_full_rect;
    }
    else if (src_rect->size.width < 0 || src_rect->size.height < 0){
        return EXIT_FAILURE;
    }
    if (dst_rect == NULL){
        dst_rect = &dst_full_rect;
    }
    else if (dst_rect->size.width < 0 || dst_rect->size.height < 0) {
        return EXIT_FAILURE;
    }

    ei_rect_t* copy_area = intersect_rect(src_rect, &src_full_rect);
    ei_rect_t* paste_area = intersect_rect(dst_rect, &dst_full_rect);

    if (copy_area == NULL || paste_area == NULL) {
        return EXIT_FAILURE;
    }
    ei_size_t src_size, dst_size;
    src_size = copy_area->size;
    dst_size = paste_area->size;

    if (src_size.width != dst_size.width || src_size.height != dst_size.height) {
        return EXIT_FAILURE;
    }

    ei_point_t src_pnt,dst_pnt;
    uint32_t* pixel_src;
    uint32_t* pixel_dst;

    int id_r, id_g, id_b, id_a;
    hw_surface_get_channel_indices(source, &id_r, &id_g, &id_b, &id_a);

    for (int y_counter = 0; y_counter < src_size.height; y_counter++) {
        for (int x_counter = 0; x_counter < src_size.width; x_counter++) {
            src_pnt.y = (copy_area->top_left).y + y_counter;
            src_pnt.x = copy_area->top_left.x + x_counter;
            dst_pnt.y = (paste_area->top_left).y + y_counter;
            dst_pnt.x = paste_area->top_left.x + x_counter;
            pixel_src = point_to_pixel(&src_pnt, source);
            pixel_dst = point_to_pixel(&dst_pnt, destination);
            if (!alpha) {
                memcpy(pixel_dst, pixel_src, 4);
            }
            else {
                *pixel_dst = combine_pixels(*pixel_src, *pixel_dst, id_r, id_g, id_b, id_a);
            }

        }
    }

    return EXIT_SUCCESS;
}

void	ei_draw_text (ei_surface_t	surface,
                      const ei_point_t*	where,
                      ei_const_string_t	text,
                      ei_font_t		font,
                      ei_color_t		color,
                      const ei_rect_t*	clipper) {
    // Text drawing function in a given surface

    if (font == NULL){
        font = ei_default_font;
    }

    ei_surface_t text_surface = hw_text_create_surface(text, font, color);
    hw_surface_lock(text_surface);
    ei_size_t size_txt_srfc = hw_surface_get_size(text_surface);
    ei_rect_t dst_rect = {*where, size_txt_srfc};
    ei_rect_t*  text_paste_area = malloc(sizeof(ei_rect_t));

    if (clipper == NULL){
        ei_rect_t full_size = {ei_point_zero(), hw_surface_get_size(surface)};
        text_paste_area = intersect_rect(&dst_rect, &full_size);
    }
    else {
        text_paste_area =intersect_rect(&dst_rect, clipper);
    }

    ei_rect_t* text_copy_area = malloc(sizeof(ei_rect_t));
    if(text_paste_area) {
        *text_copy_area = (ei_rect_t){ei_point_zero(), text_paste_area->size};
    }
    else {
        text_copy_area = NULL;
    }
    ei_copy_surface(surface, text_paste_area, text_surface, text_copy_area, true);
    free(text_paste_area);
    free(text_copy_area);
    hw_surface_unlock(text_surface);
    hw_surface_free(text_surface);
    //hw_surface_unlock(surface);
}
