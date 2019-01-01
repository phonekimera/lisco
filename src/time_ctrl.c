/* time_ctrl.c - handle time controls.
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

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "time_ctrl.h"
#include "tate.h"

long int moves_to_tc  = 40;
long int min_per_game = 5;
long int sec_per_game = 0;
long int inc = 0;
long int time_left = 0;
long int opp_time = 30000;
long int time_cushion = 0;

int go_fast = 0;
int fixed_time = 0;

rtime_t start_time;

void
parse_level (level_str)
     const char* level_str;
{
    long int new_moves_to_tc;
    long int new_min_per_game;
    long int new_sec_per_game = 0;
    long int new_inc;
    char* endptr;
    const char* startptr = level_str;

    new_moves_to_tc = strtoul (startptr, &endptr, 10);
    if (new_moves_to_tc == 0 && startptr == endptr) {
	fprintf (stdout, "error (illegal level specification): %s",
		 level_str);
	return;
    }

    startptr = endptr;
    new_min_per_game = strtoul (startptr, &endptr, 10);
    if (new_min_per_game == 0 && startptr == endptr) {
	fprintf (stdout, "error (illegal level specification): %s",
		 level_str);
	return;
    }

    startptr = endptr;
    
    if (*startptr == ':') {
	++startptr;
	new_sec_per_game = strtoul (startptr, &endptr, 10);
	if (new_sec_per_game == 0 && startptr == endptr) {
	    fprintf (stdout, "error (illegal level specification): %s",
		     level_str);
	    return;
	}
	if (new_sec_per_game >= 60) {
	    fprintf (stdout, "error (illegal level specification): %s",
		     level_str);
	    return;
	}
    }

    startptr = endptr;
    new_inc = strtoul (startptr, &endptr, 10);
    if (new_inc == 0 && startptr == endptr) {
	fprintf (stdout, "error (illegal level specification): %s",
		 level_str);
	return;
    }
  
    moves_to_tc = new_moves_to_tc;
    min_per_game = new_min_per_game;
    sec_per_game = new_sec_per_game;
    inc = new_inc;
}

/* The rest of this file is mostly stolen from Sjeng.  */

long int 
allocate_time () 
{
    float allocated_time = 0.0, move_speed = 20.0;
    
    /* Sudden death time allocation.  */
    if (!moves_to_tc) {
	/* Calculate move speed.  The idea is that if we are behind, we move
	   faster, and if we have < 1 min left and a small increment, we REALLY
	   need to start moving fast.  Also, if we aren't in a super fast
	   game, don't worry about being behind on the clock at the beginning,
	   because some players will make instant moves in the opening, and 
	   Sjeng will play poorly if it tries to do the same.  */
	
	/* Check to see if we're behind on time and need to speed up.  */
	if ((min_per_game < 6 && !inc) 
	    || time_left < (((min_per_game * 6000) + (sec_per_game * 100))
			  * 4.0 / 5.0)) 
	{
	    if ((opp_time - time_left) > (opp_time / 5.0) && xboard)
		move_speed = 40.0;
	    else if ((opp_time - time_left) > (opp_time / 10.0) && xboard)
		move_speed = 30.0;
	    else if ((opp_time - time_left) > (opp_time / 20.0) && xboard)
		move_speed = 25.0;
	}
	
	if ((time_left - opp_time) > (time_left / 5.0) && xboard)
	    move_speed -= 10;
	else if ((time_left - opp_time) > (time_left / 10.0) && xboard)
	    move_speed -= 5;
	
	/* Allocate our base time.  */
	allocated_time = time_left / move_speed;
	
	/* Add our increment if applicable.  */
	if (inc) {
	    if (time_left - allocated_time-inc > 500) {
		allocated_time += inc;
	    } else if (time_left - allocated_time - (inc * 2.0 / 3.0) > 100) {
		allocated_time += inc * 2.0 / 3.0;
	    }
	}
    } else {
	/* Conventional clock time allocation.  */
	allocated_time = (((float) min_per_game * 6000.0
			   + (float) sec_per_game * 100.0) / 
			  (float) moves_to_tc) - 100.0;

	/* If we've got extra time, use some of it.  */
	if (time_cushion) {
	    float cushion = time_cushion * 2.1 / 3.0;

	    allocated_time += cushion;
	    time_cushion -= cushion;
	}
    }
    
    return ((long int) allocated_time);
}

#if 0
void 
rdelay (time_in_s)
     int time_in_s;
{
     /* My delay function to cause a delay of time_in_s seconds */
     rtime_t time1, time2;
     long int timer = 0;

     time1 = rtime ();
     while (timer / 100 < time_in_s) {
	 time2 = rtime ();
	 timer = rdifftime (time2, time1);
     }
}
#endif

long int 
rdifftime (end, start)
     rtime_t end; 
     rtime_t start; 
{
    /* Determine the time taken between start and the current time in
       centi-seconds.  */

    /* Using ftime().  */
#if defined(HAVE_SYS_TIMEB_H) && (defined(HAVE_FTIME) || defined(HAVE_GETTIMEOFDAY))
    return ((end.time - start.time) * 100 + 
	    (end.millitm-start.millitm) / 10);

    /* -------------------------------------------------- */

    /* Using time(). */
#else
    return (100 * (long int) difftime (end, start));
#endif
}

rtime_t 
rtime () 
{
  /* Using ftime().  */
#if defined(HAVE_FTIME) && defined(HAVE_SYS_TIMEB_H)
    rtime_t temp;
    ftime (&temp);
    return temp;

    /* -------------------------------------------------- */
    
    /* gettimeofday replacement by Daniel Clausen.  */
#else
# if defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIMEB_H)
    rtime_t temp;
    struct timeval tmp;

    gettimeofday (&tmp, NULL);
    temp.time = tmp.tv_sec;
    temp.millitm = tmp.tv_usec / 1000;
    temp.timezone = 0;
    temp.dstflag = 0;
    
    return temp;

# else
    return (time (0));
# endif  
#endif
}
