/* ---------------------------------------------------------------------- *
 * light_industry.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * (c) Corey Keasling, 2004
 * ---------------------------------------------------------------------- */
#include <string.h>
#include "modules.h"
#include "light_industry.h"


static void do_animate(int x, int y)
{
		MP_INFO(x,y).int_7 = real_time + INDUSTRY_L_ANIM_SPEED;
		switch (MP_TYPE(x,y))
		{
		case (CST_INDUSTRY_L_Q1):
			MP_TYPE(x,y) = CST_INDUSTRY_L_Q2;
			break;
		case (CST_INDUSTRY_L_Q2):
			MP_TYPE(x,y) = CST_INDUSTRY_L_Q3;
			break;
		case (CST_INDUSTRY_L_Q3):
			MP_TYPE(x,y) = CST_INDUSTRY_L_Q4;
			break;
		case (CST_INDUSTRY_L_Q4):
			MP_TYPE(x,y) = CST_INDUSTRY_L_Q1;
			break;
		case (CST_INDUSTRY_L_L1):
			MP_TYPE(x,y) = CST_INDUSTRY_L_L2;
			break;
		case (CST_INDUSTRY_L_L2):
			MP_TYPE(x,y) = CST_INDUSTRY_L_L3;
			break;
		case (CST_INDUSTRY_L_L3):
			MP_TYPE(x,y) = CST_INDUSTRY_L_L4;
			break;
		case (CST_INDUSTRY_L_L4):
			MP_TYPE(x,y) = CST_INDUSTRY_L_L1;
			break;
		case (CST_INDUSTRY_L_M1):
			MP_TYPE(x,y) = CST_INDUSTRY_L_M2;
			break;
		case (CST_INDUSTRY_L_M2):
			MP_TYPE(x,y) = CST_INDUSTRY_L_M3;
			break;
		case (CST_INDUSTRY_L_M3):
			MP_TYPE(x,y) = CST_INDUSTRY_L_M4;
			break;
		case (CST_INDUSTRY_L_M4):
			MP_TYPE(x,y) = CST_INDUSTRY_L_M1;
			break;
		case (CST_INDUSTRY_L_H1):
			MP_TYPE(x,y) = CST_INDUSTRY_L_H2;
			break;
		case (CST_INDUSTRY_L_H2):
			MP_TYPE(x,y) = CST_INDUSTRY_L_H3;
			break;
		case (CST_INDUSTRY_L_H3):
			MP_TYPE(x,y) = CST_INDUSTRY_L_H4;
			break;
		case (CST_INDUSTRY_L_H4):
			MP_TYPE(x,y) = CST_INDUSTRY_L_H1;
			break;
		}

		return ;
}

/*
   int_6 is the percent of capacity last month.
   this function chooses the base animation (-group) like *_H, *_M, *_L
   while do_animation() will then cycle truh

*/

static void choose_animation(int x, int y)
{

	MP_INFO(x,y).int_6 = (MP_INFO(x,y).int_1) / (INDUSTRY_L_MAKE_GOODS * 8);

	MP_INFO(x,y).int_1 = 0;
	if (MP_INFO(x,y).int_6 > 80)
	{
		switch (MP_TYPE(x,y))
		{
		case (CST_INDUSTRY_L_H1):
		case (CST_INDUSTRY_L_H2):
		case (CST_INDUSTRY_L_H3):
		case (CST_INDUSTRY_L_H4):
			break;
		default:
			MP_TYPE(x,y) = CST_INDUSTRY_L_H1;
		}
	}
	else if (MP_INFO(x,y).int_6 > 55)
	{
		switch (MP_TYPE(x,y))
		{
		case (CST_INDUSTRY_L_M1):
		case (CST_INDUSTRY_L_M2):
		case (CST_INDUSTRY_L_M3):
		case (CST_INDUSTRY_L_M4):
			break;
		default:
			MP_TYPE(x,y) = CST_INDUSTRY_L_M1;
		}
	}
	else if (MP_INFO(x,y).int_6 > 25)
	{
		switch (MP_TYPE(x,y))
		{
		case (CST_INDUSTRY_L_L1):
		case (CST_INDUSTRY_L_L2):
		case (CST_INDUSTRY_L_L3):
		case (CST_INDUSTRY_L_L4):
			break;
		default:
			MP_TYPE(x,y) = CST_INDUSTRY_L_L1;
		}
	}
	else if (MP_INFO(x,y).int_6 > 0)
	{
		switch (MP_TYPE(x,y))
		{
		case (CST_INDUSTRY_L_Q1):
		case (CST_INDUSTRY_L_Q2):
		case (CST_INDUSTRY_L_Q3):
		case (CST_INDUSTRY_L_Q4):
			break;
		default:
			MP_TYPE(x,y) = CST_INDUSTRY_L_Q1;
		}
	}
	else
		MP_TYPE(x,y) = CST_INDUSTRY_L_C;

	return;
}
/*
  check for available worker
   int_5 is the jobs stored.
 */

static int check_jobs(int x,int y)
{
	/* if we don't have enough jobs we can't do anything */
	if (MP_INFO(x,y).int_5 < MIN_JOBS_AT_INDUSTRY_L)
		return -1;

        /* first get some jobs */
	if (MP_INFO(x,y).int_5 +INDUSTRY_L_GET_JOBS < MAX_JOBS_AT_INDUSTRY_L)
	{
		if (get_jobs (x, y, INDUSTRY_L_GET_JOBS) != 0)
			MP_INFO(x,y).int_5 += INDUSTRY_L_GET_JOBS;
		else if (get_jobs (x, y, INDUSTRY_L_GET_JOBS / 10) != 0)
			MP_INFO(x,y).int_5 += INDUSTRY_L_GET_JOBS / 10;
	}
	return 0;
}

/*
  int_3 is the amount of ore in store.
   int_5 is the jobs in store
*/


static void _check_ore (int x, int y,int x1, int y1)
{

/*
  check for transport a x1,y1
  no transport -> do nothing
 */
	if ( (MP_INFO(x1,y1).flags & FLAG_IS_TRANSPORT)  == 0 )
		return ;
/*
  is is something, do we have worker ?
 */
	if ( MP_INFO(x1,y1).int_5 > 0)
	{
		if (MP_INFO(x1,y1).int_5 >= INDUSTRY_L_GET_ORE)
		{
			MP_INFO(x,y).int_3 += INDUSTRY_L_GET_ORE;
			MP_INFO(x1,y1).int_5 -= INDUSTRY_L_GET_ORE;
		}
		else
		{
			MP_INFO(x,y).int_3 += MP_INFO(x1,y1).int_5;
			MP_INFO(x1,y1).int_5 = 0;
		}
		MP_INFO(x,y).int_5 -= INDUSTRY_L_JOBS_LOAD_ORE;
	}

}

static int check_ore(int x, int y)
{

	/* check for space to store  ore */
	if (MP_INFO(x,y).int_3 + INDUSTRY_L_GET_ORE >  MAX_ORE_AT_INDUSTRY_L)
		return -1;

	_check_ore(x, y,x-1,y);
	_check_ore(x, y,x,y-1);
	return 0;

#if 0
	if ((MP_INFO(x1,y).flags & FLAG_IS_TRANSPORT) != 0
	    && MP_INFO(x1,y).int_5 > 0)
	{
		if (MP_INFO(x1,y).int_5 >= INDUSTRY_L_GET_ORE)
		{
			MP_INFO(x,y).int_3 += INDUSTRY_L_GET_ORE;
			MP_INFO(x1,y).int_5 -= INDUSTRY_L_GET_ORE;
		}
		else
		{
			MP_INFO(x,y).int_3 += MP_INFO(x1,y).int_5;
			MP_INFO(x1,y).int_5 = 0;
		}
		MP_INFO(x,y).int_5 -= INDUSTRY_L_JOBS_LOAD_ORE;
	}

	return 0;
#endif
}
/*
   int_4 is the amount of steel in store.
   int_5 is the jobs stored.
   int_6 is the percent of capacity last month.
*/

static void  _check_steel(int x, int y,int x1,int y1)
{

	if ( (MP_INFO(x1,y1).flags & FLAG_IS_TRANSPORT)  == 0 )
		return ;

	if ( MP_INFO(x1,y1).int_6 > 0)
	{
		if (MP_INFO(x1,y1).int_6 >= INDUSTRY_L_GET_STEEL)
		{
			MP_INFO(x,y).int_4 += INDUSTRY_L_GET_STEEL;
			MP_INFO(x1,y1).int_6 -= INDUSTRY_L_GET_STEEL;
		}
		else
		{
			MP_INFO(x,y).int_4 += MP_INFO(x,y - 1).int_6;
			MP_INFO(x1,y1).int_6 = 0;
		}
		MP_INFO(x,y).int_5 -= INDUSTRY_L_JOBS_LOAD_STEEL;
	}

}
static int  check_steel(int x, int y)
{

	/* check for space to store  steel */
	if (MP_INFO(x,y).int_4  + INDUSTRY_L_GET_STEEL > MAX_STEEL_AT_INDUSTRY_L )
			return -1;

	_check_steel(x, y,x-1,y);
	_check_steel(x, y,x,y-1);
	return 0;

#if 0
	if ((MP_INFO(x,y1).flags & FLAG_IS_TRANSPORT) != 0
	    && MP_INFO(x,y1).int_6 > 0)
	{
		if (MP_INFO(x,y1).int_6 >= INDUSTRY_L_GET_STEEL)
		{
			MP_INFO(x,y).int_4 += INDUSTRY_L_GET_STEEL;
			MP_INFO(x,y1).int_6 -= INDUSTRY_L_GET_STEEL;
		}
		else
		{
			MP_INFO(x,y).int_4 += MP_INFO(x,y - 1).int_6;
			MP_INFO(x,y1).int_6 = 0;
		}
		MP_INFO(x,y).int_5 -= INDUSTRY_L_JOBS_LOAD_STEEL;
	}
	return 0;
#endif
}
/*
check inventory for space
 make some goods from
 int_1 is the goods produced this month so far
 int_2 is the amount of goods in store
 int_3 is the amount of ore in store
 int_5 is the jobs stored
 int_4 is the amount of steel in store

 */
static int make_goods(int x,int y, int *prod)
{
	int goods = *prod;
	int storage;
	int power;

/*
	    required ore
            who many INDUSTRY_L_MAKE_GOODS can i store ?
 */
	    storage=(MAX_GOODS_AT_INDUSTRY_L-MP_INFO(x,y).int_2)/INDUSTRY_L_MAKE_GOODS;

	    if (storage < 1  ||  MP_INFO(x,y).int_3 <  INDUSTRY_L_ORE_USED )
	    {
		    goods = *prod;
		    return -1;
	    }

	    /* storage >= 1 */
	    goods += INDUSTRY_L_MAKE_GOODS;
	    MP_INFO(x,y).int_3 -= INDUSTRY_L_ORE_USED;
	    ore_used += INDUSTRY_L_ORE_USED;
	    MP_INFO(x,y).int_5 -= INDUSTRY_L_JOBS_USED;

	    /* Pollution is now determined by amount of goods made and affected by
	       technology
	       MP_POL(x,y) += INDUSTRY_L_POLLUTION; */

	    /* multiply by 2 if we have steel and space. */
	    if ( storage >= 2  )
		    if (MP_INFO(x,y).int_4 >= INDUSTRY_L_STEEL_USED)
		    {
			    MP_INFO(x,y).int_4 -= INDUSTRY_L_STEEL_USED;
			    goods += goods;
		    }

	    /* multipy by 4 if we can get power. */
	    power=get_power (x, y, goods * 10, 1);

	    if ( storage >= 8  )
		    if (power && MP_INFO(x,y).int_3 >= INDUSTRY_L_ORE_USED)
		    {
			    goods *= 4;
			    MP_INFO(x,y).flags |= FLAG_POWERED;
			    /* and use more ore */
			    MP_INFO(x,y).int_3 -= INDUSTRY_L_ORE_USED;
			    ore_used += INDUSTRY_L_ORE_USED;
		    }

	    if (power==0)
		    MP_INFO(x,y).flags &= (0xffffffff - FLAG_POWERED);

	    MP_INFO(x,y).int_1 += goods;
	    MP_INFO(x,y).int_2 += goods;
	    goods_made += goods;
	    goods = *prod;
	    return 0;
}

/* XXX: it would be nice to convert tech-reduced air pollution into waste
	   to be hauled to a tip or recycled, and reduce that above tl 2000 */

static void  make_polution(int x,int y, int goods)
{
	double tmp_pol;
       tmp_pol = (double)(INDUSTRY_L_POL_PER_GOOD * goods);

	if (tech_level > 1000) {
		double d;
		d = (tech_level - 1000);
		if (d > 1000)
			d = 1000;
		d /= 1000;
		tmp_pol -= (tmp_pol * d);
		if (tmp_pol < 0)
			tmp_pol = 0;
	}

	MP_POL(x,y) += (int)tmp_pol;
}

/*
   we can sell stuff if it is on top/left or top/button
   a road/rail/tack
 */

static void _sell_stuff(int x,int y, int x1, int y1)
{
	if (MP_GROUP(x1,y1) == GROUP_ROAD
	    && (MAX_GOODS_ON_ROAD - MP_INFO(x1,y1).int_4) <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_ROAD -  MP_INFO(x1,y1).int_4);
		MP_INFO(x1,y1).int_4 = MAX_GOODS_ON_ROAD;
	}
	else if (MP_GROUP(x1,y1) == GROUP_RAIL
		 && (MAX_GOODS_ON_RAIL - MP_INFO(x1,y1).int_4) <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_RAIL  -  MP_INFO(x1,y1).int_4);
		MP_INFO(x1,y1).int_4 = MAX_GOODS_ON_RAIL;
	}
	else if (MP_GROUP(x1,y1) == GROUP_TRACK
		 && (MAX_GOODS_ON_TRACK - MP_INFO(x1,y1).int_4)  <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_TRACK - MP_INFO(x1,y1).int_4);
		MP_INFO(x1,y1).int_4 = MAX_GOODS_ON_TRACK;
	}
}


static void sell_stuff(int x,int y)
{
		_sell_stuff(x, y,x-1,y);
		_sell_stuff(x, y,x,y-1);
		return ;
}

#if 0
static void sell_stuff(int x,int y)
{
/*
    (x,y-1)
 */
	int x1, y1;
	y1=y-1;
	if (y<0)
		return ;

	if (MP_GROUP(x,y1) == GROUP_ROAD
	    && (MAX_GOODS_ON_ROAD - MP_INFO(x,y1).int_4) <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_ROAD - MP_INFO(x,y1).int_4);
		MP_INFO(x,y1).int_4 = MAX_GOODS_ON_ROAD;
	}
	else if (MP_GROUP(x,y1) == GROUP_RAIL
		 && (MAX_GOODS_ON_RAIL - MP_INFO(x,y1).int_4)
		 <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_RAIL
				       - MP_INFO(x,y1).int_4);
		MP_INFO(x,y1).int_4 = MAX_GOODS_ON_RAIL;
	}
	else if (MP_GROUP(x,y1) == GROUP_TRACK
		 && (MAX_GOODS_ON_TRACK - MP_INFO(x,y1).int_4)
		 <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_TRACK
				       - MP_INFO(x,y1).int_4);
		MP_INFO(x,y1).int_4 = MAX_GOODS_ON_TRACK;
	}

		x1=x-1;
	if (x<0)
		return;

/*
    (x-1,y)
 */

	if (MP_GROUP(x1,y) == GROUP_ROAD
	    && (MAX_GOODS_ON_ROAD - MP_INFO(x1,y).int_4)
	    <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_ROAD
				       - MP_INFO(x1,y).int_4);
		MP_INFO(x1,y).int_4 = MAX_GOODS_ON_ROAD;
	}
	else if (MP_GROUP(x1,y) == GROUP_RAIL
		 && (MAX_GOODS_ON_RAIL - MP_INFO(x1,y).int_4)
		 <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_RAIL
				       - MP_INFO(x1,y).int_4);
		MP_INFO(x1,y).int_4 = MAX_GOODS_ON_RAIL;
	}
	else if (MP_GROUP(x1,y) == GROUP_TRACK
		 && (MAX_GOODS_ON_TRACK - MP_INFO(x1,y).int_4)
		 <= MP_INFO(x,y).int_2)
	{
		MP_INFO(x,y).int_2 -= (MAX_GOODS_ON_TRACK
				       - MP_INFO(x1,y).int_4);
		MP_INFO(x1,y).int_4 = MAX_GOODS_ON_TRACK;
	}

}
#endif

static char state[]="xxxxx";

void
do_industry_l (int x, int y)
{
	int goods = 0;
//	double tmp_pol;
	/*
	// int_1 is the goods produced this month so far
	// int_2 is the amount of goods in store.
	// int_3 is the amount of ore in store.
	// int_4 is the amount of steel in store.
	// int_5 is the jobs stored.
	// int_6 is the percent of capacity last month.
	// int 7 is the next animation frame time.
	*/

	strcpy(state,"-----");

	/* if we don't have enough jobs we can't do anything */
	if ( check_jobs(x,y) < 0)
		return ;
	state[0]='J';

	/* get some ore */
       /* if available get some more */
	if (check_ore(x,y) == 0 )
		check_ore(x,y);
	state[1]='O';

        /*  get some steel */
       /* if available get some more */
	if (check_steel(x,y) == 0)
		check_steel(x,y);
	state[2]='S';
	/* now make some goods if there is room in inventory*/
	make_goods(x,y,&goods);

	if (goods > 0)
		state[3]='G';
	else
		state[3]='g';
	/* Pollution is based on goods produced and is affected by tech level above
	   1000 (displayed as 100) whereupon it is reduced by one percent per 10
	   tech_level points.
	*/
	make_polution(x,y,goods);

	/* now sell the goods to the road/rail/track */
	sell_stuff(x,y);
	state[4]='+';
	/* now choose a graphic every month */
	if ((total_time % NUMOF_DAYS_IN_MONTH) == NUMOF_DAYS_IN_MONTH - 1)
	{
		//choose_animation(x,y);

		MP_INFO(x,y).int_6 = (MP_INFO(x,y).int_1)
			/ (INDUSTRY_L_MAKE_GOODS * 8);
		MP_INFO(x,y).int_1 = 0;
		if (MP_INFO(x,y).int_6 > 80)
		{
			switch (MP_TYPE(x,y))
			{
			case (CST_INDUSTRY_L_H1):
			case (CST_INDUSTRY_L_H2):
			case (CST_INDUSTRY_L_H3):
			case (CST_INDUSTRY_L_H4):
				break;
			default:
				MP_TYPE(x,y) = CST_INDUSTRY_L_H1;
			}
		}
		else if (MP_INFO(x,y).int_6 > 55)
		{
			switch (MP_TYPE(x,y))
			{
			case (CST_INDUSTRY_L_M1):
			case (CST_INDUSTRY_L_M2):
			case (CST_INDUSTRY_L_M3):
			case (CST_INDUSTRY_L_M4):
				break;
			default:
				MP_TYPE(x,y) = CST_INDUSTRY_L_M1;
			}
		}
		else if (MP_INFO(x,y).int_6 > 25)
		{
			switch (MP_TYPE(x,y))
			{
			case (CST_INDUSTRY_L_L1):
			case (CST_INDUSTRY_L_L2):
			case (CST_INDUSTRY_L_L3):
			case (CST_INDUSTRY_L_L4):
				break;
			default:
				MP_TYPE(x,y) = CST_INDUSTRY_L_L1;
			}
		}
		else if (MP_INFO(x,y).int_6 > 0)
		{
			switch (MP_TYPE(x,y))
			{
			case (CST_INDUSTRY_L_Q1):
			case (CST_INDUSTRY_L_Q2):
			case (CST_INDUSTRY_L_Q3):
			case (CST_INDUSTRY_L_Q4):
				break;
			default:
				MP_TYPE(x,y) = CST_INDUSTRY_L_Q1;
			}
		}
		else
			MP_TYPE(x,y) = CST_INDUSTRY_L_C;
	}
	/* now animate */
	if (real_time >= MP_INFO(x,y).int_7)
	{
		do_animate(x,y);
	}
}

void
mps_light_industry (int x, int y)
{
	int i = 0, tmp;
	char * p;

	mps_store_title(i++,_("Light"));
	mps_store_title(i++,_("Industry"));

	i++;

	p = ((MP_INFO(x,y).flags & FLAG_POWERED) != 0) ? _("YES") : _("NO");
	mps_store_ss(i++,_("Power"),p);

#ifdef DEBUG
	mps_store_title(i++,state);
	mps_store_title(i++,get_powerstate());
#endif

	mps_store_sd(i++,_("Output"),MP_INFO(x,y).int_1);

	mps_store_sfp(i++,_("Store"),
		      MP_INFO(x,y).int_2 * 100.0 / MAX_GOODS_AT_INDUSTRY_L);
	mps_store_sfp(i++,_("Ore"),
		      MP_INFO(x,y).int_3 * 100.0 / MAX_ORE_AT_INDUSTRY_L);
	mps_store_sfp(i++,_("Steel"),
		      MP_INFO(x,y).int_4 * 100.0 / MAX_STEEL_AT_INDUSTRY_L);
	tmp=get_jobs (x, y, INDUSTRY_L_GET_JOBS);
	if (tmp)
		mps_store_ss(i++,_("no Jobs"),"");

	mps_store_sfp(i++,_("Jobs:"), MP_INFO(x,y).int_5*100.0/MAX_JOBS_AT_INDUSTRY_L);

	mps_store_sfp(i++,_("Capacity"), MP_INFO(x,y).int_6);
}
