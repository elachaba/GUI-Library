
/**
 *\brief Edge structure that contains the necessary information to perform the polygon filling algorithm
 */
struct edge{
    float ymin;
    float ymax;
    float xymin; // float xymin
    float bres;

    struct edge *next;
};


/**
 *\brief Definition of a hashtable structure necessary to the TCA definition
 */
struct hashtable{
    struct edge **table;
    int size;
    int first_scan;
};


/**
 *\brief Table size structure that contains an array and its size
 */
struct table_size {
    ei_point_t* points_array;
    size_t points_array_size;
};
