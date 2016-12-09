/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      gradient.c

 * version:     1.1 12/13/88 15:04:23

 * facility:	Marching Cubes triangle generator for sampled data

 * abstract:
	These routines perform surface specific operations on surfaces
	defined as gradients

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: 12/10/88

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)gradient.c	1.1";
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
	int	low;
	int	high;
} GRADIENT;

/*
 * macros:
 */

/*
 * own storage:
 */

LOCAL int cubes_build_gradient_index ();

/*
 * external references:
 */

extern SOLID *cubes_new_solid ();

extern FILE *vertex_file;
extern VERTEX *vertex;
extern float   scale_x;
extern float   scale_y;
extern float   scale_z;
extern float   aspect_xy_to_z;
extern int line;
extern int pixel;
extern int slice;
extern int pixels_per_line;


/*++
 *
 * routine cubes_new_gradient (name, sense, pixel_gradient)

 * functional description:
	creates a new gradient surface
 * formal parameters:
	name - name of solid
	sense - inside or outside
	pixel_gradient - pixel gradient for this solid

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_gradient (name, sense, low, high)
    char *name;
    char *sense;
    int	low;
    int high;
{
	SOLID	*solid;
	GRADIENT *gradient;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid (name, "gradient");

	/*
	 * add gradient specific stuff
	 */

	gradient = (GRADIENT *) malloc (sizeof (GRADIENT));

	gradient->low = low;
	gradient->high = high;

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) gradient;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_gradient_index;

	/*
	 * set edge interpolation procedures (none)
	 */

	solid->solid_interpolate = NULL;

	/*
	 * no intersection calcultaion needed (none)
	 */

	solid->solid_intersect = NULL;

	/*
	 * gradient is invisible
	 */

	solid->solid_visible = 0;

	/*
	 * set inside / outside
	 */

	gradient->in = 255;
	gradient->out = 0;
	if (strncmp (sense, "outside", 4) == 0) {
		gradient->in = 0;
		gradient->out = 255;
	}
}


/*++
 *
 * routine cubes_build_gradient_index (gradient, slice_1, slice_2)

 * functional description:
	creates an index from 0 - 255 based on the gradient value
	at the eight nodes making the cube.  Each node is assigned
	a 0 if does not have the gradient bit set
	1 if it is has the gradient bit set. The index is formed from
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

LOCAL int cubes_build_gradient_index (gradient, slice_1, slice_2, slice_3)

GRADIENT *gradient;
register PIXEL *slice_1;
register PIXEL *slice_2;
register PIXEL *slice_3;
{
register int gradient_index;
	int next_pixel = 1;
	int next_line = pixels_per_line;
	PIXEL	p1, p2, p3;
	int	gradient;
	int	gx, gy, gz;

	/*
	 * calculate components of gradient
	 */

	gx = (*(slice_1 + 1) & data_mask) - (*(slice_1 - 1) & data_mask);
	gy = (*(slice_1 + pixels_per_line) & data_mask) - (*(slice_1 - pixels_per_line) & data_mask);
	gz = ((*(slice_3) & data_mask) - (*(slice_1) & data_mask)) / aspect_xy_to_z;;

	/*
	 * calculate square of gradient
	 */

	gradient = gx * gx + gy * gy + gz * gz;

	/*
	 * return inside if within gradient range
	 */

	if (gradient >= low && gradient <= high) gardient_index = gradient->in;
	else gardient_index = gradient->out;

	return (gradient_index);
}

