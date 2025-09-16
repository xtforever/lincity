/* ---------------------------------------------------------------------- *
 * simulate.h
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * ---------------------------------------------------------------------- */
#ifndef __simulate_h__
#define __simulate_h__

void clear_mappoint (short fill, int x, int y);  // used by engine.c
void new_city (int* originx, int* originy, int random_village);
void count_all_groups (int* group_count); //used by  main.c
void do_time_step (void); // used by main.c

#endif	/* __simulate_h__ */
