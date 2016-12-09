/*
 * copyright (c) 1986 general electric company
 */

/*++

 * module:      parser.c

 * version:     1.3 03/21/89 07:13:23

 * facility:
		Marching Cubes triangle generator for sampled data
 * abstract:
	This module parses command files

 * environment:
	VAX VMS

 * author:      , creation date:
	W. Lorensen     2/20/86

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
#define MINMATCH	4
#define COMMENT		(strncmp (keyword, "!",MINMATCH) == 0)
#define DEBUG		(strncmp (keyword, "debug",MINMATCH) == 0)
#define DATAMASK	(strncmp (keyword, "datamask",MINMATCH) == 0)
#define GO		(strncmp (keyword, "go",MINMATCH) == 0)
#define HELP		(strncmp (keyword, "help",MINMATCH) == 0)
#define INPUT		(strncmp (keyword, "input",MINMATCH) == 0)
#define OUTPUT		(strncmp (keyword, "output",MINMATCH) == 0)
#define RESOLUTION	(strncmp (keyword, "resolution",MINMATCH) == 0)
#define SCALE		(strncmp (keyword, "scale",MINMATCH) == 0)
#define START		(strncmp (keyword, "start",MINMATCH) == 0)
#define END		(strncmp (keyword, "end",MINMATCH) == 0)
#define VALUE		(strncmp (keyword, "value",MINMATCH) == 0)
#define MIN		(strncmp (keyword, "minimum",MINMATCH) == 0)
#define MAX		(strncmp (keyword, "maximum",MINMATCH) == 0)

#define CUT		(strncmp (keyword, "cut",MINMATCH) == 0)
#define CAP		(strncmp (keyword, "cap",MINMATCH) == 0)
#define GRADIENT	(strncmp (keyword, "gradient",MINMATCH) == 0)
#define REFORMAT	(strncmp (keyword, "reformat",MINMATCH) == 0)
#define SURFACE		(strncmp (keyword, "surface",MINMATCH) == 0)
#define SPHERE		(strncmp (keyword, "sphere",MINMATCH) == 0)
#define OCTANT		(strncmp (keyword, "octant",MINMATCH) == 0)
#define MASK		(strncmp (keyword, "mask",MINMATCH) == 0)
#define SIGNA		(strncmp (keyword, "signa",MINMATCH) == 0)
#define XIM		(strncmp (keyword, "xim",MINMATCH) == 0)
#define CT8800		(strncmp (keyword, "8800",MINMATCH) == 0)
#define CT9800		(strncmp (keyword, "9800",MINMATCH) == 0)
#define CCT9800		(strncmp (keyword, "c9800",MINMATCH) == 0)
#define FLAT		(strncmp (keyword, "flat",MINMATCH) == 0)

/*
 * own storage:
 */

/*
 * external references:
 */

extern int      interactive;
extern float	value;
extern PIXEL	mask;
extern int      debug;
extern PIXEL	data_mask;
extern int      lines_per_slice;
extern int      number_slices;
extern int      pixels_per_line;
extern float    scale_x;
extern float    scale_y;
extern float    scale_z;
extern int      start_slice;
extern int      end_slice;
extern int      start_x;
extern int      end_x;
extern int      start_y;
extern int      end_y;
extern float	value;
extern char     slice_filename[];
extern char     output_prefix[];


/*++
 *
 * routine cubes_parse_commands (command_file)

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

cubes_parse_commands (command_ptr)

    FILE *command_ptr;
{
#define MAX_LINE	255
	char	file[80];
	char	keyword[80];
	char	line[MAX_LINE];
	char	*line_ptr;
	char	name[80];
	float	nx, ny, nz;
	float	x, y, z;
	int	int_mask;
	int	min, max;
	int	number;

	/*
	 * keep reading file until we're done
	 */

	while (fgets (line, MAX_LINE, command_ptr)) {

		line_ptr = line;

		/*
		 * pick up keyword field
		 */

		sscanf (line_ptr, "%s", keyword);
		line_ptr += strlen (keyword) + 1;

		/*
		 * dispatch off keyword
		 */

		if (COMMENT) {}

		else if (DEBUG) {
			debug = 1;
		}

		else if (GO) {
			interactive = 0;
		}

		else if (HELP) {
			cubes_help ();
		}

		else if (INPUT) {
			sscanf (line_ptr, "%s", slice_filename);
		}

		else if (FLAT) {
			cubes_set_2d_noheaders ();
		}

		else if (SIGNA) {
			cubes_set_signa ();
		}

		else if (CT8800) {
			cubes_set_8800 ();
		}

		else if (CT9800) {
			cubes_set_9800 ();
		}

		else if (CCT9800) {
			cubes_set_9800_compressed ();
		}

		else if (XIM) {
			cubes_set_xim ();
		}
		else if (OUTPUT) {
			sscanf (line_ptr, "%s", output_prefix);
		}
		else if (RESOLUTION) {
			sscanf (line_ptr, "%d%*c%d%*c%d", &pixels_per_line,
							&lines_per_slice,
							&number_slices);
		}
		else if (SCALE) {
			sscanf (line_ptr, "%f%*c%f%*c%f", &scale_x,
							&scale_y,
							&scale_z);
		}
		else if (MIN) {
			sscanf (line_ptr, "%d%*c%d%*c%d", &start_x,
							&start_y,
							&start_slice);
		}
		else if (MAX) {
			sscanf (line_ptr, "%d%*c%d%*c%d", &end_x,
							&end_y,
							&end_slice);
		}
		else if (DATAMASK) {
			sscanf (line_ptr, "%x", &int_mask);
			data_mask = int_mask;
		}
		else if (START) {
			sscanf (line_ptr, "%d", &start_slice);
		}
		else if (END) {
			sscanf (line_ptr, "%d", &end_slice);
		}
		else if (VALUE) {
			sscanf (line_ptr, "%f", &value);
			cubes_new_surface ("none", output_prefix, value);
		}
		else if (CAP) {
			sscanf (line_ptr, "%s%s%f%*c%f%*c%f%*c%f%*c%f%*c%f",
				name, file, &x, &y, &z, &nx, &ny, &nz);
			cubes_new_cap (name, file, x, y, z, nx, ny, nz);
		}
		else if (CUT) {
			sscanf (line_ptr, "%s%s%f%*c%f%*c%f%*c%f%*c%f%*c%f",
				name, file, &x, &y, &z, &nx, &ny, &nz);
			cubes_new_cut (name, file, x, y, z, nx, ny, nz);
		}
		else if (SPHERE) {
			min = 0;
			max = 0;
			sscanf (line_ptr, "%s%s%f%*c%f%*c%f%*c%f%*c%d%*c%d",
				name, file, &x, &y, &z, &nx, &min, &max);
			cubes_new_sphere (name, file, x, y, z, nx, min, max);
		      }
		else if (OCTANT) {
			min = 0;
			max = 0;
			sscanf (line_ptr, "%s%s%d%*c%f%*c%f%*c%f%*c%d%*c%d",
				name, file, &number, &x, &y, &z, &min, &max);
			cubes_new_octant (name, file, number, x, y, z, min, max);
		}
		else if (REFORMAT) {
			sscanf (line_ptr, "%s%s%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%d%*c%d",
				name, file, &x, &y, &z, &nx, &ny, &nz, &min, &max);
			cubes_new_reformat (name, file, x, y, z, nx, ny, nz,
						min, max);
		}
		else if (SURFACE) {
			sscanf (line_ptr, "%s%s%f", name, file, &value);
			cubes_new_surface (name, file, value);
		}
		else if (GRADIENT) {
			sscanf (line_ptr, "%s%s%d%*c%d", name, file, &min, &max);
			cubes_new_gradient (name, file, min, max);
		}
		else if (MASK) {
			sscanf (line_ptr, "%s%s%x", name, file, &int_mask);
			mask = int_mask;
			cubes_new_mask (name, file, mask);
		}
		else {
			fprintf (stderr, 
				"CUBES: unrecognized keyword %s\n", keyword);
		}
	}
}
