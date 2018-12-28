/* chitest.c - Functions for board manipulation.
 * Copyright (C) 2002 Guido Flohr (guido@imperia.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <system.h>

#include <libchi.h>

#include <stdio.h>
#include <sys/types.h>

#include <string.h>

#include <time.h>
#include <sys/times.h>

int
main (int argc, char* argv[])
{
    chi_pos my_pos;
    chi_pos* pos = &my_pos;
    char* buf = NULL;
    size_t bufsize;
    int file, rank;
    chi_move move;
    chi_move move_stack[CHI_MAX_MOVES];
    chi_move* move_stack_end = move_stack;
    chi_move* move_ptr;

    move.packed = 0;

    printf ("libchi test\n");
    printf ("===========\n");

    chi_init_position (pos);

    move_stack_end = chi_generate_captures (pos, move_stack);
    move_stack_end = chi_generate_non_captures (pos, move_stack_end);
    for (move_ptr = move_stack; move_ptr < move_stack_end; ++move_ptr) {
	int errnum;

	printf ("Reply: ");
	printf ("(%02u->%02u) ", chi_from (*move_ptr), chi_to (*move_ptr));
	errnum = chi_print_move (pos, *move_ptr, &buf, &bufsize, 0);
	puts (buf);
    }
    chi_from (move) = chi_coords2shift (CHI_FILE_G, CHI_RANK_1);
    chi_to (move) = chi_coords2shift (CHI_FILE_F, CHI_RANK_3);
    chi_print_move (pos, move, &buf, &bufsize, 0);
    printf ("My move: ");
    puts (buf);
    chi_make_move (pos, move);

    move_stack_end = chi_generate_captures (pos, move_stack);
    move_stack_end = chi_generate_non_captures (pos, move_stack_end);
    for (move_ptr = move_stack; move_ptr < move_stack_end; ++move_ptr) {
	int errnum;

	printf ("Reply: ");
	printf ("(%02u->%02u) ", chi_from (*move_ptr), chi_to (*move_ptr));
	errnum = chi_print_move (pos, *move_ptr, &buf, &bufsize, 0);
	puts (buf);
    }
    chi_from (move) = chi_coords2shift (CHI_FILE_E, CHI_RANK_7);
    chi_to (move) = chi_coords2shift (CHI_FILE_E, CHI_RANK_5);
    chi_print_move (pos, move, &buf, &bufsize, 0);
    printf ("My move: ");
    puts (buf);
    chi_make_move (pos, move);

    move_stack_end = chi_generate_captures (pos, move_stack);
    move_stack_end = chi_generate_non_captures (pos, move_stack_end);
    for (move_ptr = move_stack; move_ptr < move_stack_end; ++move_ptr) {
	int errnum;

	printf ("Reply: ");
	printf ("(%02u->%02u) ", chi_from (*move_ptr), chi_to (*move_ptr));
	errnum = chi_print_move (pos, *move_ptr, &buf, &bufsize, 0);
	puts (buf);
    }
    chi_from (move) = chi_coords2shift (CHI_FILE_D, CHI_RANK_2);
    chi_to (move) = chi_coords2shift (CHI_FILE_D, CHI_RANK_4);
    chi_print_move (pos, move, &buf, &bufsize, 0);
    printf ("My move: ");
    puts (buf);
    chi_make_move (pos, move);
    
    move_stack_end = chi_generate_captures (pos, move_stack);
    move_stack_end = chi_generate_non_captures (pos, move_stack_end);
    for (move_ptr = move_stack; move_ptr < move_stack_end; ++move_ptr) {
	int errnum;

	printf ("Reply: ");
	printf ("(%02u->%02u) ", chi_from (*move_ptr), chi_to (*move_ptr));
	errnum = chi_print_move (pos, *move_ptr, &buf, &bufsize, 0);
	puts (buf);
    }
    chi_from (move) = chi_coords2shift (CHI_FILE_G, CHI_RANK_7);
    chi_to (move) = chi_coords2shift (CHI_FILE_G, CHI_RANK_5);
    chi_print_move (pos, move, &buf, &bufsize, 0);
    printf ("My move: ");
    puts (buf);
    chi_make_move (pos, move);
    
    chi_dump_pieces (pos, &buf, &bufsize);

    printf ("    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	printf (" %c  ", chi_file2char(file));
    }
    printf ("\n   +---+---+---+---+---+---+---+---+\n");

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	printf (" %c ", chi_rank2char (rank));
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int shift = chi_coords2shift (file, rank);
	    printf ("| %c ", buf[shift]);
	}
	printf ("|\n   +---+---+---+---+---+---+---+---+\n");
    }

    printf ("    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	printf (" %c  ", chi_file2char(file));
    }
    printf ("\n");

    if (chi_ep (pos)) {
	printf ("En passant possible on file %c.\n", 
		chi_ep_file (pos) + 'a');
    } else {
	printf ("En passant not possible.\n");
    }

    printf ("White king castle: %s.\n", chi_wk_castle (pos) ? "yes" : "no");
    printf ("White queen castle: %s.\n", chi_wq_castle (pos) ? "yes" : "no");
    printf ("Black king castle: %s.\n", chi_bk_castle (pos) ? "yes" : "no");
    printf ("Black queen castle: %s.\n", chi_bq_castle (pos) ? "yes" : "no");
    printf ("Half move clock (50 moves): %d.\n", pos->half_move_clock);
    printf ("Half moves: %d.\n", pos->half_moves);
    printf ("Next move: %s.\n", chi_to_move (pos) ? "black" : "white");

#define MAX_POS 5000000
 {
     struct tms start, end;
     clock_t s, e;
     double sys_time_used;
     double user_time_used;
     int i;

     s = times (&start);
     for (i = 0; i < MAX_POS; ++i) {
	 move_stack_end = chi_generate_non_captures (pos, move_stack);
     }
     e = times (&end);

     user_time_used = ((double) (end.tms_utime - start.tms_utime)) /
	 100;
     sys_time_used = ((double) (end.tms_stime - start.tms_stime)) /
	 100;

     printf ("findbit: %d positions: %G s (user), %G (sys)\n",
	     MAX_POS, user_time_used, sys_time_used);

 }

    return 0;
}
