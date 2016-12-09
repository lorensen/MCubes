/*
 * copyright (c) 1987 general electric company
 */

/*++

 * module:      octant.c

 * version:     1.2 09/01/88 14:13:07

 * facility:	Edge Interpolator for Marching Cubes

 * abstract:
	These routines perform vertex interpolation and recovery
	for octants in the Marching Cubes algorithm.

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: 5/6/87

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)octant.c	1.2";
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
	int	octant_number;
	float	center_x;
	float	center_y;
	float	center_z;
	int	(*octant_find_normal) ();
	int	reformat_min;
	int	reformat_max;
	int	reformat_window;
	int	inside;
	int	outside;
} OCTANT;

/*
 * macros:
 */

/*
 * own storage:
 */

static PIXEL *p1, *p2;
static float t, one_minus_t;
static int offset_1, offset_2;
static VERTEX v1, v2;

LOCAL VERTEX interpolate_octant_1 ();
LOCAL VERTEX interpolate_octant_2 ();
LOCAL VERTEX interpolate_octant_3 ();
LOCAL VERTEX interpolate_octant_4 ();
LOCAL VERTEX interpolate_octant_5 ();
LOCAL VERTEX interpolate_octant_6 ();
LOCAL VERTEX interpolate_octant_7 ();
LOCAL VERTEX interpolate_octant_8 ();
LOCAL VERTEX interpolate_octant_9 ();
LOCAL VERTEX interpolate_octant_10 ();
LOCAL VERTEX interpolate_octant_11 ();
LOCAL VERTEX interpolate_octant_12 ();

LOCAL int cubes_build_octant_index ();
LOCAL int cubes_octant_inside_1 ();
LOCAL int cubes_octant_inside_2 ();
LOCAL int cubes_octant_inside_3 ();
LOCAL int cubes_octant_inside_4 ();
LOCAL int cubes_octant_inside_5 ();
LOCAL int cubes_octant_inside_6 ();
LOCAL int cubes_octant_inside_7 ();
LOCAL int cubes_octant_inside_8 ();
LOCAL int cubes_octant_normal ();
LOCAL int cubes_octant_reformat ();
LOCAL VERTEX *cubes_octant_intersect ();
typedef int (*PROCEDURE)();
static PROCEDURE cubes_octant_insides[9] = {
	0,
	cubes_octant_inside_1,
	cubes_octant_inside_2,
	cubes_octant_inside_3,
	cubes_octant_inside_4,
	cubes_octant_inside_5,
	cubes_octant_inside_6,
	cubes_octant_inside_7,
	cubes_octant_inside_8
};

static EDGE_PROCEDURE interpolate_octant_edges[13] = {
	0,
	interpolate_octant_1,
	interpolate_octant_2,
	interpolate_octant_3,
	interpolate_octant_4,
	interpolate_octant_5,
	interpolate_octant_6,
	interpolate_octant_7,
	interpolate_octant_8,
	interpolate_octant_9,
	interpolate_octant_10,
	interpolate_octant_11,
	interpolate_octant_12
};


/*
 * external references:
 */

extern SOLID *cubes_new_solid ();
extern double sqrt ();
extern float scale_x;
extern float scale_y;
extern float scale_z;
extern PIXEL data_mask;
extern VERTEX *vertex;
extern int line;
extern int pixel;
extern int slice;
extern int pixels_per_line;


/*++
 *
 * routine cubes_new_octant (name, file, number, x, y, z, min, max)

 * functional description:
	creates a new octant surface

 * formal parameters:
	name - name of solid
	file - file to store output
	x, y, z - center of octant
	radius - radius of octant

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_octant (stype, sense, number, x, y, z, min, max)
    char *stype;
    char *sense;
    int number;
    float x, y, z;
    int	min, max;
{
	SOLID	*solid;
	OCTANT	*octant;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid ("octant", "");

	/*
	 * add octant specific stuff
	 */

	octant = (OCTANT *) malloc (sizeof (OCTANT));
	octant->octant_number = number;
	octant->center_x = x;
	octant->center_y = y;
	octant->center_z = z;

	/*
	 * save range and calculate window
	 */

	octant->reformat_min = min;
	octant->reformat_max = max;
	octant->reformat_window = max - min;

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) octant;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_octant_index;

	/*
	 * set edge interpolation procedures
	 */

	solid->solid_interpolate = interpolate_octant_edges;

	/*
	 * set inside procedure
	 */

	switch (octant->octant_number) {
	    case 1:
		solid->solid_inside = cubes_octant_inside_1;
		break;
	    case 2:
		solid->solid_inside = cubes_octant_inside_2;
		break;
	    case 3:
		solid->solid_inside = cubes_octant_inside_3;
		break;
	    case 4:
		solid->solid_inside = cubes_octant_inside_4;
		break;
	    case 5:
		solid->solid_inside = cubes_octant_inside_5;
		break;
	    case 6:
		solid->solid_inside = cubes_octant_inside_6;
		break;
	    case 7:
		solid->solid_inside = cubes_octant_inside_7;
		break;
	    case 8:
		solid->solid_inside = cubes_octant_inside_8;
		break;
	 }

	/*
	 * set surface intersection procedure
	 */

	solid->solid_intersect = cubes_octant_intersect;

	/*
	 * set normal calculation procedure, depending on type
	 */

	if (strncmp(stype, "open", 4) == 0) {
		octant->octant_find_normal = NULL;
		solid->solid_visible = 0;
	}
	else if (strncmp (stype, "cap", 3) == 0) {
		octant->octant_find_normal = NULL;
	}
	else if (strncmp (stype, "reformat", 4) == 0) {		
		octant->octant_find_normal = cubes_octant_reformat;
	}
	else {
		octant->octant_find_normal = cubes_octant_reformat;
	}

	/*
	 * set inside / outside meaning, depending on sense
	 */

	if (strncmp (sense, "inside", 4) == 0) {
		octant->inside = 0;
		octant->outside = 1;
	}
	else {
		octant->inside = 1;
		octant->outside = 0;
	}

} /* cubes_new_octant */


/*++
 *
 * routine interpolate_octant_x (octant, slice_0, slice_1, slice_2, slice_3)

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
	octant - pointer to a octant structure
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

LOCAL VERTEX interpolate_octant_1 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	/*
	 * find vertex coordinate
	 */

	v1.x = pixel;
	v1.y = line;
	v1.z = slice;
	v2 = v1;
	v2.x += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	/*
	 * return vertex
	 */

	return (*vertex);

} /* interpolate_octant_1 */
	
LOCAL VERTEX interpolate_octant_2 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 1;
	offset_2 = pixels_per_line + 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	v1.x = pixel + 1;
	v1.y = line;
	v1.z = slice;
	v2 = v1;
	v2.y += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_2 */

LOCAL VERTEX interpolate_octant_3 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line + 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	v1.x = pixel;
	v1.y = line + 1;
	v1.z = slice;
	v2 = v1;
	v2.x += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_3 */
	
LOCAL VERTEX interpolate_octant_4 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = pixels_per_line;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	v1.x = pixel;
	v1.y = line;
	v1.z = slice;
	v2 = v1;
	v2.y += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_4 */
	
LOCAL VERTEX interpolate_octant_5 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel;
	v1.y = line;
	v1.z = slice + 1;
	v2 = v1;
	v2.x += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_5 */
	
LOCAL VERTEX interpolate_octant_6 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 1;
	offset_2 = pixels_per_line + 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel + 1;
	v1.y = line;
	v1.z = slice + 1;
	v2 = v1;
	v2.y += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_6 */
	
LOCAL VERTEX interpolate_octant_7 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line + 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel;
	v1.y = line + 1;
	v1.z = slice + 1;
	v2 = v1;
	v2.x += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_7 */
	
LOCAL VERTEX interpolate_octant_8 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = pixels_per_line;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel;
	v1.y = line;
	v1.z = slice + 1;
	v2 = v1;
	v2.y += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_8 */
	
LOCAL VERTEX interpolate_octant_9 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	p1 = slice_1;
	p2 = slice_2;

	v1.x = pixel;
	v1.y = line;
	v1.z = slice;
	v2 = v1;
	v2.z += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_9 */
	
LOCAL VERTEX interpolate_octant_10 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 1;
	offset_2 = 1;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel + 1;
	v1.y = line;
	v1.z = slice;
	v2 = v1;
	v2.z += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_10 */
	
LOCAL VERTEX interpolate_octant_11 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel;
	v1.y = line + 1;
	v1.z = slice;
	v2 = v1;
	v2.z += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_11 */

LOCAL VERTEX interpolate_octant_12 (octant, slice_0, slice_1, slice_2, slice_3)
    OCTANT *octant;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line + 1;
	offset_2 = offset_1;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	v1.x = pixel + 1;
	v1.y = line + 1;
	v1.z = slice;
	v2 = v1;
	v2.z += 1.0;

	vertex = cubes_octant_intersect (octant, &v1, &v2);

	return (*vertex);

} /* interpolate_octant_12 */
	

/*++
 *
 * routine cubes_build_octant_index (octant, slice_1, slice_2)

 * functional description:
	creates an index from 0 - 255 based on the value of a octant
	equation at the eight nodes making the cube.  Each node is assigned
	a 0 if it is outside the octant region
	1 if it is inside. The index is formed from
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
	octant - pointer to a octant definition
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

LOCAL int cubes_build_octant_index (octant, slice_1, slice_2)

OCTANT *octant;
register PIXEL *slice_1;
register PIXEL *slice_2;
{
register int octant_index = 0;

	PROCEDURE cubes_octant_inside;

	cubes_octant_inside = *(cubes_octant_insides + octant->octant_number);

	/*
	 * calculate f(x,y,z) at corners of cube
	 */

	vertex->x = pixel;
	vertex->y = line;
	vertex->z = slice;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 1;
	vertex->x += 1.0;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 2;
	vertex->y += 1.0;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 4;
	vertex->x = pixel;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 8;
	vertex->y = line;
	vertex->z = slice + 1;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 16;
	vertex->x += 1.0;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 32;
	vertex->y += 1.0;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 64;
	vertex->x = pixel;
	if ((*cubes_octant_inside) (octant, vertex)) octant_index += 128;

	return (octant_index);
} /* cubes_build_octant_index */

LOCAL int cubes_octant_inside_1 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x <= octant->center_x &&
	    vertex->y <= octant->center_y &&
	    vertex->z <= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_1 */

LOCAL int cubes_octant_inside_2 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x >= octant->center_x &&
	    vertex->y <= octant->center_y &&
	    vertex->z <= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_2 */

LOCAL int cubes_octant_inside_3 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x <= octant->center_x &&
	    vertex->y >= octant->center_y &&
	    vertex->z <= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_3 */

LOCAL int cubes_octant_inside_4 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x >= octant->center_x &&
	    vertex->y >= octant->center_y &&
	    vertex->z <= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_4 */

LOCAL int cubes_octant_inside_5 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x <= octant->center_x &&
	    vertex->y <= octant->center_y &&
	    vertex->z >= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_5 */

LOCAL int cubes_octant_inside_6 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x >= octant->center_x &&
	    vertex->y <= octant->center_y &&
	    vertex->z >= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_6 */

LOCAL int cubes_octant_inside_7 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x <= octant->center_x &&
	    vertex->y >= octant->center_y &&
	    vertex->z >= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_7 */

LOCAL int cubes_octant_inside_8 (octant, vertex)
    OCTANT *octant;
    VERTEX *vertex;
{
	if (vertex->x >= octant->center_x &&
	    vertex->y >= octant->center_y &&
	    vertex->z >= octant->center_z) return (octant->outside);

	return (octant->inside);
} /* cubes_octant_inside_8 */

LOCAL VERTEX *cubes_octant_intersect (octant, vertex_1, vertex_2)
    VERTEX *vertex_1;
    VERTEX *vertex_2;
    OCTANT  *octant;
{
	float	dx, dy, dz;

	/*
	 * find intersections with x, y, and z planes
	 */

	dx = vertex_2->x - vertex_1->x;
	dy = vertex_2->y - vertex_1->y;
	dz = vertex_2->z - vertex_1->z;

	vertex->nx = 0.0;
	vertex->ny = 0.0;
	vertex->nz = 0.0;

	for (;;) {
		if (dx != 0.0) {
			t = (octant->center_x - vertex_1->x) / dx;
			if (t >= 0.0 && t <= 1.0) {
				vertex->nx = 1.0;
				break;
			}
		}

		if (dy != 0.0) {
			t = (octant->center_y - vertex_1->y) / dy;
			if (t >= 0.0 && t <= 1.0) {
				vertex->ny = 1.0;
				break;
			}
		}

		if (dz != 0.0) {
			t = (octant->center_z - vertex_1->z) / dz;
			if (t >= 0.0 && t <= 1.0) {
				vertex->nz = 1.0;
				break;
			}
		}
	}

	one_minus_t = 1.0 - t;

	/*
	 * calculate coordinates for the intersection point
	 */

	vertex->x = vertex_1->x * one_minus_t + vertex_2->x * t;
	vertex->y = vertex_1->y * one_minus_t + vertex_2->y * t;
	vertex->z = vertex_1->z * one_minus_t + vertex_2->z * t;

	/*
	 * adjust normal if necessary
	 */

	if (octant->octant_find_normal != NULL) (*octant->octant_find_normal) (octant);

	return (vertex);

} /* cubes_octant_intersect */

LOCAL int cubes_octant_reformat (octant)
    OCTANT *octant;
{
	float	attenuate;

	/*
	 * normal is octant normal attenuated by data scale factor
	 */

	if (octant->reformat_window == 0) {
		attenuate = 1.0;
	}
	else {
		attenuate = (*p1 & data_mask) +
			t * ((*p2 & data_mask) - (*p1 & data_mask));
		attenuate = (attenuate - octant->reformat_min) / (octant->reformat_window);
	}

	/*
	 * keep attenuation within range
	 */

	if (attenuate > 1.0) attenuate = 1.0;
	if (attenuate < 0.0) attenuate = 0.0;

	vertex->nx *= attenuate;
	vertex->ny *= attenuate;
	vertex->nz *= attenuate;

} /* cubes_octant_reformat */
