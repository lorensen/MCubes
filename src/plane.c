/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      plane.c

 * version:     1.4 03/21/89 07:11:11

 * facility:	Edge Interpolator for Marching Cubes

 * abstract:
	These routines perform vertex interpolation and recovery
	for planes in the Marching Cubes algorithm.

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: 2/19/86

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)plane.c	1.4";
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
	float	plane_a;
	float	plane_b;
	float	plane_c;
	float	plane_d;
	int	(*plane_find_normal) ();
	int	reformat_min;
	int	reformat_max;
	int	reformat_window;
} PLANE;

/*
 * macros:
 */

#define EVALUATE_PLANE(x,y,z)\
	(plane->plane_a * (x) + plane->plane_b * (y) + plane->plane_c * (z) +\
	plane->plane_d)		

#define NORMALIZE_XYZ(xyz)	\
	length = sqrt ((xyz)->nx * (xyz)->nx +\
		       (xyz)->ny * (xyz)->ny +\
		       (xyz)->nz * (xyz)->nz);\
	if (length != 0.0) {\
		 (xyz)->nx /= length;\
		 (xyz)->ny /= length;\
		 (xyz)->nz /= length;\
	}

/*
 * own storage:
 */

static PIXEL *p1, *p2;
static float t, one_minus_t;
static int offset_1, offset_2;

LOCAL VERTEX interpolate_plane_1 ();
LOCAL VERTEX interpolate_plane_2 ();
LOCAL VERTEX interpolate_plane_3 ();
LOCAL VERTEX interpolate_plane_4 ();
LOCAL VERTEX interpolate_plane_5 ();
LOCAL VERTEX interpolate_plane_6 ();
LOCAL VERTEX interpolate_plane_7 ();
LOCAL VERTEX interpolate_plane_8 ();
LOCAL VERTEX interpolate_plane_9 ();
LOCAL VERTEX interpolate_plane_10 ();
LOCAL VERTEX interpolate_plane_11 ();
LOCAL VERTEX interpolate_plane_12 ();

LOCAL int cubes_build_plane_index ();
LOCAL int cubes_plane_inside ();
LOCAL int cubes_plane_normal ();
LOCAL int cubes_reformat_normal ();
LOCAL VERTEX *cubes_plane_intersect ();

static EDGE_PROCEDURE interpolate_plane_edges[13] = {
	0,
	interpolate_plane_1,
	interpolate_plane_2,
	interpolate_plane_3,
	interpolate_plane_4,
	interpolate_plane_5,
	interpolate_plane_6,
	interpolate_plane_7,
	interpolate_plane_8,
	interpolate_plane_9,
	interpolate_plane_10,
	interpolate_plane_11,
	interpolate_plane_12
};


/*
 * external references:
 */

extern SOLID *cubes_new_solid ();
extern double sqrt ();

extern PIXEL data_mask;
extern VERTEX *vertex;
extern int line;
extern int pixel;
extern int slice;
extern int pixels_per_line;


/*++
 *
 * routine cubes_new_cap (name, file, x, y, z, nx, ny, nz)

 * functional description:
	creates a new cap surface

 * formal parameters:
	name - name of solid
	file - file to store output
	x, y, z - point on plane
	nx, ny, nz - normal to plane

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_cap (name, file, x, y, z, nx, ny, nz)
    char *name;
    char *file;
    float x, y, z;
    float nx, ny, nz;
{
	SOLID	*solid;
	PLANE	*plane;
	float	length;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid (name, file);

	/*
	 * add plane specific stuff
	 */

	plane = (PLANE *) malloc (sizeof (PLANE));

	/*
	 * make a unit normal
	 */

	length = sqrt (nx * nx + ny * ny + nz * nz);
	plane->plane_a = nx / length;
	plane->plane_b = ny / length;
	plane->plane_c = nz / length;

	/*
	 * calculate 4th coefficient using given point
	 */

	plane->plane_d = - (plane->plane_a * x +
			    plane->plane_b * y +
			    plane->plane_c * z);

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) plane;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_plane_index;

	/*
	 * set edge interpolation procedures
	 */

	solid->solid_interpolate = interpolate_plane_edges;

	/*
	 * set inside procedure
	 */

	solid->solid_inside = cubes_plane_inside;

	/*
	 * set surface intersection procedure
	 */

	solid->solid_intersect = cubes_plane_intersect;

	/*
	 * set normal calculation procedure
	 */

	plane->plane_find_normal = cubes_plane_normal;

} /* cubes_new_cap */


/*++
 *
 * routine cubes_new_cut (name, file, x, y, z, nx, ny, nz)

 * functional description:
	creates a new cut surface

 * formal parameters:
	name - name of solid
	file - file to store output
	x, y, z - point on plane
	nx, ny, nz - normal to plane

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_cut (name, file, x, y, z, nx, ny, nz)
    char *name;
    char *file;
    float x, y, z;
    float nx, ny, nz;
{
	SOLID	*solid;
	PLANE	*plane;
	float	length;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid (name, file);

	/*
	 * add plane specific stuff
	 */

	plane = (PLANE *) malloc (sizeof (PLANE));

	/*
	 * make a unit normal
	 */

	length = sqrt (nx * nx + ny * ny + nz * nz);
	plane->plane_a = nx / length;
	plane->plane_b = ny / length;
	plane->plane_c = nz / length;

	/*
	 * calculate 4th coefficient using given point
	 */

	plane->plane_d = - (plane->plane_a * x +
			    plane->plane_b * y +
			    plane->plane_c * z);

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) plane;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_plane_index;

	/*
	 * set edge interpolation procedures
	 */

	solid->solid_interpolate = interpolate_plane_edges;

	/*
	 * set inside procedure
	 */

	solid->solid_inside = cubes_plane_inside;

	/*
	 * set surface intersection procedure
	 */

	solid->solid_intersect = cubes_plane_intersect;

	/*
	 * set normal calculation procedure
	 */

	plane->plane_find_normal = cubes_plane_normal;

	/*
	 * a cut plane is not visible
	 */

	solid->solid_visible = 0;

} /* cubes_new_cut */


/*++
 *
 * routine cubes_new_reformat (name, file, x, y, z, nx, ny, nz, min, max)

 * functional description:
	creates a new reformat surface

 * formal parameters:
	name - name of solid
	file - file to store output
	x, y, z - point on reformat
	nx, ny, nz - normal to reformat
	min - min value of pixel data
	max - max value of pixel data

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_reformat (name, file, x, y, z, nx, ny, nz, min, max)
    char *name;
    char *file;
    float x, y, z;
    float nx, ny, nz;
    int min, max;
{
	SOLID	*solid;
	PLANE	*reformat;
	float	length;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid (name, file);

	/*
	 * add reformat specific stuff
	 */

	reformat = (PLANE *) malloc (sizeof (PLANE));

	/*
	 * make a unit normal
	 */

	length = sqrt (nx * nx + ny * ny + nz * nz);
	reformat->plane_a = nx / length;
	reformat->plane_b = ny / length;
	reformat->plane_c = nz / length;

	/*
	 * calculate 4th coefficient using given point
	 */

	reformat->plane_d = - (reformat->plane_a * x +
			    reformat->plane_b * y +
			    reformat->plane_c * z);

	/*
	 * save range and calculate window
	 */

	reformat->reformat_min = min;
	reformat->reformat_max = max;
	reformat->reformat_window = max - min;

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) reformat;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_plane_index;

	/*
	 * set surface intersection procedure
	 */

	solid->solid_intersect = cubes_plane_intersect;

	/*
	 * set inside procedure
	 */

	solid->solid_inside = cubes_plane_inside;

	/*
	 * set edge interpolation procedures
	 */

	solid->solid_interpolate = interpolate_plane_edges;

	/*
	 * set normal calculation procedure
	 */

	reformat->plane_find_normal = cubes_reformat_normal;

} /* cubes_new_reformat */


/*++
 *
 * routine interpolate_plane_x (plane, slice_0, slice_1, slice_2, slice_3)

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
	plane - pointer to a plane structure
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

LOCAL VERTEX interpolate_plane_1 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	/*
	 * find vertex coordinate
	 */

	if (plane->plane_a != 0.0) {
		t = - EVALUATE_PLANE (pixel, line, slice) / plane->plane_a;
	}
	else {
		t = 0.0;
	}
	vertex->x = pixel + t;
	vertex->y = line;
	vertex->z = slice;

	/*
	 * normal calculation depends on the type of plane
	 */

	(*plane->plane_find_normal) (plane);

	/*
	 * return vertex
	 */

	return (*vertex);

} /* interpolate_plane_1 */
	
LOCAL VERTEX interpolate_plane_2 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 1;
	offset_2 = pixels_per_line + 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	vertex->x = pixel + 1;
	if (plane->plane_b != 0.0) {
		t = - EVALUATE_PLANE (pixel+1, line, slice) / plane->plane_b;
	}
	else {
		t = 0.0;
	}
	vertex->y = line + t;
	vertex->z = slice;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_2 */

LOCAL VERTEX interpolate_plane_3 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line + 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	if (plane->plane_a != 0.0) {
		t = - EVALUATE_PLANE (pixel, line + 1, slice) / plane->plane_a;
	}
	else {
		t = 0.0;
	}
	vertex->x = pixel + t;
	vertex->y = line + 1;
	vertex->z = slice;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_3 */
	
LOCAL VERTEX interpolate_plane_4 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = pixels_per_line;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	vertex->x = pixel;
	if (plane->plane_b != 0.0) {
		t = - EVALUATE_PLANE (pixel, line, slice) / plane->plane_b;
	}
	else {
		t = 0.0;
	}
	vertex->y = line + t;
	vertex->z = slice;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_4 */
	
LOCAL VERTEX interpolate_plane_5 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	if (plane->plane_a != 0.0) {
		t = - EVALUATE_PLANE (pixel, line, slice + 1) / plane->plane_a;
	}
	else {
		t = 0.0;
	}
	vertex->x = pixel + t;
	vertex->y = line;
	vertex->z = slice + 1;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_5 */
	
LOCAL VERTEX interpolate_plane_6 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 1;
	offset_2 = pixels_per_line + 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	vertex->x = pixel + 1;
	if (plane->plane_b != 0.0) {
		t = - EVALUATE_PLANE (pixel + 1, line, slice + 1) / plane->plane_b;
	}
	else {
		t = 0.0;
	}
	vertex->y = line + t;
	vertex->z = slice + 1;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_6 */
	
LOCAL VERTEX interpolate_plane_7 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line + 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	if (plane->plane_a != 0.0) {
		t = - EVALUATE_PLANE (pixel, line + 1, slice + 1) / plane->plane_a;
	}
	else {
		t = 0.0;
	}
	vertex->x = pixel + t;
	vertex->y = line + 1;
	vertex->z = slice + 1;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_7 */
	
LOCAL VERTEX interpolate_plane_8 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 0;
	offset_2 = pixels_per_line;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	vertex->x = pixel;
	if (plane->plane_b != 0.0) {
		t = - EVALUATE_PLANE (pixel, line, slice + 1) / plane->plane_b;
	}
	else {
		t = 0.0;
	}
	vertex->y = line + t;
	vertex->z = slice + 1;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_8 */
	
LOCAL VERTEX interpolate_plane_9 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	p1 = slice_1;
	p2 = slice_2;

	vertex->x = pixel;
	vertex->y = line;
	if (plane->plane_c != 0.0) {
		t = - EVALUATE_PLANE (pixel, line, slice) / plane->plane_c;
	}
	else {
		t = 0.0;
	}
	vertex->z = slice + t;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_9 */
	
LOCAL VERTEX interpolate_plane_10 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = 1;
	offset_2 = 1;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	vertex->x = pixel + 1;
	vertex->y = line;
	if (plane->plane_c != 0.0) {
		t = - EVALUATE_PLANE (pixel + 1, line, slice) / plane->plane_c;
	}
	else {
		t = 0.0;
	}
	vertex->z = slice + t;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_10 */
	
LOCAL VERTEX interpolate_plane_11 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	vertex->x = pixel;
	vertex->y = line + 1;
	if (plane->plane_c != 0.0) {
		t = - EVALUATE_PLANE (pixel, line + 1, slice) / plane->plane_c;
	}
	else {
		t = 0.0;
	}

	vertex->z = slice + t;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_11 */

LOCAL VERTEX interpolate_plane_12 (plane, slice_0, slice_1, slice_2, slice_3)
register    PLANE *plane;
register    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	offset_1 = pixels_per_line + 1;
	offset_2 = offset_1;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	vertex->x = pixel + 1;
	vertex->y = line + 1;
	if (plane->plane_c != 0.0) {
		t = - EVALUATE_PLANE (pixel + 1, line + 1, slice) / plane->plane_c;
	}
	else {
		t = 0.0;
	}
	vertex->z = slice + t;

	(*plane->plane_find_normal) (plane);

	return (*vertex);

} /* interpolate_plane_12 */
	

/*++
 *
 * routine cubes_build_plane_index (plane, slice_1, slice_2)

 * functional description:
	creates an index from 0 - 255 based on the value of a plane
	equation at the eight nodes making the cube.  Each node is assigned
	a 0 if it is outside the plane region
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
	plane - pointer to a plane definition
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

LOCAL int cubes_build_plane_index (plane, slice_1, slice_2)

register PLANE *plane;
register PIXEL *slice_1;
register PIXEL *slice_2;
{
register int plane_index = 0;
register float	p1, p2, p3, p4, p5, p6, p7, p8;

	/*
	 * calculate f(x,y,z) at corners of cube
	 */

	p1 = EVALUATE_PLANE (pixel, line, slice);

	if (p1 <= 0.0) plane_index += 1;
	p2 = p1 + plane->plane_a;
	if (p2 <= 0.0) plane_index += 2;
	p3 = p2 + plane->plane_b;
	if (p3 <= 0.0) plane_index += 4;	
	p4 = p1 + plane->plane_b;
	if (p4 <= 0.0) plane_index += 8;
	p5 = p1 + plane->plane_c;
	if (p5 <= 0.0) plane_index += 16;
	p6 = p5 + plane->plane_a;
	if (p6 <= 0.0) plane_index += 32;
	p7 = p6 + plane->plane_b;
	if (p7 <= 0.0) plane_index += 64;
	p8 = p5 + plane->plane_b;
	if (p8 <= 0.0) plane_index += 128;

	return (plane_index);
} /* cubes_build_plane_index */

LOCAL int cubes_plane_inside (plane, vertex)
register    PLANE *plane;
register    VERTEX *vertex;
{
register float dot;

	dot =	vertex->x * plane->plane_a +
		vertex->y * plane->plane_b +
		vertex->z * plane->plane_c + plane->plane_d;

	if (dot <= 0.0) return (1);

	return (0);
} /* cubes_plane_inside */

LOCAL VERTEX *cubes_plane_intersect (plane, vertex_1, vertex_2)
register    VERTEX *vertex_1;
register    VERTEX *vertex_2;
register    PLANE  *plane;
{
	float	x, y, z;
	float	dx, dy, dz;
	float	length;
	float	v1x = vertex_1->x;
	float	v1y = vertex_1->y;
	float	v1z = vertex_1->z;
	float	v2x = vertex_2->x;
	float	v2y = vertex_2->y;
	float	v2z = vertex_2->z;
register float	t;

	/*
	 * calculate delta in x, y, z
	 */

	dx = v2x - v1x;
	dy = v2y - v1y;
	dz = v2z - v1z;

	/*
	 * calculate t from the parametric representation of a line
	 * and the plane equation
	 */

	t =	-plane->plane_d
		-plane->plane_a * vertex_1->x
		-plane->plane_b * vertex_1->y
		-plane->plane_c * vertex_1->z;

	t /= (	plane->plane_a * dx
		+ plane->plane_b * dy
		+ plane->plane_c * dz);

	/*
	 * calculate coordinates for the intersection point
	 */

	x = v1x + dx * t;
	y = v1y + dy * t;
	z = v1z + dz * t;

	/*
	 * calculate normal at intersection point
	 */

	vertex->nx = vertex_1->nx * one_minus_t + vertex_2->nx * t;
	vertex->ny = vertex_1->ny * one_minus_t + vertex_2->ny * t;
	vertex->nz = vertex_1->nz * one_minus_t + vertex_2->nz * t;

	NORMALIZE_XYZ(vertex);

	vertex->x = x; vertex->y = y; vertex->z = z;
	return (vertex);

} /* cubes_plane_intersect */

LOCAL int cubes_plane_normal (plane)
register PLANE *plane;
{
	/*
	 * normal is just normal of the plane
	 */

	vertex->nx = -plane->plane_a;
	vertex->ny = -plane->plane_b;
	vertex->nz = -plane->plane_c;

} /* cubes_plane_normal */

LOCAL int cubes_reformat_normal (plane)
register PLANE *plane;
{
	register float	attenuate;

	/*
	 * normal is plane normal attenuated by data scale factor
	 */

	attenuate = (*p1 & data_mask) +
		t * ((*p2 & data_mask) - (*p1 & data_mask));
	attenuate = (attenuate - plane->reformat_min) / (plane->reformat_window);
	/*
	 * keep attenuation within range
	 */

	if (attenuate > 1.0) attenuate = 1.0;
	if (attenuate < 0.0) attenuate = 0.0;

	vertex->nx = plane->plane_a * attenuate;
	vertex->ny = plane->plane_b * attenuate;
	vertex->nz = plane->plane_c * attenuate;

} /* cubes_reformat_normal */
