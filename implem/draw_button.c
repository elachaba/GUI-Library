#include "ei_types.h"
#include "structures.h"
#include "draw_tools.h"
#include "ei_draw.h"
#include "ei_utils.h"
#include <math.h>
/*------------------------- Draw arc -----------------------------*/

ei_point_t arc_point(float angle, ei_point_t center, float radius);

double* nbre_of_points_angle_step(ei_point_t center, float radius, float start_angle, float end_angle){
    // Function that return an array where its first element is the number of points that will be allocated in the new array , and its second represent the elementary angle
    double* tab = malloc(2 * sizeof(double));
    double arc_length =1.5* (end_angle - start_angle) * radius;

    tab[0] = ceil(arc_length + 1) ;
    tab[1] = (end_angle - start_angle) / tab[0];
    return tab;
}

ei_point_t* draw_arc(ei_point_t center, float radius, float start_angle, float end_angle ) {
    /*Les angles de début et fin d'arcs sont des angles orientés mesurés
     * par rapport à l'horizontale et dont le sens est le sens trigonométrique
     * On a alors : end_angle> start_angle */
    if (!radius){
        // Case where radius is 0. The desired output is a simple rectangle
        ei_point_t* center_ptr = malloc(sizeof(ei_point_t));
        *center_ptr = center;
        return center_ptr;
    }
    else {
        double *arc_info = nbre_of_points_angle_step(center, radius, start_angle, end_angle);
        double nb_points = arc_info[0];
        ei_point_t *point_array = (ei_point_t *) malloc((nb_points + 2) * sizeof(ei_point_t));

        point_array[0] = arc_point(start_angle, center, radius);
        double angle_step = arc_info[1];
        double current_angle = start_angle;
        uint32_t i = 1;
        while (i <= nb_points) {
            current_angle = start_angle + i * angle_step;
            point_array[i] = arc_point(current_angle, center, radius);
            i++;
        }
        point_array[i] = arc_point(end_angle, center, radius);

        free(arc_info);
        return point_array;
    }
}

ei_point_t arc_point(float angle, ei_point_t center, float radius) {
    // Attention, ce qui est à l'intérieur du sin() est en radian
    ei_point_t point;
    point.x= (uint32_t) (center.x + radius * sin(angle));
    point.y= (uint32_t) (center.y + radius * cos(angle));
    return point;
}


struct table_size rounded_frame(const ei_rect_t rectangle, const float radius, bool only_top, bool only_bottom) {
//TOP_LEFT
    ei_point_t center_top_left;
    center_top_left.x = rectangle.top_left.x + radius;
    center_top_left.y = rectangle.top_left.y + radius;
    ei_point_t* top_left = draw_arc(center_top_left, radius, M_PI, 3 * M_PI / 2);
    int top_left_points;
    if (!radius){
        top_left_points = 1;
    }
    else {
        double *top_left_info = nbre_of_points_angle_step(center_top_left, radius, M_PI, 3 * M_PI / 2);
        top_left_points = (int) top_left_info[0];
        free(top_left_info);
    }

    //TOP_RIGHT
    ei_point_t center_top_right;
    center_top_right.x = rectangle.top_left.x + rectangle.size.width - radius;
    center_top_right.y = rectangle.top_left.y + radius;
    ei_point_t * top_right = draw_arc(center_top_right, radius, (M_PI/2) + only_top * M_PI / 4, M_PI - only_bottom * M_PI / 4);
    int top_right_points;
    if (!radius){
        top_right_points = 1;
    }
    else {
        double *top_right_info = nbre_of_points_angle_step(center_top_right, radius, (M_PI / 2) + only_top * M_PI / 4,
                                                           M_PI - only_bottom * M_PI / 4);
        top_right_points = (int) top_right_info[0];
        free(top_right_info);
    }
    //BOTTOM_LEFT
    ei_point_t center_bottom_left;
    center_bottom_left.x = rectangle.top_left.x + radius;
    center_bottom_left.y = rectangle.top_left.y + rectangle.size.height - radius;
    ei_point_t * bottom_left;
    bottom_left = draw_arc(center_bottom_left, radius, 3 * M_PI / 2 + only_bottom * M_PI / 4,
                               2 * M_PI - only_top * M_PI / 4);
    int bottom_left_points;
    if (!radius ){
        bottom_left_points = 1;
    }
    else {
        double *bottom_left_info = nbre_of_points_angle_step(center_bottom_left, radius,
                                                             3 * M_PI / 2 + only_bottom * M_PI / 4,
                                                             2 * M_PI - only_top * M_PI / 4);
        bottom_left_points = (int) bottom_left_info[0];
        free(bottom_left_info);
    }

    //BOTTOM_RIGHT
    ei_point_t center_bottom_right;
    center_bottom_right.x =(int) rectangle.top_left.x + rectangle.size.width - radius;
    center_bottom_right.y =(int) rectangle.top_left.y + rectangle.size.height - radius;
    ei_point_t * bottom_right;
    bottom_right = draw_arc(center_bottom_right, radius, 0, M_PI / 2);

    int bottom_right_points;
    if (!radius){
        bottom_right_points = 1;
    }
    else {
        double *bottom_right_info = nbre_of_points_angle_step(center_bottom_right, radius, 0, M_PI / 2);
        bottom_right_points = (int) bottom_right_info[0];
        free(bottom_right_info);
    }

    //Optional points
    int h = min_function(rectangle.size.height,rectangle.size.width) /2;
    ei_point_t point1 = {rectangle.top_left.x+h,rectangle.top_left.y+h};
    ei_point_t point2 = {point1.x+h,point1.y};

    // Total number of points
    int total_points = ((1 - only_top) * (1 - only_bottom) + only_top) * top_left_points + top_right_points + bottom_left_points + ((1 - only_top) * (1 - only_bottom) + only_bottom) * bottom_right_points + 2 * (only_top + only_bottom);

    // Copying the relevant list of points
    ei_point_t *point_frame = malloc(total_points * sizeof(ei_point_t));
    ei_point_t *ptr = point_frame;
    if ((1 - only_top) * (1 - only_bottom) + only_top){
        memcpy(ptr, top_left, top_left_points * sizeof(ei_point_t));
        ptr += top_left_points;
        memcpy(ptr, bottom_left, bottom_left_points * sizeof(ei_point_t));
        ptr += bottom_left_points;
        if (only_top){
            *ptr=point1;
            ptr++;
            *ptr=point2;
            ptr++;
            memcpy(ptr, top_right, top_right_points * sizeof(ei_point_t));
        }
    }

    if ((1 - only_top) * (1 - only_bottom) + only_bottom){
        memcpy(ptr, bottom_right, bottom_right_points * sizeof(ei_point_t));
        ptr += bottom_right_points;
        memcpy(ptr, top_right, top_right_points * sizeof(ei_point_t));
        if (only_bottom){
            ptr += top_right_points;
            *ptr=point2;
            ptr++;
            *ptr=point1;
            ptr++;
            memcpy(ptr, bottom_left, bottom_left_points * sizeof(ei_point_t));
        }
    }

    free(top_left);
    free(top_right);
    free(bottom_left);
    free(bottom_right);
    struct table_size table;
    table.points_array = point_frame;
    table.points_array_size = total_points;

    return table;
}
/* ----------------------- Draw Button --------------------------- */

void draw_button(ei_surface_t surface ,ei_rect_t rectangle,ei_rect_t *clipper,ei_color_t color_frame,int border_width, float radius, ei_relief_t relief) {
// choosing percentage of white to add
    float t = 0.2;
    // Setting colors that will be used
    ei_color_t shaded_color = {color_frame.red * (1 - t), color_frame.green * (1 - t), color_frame.blue * (1 - t), color_frame.alpha};
    ei_color_t tinted_color = {color_frame.red + t * (255 - color_frame.red), color_frame.green + t * (255 - color_frame.green),
                               color_frame.blue + t * (255 - color_frame.blue),color_frame.alpha};
    // smaller rectangle
    ei_rect_t center_frame ={{rectangle.top_left.x+border_width,rectangle.top_left.y+border_width},{rectangle.size.width-2*border_width,rectangle.size.height-2*border_width}};

    if (relief != ei_relief_none) {
        struct table_size tab_up = rounded_frame(rectangle, radius,true,false);
        ei_point_t* up_rounded_frame = tab_up.points_array ;
        size_t up_rounded_frame_size = tab_up.points_array_size ;

        struct table_size tab_down = rounded_frame(rectangle,radius,false,true);
        ei_point_t* down_rounded_frame = tab_down.points_array;
        size_t down_rounded_frame_size = tab_down.points_array_size;

        struct table_size tab_center = rounded_frame(center_frame,radius,false,false);
        ei_point_t *center_rounded_frame = tab_center.points_array;
        size_t center_rounded_frame_size = tab_center.points_array_size;
        if (relief == ei_relief_sunken) {
            // Case where the button is sunken
            ei_draw_polygon(surface, up_rounded_frame, up_rounded_frame_size, shaded_color, clipper);
            ei_draw_polygon(surface, down_rounded_frame, down_rounded_frame_size, tinted_color, clipper);
        }
        else if (relief == ei_relief_raised) {
            // Case where the button is raised
            ei_draw_polygon(surface, up_rounded_frame, up_rounded_frame_size, tinted_color, clipper);
            ei_draw_polygon(surface, down_rounded_frame, down_rounded_frame_size, shaded_color, clipper);
        }
        ei_draw_polygon(surface,center_rounded_frame,center_rounded_frame_size,color_frame,clipper);
        free(up_rounded_frame);
        free(down_rounded_frame);
        free(center_rounded_frame);
    }
    else
    {

        struct table_size tab_round = rounded_frame(rectangle,radius,0,0);
        ei_point_t *round_frame =tab_round.points_array ;
        size_t rounded_frame_size = tab_round.points_array_size;
        ei_draw_polygon(surface,round_frame,rounded_frame_size,color_frame,clipper);
        free(round_frame);
    }
}

void top_level(ei_surface_t surface ,ei_rect_t rectangle,ei_rect_t *clipper,ei_color_t color_frame, float radius) {
/*Radius!=0*/
//TOP_LEFT
    ei_point_t center_top_left;
    center_top_left.x = rectangle.top_left.x + radius;
    center_top_left.y = rectangle.top_left.y + radius;
    ei_point_t* top_left = draw_arc(center_top_left, radius, M_PI, 3 * M_PI / 2);
    double *top_left_info = nbre_of_points_angle_step(center_top_left, radius, M_PI, 3 * M_PI / 2);
    int top_left_points = (int)top_left_info[0];


//TOP_RIGHT
    ei_point_t center_top_right;
    center_top_right.x = rectangle.top_left.x + rectangle.size.width - radius;
    center_top_right.y = rectangle.top_left.y + radius;
    ei_point_t * top_right = draw_arc(center_top_right, radius, M_PI/2, M_PI);
    double *top_right_info = nbre_of_points_angle_step(center_top_right, radius, M_PI/2, M_PI );
    int top_right_points = (int)top_right_info[0];


//BOTTOM_LEFT

   ei_point_t bottom_left_point = ei_point(rectangle.top_left.x,rectangle.top_left.y +rectangle.size.height);
   ei_point_t* bottom_left = (ei_point_t*) malloc(sizeof(ei_point_t));
   bottom_left= &bottom_left_point ;
    int bottom_left_points = 1;

//BOTTOM_RIGHT

    ei_point_t bottom_right_point =ei_point(rectangle.top_left.x+rectangle.size.width,rectangle.top_left.y +rectangle.size.height );
    ei_point_t* bottom_right =(ei_point_t*) malloc(sizeof(ei_point_t));
     bottom_right =& bottom_right_point ;
    int bottom_right_points = 1;





// Total number of points
    int total_points = top_left_points + top_right_points + bottom_left_points + bottom_right_points;

// Fill top_level
    ei_point_t *point_frame = malloc(total_points * sizeof(ei_point_t));
    ei_point_t *ptr = point_frame;
    memcpy(ptr, top_left, top_left_points * sizeof(ei_point_t));
    ptr += top_left_points;
    memcpy(ptr, bottom_left, bottom_left_points * sizeof(ei_point_t));
    ptr += bottom_left_points;
    memcpy(ptr, bottom_right, bottom_right_points * sizeof(ei_point_t));
    ptr += bottom_right_points;
    memcpy(ptr, top_right, top_right_points * sizeof(ei_point_t));
    ei_draw_polygon(surface, point_frame, total_points, color_frame, clipper);


// Freeing
    free(top_left_info);
    free(top_right_info);

}
