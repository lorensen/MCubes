/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      mask.c

 * version:     1.3 03/21/89 07:14:15

 * facility:	Marching Cubes triangle generator for sampled data

 * abstract:
	These routines perform surface specific operations on solid
	masks.

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: 2/19/86

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)mask.c	1.3";
#endif

/*
 * include files:
 */

#include <stdio.h>
#include "cubes.h"

/*
 * typedefs
 */

typedef struct {
	PIXEL	mask_mask;
	int	in;
	int	out;
} MASK;

/*
 * macros:
 */

/*
 * own storage:
 */
LOCAL int cubes_build_mask_index ();

/*
 * external references:
 */

extern SOLID *cubes_new_solid ();

extern int line;
extern int pixel;
extern int slice;
extern int pixels_per_line;


/*++
 *
 * routine cubes_new_mask (name, sense, pixel_mask)

 * functional description:
	creates a new mask surface
 * formal parameters:
	name - name of solid
	sense - inside or outside
	pixel_mask - pixel mask for this solid

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_mask (name, sense, pixel_mask)
    char *name;
    char *sense;
    PIXEL pixel_mask;
{
	SOLID	*solid;
	MASK	*mask;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid (name, "mask");

	/*
	 * add mask specific stuff
	 */

	mask = (MASK *) malloc (sizeof (MASK));

	mask->mask_mask = pixel_mask;

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) mask;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_mask_index;

	/*
	 * set edge interpolation procedures (none)
	 */

	solid->solid_interpolate = NULL;

	/*
	 * no intersection calculation needed
	 */

	solid->solid_intersect = NULL;

	/*
	 * mask is invisible
	 */

	solid->solid_visible = 0;

	/*
	 * set inside / outside
	 */

	mask->in = 255;
	mask->out = 0;
	if (strncmp (sense, "outside", 4) == 0) {
		mask->in = 0;
		mask->out = 255;
	}
}

/*++
 *
 * routine cubes_build_mask_index (mask, slice_1, slice_2)

 * functional description:
	creates an index from 0 - 255 based on the mask value
	at the eight nodes making the cube.  Each node is assigned
	a 0 if does not have the mask bit set
	1 if it is has the mask bit set. The index is formed from
	these numbers using the binary number 87654321.

		 8***************7
		**              **
	       * *             * *
	      *  *            *  *
	     *   *           *   *
	    4***************3    *
	    *    *          *    *
	    *    5***************6
	    *   *           *   *
	    *  *            *  *
	    * *             * *
	    **              **
	    1***************2

 * formal parameters:
	slice_1 - pointer to pixel on slice 1
	slice_2 - pointer to pixel on slice 2

 * implicit inputs:
	pixels_per_line - number of pixels per line

 * implicit outputs:
	none

 * routine value:
 * completion codes:
	none

 * side effects:
	none

 *
 */

LOCAL int cubes_build_mask_index (mask, slice_1)

register MASK *mask;
register PIXEL *slice_1;
{
	if ((unsigned short) *slice_1 & (unsigned short) mask->mask_mask) return (mask->in);
	else return (mask->out);
}

