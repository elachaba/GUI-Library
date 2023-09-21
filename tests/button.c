#include <stdio.h>
#include <stdlib.h>

#include "ei_application.h"
#include "ei_event.h"
#include "hw_interface.h"
#include "ei_widget_configure.h"


/*
 * button_press --
 *
 *	Callback called when a user clicks on the button.
 */
void button_press(ei_widget_t widget, ei_event_t* event, ei_user_param_t user_param)
{
	printf("Click !\n");
}

/*
 * process_key --
 *
 *	Callback called when any key is pressed by the user.
 *	Simply looks for the "Escape" key to request the application to quit.
 */
bool process_key(ei_event_t* event)
{
	if ( (event->type == ei_ev_close) ||
	    ((event->type == ei_ev_keydown) && (event->param.key.key_code == SDLK_ESCAPE))) {
		ei_app_quit_request();
		return true;
	} else
		return false;
}

/*
 * ei_main --
 *
 *	Main function of the application.
 */
int main(int argc, char** argv)
{
	ei_widget_t	button;

	/* Create the application and change the color of the background. */
	ei_app_create			((ei_size_t){600, 600}, false);
	ei_frame_set_bg_color		(ei_app_root_widget(), (ei_color_t){0x52, 0x7f, 0xb4, 0xff});
	ei_event_set_default_handle_func(process_key);

	/* Create, configure and place the button on screen. */
	button = ei_widget_create	("button", ei_app_root_widget(), NULL, NULL);
	ei_button_configure		(button, &((ei_size_t){300, 200}),
						&(ei_color_t){0x88, 0x88, 0x88, 0xff},
					 	&(int){6},
					 	&(int){40},
					 	&(ei_relief_t){ei_relief_raised},
					 	&(ei_string_t){"Mon premier Bouton!"}, NULL,
					 	&(ei_color_t){0x00, 0x00, 0x00, 0xff}, NULL, NULL, NULL, NULL,
					 	&(ei_callback_t){button_press}, NULL);
	ei_place_xy			(button, 150, 200);

	/* Run the application's main loop. */
	ei_app_run();

	/* We just exited from the main loop. Terminate the application (cleanup). */
	ei_app_free();

	return (EXIT_SUCCESS);
}
