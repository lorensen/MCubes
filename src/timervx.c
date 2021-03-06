/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      timer.c

 * version:     1.1 5/28/85 12:30:37

 * facility:
		CPU time measurement

 * abstract:
	Unix run time routines are used to maintain and extract cpu
	time utilization

 * environment:
	Berkeley 4.2 Unix

 * author: W. Lorensen, creation date: 3/26/85

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)timer.c	1.1";
#endif

/*
 * include files:
 */

#include <sys/time.h>
#include <sys/resource.h>

/*
 * macros:
 */

/*
 * own storage:
 */

static  float   cpu_accumulated = 0.0;

/*
 * external references:
 */


/*++
 *
 * routine timer_init ()

 * functional description:
	initializes timer package

 * formal parameters:
	none

 * implicit inputs:
	none

 * implicit outputs:

	cpu_accumulated - accumulated cpu time since call to initialize

 * routine value:

 * completion codes:
	none

 * side effects:
	none

 *
 */

timer_init ()

{
	/*
	 * set cpu time to zero
	 */

	cpu_accumulated = 0.0;

	return (1);
}

/*++
 *
 * routine timer_get_cpu (cpu_since_last)

 * functional description:

	calculates cpu time since last call to get it

 * formal parameters:
	output
		cpu_since_last - cpu time since last call to get it
				 in floating point seconds

 * implicit inputs:
	cpu_accumulated - accumulated cpu time since call to initialize

 * implicit outputs:
	cpu_accumulated - accumulated cpu time since call to initialize

 * routine value:

 * completion codes:
	none

 * side effects:
	none

 *
 */

timer_get_cpu (cpu_since_last)

    float *cpu_since_last;
{
	struct rusage time;
	struct timeval user_time;
	float   cpu_current;
	
	/*
	 * get cpu time in 1/60 of a second
	 */

	getrusage (RUSAGE_SELF, &time);
	cpu_current = (float) time.ru_utime.tv_sec + (float) time.ru_utime.tv_usec / 1000000.0;
	/*
	 * calculate time since last call
	 */

	*cpu_since_last = cpu_current - cpu_accumulated;

	/*
	 * update accumulated cpu time
	 */

	cpu_accumulated = cpu_current;
}
