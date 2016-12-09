/*
 * copyright (c) 1984 general electric company
 */

/*++

 * module:      cubes.c

 * version:     1.6 04/26/89 07:44:48

 * facility:
		Marching Cubes triangle generator for sampled data
 * abstract:
	This program generates triangles from sampled 3D data which
	is stored in text or memory mappable files on disk

 * environment:
	VAX VMS

 * author:      , creation date:
	W. Lorensen     7/11/84

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
#include "init.h"

/*
 * macros:
 */

#define OUTSIDE	0
#define INSIDE	255

#ifdef u370
#define BLOCK_SIZE 4096
#else
#define BLOCK_SIZE 512
#endif

/*
 * own storage:
 */

static	char	file_name[80];
static VERTEX	output_vertex;
static	int	number_edges = 0;
#ifndef lint
static	char    *sccs_id = "@(#)cubes.c	1.6";
#endif
static	int *save_ptr;
static	int	file_number = 0;

/*
 * external references:
 */

PIXEL *cubes_get_slice ();
CONNECTIVITY *cubes_triangulate ();
extern POLY_CASES poly_cases[];
PIXEL *slice_0;		/* pointer to slice i - 1		*/
PIXEL *slice_1;		/* pointer to slice i			*/
PIXEL *slice_2;		/* pointer to slice i + 1		*/
PIXEL *slice_3;		/* pointer to slice i + 2		*/

main (argc, argv)

    int argc;
    char **argv;
{
	FILE	*command_ptr;

	/*
	 * advertise a little
	 */

	fprintf (stderr, "\nC U B E S - Marching Cubes Method for 3D Surface Construction v%s %s\n", "1.6", "04/26/89");

	/*
	 * user input can come from three sources
	 *
	 *	1) command line
	 *	2) command file
	 *	3) interactive menu


	 * 1) process command line
	 */

	cubes_options (argc, argv);

	/*
	 * 2) process command file
	 */

	if (command_file[0] != '\0') {
		command_ptr = fopen (command_file, "r");
		if (command_ptr == NULL) {
			fprintf (stderr,
				"CUBES: cannot open command file %s\n",
				command_file);
			perror ("CUBES");
			exit (1);
		}
		cubes_parse_commands (command_ptr);
	}

	/*
	 * write limits
	 */

	cubes_limits ();

	/*
	 * initialize cpu timer
	 */

	timer_init ();

	/*
	 * initialize tables, etc.
	 */

	cubes_init ();

	/*
	 * generate triangles
	 */

	cubes_generate ();

	/*
	 * write range of vertices
	 */

	cubes_range ();

	/*
	 * summarize the run, once on standard output, once in .sum file
	 */

	cubes_summary ();

	/*
	 * close vertex file
	 */

	fclose (vertex_file);
}


/*++
 *
 * routine cubes_init ()

 * functional description:

	initializes cubes for processing of CT or NMR data files

 * formal parameters:
	none

 * implicit inputs:
	none

 * implicit outputs:

 * routine value:
 * completion codes:
	none

 * side effects:
	none

 *
 */

cubes_init ()

{

	/*
	 * get memory for survivors
	 */

#define MAX_SURVIVORS 20
	if ((survivors = 
		(SURVIVOR *) malloc ((unsigned) (MAX_SURVIVORS *
			sizeof(SURVIVOR)))) == NULL) {
		fprintf (stderr, "CUBES: Insufficient memory for survivors\n");
		exit (1);
	}

	/*
	 * get memory for node and normal
	 */

	if ((vertex =
		(VERTEX *) malloc ((unsigned) (sizeof (VERTEX)))) == NULL) {
		fprintf (stderr, "CUBES: Insufficient memory for vertex\n");
		exit (1);
	}

	/*
	 * get memory to save last and current cube and line
	 */

	if ((old_cube =
		(CUBE_EDGES *) malloc ((unsigned) (sizeof (CUBE_EDGES))))
		== NULL) {
		fprintf (stderr,
			"CUBES: Insufficient memory for old cube edges\n");
		exit (1);
	}

	if ((new_cube =
		(CUBE_EDGES *) malloc ((unsigned) (sizeof (CUBE_EDGES))))
		== NULL) {
		fprintf (stderr,
			"CUBES: Insufficient memory for new cube edges\n");
		exit (1);
	}

	if ((old_line =
		(LINE_EDGES *) malloc ((unsigned) (pixels_per_line *
		sizeof (LINE_EDGES)))) == NULL) {
		fprintf (stderr,
			"CUBES: Insufficient memory for old line edges\n");
		exit (1);
	}

	if ((new_line =
		(LINE_EDGES *) malloc ((unsigned) (pixels_per_line *
		sizeof (LINE_EDGES)))) == NULL) {
		fprintf (stderr,
			"CUBES: Insufficient memory for new line edges\n");
		exit (1);
	}

	/*
	 * open file for triangles
	 */

	sprintf (file_name, "%s.tri", output_prefix);
#ifdef vms
	vertex_file = fopen (file_name, "wb");
#else
	vertex_file = fopen (file_name, "w");
#endif
	if (vertex_file == NULL) {
		fprintf (stderr, "CUBES: cannot open vertex file %s\n", file_name);
		exit (1);
	}

}


/*++
 *
 * routine cubes_generate ()

 * functional description:
	generates triangles from sampled data

 * formal parameters:
	none

 * implicit inputs:
	lines_per_slice - number of lines per slice
	number_slices   - number of slices
	pixels_per_line - number of pixels per line

 * implicit outputs:
	line            - number of current line
	pixel           - number of current pixel
	slice           - number of current slice

 * routine value:
 * completion codes:
	none

 * side effects:
	none

 *
 */

cubes_generate ()

{
register PIXEL *slice_0_ptr;	/* current pointer within slice i - 1	*/
register PIXEL *slice_1_ptr;	/* current pointer within slice i	*/
register PIXEL *slice_2_ptr;	/* current pointer within slice i + 1	*/
register PIXEL *slice_3_ptr;	/* current pointer within slice i + 2	*/
	int	line_offset;

	/*
	 * set mins and maxs if not already done
	 */

	if (start_x < 0) start_x = 0;
	if (start_y < 0) start_y = 0;
	if (start_slice < 1) start_slice = 1;

	if (end_x > pixels_per_line) end_x = pixels_per_line;
	if (end_x <= start_x) end_x = pixels_per_line;
	end_x -= 2;

	if (end_y > lines_per_slice) end_y = lines_per_slice;
	if (end_y <= start_y) end_y = lines_per_slice;
	end_y -= 2;

	if (end_slice <= start_slice) end_slice = number_slices;

	/*
	 * calculate aspect ratio between xy and z
	 */

	aspect_xy_to_z = scale_z / scale_x;

	/*
	 * get pointer for first three slices to start things off
	 */

	slice_1 = (PIXEL *) cubes_get_slice (start_slice);
	slice_2 = (PIXEL *) cubes_get_slice (start_slice + 1);
	if (start_slice <= 1) slice_0 = slice_1;
	else slice_0 = (PIXEL *) cubes_get_slice (start_slice - 1);

	/*
	 * loop through slices and make triangles
	 */

	for (slice = start_slice;
		slice < end_slice;
		slice++) {

		/*
		 * get i + 2
		 */

		if (slice < (number_slices - 1)) {
			slice_3 = (PIXEL *) cubes_get_slice (slice + 2);
		}
		else {
			slice_3 = slice_2;
		}

		/*
		 * march across both slices, building triangles
		 */

		for (line = start_y;
			line < end_y;
			line ++) {

			/*
			 * set slice pointers to start of line
			 */

			line_offset = pixels_per_line * line;
			slice_0_ptr = slice_0 + line_offset + start_x;
			slice_1_ptr = slice_1 + line_offset + start_x;
			slice_2_ptr = slice_2 + line_offset + start_x;
			slice_3_ptr = slice_3 + line_offset + start_x;

			for (pixel = start_x;
				pixel < end_x;
					pixel++,
	      				slice_0_ptr++,
					slice_1_ptr++,
					slice_2_ptr++,
					slice_3_ptr++) {

				/*
				 * generate triangles for this cube and this case
				 */

				cubes_triangulate (	slice_0_ptr,
							slice_1_ptr,
							slice_2_ptr,
							slice_3_ptr);

			} /* end of line */

		} /* end of slice */

		/*
		 * move slice pointers
		 */

		slice_0 = slice_1;
		slice_1 = slice_2;
		slice_2 = slice_3;
	}

}


/*++
 *
 * routine cubes_triangulate (slice_0, slice_1, slice_2, slice_3)

 * functional description:
	produces the connectivity for a cube based on its index

 * formal parameters:
	slice_0 - pointer to pixel on slice i - 1
	slice_1 - pointer to pixel on slice i
	slice_2 - pointer to pixel on slice i + 1
	slice_3 - pointer to pixel on slice i + 2

 * implicit inputs:
	poly_cases - pointer to structure containing 256 cases for the cube

 * implicit outputs:
	number_elements - running count of number of triangles

 * routine value:
	returns next available location in connectivity

 * completion codes:
	none

 * side effects:
	none

 *
 */

CONNECTIVITY *
cubes_triangulate (slice_0, slice_1, slice_2, slice_3)

    PIXEL *slice_0;
    PIXEL *slice_1;
    PIXEL *slice_2;
    PIXEL *slice_3;
{
	EDGE_PROCEDURE *interpolate;
	POLY_CASES *poly_case;
	SOLID	*solid_ptr;
	SOLID	*surviving_solid;
	SURVIVOR *last_survivor;
register SURVIVOR *survivor;
	VERTEX	*vertex_ptr;
	int	number_survivors;
	int     index;
register EDGE_LIST *edge;

	/*
	 * build an index for each solid from the eight neighbors
	 */

	for (survivor = survivors, solid_ptr = solids;
	     solid_ptr != NULL;
	     solid_ptr = solid_ptr->solid_next) {
		index = (*solid_ptr->solid_index) (solid_ptr->solid_object, slice_1, slice_2, slice_3);
		if (index == OUTSIDE) {
			survivor = survivors;
			return;
		}
		if (index == INSIDE) continue;
		survivor->solid = solid_ptr;
		survivor->index = index;
		survivor++;
	}

	/*
	 * for each surviving solid, find vertices of triangles
	 */

	number_survivors = survivor - survivors;
	last_survivor = survivor;
	for (survivor = survivors;
	     survivor < last_survivor;
	     survivor++) {

		survivor->vertex_count = 0;
		surviving_solid = survivor->solid;

		if (surviving_solid->solid_interpolate == NULL) continue;

		/*
		 * access the proper set of triangles for this case
		 */

		poly_case = poly_cases + survivor->index;
		edge = poly_case->edges;
		vertex_ptr = survivor->vertices;

		while (*edge != 0) {

			/*
			 * interpolate using the proper solid interpolation
			 * routines for the selected edge
			 */

			interpolate = surviving_solid->solid_interpolate +
					*edge;
			*vertex_ptr = (**interpolate)
				(surviving_solid->solid_object,
				slice_0, slice_1, slice_2, slice_3);
			vertex_ptr++;
			survivor->vertex_count++;

			/*
			 * if the next edge is 0, we've completed another
			 * triangle
			 */

			edge++;
			if (*edge == 0) {
				edge++;
				continue;
			}

		} /* end edge loop */

	} /* end survivor loop */

	/*
	 * at this point, all surviving solids have been interpolated.
	 * now, we must clip each triangle against the triangles of
	 * the other survivors
	 */

	if (number_survivors > 1) {
		cubes_clip_solids (number_survivors, survivors);
	}

	/*
	 * output the vertices
	 */

	cubes_output_vertices (number_survivors, survivors);

} /* cubes_triangulate */


/*++
 *
 * routine cubes_open ()

 * functional description:
	open text and geometry files

 * formal parameters:
	none

 * implicit inputs:
	output_prefix  - prefix for output files
	slice_filename - slice file name
	text_mode      - text mode indicator

 * implicit outputs:
	limits_file - file pointer for limits file
	slice_file  - file pointer to slice file (text mode only)

 * routine value:
 * completion codes:
	error exit if files cannot be opened

 * side effects:
	none

 *
 */

cubes_limits ()
{
	FILE	*limits_file;
	float	x_min, x_max;
	float	y_min, y_max;
	float	z_min, z_max;

	/*
	 * output limits file with ranges
	 */

	sprintf (file_name, "%s.lim", output_prefix);
#ifdef vms
	limits_file = fopen (file_name, "wb");
#else
	limits_file = fopen (file_name, "w");
#endif
	if (limits_file == NULL) {
		fprintf (stderr, "CUBES: cannot open limits file %s\n", file_name);
		exit (1);
	}
	
	/*
	 * write x, y, z ranges into limits file
	 */

	x_min = scale_x;
	x_max = scale_x * pixels_per_line;
	y_min = scale_y;
	y_max = scale_y * lines_per_slice;
	z_min = scale_z;
	z_max = scale_z * number_slices;

	fwrite ((char *) &x_min, sizeof(float), 1, limits_file);
	fwrite ((char *) &x_max, sizeof(float), 1, limits_file);
	fwrite ((char *) &y_min, sizeof(float), 1, limits_file);
	fwrite ((char *) &y_max, sizeof(float), 1, limits_file);
	fwrite ((char *) &z_min, sizeof(float), 1, limits_file);
	fwrite ((char *) &z_max, sizeof(float), 1, limits_file);
	fclose (limits_file);
}


/*++
 *
 * routine cubes_range ()

 * functional description:
	write the actual range of x, y, z in the limits file

 * formal parameters:
	none

 * implicit inputs:
	output_prefix  - prefix for output files
	slice_filename - slice file name

 * implicit outputs:
	limits_file - file pointer for limits file

 * routine value:
 * completion codes:
	error exit if files cannot be opened

 * side effects:
	none

 *
 */

cubes_range ()
{
	FILE	*limits_file;

	/*
	 * output limits file with ranges
	 */

	sprintf (file_name, "%s.lim", output_prefix);
#ifdef vms
	limits_file = fopen (file_name, "ab");
#else
	limits_file = fopen (file_name, "a");
#endif
	if (limits_file == NULL) {
		fprintf (stderr, "CUBES: cannot open limits file %s\n", file_name);
		exit (1);
	}
	
	/*
	 * write x, y, z ranges into limits file
	 */

	fwrite ((char *) &x_min, sizeof(float), 1, limits_file);
	fwrite ((char *) &x_max, sizeof(float), 1, limits_file);
	fwrite ((char *) &y_min, sizeof(float), 1, limits_file);
	fwrite ((char *) &y_max, sizeof(float), 1, limits_file);
	fwrite ((char *) &z_min, sizeof(float), 1, limits_file);
	fwrite ((char *) &z_max, sizeof(float), 1, limits_file);
	fclose (limits_file);
}


/*++
 *
 * routine cubes_summary ()

 * functional description:
	summarize the run once on standard output, once on prefix.sum

 * formal parameters:
	none

 * implicit inputs:
	none

 * implicit outputs:
	none

 * routine value:

 * completion codes:
	none

 *
 */
cubes_summary ()
{
	int	i;
	int	total_pixels;
	float	cpu_time;
	FILE	*summary_ptr;
	char	summary_file[80];

	timer_get_cpu (&cpu_time);

	for (summary_ptr = stdout, i = 0; i < 2; i++) {
		fprintf (summary_ptr, "Summary of Marching Cubes run\n");
		fprintf (summary_ptr, "    input\n");
		fprintf (summary_ptr, "	Slice prefix:	%s\n", slice_filename);
		fprintf (summary_ptr, "	# of slices:	%d\n", number_slices);
		fprintf (summary_ptr, "	x,y resolution:	%d,%d\n", pixels_per_line, lines_per_slice);
		fprintf (summary_ptr, "	surface value:	%f\n", value);
		fprintf (summary_ptr, "	connectivity mask: 0x%x\n", mask);
		fprintf (summary_ptr, "    output\n");
		fprintf (summary_ptr, "	# of vertices:	%d\n", number_edges);
		fprintf (summary_ptr, "	# of elements:	%d\n", number_edges / 3);
		fprintf (summary_ptr, "	limits file:	%s.lim\n", output_prefix);
		fprintf (summary_ptr, "	triangle file:	%s.tri\n", output_prefix);
		fprintf (summary_ptr, "    statistics\n");
		total_pixels = number_slices * pixels_per_line * lines_per_slice;
		fprintf (summary_ptr, "	# of pixels:	%d\n", total_pixels);
		fprintf (summary_ptr, "	cpu time:	%.2f seconds\n", cpu_time);
		fprintf (summary_ptr, "	speed:		%.2f pixels / second\n", (float) total_pixels / cpu_time);

		/*
		 * open summary file
		 */

		if (i == 1) break;
		sprintf (summary_file, "%s.sum", output_prefix);
		summary_ptr = fopen (summary_file, "w");
		if (summary_ptr == NULL) {
			fprintf (stderr, "cubes: cannot open summary file %s\n", summary_file);
			break;
		}
	}
}


/*++
 *
 * routine cubes_output_vertices (number_survivors, survivors)

 * functional description:
	Outputs vertices from each surviving solid

 * formal parameters:
	input
	    int number_survivors - number of survivors
	    SURVIVOR *survivors - pointer to survivors

 * implicit inputs:
	int number_edges - number of edges output so far
	FILE *vertex_file - file pointer for vertex file

 * implicit outputs:
	int number_edges - number of edges output so far

 * routine value:

 * completion codes:
	none

 *
 */

cubes_output_vertices (number_survivors, survivors)
    int number_survivors;
    SURVIVOR *survivors;
{
	SURVIVOR *last_survivor;
	SURVIVOR *survivor;
	VERTEX	 *last_vertex;
	VERTEX	 *vertex;
	int	 vertex_count;

	last_survivor = survivors + number_survivors;

	for (survivor = survivors;
	     survivor < last_survivor;
	     survivor++) {

		if (survivor->solid->solid_visible == 0) continue;

		vertex_count = survivor->vertex_count;
		number_edges += vertex_count;

		/*
		 * output this survivors vertices
		 */

		last_vertex = survivor->vertices + vertex_count;
		for (vertex = survivor->vertices;
		     vertex < last_vertex;
		     vertex++) {

			WRITE_VERTEX (vertex);
		}
	}
}
