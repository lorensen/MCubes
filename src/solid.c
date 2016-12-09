/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      solid.c

 * version:     1.1 05/17/88 07:28:36

 * facility:	Edge Interpolator for Marching Cubes

 * abstract:
	These routines perform general solid functions
	in the Marching Cubes algorithm.

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: 2/19/86

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)solid.c	1.1";
#endif

/*
 * include files:
 */

#include <stdio.h>
#include "cubes.h"

/*
 * typedefs
 */

/*
 * macros:
 */

/*
 * own storage:
 */

/*
 * external references:
 */

extern SOLID *solids;


/*++
 *
 * routine cubes_new_solid (name, file)

 * functional description:
	creates a new solid

 * formal parameters:
	name - name of solid
	file - file to store output

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

SOLID *cubes_new_solid (name, file)
    char *name;
    char *file;
{
	SOLID	*current;
	SOLID	*solid;

	/*
	 * get memory for solid
	 */

	solid = (SOLID *) malloc (sizeof (SOLID));

	/*
	 * find end of solids list
	 */

	current = solids;
	if (current != (SOLID *) NULL) {

		while (current->solid_next != (SOLID *) NULL) {
			current = current->solid_next;
		}

		/*
		 * insert new solid
		 */
	
		current->solid_next = solid;
	}
	else {
		solids = solid;
	}

	solid->solid_next = (SOLID *) NULL;	
	solid->solid_name = (char *) malloc (strlen(name) + 1);
	strcpy (solid->solid_name, name);

	/*
	 * for now, assume visbile for all solids
	 */

	solid->solid_visible = 1;

	return (solid);
} /* cubes_new_solid */

