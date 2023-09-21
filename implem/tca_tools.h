#include "ei_types.h"
#include "hw_interface.h"
#include "structures.h"

/**
 * \brief Creates a hashtable
 * @param size The size of hashtable to create
 * @return An empty hashtable of size @param size
 */

struct hashtable* initialise_hashtable(int size);

/**
 * \brief A function that relates the ordinates of the points to an index of the hash table
 * @param y The ordinate
 * @param size The size of the hashtable
 * @return The index in the hashtable of the ordinate @param y
 */

int hash_function(int y, int size);

/**
 * \brief Adds an edge to the TC.
 * @param TC The TC to be modified
 * @param cell The edge to be added
 */

void add_edge(struct hashtable* TC, struct edge* cell);

/**
 * \brief Constructs a TC that corresponds to the array of points
 * @param TC The empty hashtable to be filled
 * @param point_array The array of vertices.
 * @param point_array_size The size of the array of vertices.
 */

void TC_construction(struct hashtable* TC, ei_point_t* point_array, size_t point_array_size);

/**
 * \brief Adds an edge from TC to TCA
 * @param TCA The TCA to be modified
 * @param TC The initial TC
 * @param y The ordinate of the point corresponding to the edge to add
 */

void add_to_TCA(struct edge **TCA, struct hashtable *TC, int y);

/**
 * \brief Removes an edge from TCA
 * @param TCA The TCA to be modified
 * @param y The ordinate of the edge to be removed from the TCA
 */

void remove_from_TCA(struct edge **TCA,int y); //remove cell with y=ymax

/**
 * \brief A compare function that the determines the direction of the sort
 * @param a
 * @param b
 * @return ...
 */

int compareFunction(const void *a, const void *b);

/**
 * Sorts the TCA passed in parameters
 * @param TCA The TCA to be sorted
 */

void sort_TCA(struct edge **TCA);

/**
 * \brief Increments the value of xymin the TCA
 * @param TCA
 */

void update_xymin(struct edge *TCA );

/**
 * \brief Colors a line inside a polygon using the scanline method
 * @param surface The surface on which to color the polygon
 * @param TCA The active edges of the polygon to be drawn
 * @param y The ordinate of the line to be drawn
 * @param color if NULL, it colors in black.
 * @param clipper if not NULL, colors only the part of the polygon inside the clipper.
 */

void scanline(ei_surface_t surface, struct edge *TCA, int y , ei_color_t color, const ei_rect_t* clipper);

/**
 * \brief Determines the first line to be colored.
 * @param TC The table of edges
 * @return the y = ymax of the first line to be drawn
 */
int get_first_scanline(struct hashtable *TC);

/**
 * \brief Checks of a hashtable is empty.
 * @param TC The hashtable of edges to be checked
 * @return 1 if TC is empty, else 0
 */
int is_TC_empty(struct hashtable* TC);