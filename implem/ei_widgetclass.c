#include <ei_utils.h>
#include <string.h>
#include "ei_frame.h"

ei_widgetclass_t * widget_classes = NULL;

void ei_widgetclass_register(ei_widgetclass_t* widgetclass) {

        widgetclass->next = widget_classes;
        widget_classes = widgetclass;
}



ei_widgetclass_t*	ei_widgetclass_from_name(ei_const_string_t name)
{
    ei_widgetclass_t* wclass = widget_classes;
    while (wclass)
    {
        if (strcmp(wclass->name, name) != 0)
        {
            wclass = wclass->next;
        }
        else
        {
            break;
        }
    }
    return wclass;
}