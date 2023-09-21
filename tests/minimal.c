#include <stdlib.h>

#include "ei_types.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "hw_interface.h"

int main(int argc, char* argv[])
{
	ei_surface_t			main_window		= NULL;
	ei_size_t			main_window_size	= ei_size(640, 480);
	ei_event_t			event;
	uint32_t			white			= 0xffffffff;
	uint8_t 			red[4]			= { 255, 255, 255, 255 };
	uint32_t*			pixel_ptr;
	int				i, ir, ig, ib, ia;

	// Init acces to hardware.
	hw_init();

	// Create the main window.
	main_window = hw_create_window(main_window_size, false);

	// Compute the value of a red pixel for this surface.
	hw_surface_get_channel_indices(main_window, &ir, &ig, &ib, &ia);
	red[ir]	= 255;
	red[ig]	= red[ib] = 0;

	// Lock the surface for drawing,
	hw_surface_lock(main_window);

	// fill the top half in white, the bottom half in red (hurray Poland!)
	pixel_ptr = (uint32_t*)hw_surface_get_buffer(main_window);

	for (i = 0; i < (main_window_size.width * (main_window_size.height/2)); i++)
		*pixel_ptr++ = white;

	for (i = 0; i < (main_window_size.width * (main_window_size.height/2)); i++)
		*pixel_ptr++ = *((uint32_t*)red);

	// unlock, update screen.
	hw_surface_unlock(main_window);
	hw_surface_update_rects(main_window, NULL);

	// Wait for a key press.
	event.type = ei_ev_none;
	while ((event.type != ei_ev_close) && (event.type != ei_ev_keydown))
		hw_event_wait_next(&event);

	// Free hardware resources.
	hw_quit();

	// Terminate program with no error code.
	return 0;
}
