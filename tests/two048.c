/*
 *  two048.c
 *  ig
 *
 *  Created by Francois Berard on May 6th, 2015.
 *  Copyright 2011 Ensimag. All rights reserved.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ei_application.h"
#include "ei_widget_configure.h"
#include "ei_widget_attributes.h"
#include "ei_utils.h"
#include "ei_event.h"





#ifndef random
	long	 random(void);
#endif
double randomf()
{
	#ifndef __WIN__
		return (double)(random()) / (double)((1ul << 31) - 1ul);
	#else
		return (double)(rand()) / (double)(32767);
	#endif
}



ei_size_t			g_root_window_size	= { 1024, 768 };


ei_color_t			g_bg_col	=	{ 0xbd, 0xb1, 0xa3, 0xcc };
ei_color_t			g_bd_col	=	{ 0xa9, 0x99, 0x8c, 0xcc };
ei_color_t			g_fg_col	=	{ 0x7f, 0x79, 0x6d, 0xff };
ei_color_t			g_empty_col	=	{ 0xd1, 0xc7, 0xbb, 0xff };

ei_color_t			g_tile_colors[14] = {
							{  0x0,  0x0,  0x0, 0xff },
							{ 0xf1, 0xe8, 0xdb, 0xff },
							{ 0xf0, 0xe3, 0xca, 0xff },
							{ 0xf5, 0xb7, 0x81, 0xff },
							{ 0xf8, 0x9c, 0x6f, 0xff },
							{ 0xf8, 0x87, 0x6e, 0xff },
							{ 0xf7, 0x6d, 0x4f, 0xff },
							{ 0xef, 0xd2, 0x7d, 0xff },
							{ 0xf0, 0xcf, 0x6d, 0xff },
							{ 0xf2, 0xcc, 0x63, 0xff },
							{ 0xf1, 0xc9, 0x58, 0xff },
							{ 0xef, 0xc5, 0x49, 0xff },
							{ 0xef, 0xc5, 0x49, 0xff },
							{ 0xef, 0xc5, 0x49, 0xff }
						};



// struct direction_t --
//
//	Store the scanning parameters for the 4 possible directions.

typedef struct direction_t
{
	int			start;
	int			end;
	int			incr;
	int			t_end;
	int			t_mul;
	int			s_mul;
} direction_t;

void set_direction(direction_t* dir, int start, int end, int incr, int t_end, int t_mul, int s_mul)
{
	dir->start			= start;
	dir->end			= end;
	dir->incr			= incr;
	dir->t_end			= t_end;
	dir->t_mul			= t_mul;
	dir->s_mul			= s_mul;
}

// struct game_t --
//
//	Stores all the parameters of a game.

typedef struct game_t
{
	int				nb_tile_x;
	int				nb_tile_y;
	int				nb_tile;
	int				tile_size;
	int				tile_bd;

	ei_widget_t			toplevel;
	int*				tile_values;
	ei_widget_t*			tile_widgets;
	ei_font_t			tile_font;

	direction_t			dirs[4];

	struct game_t*			next;
} game_t;

void handle_dir_key			(game_t* g, int dir_code, int* can_move);
void destroy_notify			(ei_widget_t widget);

game_t*					g_game_list_head	= NULL;



// create_window --

void create_window(game_t* g)
{
	ei_size_t			toplevel_size;
	static ei_point_t		toplevel_pos		= {20, 20};
	int				toplevel_border		= 0;
	ei_axis_set_t			toplevel_resizable	= ei_axis_none;
	ei_string_t			toplevel_title		= "2048";

	int				x, y;
	ei_widget_t			tile_bg;
	ei_size_t			tile_bg_size		= ei_size(g->tile_size, g->tile_size);
	int				tile_bg_borderwidth	= 0;
	ei_point_t			tile_bg_pos;
	ei_size_t			tile_fullsize		= ei_size(g->tile_size + 2 * g->tile_bd, g->tile_size + 2 * g->tile_bd);


	g->toplevel	= ei_widget_create("toplevel", ei_app_root_widget(), g, destroy_notify);
	toplevel_size	= ei_size(g->nb_tile_x * tile_fullsize.width + 2 * g->tile_bd, g->nb_tile_y * tile_fullsize.height + 2 * g->tile_bd);
	ei_toplevel_configure(g->toplevel, &toplevel_size, &g_bd_col, &toplevel_border, &toplevel_title, NULL, &toplevel_resizable, NULL);
	ei_place_xy(g->toplevel, toplevel_pos.x, toplevel_pos.y);

	tile_bg_pos.y		= 2 * g->tile_bd;
	for (y = 0; y < g->nb_tile_y; y++) {
		tile_bg_pos.x	= 2 * g->tile_bd;
		
		for (x = 0; x < g->nb_tile_x; x++) {
			tile_bg		= ei_widget_create("frame", g->toplevel, NULL, NULL);
			ei_frame_configure(tile_bg, &tile_bg_size, &g_bg_col, &tile_bg_borderwidth, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			ei_place_anchored_xy(tile_bg, ei_anc_northwest, tile_bg_pos.x, tile_bg_pos.y);
			tile_bg_pos.x	+= tile_fullsize.width;
		}
		tile_bg_pos.y		+= tile_fullsize.height;
	}

	// Prepare the position of the next toplevel to be created.

	toplevel_pos			= ei_point_add(toplevel_pos, ei_point(40, 40));
	if (toplevel_pos.y > g_root_window_size.height - 80) {
		toplevel_pos.y		= 20;
		toplevel_pos.x		-= 80;
	}
}



void loose()
{
	printf("Lose !\n");
}


// update_board --

void update_board(game_t* g)
{
	int			x, y;
	ei_point_t		tile_pos;
	ei_widget_t		tile_w;
	ei_widget_t*		tile_w_ptr;
	int*			tile_v_ptr;
	int			tile_value;

	ei_size_t		tile_size	= ei_size(g->tile_size, g->tile_size);
	int			tile_border	= 0;
	ei_relief_t		tile_relief	= ei_relief_none;

	char			label[20];
	ei_string_t		label_ptr	= label;

	// Scan all the board positions top to bottom, left to right.

	tile_w_ptr		= g->tile_widgets;
	tile_v_ptr		= g->tile_values;
	tile_pos.y		= 2 * g->tile_bd;
	for (y = 0; y < g->nb_tile_y; y++) {
		tile_pos.x	= 2 * g->tile_bd;
		
		for (x = 0; x < g->nb_tile_x; x++) {
			tile_w		= *tile_w_ptr;
			tile_value	= *tile_v_ptr;

			if (tile_value == -1) {
				if (tile_w != NULL) {

					// Tile was removed, delete the widget.

					ei_widget_destroy(tile_w);
					*tile_w_ptr	= NULL;
				}
			} else {
				if (tile_w == NULL) {
				
					// The board has a value here, but the widget does not exist: create it.

					tile_w		= ei_widget_create("frame", g->toplevel, NULL, NULL);
					*tile_w_ptr	= tile_w;
					ei_place_anchored_xy(tile_w, ei_anc_northwest, tile_pos.x, tile_pos.y);
				}

				snprintf(label, 20, "%d", 1 << tile_value);
				ei_frame_configure(tile_w, &tile_size, &g_tile_colors[tile_value], &tile_border, &tile_relief,
							&label_ptr, &g->tile_font, NULL, NULL, NULL, NULL, NULL);
			}

			tile_pos.x	+= g->tile_size + 2 * g->tile_bd;

			tile_v_ptr++;
			tile_w_ptr++;
		}
		tile_pos.y		+= g->tile_size + 2 * g->tile_bd;
	}
}

// add_tile --

void add_tile(game_t* g)
{
	int		i;
	int*		curs;
	int		nb_empty;
	int		n_new;
	int		val_new;
	int		can_move	= false;

	// Count empty tiles

	nb_empty	= 0;
	for (curs = g->tile_values, i = 0; i < g->nb_tile; curs++, i++)
		if (*curs == -1)
			nb_empty++;

	// Chose randomly an empty tile.

	n_new		= (int)((double)nb_empty * randomf());

	// Chose the value of the tile, either 2 (2^1) or 4 (2^2), with 0.8 probability for 2.

	if (randomf() > 0.8)
		val_new	= 2;
	else
		val_new	= 1;

	// Set the new tile.

	nb_empty	= 0;
	for (curs = g->tile_values, i = 0; i < g->nb_tile; curs++, i++)
		if (*curs == -1) {
			if (nb_empty == n_new) {
				*curs	= val_new;
				i	= g->nb_tile;
			} else
				nb_empty++;
		}

	update_board(g);

	// Check that at least a move is possible in one of the 4 directions,
	//	otherwise this means that the game is lost.

	for (i = 0; (i < 4) && (can_move == false); i++)
		handle_dir_key(g, i, &can_move);

	if (!can_move)
		loose();
}

// new_game --

game_t* new_game(int nb_tile_x, int nb_tile_y, int tile_size, int tile_bd)
{
	game_t*		g;
	game_t*		curr	= g_game_list_head;
	int		i;
	int*		curs;
	ei_widget_t*	tile_w;

	g			= (game_t*)malloc(sizeof(game_t));
	memset(g, 0, sizeof(game_t));

	g->nb_tile_x		= nb_tile_x;
	g->nb_tile_y		= nb_tile_y;
	g->nb_tile		= g->nb_tile_x * g->nb_tile_y;
	g->tile_size		= tile_size;
	g->tile_bd		= tile_bd;

	g->tile_values	= (int*)malloc(sizeof(int) * (size_t)g->nb_tile);
	g->tile_widgets	= (ei_widget_t*)malloc(sizeof(ei_widget_t) * (size_t)g->nb_tile);

	for (tile_w = g->tile_widgets, curs = g->tile_values, i = 0; i < g->nb_tile; tile_w++, curs++, i++) {
		*curs			= -1;
		*tile_w			= NULL;
	}

	g->tile_font	= hw_text_font_create(ei_default_font_filename, ei_style_normal, g->tile_size / 2);
	g->next		= NULL;

	set_direction(&(g->dirs[0]), 0,			g->nb_tile_y,	1, 	g->nb_tile_x, 1, g->nb_tile_x);			/* up		*/
	set_direction(&(g->dirs[1]), g->nb_tile_y - 1,	-1,		-1, 	g->nb_tile_x, 1, g->nb_tile_x);			/* down 	*/
	set_direction(&(g->dirs[2]), g->nb_tile_x - 1,	-1, 		-1,	g->nb_tile_y, g->nb_tile_x, 1);			/* right	*/
	set_direction(&(g->dirs[3]), 0,			g->nb_tile_x,	1,	g->nb_tile_y, g->nb_tile_x, 1);			/* left		*/

	create_window(g);
	add_tile(g);

	// Add game to the list.

	if (curr != NULL) {
		while (curr->next != NULL)
			curr		= curr->next;
		curr->next		= g;
	} else
		g_game_list_head	= g;

	return g;
}

// destroy_notify --

void destroy_notify	(ei_widget_t		widget)
{
	game_t*		g		= (game_t*)(ei_widget_get_user_data(widget));
	game_t*		curr		= g_game_list_head;
	game_t*		pred		= NULL;

	// Remove game from the list.

	while (curr != g) {
		pred			= curr;
		curr			= curr->next;
	}
	if (pred == NULL)
		g_game_list_head	= curr->next;
	else
		pred->next		= curr->next;

	free((void*)(g->tile_values));
	free((void*)(g->tile_widgets));		// The widget themselves are destroyed as children of the toplevel.
	hw_text_font_free(g->tile_font);
	free((void*)g);
}



// handle_dir_key --
//
//	Handles the pressing of a direction key (up, down, left or right).

void handle_dir_key(game_t* g, int dir_code, int* can_move)
{
	bool				move		= false;
	bool				space_armed;
	int				s, t, q, tile_n;
	direction_t*			dd		= &(g->dirs[dir_code]);
	size_t				nb		= g->nb_tile_x > g->nb_tile_y ? (size_t)g->nb_tile_x : (size_t)g->nb_tile_y;
	int*				val_list	= (int*)malloc((sizeof(int) * nb));
	int				val_list_n;
	int*				new_list	= (int*)malloc((sizeof(int) * nb));
	int				new_list_n;

	// Scan all rows (left-right key) or columns (up-down key),
	//	unless this is only a test if a move is possible. In that case,
	//	stop as soon as a possible move is found.

	for (t = 0; (t < dd->t_end) && ((can_move == NULL) || (move == false)); t++) {
		val_list_n				= 0;
		space_armed				= false;

		// Accumulate all values, check if there is space for the motion of a tile

		for (s = dd->start; s != dd->end; s += dd->incr) {
			tile_n				= t * dd->t_mul + s * dd->s_mul;
			if (g->tile_values[tile_n] != -1) {
				val_list[val_list_n++]	= g->tile_values[tile_n];
				if (space_armed)
					move		= true;
			} else
				space_armed		= true;
		}

		// Compute the new row or column by merging adjacent tiles that have the same value.

		new_list_n					= 0;
		for (q = 0; q < val_list_n; q++)
			if ((q < (val_list_n-1)) && (val_list[q] == val_list[q+1])) {
				new_list[new_list_n++]	= val_list[q]+1;
				q++;
				move			= true;
			} else
				new_list[new_list_n++]	= val_list[q];


		// Update the row or column in the board, but not if we are only testing
		//	for a possible move

		if (can_move == NULL)
			for (s = dd->start, q = 0; s != dd->end; s += dd->incr, q++) {
				tile_n				= t * dd->t_mul + s * dd->s_mul;
				if (q < new_list_n)
					g->tile_values[tile_n]	= new_list[q];
				else
					g->tile_values[tile_n]	= -1;
		}
	}

	free((void*)val_list);
	free((void*)new_list);

	if (can_move != NULL)
		*can_move				= (int)move;
	else {
		// If there was at least one tile motion, create a new tile on the board

		if (move)
			add_tile(g);

		update_board(g);
	}
}



// topmost_game --

game_t* topmost_game()
{
	ei_widget_t		topmost_window	= ei_widget_get_last_child(ei_app_root_widget());
	game_t*			curr		= g_game_list_head;

	if (topmost_window == NULL)
		return NULL;

	while (curr->toplevel != topmost_window)
		curr		= curr->next;

	return curr;
}


// global_event_handler --

bool global_event_handler(ei_event_t* event)
{
	int		dir_code;
	game_t*		tm_game;
	ei_widget_t 	topmost_window;

	uint32_t	cmd_mask = (1 << ei_mod_ctrl_left) | (1 << ei_mod_ctrl_right) |
					(1 << ei_mod_meta_left) | (1 << ei_mod_meta_right);

	if ( (event->type == ei_ev_close) ||
	    ((event->type == ei_ev_keydown) && (event->param.key.key_code == SDLK_ESCAPE))) {
		ei_app_quit_request();
		return true;

	} else if (event->type == ei_ev_keydown) {
		dir_code		= (event->param.key.key_code == SDLK_UP) ? 0 :
					  (event->param.key.key_code == SDLK_DOWN) ? 1 :
					  (event->param.key.key_code == SDLK_RIGHT) ? 2 :
					  (event->param.key.key_code == SDLK_LEFT) ? 3 : -1;
		if (dir_code != -1) {
			tm_game		= topmost_game();
			if (tm_game != NULL) {
				handle_dir_key(tm_game, dir_code, NULL);
				return true;
			}
		}
		
		if (event->param.key.modifier_mask & cmd_mask) {
			if (event->param.key.key_code == SDLK_n) {
				// Create a new game.
				new_game(4, 4, 100, 4);
				return true;
			}
			if (event->param.key.key_code == SDLK_w) {
				// Destroy topmost window.
				if ((topmost_window = ei_widget_get_last_child(ei_app_root_widget())) != NULL)
					ei_widget_destroy(topmost_window);
				return true;
			}
		}
	}

	return false;
}


// main --

int main(int argc, char* argv[])
{
	ei_color_t	root_bgcol			= { 0x52, 0x7f, 0xb4, 0xff };
	bool		fullscreen			= false;

	ei_app_create(g_root_window_size, fullscreen);
	ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	new_game(4, 4, 80, 4);
	//new_game(4, 4, 100, 4);

	ei_event_set_default_handle_func(global_event_handler);

	ei_app_run();
	
	ei_app_free();

	return (EXIT_SUCCESS);
}
