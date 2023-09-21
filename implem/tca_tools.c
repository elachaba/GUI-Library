#include <stdlib.h>
#include <limits.h>
#include "ei_types.h"
#include "hw_interface.h"
#include "structures.h"
#include "draw_tools.h"


struct hashtable* initialise_hashtable(int size)
{
    struct hashtable *hash_t=malloc(sizeof(struct hashtable));
    hash_t->table= calloc(size,sizeof(struct edge));
    hash_t->size= size;
    hash_t->first_scan = INT_MAX;
    return hash_t;
}

int hash_function(int y, int size) {
    y = abs(y);
    return (y * 31) % size; // 31 is a prime number, which can improve the distribution
}


void add_edge(struct hashtable* TC, struct edge* cell) {
    int idx = hash_function(cell->ymin,TC->size);
    if (TC->table[idx] == NULL) {
        TC->table[idx] = cell;

    } else {
        struct edge* current = TC->table[idx];
        struct edge* previous = NULL;
        while (current != NULL && current->ymin < cell->ymin) {
            previous = current;
            current = current->next;
        }

        if (previous == NULL) {
            cell->next = TC->table[idx];
            TC->table[idx] = cell;
        } else {
            cell->next = previous->next;
            previous->next = cell;
        }

    }
    if (cell->ymin < TC->first_scan) {
        TC->first_scan = cell->ymin;
    }
}

void TC_construction(struct hashtable* TC, ei_point_t* point_array, size_t point_array_size) {
    for (size_t i = 0; i < point_array_size; i++) {
        ei_point_t p1 = point_array[i];
        ei_point_t p2 = point_array[(i + 1) % point_array_size];

        if (p1.y == p2.y) {
            continue;
        }

        struct edge* edge;
        edge = malloc(sizeof(struct edge));
        edge->next = NULL;


        if (p1.y < p2.y) {
            edge->ymin = (float)p1.y;
            edge->ymax = (float)p2.y;
            edge->xymin = (float)p1.x;
            edge->bres = (float)(p2.x - p1.x) / (p2.y - p1.y);

        } else {
            edge->ymin = (float)p2.y;
            edge->ymax = (float)p1.y;
            edge->xymin = (float)p2.x;
            edge->bres = (float)(p1.x - p2.x) / (p1.y - p2.y);
        }

// Add the cellule to the hashtable based on its ymin value
        add_edge(TC, edge);
    }

}

void add_to_TCA(struct edge **TCA, struct hashtable *TC, int y) {
    int index = hash_function(y, TC->size);
    struct edge *current_edge = TC->table[index];
    struct edge *prev_edge = NULL;

    while (current_edge) {
        if ((int)current_edge->ymin == y) {
            struct edge *temp_edge = current_edge;
            current_edge = current_edge->next;
            if (prev_edge) {
                prev_edge->next = current_edge;
            } else {
                TC->table[index] = current_edge;
            }

            temp_edge->next = *TCA;
            *TCA = temp_edge;
        } else {
            prev_edge = current_edge;
            current_edge = current_edge->next;
        }
    }
}

void remove_from_TCA(struct edge **TCA,int y) {
    //remove cell with y=ymax
    struct edge *prev_edge = NULL;
    struct edge *current_edge = *TCA;
    while (current_edge)
    {
        if (current_edge->ymax == y)
        {
            if (prev_edge == NULL) //verifier si le cote courant est le premier cote du TCA
            {
                *TCA=current_edge->next; // maj la tete du TCA avec next cote
            }
            else
            {
                prev_edge->next=current_edge->next;//skip current_cell
            }
            struct edge *free_ptr = current_edge;
            current_edge= current_edge->next;
            free(free_ptr);
        }
        else
        {
            prev_edge = current_edge;
            current_edge = current_edge->next;
        }
    }
}

int compareFunction(const void *a, const void *b) {
    int first = (int)((*(struct edge **)a)->xymin);
    int second = (int)((*(struct edge **)b)->xymin);
    return first - second;
}

void sort_TCA(struct edge **TCA) {
    //Verifier si TCA contient au plus un coté

    if(*TCA == NULL || (*TCA)->next == NULL)
    {
        return;
    }

    uint32_t size_TCA=0;
    struct edge *current_edge = *TCA;
    while (current_edge) //compter le nombre de cellules dans TCA
    {
        size_TCA++;
        current_edge = current_edge->next;
    }
    // allocation pour store les cellules de TCA
    struct edge** array_edges = malloc(size_TCA*sizeof(struct cellule *));
    current_edge = *TCA;
    for(size_t i=0 ; i<size_TCA; i++)
    {
        array_edges[i] = current_edge;
        current_edge = current_edge->next;
    }
    qsort(array_edges, size_TCA, sizeof(struct cellule *), compareFunction);
    *TCA= array_edges[0];
    current_edge = *TCA; //build sorted TCA from sorted array_cellules
    for (size_t i=1; i<size_TCA ; i++){
        current_edge->next=array_edges[i];
        current_edge = current_edge->next;
    }
    current_edge->next = NULL;
    free(array_edges);
}

void update_xymin(struct edge *TCA ) {
    struct edge *current_edge = TCA;

    while (current_edge) {
        current_edge->xymin += current_edge->bres;
        current_edge = current_edge->next;
    }
}

void scanline(ei_surface_t surface, struct edge *TCA,int y , ei_color_t color,const ei_rect_t*clipper) {
    struct edge* current_edge = TCA;
    uint32_t *pixel = NULL;
    int id_r, id_g, id_b, id_a;
    hw_surface_get_channel_indices(surface, &id_r, &id_g, &id_b, &id_a);
    while (current_edge != NULL && current_edge->next != NULL) {
        //to make sure there is a pair of vertices.
        // Respect the rule of filling
        int x_start = ceil(current_edge->xymin);
        int x_end = floor(current_edge->next->xymin);
        for (int x = x_start; x<=x_end; x++)
        {
            ei_point_t point = {x, y};
            if (point_in_clipper(&point,clipper))
            {
                pixel = point_to_pixel(&point,surface);
                get_pixel_color_var(pixel, &color, id_r, id_g, id_b, id_a);
            }
        }
        current_edge = current_edge->next->next;
    }
}

int get_first_scanline(struct hashtable *TC) {
    return TC->first_scan;
}

int is_TC_empty(struct hashtable* TC) {
    for (int i = 0; i < TC->size; i++)
    {
        if (TC->table[i] != NULL)
        {
            return 0; // Not empty
        }
    }
    return 1; // Empty
}