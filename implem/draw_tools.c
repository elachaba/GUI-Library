#include <stdlib.h>
#include "hw_interface.h"
#include "ei_types.h"

int max_function(int val_1, int val_2){
    if (val_1 > val_2){
        return val_1;
    }
    return val_2;
}

int min_function(int val_1, int val_2){
    if (val_1 < val_2){
        return val_1;
    }
    return val_2;
}

uint32_t* point_to_pixel(ei_point_t* point, ei_surface_t surface){
    // Function that returns the pixel associated to a given point
    uint32_t* pixel_courant = (uint32_t*) hw_surface_get_buffer(surface);
    int translation = ((*point).y) * (hw_surface_get_size(surface).width);
    uint32_t* pixel = pixel_courant + (*point).x + translation;
    return pixel;
}

uint32_t get_pixel_color_unif(ei_surface_t surface, const ei_color_t* color) {
    if (color == NULL) {
        return 0x000f;
    }
    else {
        uint32_t pixel_color;
        int idx_blue, idx_green, idx_red, idx_alpha;
        hw_surface_get_channel_indices(surface, &idx_red, &idx_green, &idx_blue, &idx_alpha);

        uint8_t* color_ptr = (uint8_t*)&pixel_color;
        color_ptr[idx_red] = color->red;
        color_ptr[idx_green] = color->green;
        color_ptr[idx_blue] = color->blue;
        color_ptr[idx_alpha] = color->alpha;

        return pixel_color;
    }
}

void get_pixel_color_var(uint32_t* pixel, const ei_color_t* color, int id_r, int id_g, int id_b, int id_a) {

    int pa, pr, pg, pb;

    if (color == NULL){
        pa = 255;
        pr = 0;
        pg = 0;
        pb = 0;
    }
    else {
        pr = color->red;
        pg = color->green;
        pb = color->blue;
        pa = color->alpha;
    }
    uint8_t* pixel_ptr = (uint8_t*) pixel;
    if (pixel == NULL){
        *(pixel_ptr + id_r) = 0;
        *(pixel_ptr + id_g) = 0;
        *(pixel_ptr + id_b) = 0;
    }
    else {
        pixel_ptr[id_r] = (pr * pa + (255 - pa) * pixel_ptr[id_r]) / 255;
        pixel_ptr[id_g] = (pg * pa + (255 - pa) * pixel_ptr[id_g]) / 255;
        pixel_ptr[id_b] = (pb * pa + (255 - pa) * pixel_ptr[id_b]) / 255;
    }
    if (id_a != -1) {
        *(pixel_ptr + id_a) = pa;
    }
}

bool point_in_clipper(ei_point_t* point, const ei_rect_t* clipper) {
    // Function that performs the necessary comparisons to check if a point belongs to the clipper
    if (clipper == NULL) {
        return true;
    }
    if ((*point).x<(*clipper).top_left.x || (*point).x>(*clipper).top_left.x + (*clipper).size.width) {
        return false;
    }
    if ((*point).y<(*clipper).top_left.y || (*point).y>(*clipper).top_left.y + (*clipper).size.height) {
        return false;
    }
    return true;
}

ei_rect_t* intersect_rect(const ei_rect_t* rect_1, const ei_rect_t* rect_2){

    if (!rect_2 || !rect_1){
        return NULL;
    }

    int x_left = max_function(rect_1->top_left.x, rect_2->top_left.x);

    int y_top = max_function( rect_1->top_left.y, rect_2->top_left.y);

    int x_right = min_function(rect_1->top_left.x + rect_1->size.width,
                               rect_2->top_left.x + rect_2->size.width);

    int y_bottom = min_function(rect_1->top_left.y + rect_1->size.height,
                                rect_2->top_left.y + rect_2->size.height);

    if (x_left <= x_right && y_top <= y_bottom){
        ei_rect_t* rect_intersection = (ei_rect_t*) malloc(sizeof(ei_rect_t));
        rect_intersection->top_left.x = x_left;
        rect_intersection->top_left.y = y_top;
        rect_intersection->size.width = x_right - x_left;
        rect_intersection->size.height = y_bottom - y_top;
        return rect_intersection;
    }
    return NULL;
}

uint32_t combine_pixels(uint32_t pixel_s, uint32_t pixel_d, int ir, int ig, int ib, int ia){
    uint32_t combo_pixel;
    uint8_t* src_ptr = (uint8_t*) &pixel_s;
    uint8_t* dst_ptr = (uint8_t*) &pixel_d;
    uint8_t* combo_ptr = (uint8_t*) &combo_pixel;
    combo_ptr[ir] = (src_ptr[ir]*src_ptr[ia] + (255 - src_ptr[ia])*dst_ptr[ir])/255;
    combo_ptr[ig] = (src_ptr[ig]*src_ptr[ia] + (255 - src_ptr[ia])*dst_ptr[ig])/255;
    combo_ptr[ib] = (src_ptr[ib]*src_ptr[ia] + (255 - src_ptr[ia])*dst_ptr[ib])/255;
    combo_ptr[ia] = 255;
    return combo_pixel;
}

void ei_fill_frame(ei_surface_t surface, ei_color_t* color, ei_rect_t* clipper) {
// Filling using analytical clipping

    int idx_blue, idx_red, idx_green, idx_alpha;
    hw_surface_get_channel_indices(surface,&idx_red, &idx_green, &idx_blue, &idx_alpha);
    uint32_t* pixel_int = (uint32_t*) hw_surface_get_buffer(surface);
    ei_size_t surface_size=hw_surface_get_size(surface);
    if (clipper == NULL){

        for (uint32_t* pixel = pixel_int; pixel < pixel_int+surface_size.width*surface_size.height; pixel++){
            get_pixel_color_var(pixel, color, idx_red, idx_green, idx_blue, idx_alpha);
        }
    }
    else {

        ei_rect_t full_screen = hw_surface_get_rect(surface);
        ei_rect_t* fill_screen = (intersect_rect(&full_screen, clipper));
        if (fill_screen)
        {
            ei_point_t starting_point = fill_screen->top_left;
            ei_point_t point;
            for (int y=starting_point.y; y <= starting_point.y + fill_screen->size.height ; y++) {
                for (int x=starting_point.x; x <= starting_point.x + fill_screen->size.width; x++){
                    point.x = x;
                    point.y = y;
                    uint32_t* pixel = point_to_pixel(&point, surface);
                    get_pixel_color_var(pixel, color, idx_red, idx_green, idx_blue, idx_alpha);
                }
            }
            free(fill_screen);
        }

    }
}