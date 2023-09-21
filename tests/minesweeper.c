/*
 *	minesweeper.c
 *	ig
 *
 *	Created by Denis Becker on 12.02.16.
 *	Copyright 2016 Ensimag. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for random seed
#include <string.h> // memset function

#include "ei_application.h"
#include "ei_widget_configure.h"
#include "ei_widget_attributes.h"
#include "ei_utils.h"
#include "ei_event.h"

/* constants */

const int			k_info_height	= 40;
ei_size_t			g_button_size	= { 28, 28 };
ei_color_t			g_bg_color	= {0x88, 0x88, 0x88, 0xff};

/* global resources */

static ei_surface_t		g_flag_img;
static ei_surface_t		g_bomb_img;
static ei_surface_t		g_reset_img;

/* structs & typedefs */

struct map_t;

typedef struct map_pos_t {

	struct map_t*		map;
	ei_point_t		coord;
	bool			has_mine;
	bool			has_flag;
	bool			is_revealed;
	int			nb_mines_around;
	ei_widget_t		widget;
	
} map_cell_t;

typedef struct map_t {

	int			width;
	int			height;
	int			nb_mines;
	map_cell_t*		cells;
	bool			game_over;
	int			nb_revealed;
	int			flag_count;
	double 			start_time;
	ei_widget_t		toplevel;
	ei_widget_t		flag_count_widget;
	ei_widget_t		victory_text_widget;
	ei_widget_t		time_widget;

} map_t;

/* utility functions */

static void create_cell_buttons(map_t* map);
void set_time(map_t *p_map, int time);

static inline map_cell_t* get_cell_at(map_t* map, int x, int y)
{
	return &map->cells[y * map->width + x];
}

static inline bool check_bounds(map_t* map, int x, int y)
{
	return x >= 0 && y >= 0 && x < map->width && y < map->height;
}

static inline bool can_play(map_cell_t* map_pos)
{
	return ((!map_pos->has_flag) && (!map_pos->is_revealed));
}

void int_to_str(int val, char *txt, int txtlen)
{
	bool		positive	= (val >= 0);
	int		idx		= txtlen - 1;

	val = (positive ? val : -val);

	do {
		txt[idx] = (char)('0' + (val % 10));
		val /= 10;
		if (val == 0)
			break;
	} while (idx-- > 0);

	if (!positive && idx > 0)
		txt[idx - 1] = '-';
}

/* core functions */

/*
 * check_victory: configures the victory message if victory.
 */
static bool check_victory(map_t* map)
{
	bool		victory = (map->nb_revealed + map->nb_mines == map->width * map->height);

	if (victory) {
		map->game_over	= true;
		map->start_time	= -1.0;
		ei_frame_configure(map->victory_text_widget, NULL,
				   	&(ei_color_t){0x33, 0x33, 0x33, 0xff}, NULL, NULL,
					&(ei_string_t){"You won!"}, NULL,
					&(ei_color_t){0x55, 0xbb, 0x55, 0xff}, NULL, NULL, NULL, NULL);
	}
	return victory;
}

void update_flag_count(map_t* map)
{
	char		txt[5];
	memset(txt, ' ', 4);
	txt[4]		= '\0';

	int_to_str(map->flag_count, txt, 4);
	ei_frame_set_text(map->flag_count_widget, txt);
}

void switch_flag(map_cell_t* map_pos)
{
	map_pos->has_flag = !map_pos->has_flag;

	if (map_pos->has_flag) {
		ei_button_set_image(map_pos->widget, g_flag_img);
		map_pos->map->flag_count--;
	} else {
		ei_button_set_image(map_pos->widget, g_reset_img);
		map_pos->map->flag_count++;
	}
	update_flag_count(map_pos->map);
}

/*
 * Reveal one position on the map, don't explore neighbors, don't check
 * if already revealed (must be done by the caller), don't end game if it's a mine.
 */
void reveal_position_no_check(map_cell_t* cell)
{
	char		nb_txt[2]	= " ";

	cell->is_revealed = true;
	cell->map->nb_revealed++;

	ei_widget_destroy(cell->widget);
	cell->widget = ei_widget_create("frame", cell->map->toplevel, (void*)cell, NULL);

	ei_place_xy(cell->widget, cell->coord.x * g_button_size.width,
		    		  cell->coord.y * g_button_size.height + k_info_height);

	if (cell->has_mine) {
		ei_frame_configure(cell->widget, &g_button_size, NULL, NULL,
				   		 &(ei_relief_t){ei_relief_none}, NULL, NULL, NULL, NULL,
					    	 &g_bomb_img, NULL, NULL);

	} else if (cell->nb_mines_around > 0) {
		int_to_str(cell->nb_mines_around, nb_txt, 1);
		ei_frame_configure(cell->widget, &g_button_size, NULL, NULL, NULL,
				   		 &(ei_string_t){nb_txt}, NULL, NULL, NULL, NULL, NULL, NULL);
	} else
		ei_frame_set_requested_size(cell->widget, g_button_size);
}

void defeat(map_t* map)
{
	map->game_over	= true;
	map->start_time	= -1.0;

	for (int i = 0; i < map->width * map->height; i++)
		if (map->cells[i].has_mine)
			reveal_position_no_check(&map->cells[i]);
	
	ei_frame_configure(map->victory_text_widget, NULL,
			   	&(ei_color_t){0x33, 0x33, 0x33, 0xff}, NULL, NULL,
			   	&(ei_string_t){"You lost!"}, NULL,
			   	&(ei_color_t){0xbb, 0x55, 0x55, 0xff}, NULL, NULL, NULL, NULL);
}

/*
 * Caller must ensure that map_cell isn't already revealed.
 */
void reveal_and_explore(map_cell_t* map_cell)
{
	int			x;
	int			y;
	int			cx	= map_cell->coord.x;
	int			cy	= map_cell->coord.y;
	map_t*			map	= map_cell->map;
	map_cell_t*		next;

	reveal_position_no_check(map_cell);
	if (map_cell->has_mine) {
		defeat(map);
		return;

	} else if (map_cell->nb_mines_around > 0)
		// explosion risk around, stop exploration
		return;

	for (y = cy - 1; y <= cy + 1; y++)
		for (x = cx - 1; x <= cx + 1; x++) {
			next = get_cell_at(map, x, y);
			if ((x != cx || y != cy) && check_bounds(map, x, y)
					&& (!next->is_revealed) && (!next->has_flag))
				reveal_and_explore(next);
		}
}

/*
 * set_mine_in_cell: places a mine in the given cell.
 */
void set_mine_in_cell(map_cell_t* map_cell)
{
	int	x;
	int	y;
	int	cx	= map_cell->coord.x;
	int	cy	= map_cell->coord.y;
	map_t*	map	= map_cell->map;

	map_cell->has_mine = true;

	// update neighbors mine count
	for (y = cy - 1; y <= cy + 1; y++)
		for (x = cx - 1; x <= cx + 1; x++)
			if ((x != cx || y != cy) && check_bounds(map, x, y))
				get_cell_at(map, x, y)->nb_mines_around++;
}

/*
 * place_mines: places the mines on an empty map.
 */
void place_mines(map_t* map)
{
	int i, n;

	for (n = 0; n < map->nb_mines; n++) {
		do {
			i = rand() % (map->width * map->height);
		} while (map->cells[i].has_mine);

		set_mine_in_cell(&map->cells[i]);
	}
}


/*
 * reset_game: creates a new grid.
 */
void reset_game(map_t* map)
{
	int		i;
	map_cell_t*	cell;

	map->flag_count			= map->nb_mines;
	map->game_over			= false;
	map->nb_revealed		= 0;
	map->start_time			= -1.0;

	for (i = 0, cell = map->cells; i < map->width * map->height; i++, cell++) {
		cell->has_mine		= false;
		cell->has_flag		= false;
		cell->is_revealed	= false;
		cell->nb_mines_around	= 0;

		ei_widget_destroy(cell->widget);
	}
	create_cell_buttons(map);

	place_mines(map);
	update_flag_count(map);
	ei_frame_configure(map->victory_text_widget, NULL,
			   		&(ei_color_t){0xbb, 0xbb, 0xbb, 0xff}, NULL, NULL,
				   	&(ei_string_t){NULL}, NULL, NULL, NULL, NULL, NULL, NULL);
}

/*
 * Create and fill the mine map.
 * 'map' must point to an allocated 'map_t' struct.
 */
void create_mine_map(map_t* map, int width, int height, int nb_mines)
{
	int		x, y;
	map_cell_t*	cell;
	
	map->width		= width;
	map->height		= height;
	map->nb_mines		= nb_mines;
	map->cells		= (map_cell_t*)calloc((size_t)(width * height), sizeof(map_cell_t));
	map->game_over		= false;
	map->nb_revealed	= 0;
	map->flag_count		= nb_mines;
	map->start_time		= -1.0;

	for (y = 0, cell = map->cells; y < map->height; y++)
		for (x = 0; x < map->width; x++, cell++) {
			cell->map	= map;
			cell->coord	= ei_point(x, y);
		}

	place_mines(map);
}

void destroy_mine_map(map_t* map)
{
	free(map->cells);
}

/* event handlers */

void handle_time(map_t* map)
{
	static int	prev_int_time		= -1;
	int 		int_time;

	if (map->start_time == -1.0)
		return;

	int_time = (int)floor(hw_now() - map->start_time);
	if (int_time != prev_int_time) {
		set_time(map, int_time);
		prev_int_time	= int_time;
	}
}

void set_time(map_t* map, int time)
{
	char 		time_str[20];

	sprintf(time_str, "%d", time);
	ei_frame_set_text(map->time_widget, time_str);
}

bool default_handler(ei_event_t* event)
{
	if ( (event->type == ei_ev_close) ||
	    ((event->type == ei_ev_keydown) && (event->param.key.key_code == SDLK_ESCAPE))) {
		ei_app_quit_request();
		return true;

	} else if (event->type == ei_ev_app) {
		handle_time((map_t*)(event->param.application.user_param));
		hw_event_schedule_app(250, event->param.application.user_param);
		return true;

	} else
		return false;
}

void handle_restart(ei_widget_t widget, ei_event_t* event, ei_user_param_t user_param)
{
	reset_game((map_t*)user_param);
}

void cell_button_handler(ei_widget_t widget, ei_event_t* event, ei_user_param_t user_param)
{
	map_cell_t*		cell = (map_cell_t*)(ei_widget_get_user_data(widget));

	if (cell->map->game_over)
		return;

	if (cell->map->start_time == -1.0)
		cell->map->start_time	= hw_now();

	if (event->param.mouse.button == ei_mouse_button_left) {
		if (can_play(cell)) {
			reveal_and_explore(cell);
			check_victory(cell->map);
			return;
		}

	} else if (event->param.mouse.button == ei_mouse_button_right) {
		if (!cell->is_revealed) {
			switch_flag(cell);
			return;
		}
	}
}

void create_cell_buttons(map_t* map)
{
	int 		x, y;
	map_cell_t*	cell;
	ei_widget_t	button;

	for (y = 0, cell = map->cells; y < map->height; y++) {
		for (x = 0; x < map->width; x++, cell++) {

			button = ei_widget_create	("button", map->toplevel, (void*)cell, NULL);
			cell->widget = button;

			ei_button_configure		(button, &g_button_size,
							    	 &g_bg_color,
							     	 &(int){2},
							      	 &(int){0},
							       	 &(ei_relief_t){ei_relief_raised}, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
								 &(ei_callback_t){cell_button_handler}, NULL);

			ei_place_xy			(button, x * g_button_size.width, k_info_height + y * g_button_size.height);
		}
	}
}

/*
 * Create the game window, 'map' must have been created before (with 'create_mine_map').
 */
void create_game_window(map_t* map)
{
	char 		label_str[80];
	ei_string_t	label_str_ptr	= label_str;
	ei_size_t	size;
	int 		v_margin	= 4;

	ei_size_t	toplevel_size	= ei_size(g_button_size.width * map->width, k_info_height + g_button_size.height * map->height);
	ei_color_t	toplevel_bg	= {0xbb, 0xbb, 0xbb, 0xff};
	ei_axis_set_t	resizable	= ei_axis_none;
	int		border_width;
	int		corner_radius	= 0;

	ei_widget_t 	widget;
	ei_color_t	light		= {0xcc, 0xcc, 0xcc, 0xcc};
	ei_callback_t	callback	= handle_restart;

	char		text[5];

	memset(text, ' ', 4);
	text[4]				= '\0';
	ei_string_t	textptr		= (ei_string_t) &text;

	// toplevel

	map->toplevel			= ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
	border_width			= 0;
	strcpy(label_str, "Minesweeper");
	ei_toplevel_configure(map->toplevel, &toplevel_size, &toplevel_bg, &border_width, &label_str_ptr, NULL, &resizable, NULL);
	ei_place_xy(map->toplevel, 40, 40);

	// restart button

	size				= ei_size(100, k_info_height - 2 * v_margin);
	border_width			= 2;
	widget				= ei_widget_create("button", map->toplevel, NULL, NULL);
	strcpy(label_str, "Restart");
	ei_button_configure(widget, &size, &g_bg_color, &border_width, &corner_radius, NULL, &label_str_ptr, NULL, NULL,
						NULL, NULL, NULL, NULL, &callback, (void*) &map);
	ei_place_anchored_xy(widget, ei_anc_north, toplevel_size.width / 2, v_margin);

	// flag count label

	size.width			= 70;
	int_to_str(map->nb_mines, text, 4);
	widget				= ei_widget_create("frame", map->toplevel, NULL, NULL);
	ei_frame_configure(widget, &size, &light, &border_width, NULL, &textptr, NULL, NULL,
							NULL, NULL, NULL, NULL);
	ei_place_anchored_xy(widget, ei_anc_northwest, 4, v_margin);
	map->flag_count_widget		= widget;

	// timer

	size.width			= 70;
	map->time_widget = ei_widget_create("frame", map->toplevel, NULL, NULL);
	char time_str[20];
	ei_string_t time_str_ptr	= time_str;
	sprintf(time_str, "%d", 0);
	ei_frame_configure(map->time_widget, &size, &light, &border_width, NULL, &time_str_ptr, NULL, NULL,
			NULL, NULL, NULL, NULL);
	ei_place_anchored_xy(map->time_widget, ei_anc_northwest, 80, v_margin);

	// game result text

	size.width			= 90;
	widget				= ei_widget_create("frame", map->toplevel, NULL, NULL);
	ei_frame_configure(widget, &size, &toplevel_bg, &border_width, NULL, NULL, NULL, NULL,
						NULL, NULL, NULL, NULL);
	ei_place_anchored_xy(widget, ei_anc_northeast, toplevel_size.width - 4, 4);
	map->victory_text_widget	= widget;

	create_cell_buttons(map);
}

int get_arg_value(char *arg, int min, int max, int def)
{
	int try = atoi(arg);
	if (try < min || try > max) {
		return def;
	}
	return try;
}



// main -

int main(int argc, char* argv[])
{
	srand((unsigned)time(NULL)); // random seed

	const int	w_min = 12, w_max = 32;
	const int	h_min = 4, h_max = 22;
	int		size_w = 20;
	int		size_h = 16;
	int		nb_mines = 40;

	if (argc == 4) {
		size_w = get_arg_value(argv[1], w_min, w_max, size_w);
		size_h = get_arg_value(argv[2], h_min, h_max, size_h);
		nb_mines = get_arg_value(argv[3], 1, size_w * size_h, nb_mines);
	} else if (argc > 1) {
		printf("Using default values\n");
		printf("Usage: minesweeper WIDTH HEIGHT NB_MINES\n	WIDTH range = (%d, %d) def. %d\n", w_min, w_max, size_w);
		printf("  HEIGHT range = (%d, %d) def. %d\n	 NB_MINES > 0 and < WIDTH * HEIGHT def. %d\n", h_min, h_max, size_h, nb_mines);
	}

	map_t		map;
	ei_size_t	root_window_size		= { 1024, 768 };
	ei_color_t	root_bgcol			= { 0x52, 0x7f, 0xb4, 0xff };
	bool		fullscreen			= false;

	ei_app_create(root_window_size, fullscreen);
	ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	if ((g_flag_img = hw_image_load("misc/flag.png", ei_app_root_surface())) == NULL) {
		printf("ERROR: could not load image \"misc/flag.png\"");
		return 1;
	}
	if ((g_bomb_img = hw_image_load("misc/bomb.png", ei_app_root_surface())) == NULL) {
		printf("ERROR: could not load image \"misc/bomb.png\"");
		return 1;
	}
	g_reset_img = NULL;

	create_mine_map(&map, size_w, size_h, nb_mines);
	create_game_window(&map);

	ei_event_set_default_handle_func(&default_handler);

	hw_event_schedule_app(250, &map);
	ei_app_run();

	destroy_mine_map(&map);

	hw_surface_free(g_flag_img);
	hw_surface_free(g_bomb_img);

	ei_app_free();

	return (EXIT_SUCCESS);
}
