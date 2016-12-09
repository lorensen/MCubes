/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      surface.c

 * version:     1.6 01/06/89 17:06:09

 * facility:	Surface Interpolator for Marching Cubes

 * abstract:
	These routines perform surface specific operations
	for the Marching Cubes algorithm.

 * environment:	Vax VMS and Sun Workstation

 * author: W. Lorensen, creation date: May 15, 1985

 * modified by:

	, : version
   01    -

 *
 */

#ifndef lint
static char    *sccs_id = "@(#)surface.c	1.6";
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
	float	surface_value;
} SURFACE;

/*
 * macros:
 */

#define FIND_T	\
	p2mp1 = ((*p2 & data_mask) - (*p1 & data_mask));\
	if (p2mp1 == 0) t = 0.0;\
	else t = (surface->surface_value - (*p1 & data_mask)) / p2mp1;\
	if (t < 0.0) t = 0.0;\
	if (t > 1.0) t = 1.0;\
	one_minus_t = 1.0 - t

#define NORMAL_X(p1)		\
	((*(p1 + 1) & data_mask) - (*(p1 - 1) & data_mask))
#define NORMAL_Y(p1)		\
	((*(p1 + pixels_per_line) & data_mask) - (*(p1 - pixels_per_line) & data_mask))
#define NORMAL_Z(p1, p2, p3)	\
	((*(p3) & data_mask) - (*(p1) & data_mask)) / aspect_xy_to_z

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

LOCAL VERTEX	interpolate_surface_1 ();
LOCAL VERTEX	interpolate_surface_2 ();
LOCAL VERTEX	interpolate_surface_3 ();
LOCAL VERTEX	interpolate_surface_4 ();
LOCAL VERTEX	interpolate_surface_5 ();
LOCAL VERTEX	interpolate_surface_6 ();
LOCAL VERTEX	interpolate_surface_7 ();
LOCAL VERTEX	interpolate_surface_8 ();
LOCAL VERTEX	interpolate_surface_9 ();
LOCAL VERTEX	interpolate_surface_10 ();
LOCAL VERTEX	interpolate_surface_11 ();
LOCAL VERTEX	interpolate_surface_12 ();
LOCAL int	cubes_build_surface_index ();
LOCAL int	cubes_surface_inside ();
LOCAL VERTEX	*cubes_surface_intersect ();

static EDGE_PROCEDURE interpolate_surface_edges[13] = {
	0,
	interpolate_surface_1,
	interpolate_surface_2,
	interpolate_surface_3,
	interpolate_surface_4,
	interpolate_surface_5,
	interpolate_surface_6,
	interpolate_surface_7,
	interpolate_surface_8,
	interpolate_surface_9,
	interpolate_surface_10,
	interpolate_surface_11,
	interpolate_surface_12
};

static int	p2mp1;
static float	t;
static float	one_minus_t;
static float	length;
static PIXEL	p1, p2, p3, p4, p5, p6, p7, p8;
static int	offset_1;
static int	offset_2;
static VERTEX	output_vertex;
static VERTEX	v1;
static VERTEX	v2;
static VERTEX	local_vertex;
static VERTEX *vertex = &local_vertex;

/*
 * external references:
 */

extern SOLID *cubes_new_solid ();
extern double sqrt ();

extern PIXEL	*slice_1, *slice_2;
extern float aspect_xy_to_z;
extern PIXEL data_mask;
extern int line;
extern int number_nodes;
extern int pixel;
extern int pixels_per_line;
extern int slice;


/*++
 *
 * routine cubes_new_surface (stype, file, value)

 * functional description:
	creates a new surface
 * formal parameters:
	stype - type (open or surface)
	file - file to store output
	value - surface value

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

cubes_new_surface (stype, file, value)
    char *stype;
    char *file;
    float value;
{
	SOLID	*solid;
	SURFACE	*surface;

	/*
	 * create new solid
	 */

	solid = (SOLID *) cubes_new_solid ("surface", file);

	/*
	 * add surface specific stuff
	 */

	surface = (SURFACE *) malloc (sizeof (SURFACE));

	surface->surface_value = 1.000 * value;

	/*
	 * set solid object pointer
	 */

	solid->solid_object = (OBJECT *) surface;

	/*
	 * set index procedure
	 */

	solid->solid_index = cubes_build_surface_index;

	/*
	 * set edge interpolation procedures
	 */

	solid->solid_interpolate = interpolate_surface_edges;

	/*
	 * set inside procedure
	 */

	solid->solid_inside = cubes_surface_inside;

	/*
	 * set surface intersection procedure
	 */

	solid->solid_intersect = cubes_surface_intersect;

	/*
	 * set normal calculation procedure, depending on type
	 */

	if (strncmp(stype, "open", 4) == 0) {
		solid->solid_visible = 0;
	}
}


/*++
 *
 * routine interpolate_surface_x (slice_0, slice_1, slice_2, slice_3)

 * functional description:
	interpolates a vertex of a polygon on the edge of a cube
	also calculates and interpolates the normal

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
	slice_0 - pixel 1 on the i - 1 slice
	slice_1 - pixel 1 on the i slice
	slice_2 - pixel 2 on the i + 1 slice
	slice_3 - pixel 1 on the i + 2 slice

 * implicit inputs:
	line  - current line number
	pixel - current pixel number
	slice - current slice number
	surface->surface_value - surface value

 * implicit outputs:
	none

 * routine value:
	none
 *
 */

LOCAL VERTEX interpolate_surface_1 (surface, slice_0, slice_1, slice_2)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	PIXEL	*p1;
	PIXEL	*p2;

	/*
	 * pick up pointer to end points of edge
	 */

	offset_1 = 0;
	offset_2 = 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	/*
	 * calculate interpolation fraction (between zero and one)
	 */

	FIND_T;

	/*
	 * find vertex coordinate
	 */

	vertex->x = pixel + t;
	vertex->y = line;
	vertex->z = slice;

	/*
	 * calculate normals at points 1 and 2
	 */

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_0 + offset_2,
			  slice_1 + offset_2,
			  slice_2 + offset_2);

	/*
	 * interpolate normal
	 */

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	/*
	 * return vertex
	 */

	return (*vertex);

} /* interpolate_surface_1 */
	
LOCAL VERTEX interpolate_surface_2 (surface, slice_0, slice_1, slice_2)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	PIXEL	*p1;
	PIXEL	*p2;

	offset_1 = 1;
	offset_2 = pixels_per_line + 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	FIND_T;

	vertex->x = pixel + 1;
	vertex->y = line + t;
	vertex->z = slice;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_0 + offset_2,
			  slice_1 + offset_2,
			  slice_2 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_2 */

LOCAL VERTEX interpolate_surface_3 (surface, slice_0, slice_1, slice_2)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	PIXEL	*p1;
	PIXEL	*p2;

	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line + 1;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	FIND_T;

	vertex->x = pixel + t;
	vertex->y = line + 1;
	vertex->z = slice;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_0 + offset_2,
			  slice_1 + offset_2,
			  slice_2 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_3 */
	
LOCAL VERTEX interpolate_surface_4 (surface, slice_0, slice_1, slice_2)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = 0;
	offset_2 = pixels_per_line;

	p1 = slice_1 + offset_1;
	p2 = slice_1 + offset_2;

	FIND_T;

	vertex->x = pixel;
	vertex->y = line + t;
	vertex->z = slice;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_0 + offset_2,
			  slice_1 + offset_2,
			  slice_2 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_4 */
	
LOCAL VERTEX interpolate_surface_5 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;
	
	offset_1 = 0;
	offset_2 = 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;

	vertex->x = pixel + t;
	vertex->y = line;
	vertex->z = slice + 1;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_1 + offset_1,
			  slice_2 + offset_1,
			  slice_3 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_5 */
	
LOCAL VERTEX interpolate_surface_6 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = 1;
	offset_2 = pixels_per_line + 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;

	vertex->x = pixel + 1;
	vertex->y = line + t;
	vertex->z = slice + 1;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_1 + offset_1,
			  slice_2 + offset_1,
			  slice_3 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_6 */
	
LOCAL VERTEX interpolate_surface_7 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line + 1;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;

	vertex->x = pixel + t;
	vertex->y = line + 1;
	vertex->z = slice + 1;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_1 + offset_1,
			  slice_2 + offset_1,
			  slice_3 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_7 */
	
LOCAL VERTEX interpolate_surface_8 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = 0;
	offset_2 = pixels_per_line;

	p1 = slice_2 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;

	vertex->x = pixel;
	vertex->y = line + t;
	vertex->z = slice + 1;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_1 + offset_1,
			  slice_2 + offset_1,
			  slice_3 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_8 */
	
LOCAL VERTEX interpolate_surface_9 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	p1 = slice_1;
	p2 = slice_2;

	FIND_T;
/*	QUADRATIC_INTERPOLATION (slice_1, slice_2, slice_3);*/

	vertex->x = pixel;
	vertex->y = line;
	vertex->z = slice + t;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0,
			  slice_1,
			  slice_2);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1,
			  slice_2,
			  slice_3);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_9 */
	
LOCAL VERTEX interpolate_surface_10 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = 1;
	offset_2 = 1;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;	
/*	QUADRATIC_INTERPOLATION (slice_1 + offset_1, slice_2 + offset_1, slice_3 + offset_1);*/

	vertex->x = pixel + 1;
	vertex->y = line;
	vertex->z = slice + t;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_10 */
	
LOCAL VERTEX interpolate_surface_11 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = pixels_per_line;
	offset_2 = pixels_per_line;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;
/*	QUADRATIC_INTERPOLATION (slice_1 + offset_1, slice_2 + offset_1, slice_3 + offset_1);*/

	vertex->x = pixel;
	vertex->y = line + 1;
	vertex->z = slice + t;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_11 */

LOCAL VERTEX interpolate_surface_12 (surface, slice_0, slice_1, slice_2, slice_3)
    SURFACE *surface;
    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	PIXEL *p1;
	PIXEL *p2;

	offset_1 = pixels_per_line + 1;
	offset_2 = offset_1;

	p1 = slice_1 + offset_1;
	p2 = slice_2 + offset_2;

	FIND_T;
/*	QUADRATIC_INTERPOLATION (slice_1 + offset_1, slice_2 + offset_1, slice_3 + offset_1);*/

	vertex->x = pixel + 1;
	vertex->y = line + 1;
	vertex->z = slice + t;

	v1.nx = NORMAL_X (p1);
	v1.ny = NORMAL_Y (p1);
	v1.nz = NORMAL_Z (slice_0 + offset_1,
			  slice_1 + offset_1,
			  slice_2 + offset_1);

	v2.nx = NORMAL_X (p2);
	v2.ny = NORMAL_Y (p2);
	v2.nz = NORMAL_Z (slice_1 + offset_2,
			  slice_2 + offset_2,
			  slice_3 + offset_2);

	vertex->nx = v1.nx * one_minus_t + v2.nx * t;
	vertex->ny = v1.ny * one_minus_t + v2.ny * t;
	vertex->nz = v1.nz * one_minus_t + v2.nz * t;

	NORMALIZE_XYZ (vertex);

	return (*vertex);

} /* interpolate_surface_12 */
	

/*++
 *
 * routine cubes_build_surface_index (surface, slice_1, slice_2)

 * functional description:
	creates an index from 0 - 255 based on the function value
	at the eight nodes making the cube.  Each node is assigned
	a 0 if its value is less than the desired surface value or
	1 if it is greater than the value.  The index is formed from
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
	surface - pointer to surface description
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

LOCAL int cubes_build_surface_index (surface, slice_1, slice_2)

SURFACE *surface;
register PIXEL *slice_1;
register PIXEL *slice_2;
{
register int	index;
register PIXEL	value = surface->surface_value;
	int next_pixel = 1;
	int next_line = pixels_per_line;
register PIXEL	p;

	/*
	 * generate a cube index
	 */

	index = 0;

	p = (*slice_1 & data_mask);
	if (p >= value) index |= 1;	/* 1 */

	p = *(slice_1 + next_pixel) & data_mask;
	if (p >= value) index |= 2;	/* 2 */

	p = *(slice_1 + next_line + next_pixel) & data_mask;
	if (p >= value) index |= 4;	/* 3 */

	p = *(slice_1 + next_line) & data_mask;
	if (p >= value) index |= 8;	/* 4 */

	p = *slice_2 & data_mask;
	if (p >= value) index |= 16;	/* 5 */

	p = *(slice_2 + next_pixel) & data_mask;
	if (p >= value) index |= 32;	/* 6 */

	p = *(slice_2 + next_line + next_pixel) & data_mask;
	if (p >= value) index |= 64;	/* 7 */

	p = *(slice_2 + next_line) & data_mask;
	if (p >= value) index |= 128;	/* 8 */

	return (index);
}

LOCAL int cubes_surface_inside (surface, vertex)
    SURFACE *surface;
    VERTEX *vertex;
{
	float	fp_1, fp_2, fp_3, fp_4;
	float	fq_1, fq_2;
	float	value;
	float	x, y, z;
	PIXEL	*p1 = slice_1 + pixel + line * pixels_per_line;
	PIXEL	*p2 = p1 + 1;
	PIXEL	*p3 = p2 + pixels_per_line;
 	PIXEL	*p4 = p1 + pixels_per_line;
	PIXEL	*p5 = slice_2 + pixel + line * pixels_per_line;
	PIXEL	*p6 = p5 + 1;
	PIXEL	*p7 = p6 + pixels_per_line;
	PIXEL	*p8 = p5 + pixels_per_line;

	/*
	 * interpolate function value at vertex and compare with surface
	 * value.  if greater, vertex is inside. if less, vertex is outside
	 */

	x = vertex->x - pixel;
	y = vertex->y - line;
	z = vertex->z - slice;

	fp_1 = *p1 + x * (*p2 - *p1);
	fp_2 = *p4 + x * (*p3 - *p4);
	fp_3 = *p5 + x * (*p6 - *p5);
	fp_4 = *p8 + x * (*p7 - *p8);

	fq_1 = fp_1 + y * (fp_2 - fp_1);
	fq_2 = fp_3 + y * (fp_4 - fp_3);

	value = fq_1 + z * (fq_2 - fq_1);

	if (value >= surface->surface_value) return (1);
	return (0);

} /* cubes_surface_inside */

LOCAL VERTEX *cubes_surface_intersect (surface, vertex_1, vertex_2)
    VERTEX *vertex_1;
    VERTEX *vertex_2;
    SURFACE  *surface;
{
	float	t_1, t_2, t, one_minus_t;
	int	i;
	int	inside;
	int	inside_1;
#define MAX_ITERATIONS 12

	/*
	 * solution by bisection method
	 */

	/*
	 * set up first and second points
	 */

	if (cubes_surface_inside (surface, vertex_1)) inside_1 = 1;
	else inside_1 = -1;

	t_1 = 0.0;

	t_2 = 1.0;

	for (i = 0; i < MAX_ITERATIONS; i++) {

		/*
		 * calculate new vertex point
		 */

		t = (t_2 + t_1) / 2.0;
		one_minus_t = 1.0 - t;
		vertex->x = vertex_1->x * one_minus_t + vertex_2->x * t;
		vertex->y = vertex_1->y * one_minus_t + vertex_2->y * t;
		vertex->z = vertex_1->z * one_minus_t + vertex_2->z * t;

		/*
		 * see whether it's in or out
		 */

		if (cubes_surface_inside (surface, vertex)) inside = 1;
		else inside = -1;

		if ((inside_1 * inside) < 0) {
			t_2 = t;
		}
		else {
			inside_1 = inside;
			t_1 = t;
		}
	}

	/*
	 * interpolate normal
	 */

	vertex->nx = vertex_1->nx * one_minus_t + vertex_2->nx * t;
	vertex->ny = vertex_1->ny * one_minus_t + vertex_2->ny * t;
	vertex->nz = vertex_1->nz * one_minus_t + vertex_2->nz * t;
	NORMALIZE_XYZ (vertex);
	return (vertex);

} /* cubes_surface_intersect */
