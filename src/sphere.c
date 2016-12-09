/*
 * copyright (c) 1987 general electric company
 */

/*++

 * module:      sphere.c

 * version:     1.2 05/31/88 15:32:28

 * facility:	Edge Interpolator for Marching Cubes

 * abstract:
	These routines perform vertex interpolation and recovery
	for spheres in the Marching Cubes algorithm.

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: 5/6/87

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)sphere.c	1.2";
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
	float	radius_2;
	float	center_x;
	float	center_y;
	float	center_z;
	int	(*sphere_find_normal) ();
	int	reformat_min;
	int	reformat_max;
	int	reformat_window;
	int	inside;
	int	outside;
} SPHERE;

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

LOCAL VERTEX interpolate_sphere_1 ();
LOCAL VERTEX interpolate_sphere_2 ();
LOCAL VERTEX interpolate_sphere_3 ();
LOCAL VERTEX interpolate_sphere_4 ();
LOCAL VERTEX interpolate_sphere_5 ();
LOCAL VERTEX interpolate_sphere_6 ();
LOCAL VERTEX interpolate_sphere_7 ();
LOCAL VERTEX interpolate_sphere_8 ();
LOCAL VERTEX interpolate_sphere_9 ();
LOCAL VERTEX interpolate_sphere_10 ();
LOCAL VERTEX interpolate_sphere_11 ();
LOCAL VERTEX interpolate_sphere_12 ();

LOCAL int cubes_build_sphere_index ();
LOCAL int cubes_sphere_inside ();
LOCAL int cubes_sphere_normal ();
LOCAL int cubes_sphere_reformat ();
LOCAL VERTEX *cubes_sphere_intersect ();

static EDGE_PROCEDURE interpolate_sphere_edges[13] = {
	0,
	interpolate_sphere_1,
	interpolate_sphere_2,
	interpolate_sphere_3,
	interpolate_sphere_4,
	interpolate_sphere_5,
	interpolate_sphere_6,
	interpolate_sphere_7,
	interpolate_sphere_8,
	interpolate_sphere_9,
	interpolate_sphere_10,
	interpolate_sphere_11,
	interpolate_sphere_12
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
 * routine cubes_new_sphere (stype, sense, x, y, z, radius, min, max)

 * functional description:
	creates a new sphere surface

 * formal parameters:
	stype - type of surface treatment, OPEN, CUT, CAP, REFORMAT
	sense - INSIDE or OUTSIDE
	x, y, z - center of sphere
	radius - radius of sphere
	min, max - min, max of reformatted data

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_sphere (stype, sense, x, y, z, radius, min, max)
    char *stype;
    char *sense;
    float x, y, z;
    float radius;
    int min;
    int max;
{
	SOLID	*solid;
	SPHERE	*sphere;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid ("sphere", "");

	/*
	 * add sphere specific stuff
	 */

	sphere = (SPHERE *) malloc (sizeof (SPHERE));
	sphere->center_x = x;
	sphere->center_y = y;
	sphere->center_z = z;
	sphere->radius_2 = radius * radius;

	/*
	 * save range and calculate window
	 */

	sphere->reformat_min = min;
	sphere->reformat_max = max;
	sphere->reformat_window = max - min;

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) sphere;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_sphere_index;

	/*
	 * set edge interpolation procedures
	 */

	solid->solid_interpolate = interpolate_sphere_edges;

	/*
	 * set inside procedure
	 */

	solid->solid_inside = cubes_sphere_inside;

	/*
	 * set surface intersection procedure
	 */

	solid->solid_intersect = cubes_sphere_intersect;

	/*
	 * set normal calculation procedure, depending on stype
	 */

	if (strncmp(stype, "open", 4) == 0) {
		sphere->sphere_find_normal = cubes_sphere_normal;
		solid->solid_visible = 0;
	}
	else if (strncmp (stype, "cap", 3) == 0) {
		sphere->sphere_find_normal = cubes_sphere_normal;
	}
	else if (strncmp (stype, "reformat", 4) == 0) {		
		sphere->sphere_find_normal = cubes_sphere_reformat;
	}
	else {
		sphere->sphere_find_normal = cubes_sphere_normal;
	}

	/*
	 * set inside / outside meaning, depending on sense
	 */

	if (strncmp (sense, "inside", 4) == 0) {
		sphere->inside = 1;
		sphere->outside = 0;
	}
	else {
		sphere->inside = 0;
		sphere->outside = 1;
	}

} /* cubes_new_sphere */


/*++
 *
 * routine interpolate_sphere_x (sphere, slice_0, slice_1, slice_2, slice_3)

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
	sphere - pointer to a sphere structure
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

LOCAL VERTEX interpolate_sphere_1 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	/*
	 * return vertex
	 */

	return (*vertex);

} /* interpolate_sphere_1 */
	
LOCAL VERTEX interpolate_sphere_2 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_2 */

LOCAL VERTEX interpolate_sphere_3 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_3 */
	
LOCAL VERTEX interpolate_sphere_4 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_4 */
	
LOCAL VERTEX interpolate_sphere_5 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_5 */
	
LOCAL VERTEX interpolate_sphere_6 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_6 */
	
LOCAL VERTEX interpolate_sphere_7 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_7 */
	
LOCAL VERTEX interpolate_sphere_8 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_8 */
	
LOCAL VERTEX interpolate_sphere_9 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
    PIXEL *slice_0, *slice_1, *slice_2, *slice_3;
{
	p1 = slice_1;
	p2 = slice_2;

	v1.x = pixel;
	v1.y = line;
	v1.z = slice;
	v2 = v1;
	v2.z += 1.0;

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_9 */
	
LOCAL VERTEX interpolate_sphere_10 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_10 */
	
LOCAL VERTEX interpolate_sphere_11 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_11 */

LOCAL VERTEX interpolate_sphere_12 (sphere, slice_0, slice_1, slice_2, slice_3)
    SPHERE *sphere;
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

	vertex = cubes_sphere_intersect (sphere, &v1, &v2);

	return (*vertex);

} /* interpolate_sphere_12 */
	

/*++
 *
 * routine cubes_build_sphere_index (sphere, slice_1, slice_2)

 * functional description:
	creates an index from 0 - 255 based on the value of a sphere
	equation at the eight nodes making the cube.  Each node is assigned
	a 0 if it is outside the sphere region
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
	sphere - pointer to a sphere definition
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

LOCAL int cubes_build_sphere_index (sphere, slice_1, slice_2)

register SPHERE *sphere;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
register int sphere_index = 0;

	/*
	 * calculate f(x,y,z) at corners of cube
	 */

	vertex->x = pixel;
	vertex->y = line;
	vertex->z = slice;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 1;
	vertex->x += 1.0;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 2;
	vertex->y += 1.0;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 4;
	vertex->x = pixel;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 8;
	vertex->y = line;
	vertex->z = slice + 1;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 16;
	vertex->x += 1.0;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 32;
	vertex->y += 1.0;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 64;
	vertex->x = pixel;
	if (cubes_sphere_inside (sphere, vertex)) sphere_index += 128;

	return (sphere_index);
} /* cubes_build_sphere_index */

LOCAL int cubes_sphere_inside (sphere, vertex)
register SPHERE *sphere;
register VERTEX *vertex;
{
	float	value;
	float	offset_x, offset_y, offset_z;

	offset_x = (vertex->x - sphere->center_x) * scale_x;
	offset_y = (vertex->y - sphere->center_y) * scale_y;
	offset_z = (vertex->z - sphere->center_z) * scale_z;

	value = offset_x * offset_x + offset_y * offset_y + offset_z * offset_z
		- sphere->radius_2;

	if (value <= 0.0) return (sphere->inside);

	return (sphere->outside);
} /* cubes_sphere_inside */

LOCAL VERTEX *cubes_sphere_intersect (sphere, vertex_1, vertex_2)
    VERTEX *vertex_1;
    VERTEX *vertex_2;
    SPHERE  *sphere;
{
	double	dx, dy, dz;
	double	a, b, c, disc;
	double	offset_x, offset_y, offset_z;

	/*
	 * calculate delta in x, y, z
	 */

	dx = vertex_2->x - vertex_1->x;
	dy = vertex_2->y - vertex_1->y;
	dz = vertex_2->z - vertex_1->z;
	dx *= scale_x;
	dy *= scale_y;
	dz *= scale_z;

	offset_x = (vertex_1->x - sphere->center_x) * scale_x;
	offset_y = (vertex_1->y - sphere->center_y) * scale_y;
	offset_z = (vertex_1->z - sphere->center_z) * scale_z;

	/*
	 * calculate t from the parametric representation of a line
	 * and the sphere equation
	 */

	a = (dx * dx + dy * dy + dz * dz);
	b = 2.0 * (dx * offset_x + dy * offset_y + dz * offset_z);
	c = offset_x * offset_x +
	    offset_y * offset_y +
	    offset_z * offset_z -
	    sphere->radius_2;

	disc = b * b - 4.0 * a * c;

	if (disc < 0.0) {
		printf ("disc < 0.0 = %f\n", disc);
		t = 0.0;
	}
	else {
		disc = sqrt (disc);
		t = (-b + disc) / a;
		if (t < 0.0 || t > 1.0) t = (-b - disc) / a;
	}

	one_minus_t = 1.0 - t;

	/*
	 * calculate coordinates for the intersection point
	 */

	vertex->x = vertex_1->x * one_minus_t + vertex_2->x * t;
	vertex->y = vertex_1->y * one_minus_t + vertex_2->y * t;
	vertex->z = vertex_1->z * one_minus_t + vertex_2->z * t;

	(*sphere->sphere_find_normal) (sphere);

/*	printf ("intersect: x,y,z = %f,%f,%f\n", vertex->x,
						vertex->y,
						vertex->z);*/
	return (vertex);

} /* cubes_sphere_intersect */

LOCAL int cubes_sphere_normal (sphere)
    SPHERE *sphere;
{
	float	nx, ny, nz;
	float	length;

	/*
	 * find normal to sphere
	 */

	nx = (vertex->x - sphere->center_x) * scale_x;
	ny = (vertex->y - sphere->center_y) * scale_y;
	nz = (vertex->z - sphere->center_z) * scale_z;

	/*
	 * make it unit normal
	 */

	length = sqrt (nx * nx + ny * ny + nz * nz);

	vertex->nx = nx / length;
	vertex->ny = ny / length;
	vertex->nz = nz / length;

} /* cubes_sphere_normal */

LOCAL int cubes_sphere_reformat (sphere)
    SPHERE *sphere;
{
	float	attenuate;

	/*
	 * normal is sphere normal attenuated by data scale factor
	 */

	if (sphere->reformat_window == 0) {
		attenuate = 1.0;
	}
	else {
		attenuate = (*p1 & data_mask) +
			t * ((*p2 & data_mask) - (*p1 & data_mask));
		attenuate = (attenuate - sphere->reformat_min) / (sphere->reformat_window);
	}

	/*
	 * keep attenuation within range
	 */

	if (attenuate > 1.0) attenuate = 1.0;
	if (attenuate < 0.0) attenuate = 0.0;

	/*
	 * calculate normal
	 */

	cubes_sphere_normal (sphere);

	/*
	 * attenuate it
	 */

	vertex->nx *= attenuate;
	vertex->ny *= attenuate;
	vertex->nz *= attenuate;

} /* cubes_sphere_reformat */
