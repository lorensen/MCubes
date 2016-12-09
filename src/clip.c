/*
 * copyright (c) 1986 general electric company
 */

/*++

 * module:      clip.c

 * version:     1.3 09/01/88 14:12:52

 * facility:
	Marching Cubes triangle generator for sampled data

 * abstract:
	This module contains clipping routines

 * environment:
	VAX VMS and 4.2BSD Unix

 * author: W. Lorensen, creation date: 3/4/86

 * modified by:

	, : version
   01    -

 *
 */

/*
 * include files:
 */

#include <stdio.h>
#include "cubes.h"

/*
 * macros:
 */

typedef struct {
	VERTEX vertex_1;
	VERTEX vertex_2;
	VERTEX vertex_3;
} TRIANGLE;

/*
 * own storage:
 */

#ifndef lint
static	char    *sccs_id = "@(#)clip.c	1.3";
#endif

LOCAL int cubes_clip_boundary ();

static VERTEX local_vertex;
static VERTEX *vertex = &local_vertex;
static VERTEX clipped_vertices[100];
static VERTEX in[50];
static VERTEX out[50];
static float intercept;
static float normal_x;
static float normal_y;
static float normal_z;
static int number_in;
static int number_out;

/*
 * external references:
 */

extern double	sqrt ();
extern int pixel, line, slice;
/*++
 *
 * routine cubes_clip_solids (number_survivors, survivors)

 * functional description:
	Clips all surviving triangles against all others

 * formal parameters:
	input
	    int number_survivors - number of surviving solids
	    SURVIVOR *survivors - pointer to survivors

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:

 * completion codes:
	none

 *
 */

cubes_clip_solids (number_survivors, survivors)
    int number_survivors;
    SURVIVOR *survivors;
{
	SURVIVOR *clipper;
	SURVIVOR *last_survivor = survivors + number_survivors;
	SURVIVOR *survivor;
	TRIANGLE *last_triangle;
	TRIANGLE *triangle;
	VERTEX	 *clipped;
	VERTEX	 *vertex;
	VERTEX	 *last_clipped;
	int	clipped_something;
	int	i;

	/*
	 * for each survivor
	 */

	for (survivor = survivors;
	     survivor < last_survivor;
	     survivor++) {

/*		survivor_print ("clippee:", survivor);*/

		/*
		 * no need to clip triangles of surfaces that are not visible
		 */

		if (survivor->solid->solid_visible == 0) continue;

		last_triangle = (TRIANGLE *) survivor->vertices
				+ survivor->vertex_count / 3;

		clipped = clipped_vertices;
		clipped_something = 0;

		/*
		 * for each triangle in a survivor
		 */

		for (triangle = (TRIANGLE *) survivor->vertices;
		     triangle < last_triangle;
		     triangle ++) {

			*(in)     = triangle->vertex_1;
			*(in + 1) = triangle->vertex_2;
			*(in + 2) = triangle->vertex_3;
			number_in = 3;

			/*
			 * for each clipper
			 */

			for (clipper = survivors;
			     clipper < last_survivor;
			     clipper++) {

				/*
				 * no need to clip against one's self
				 */

				if (clipper == survivor) continue;

				/*
				 * can't clip against volumes without an
				 * intersection routine
				 */

				if (clipper->solid->solid_intersect == NULL ) continue;
				clipped_something = 1;

				/*
				 * clip the triangle
				 */

				cubes_clip_boundary (
						clipper->solid->solid_object,
						&number_in, in,
						&number_out, out,
						clipper->solid->solid_inside,
						clipper->solid->solid_intersect);

				/*
				 * copy output vertices to input
				 * for next pass
				 */

				cubes_copy_out_to_in (
						&number_in, in,
						&number_out, out);
			
			} /* for each clipper*/

			/*
			 * at this point, a triangle has been
			 * clipped against a given surviving solid.
			 * since something other than a
			 * triangle is possible after this clipping,
			 * we need to decompose it into triangles
			 */

			if (number_out > 3) {
				cubes_breakup_poly (&number_out, out);
			}

			/*
			 * save the output vertices for later
			 */

			for (i = 0; i < number_out; i++) {
				*(clipped++) = *(out + i);
			}
		} /* for each triangle */

		/*
		 * all triangles for a given solid have been clipped by
		 * the other solids. now we can transfer
		 * these clipped triangles into the survivor's vertices.
		 */

		if (clipped_something) {
			last_clipped = clipped;
			survivor->vertex_count = last_clipped - clipped_vertices;
			vertex = survivor->vertices;
			for (clipped = clipped_vertices;
			     clipped < last_clipped; ) {
				*(vertex++) = *(clipped++);
			}
/*			survivor_print ("after clipping:", survivor);*/
		}
	} /* for each survivor */

} /* cubes_clip_solids */


/*++
 *
 * routine cubes_clip_boundary (number_in, polygon_in, number_out, polygon_out)

 * functional description:
	Clips a polygon to a given clip boundary

 * formal parameters:
	input
	    int *number_in - pointer to number of vertices in input polygon
	    VERTEX *polygon_in - pointer to input polygon

	output
	    int *number_out - pointer to number of vertices in clipped polygon
	    VERTEX *polygon_out - pointer to clipped polygon

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:

 * completion codes:
	none

 *
 */

LOCAL int cubes_clip_boundary (object, 
	number_in, polygon_in, number_out, polygon_out,
	inside, intersect)

    OBJECT *object;
    int *number_in;
    VERTEX *polygon_in;
    int *number_out;
    VERTEX *polygon_out;
    int     (*inside) ();
    VERTEX *(*intersect) ();
{
	VERTEX *in = polygon_in;
	VERTEX *out = polygon_out;
	VERTEX *vertex_1;
	VERTEX *vertex_2;
	int     i;

	/*
	 * start with last vertex
	 */

	vertex_1 = in + (*number_in - 1);

	/*
	 * clip each edge of polygon
	 */

	for (i = 0; i < *number_in; i++) {

		vertex_2 = in++;

		if ((*inside) (object, vertex_2)) {
			if ((*inside) (object, vertex_1)) {

				/* both inside, output 2 */

				*(out++) = *vertex_2;
			}
			else {

				/* 2 in, 1 out, output intersect & 2 */

				*(out++) = *(*intersect) (object, vertex_1, vertex_2);
				*(out++) = *vertex_2;
			}
		}
		else {
			if ((*inside) (object, vertex_1)) {

				/* 2 out, 1 in, output intersect */

				*(out++) = *(*intersect) (object, vertex_1, vertex_2);
			}
				/* both out, output nothing */
		}

		/*
		 * make second vertex first
		 */

		vertex_1 = vertex_2;
	}

	*number_out = out - polygon_out;

} /* cubes_clip_boundary */

cubes_copy_out_to_in (number_in, in, number_out, out)
    int *number_in;
    VERTEX *in;
    int *number_out;
    VERTEX *out;
{
	int	i;

	for (i = 0; i < *number_out; i++) {
		*(in++) = *(out++);
	}
	*number_in = *number_out;
}

cubes_breakup_poly (number, vertices)
    int *number;
    VERTEX *vertices;
{
	VERTEX	save_4, save_5;

	switch (*number) {

	case 4:

		/*
		 * simple breakup strategy
		 * just break into two triangles
		 */

		*(vertices + 4) = *(vertices + 0);
		*(vertices + 5) = *(vertices + 2);
		*number = 6;
		break;

	case 5:
		/*
		 * simple breakup strategy
		 * just break into three triangles
		 */

		save_4 = *(vertices + 4);

		*(vertices + 4) = *(vertices + 0);
		*(vertices + 5) = *(vertices + 2);

		*(vertices + 6) = save_4;
		*(vertices + 7) = *(vertices + 0);
		*(vertices + 8) = *(vertices + 3);
		*number = 9;
		break;

	case 6:
		save_4 = *(vertices + 4);
		save_5 = *(vertices + 5);
		*(vertices + 4) = *(vertices + 0);
		*(vertices + 5) = *(vertices + 2);

		*(vertices + 6) = save_4;
		*(vertices + 7) = *(vertices + 0);
		*(vertices + 8) = *(vertices + 3);

		*(vertices + 9) = save_5;
		*(vertices + 10) = *(vertices + 0);
		*(vertices + 11) = save_4;
		*number = 12;
		break;

	default:
		printf ("breakup: cannot breakup poly with %d sides\n", *number);
		*number = 0;
	}
}

survivor_print (text, survivor)
    char *text;
    SURVIVOR *survivor;
{
	int	i;

	printf ("SURVIVOR (%s) pixel,line,slice (%d,%d,%d)\n", text, pixel, line, slice);
	printf ("\tname: %s\n", survivor->solid->solid_name);
	printf ("\tvertex count: %d\n", survivor->vertex_count);
	for (i = 0; i < survivor->vertex_count; i++) {
		printf ("\tvertex %d: (%f,%f,%f)\n", i,
			survivor->vertices[i].x,
			survivor->vertices[i].y,
			survivor->vertices[i].z);
	}
}
