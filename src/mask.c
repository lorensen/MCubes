/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      mask.c

 * version:     1.1 05/17/88 07:28:05

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
static char    *sccs_id = "@(#)mask.c	1.1";
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
} MASK;

/*
 * macros:
 */

/*
 * own storage:
 */

LOCAL VERTEX interpolate_mask_1 ();
LOCAL VERTEX interpolate_mask_2 ();
LOCAL VERTEX interpolate_mask_3 ();
LOCAL VERTEX interpolate_mask_4 ();
LOCAL VERTEX interpolate_mask_5 ();
LOCAL VERTEX interpolate_mask_6 ();
LOCAL VERTEX interpolate_mask_7 ();
LOCAL VERTEX interpolate_mask_8 ();
LOCAL VERTEX interpolate_mask_9 ();
LOCAL VERTEX interpolate_mask_10 ();
LOCAL VERTEX interpolate_mask_11 ();
LOCAL VERTEX interpolate_mask_12 ();

LOCAL int cubes_build_mask_index ();
static EDGE_PROCEDURE interpolate_mask_edges[13] = {
	0,
	interpolate_mask_1,
	interpolate_mask_2,
	interpolate_mask_3,
	interpolate_mask_4,
	interpolate_mask_5,
	interpolate_mask_6,
	interpolate_mask_7,
	interpolate_mask_8,
	interpolate_mask_9,
	interpolate_mask_10,
	interpolate_mask_11,
	interpolate_mask_12
};


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
 * routine cubes_new_mask (name, file, pixel_mask)

 * functional description:
	creates a new mask surface
 * formal parameters:
	name - name of solid
	file - file to store output
	pixel_mask - pixel mask for this solid

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_mask (name, file, pixel_mask)
    char *name;
    char *file;
    PIXEL pixel_mask;
{
	SOLID	*solid;
	MASK	*mask;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid (name, file);

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
	 * no intersection calcultaion needed
	 */

	solid->solid_intersect = NULL;

	/*
	 * mask is invisible
	 */

	solid->solid_visible = 0;

}


/*++
 *
 * routine interpolate_mask_x (slice_0, slice_1, slice_2, slice_3)

 * functional description:
	interpolates a vertex of a polygon on the edge of a cube

			7
			|
		 8***************7
		**              **
	   11- * *             * *
	      *  *   3    12- *  *
	     *   *-8 |       *   * -6
	    4***************3    *
	    *    *          *    *
	    *    5***************6
	 4- *   *       |   * -2 *
	    *  * -9     5   *  *
	    * *             * * -10
	    **              **
	    1***************2
		    |
		    1

 * formal parameters:
	mask - pointer to a mask structure
	slice_0 - pixel 1 on the i - 1 slice
	slice_1 - pixel 1 on the i slice
	slice_2 - pixel 2 on the i + 1 slice
	slice_3 - pixel 1 on the i + 2 slice

 * implicit inputs:
	line  - current line number
	pixel - current pixel number
	slice - current slice number
	value - surface value

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

LOCAL VERTEX interpolate_mask_1 (mask, slice_0, slice_1, slice_2)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	/*
	 * find vertex coordinate
	 */

	vertex->x = pixel + .5;
	vertex->y = line;
	vertex->z = slice;

	/*
	 * normal is just normal of the mask
	 */

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	/*
	 * return vertex
	 */

	return (*vertex);

} /* interpolate_mask_1 */
	
LOCAL VERTEX interpolate_mask_2 (mask, slice_0, slice_1, slice_2)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	vertex->x = pixel + 1;
	vertex->y = line + .5;
	vertex->z = slice;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_2 */

LOCAL VERTEX interpolate_mask_3 (mask, slice_0, slice_1, slice_2)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	vertex->x = pixel + .5;
	vertex->y = line + 1;
	vertex->z = slice;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_3 */
	
LOCAL VERTEX interpolate_mask_4 (mask, slice_0, slice_1, slice_2)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	vertex->x = pixel;
	vertex->y = line + .5;
	vertex->z = slice;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_4 */
	
LOCAL VERTEX interpolate_mask_5 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel + .5;
	vertex->y = line;
	vertex->z = slice + 1;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_5 */
	
LOCAL VERTEX interpolate_mask_6 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel + 1;
	vertex->y = line + .5;
	vertex->z = slice + 1;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_6 */
	
LOCAL VERTEX interpolate_mask_7 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel + .5;
	vertex->y = line + 1;
	vertex->z = slice;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_7 */
	
LOCAL VERTEX interpolate_mask_8 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel;
	vertex->y = line + .5;
	vertex->z = slice;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_8 */
	
LOCAL VERTEX interpolate_mask_9 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel;
	vertex->y = line;
	vertex->z = slice + .5;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_9 */
	
LOCAL VERTEX interpolate_mask_10 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel + 1;
	vertex->y = line;
	vertex->z = slice + .5;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_10 */
	
LOCAL VERTEX interpolate_mask_11 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel;
	vertex->y = line + 1;
	vertex->z = slice + .5;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_11 */

LOCAL VERTEX interpolate_mask_12 (mask, slice_0, slice_1, slice_2, slice_3)
    MASK *mask;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	vertex->x = pixel + 1;
	vertex->y = line + 1;
	vertex->z = slice + .5;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	return (*vertex);

} /* interpolate_mask_12 */
	

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

LOCAL int cubes_build_mask_index (mask, slice_1, slice_2)

MASK *mask;
register PIXEL *slice_1;
register PIXEL *slice_2;
{
register int mask_index;
	int next_pixel = 1;
	int next_line = pixels_per_line;
	PIXEL	p1, p2, p3, p4, p5, p6, p7, p8;
	PIXEL pixel_mask;

	/*
	 * set local variables to corners of cubes
	 */

/*
	p8 = *(slice_2 + next_line);
	p7 = *(slice_2 + next_line + next_pixel);
	p6 = *(slice_2 + next_pixel);
	p5 = *slice_2 ;
	p4 = *(slice_1 + next_line);
	p3 = *(slice_1 + next_line + next_pixel);
	p2 = *(slice_1 + next_pixel);
*/
	p1 = *slice_1 ;

	pixel_mask = mask->mask_mask;
	mask_index = 0;
/*
	if (p8 & pixel_mask) mask_index += 128;
	if (p7 & pixel_mask) mask_index += 64;
	if (p6 & pixel_mask) mask_index += 32;
	if (p5 & pixel_mask) mask_index += 16;
	if (p4 & pixel_mask) mask_index += 8;
	if (p3 & pixel_mask) mask_index += 4;
	if (p2 & pixel_mask) mask_index += 2;
	if (p1 & pixel_mask) mask_index += 1;
*/
	if (p1 & pixel_mask) mask_index = 255;

	return (mask_index);
}

