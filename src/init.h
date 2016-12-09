/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      init.h

 * version:     1.3 07/10/90 15:36:02

 * facility:
		Initializations for Marching Cubes externals
 * abstract:
	This module contains declarations for the Marching Cubes algorithm

 * environment:

 * author: W. Lorensen, creation date: 5/16/85

 * modified by:

	, : version
   01    -

 *
 */

/*
 * include files:
 */

/*
 * macros:
 */

/*
 * own storage:
 */

/*
 * external references:
 */

CONNECTIVITY *connectivity;
CUBE_EDGES *new_cube;
CUBE_EDGES *old_cube;
FILE    *slice_file;
FILE    *vertex_file;
LINE_EDGES *new_line;
LINE_EDGES *old_line;
PIXEL	data_mask = 0xffff;
PIXEL   *image;
SOLID	*solids = NULL;
SURVIVOR *survivors = NULL;
VERTEX  *vertex;
float	aspect_xy_to_z;
int     line;
int     pixel;
int     slice;

static float x_min = 1.e30;
static float x_max = -1.e30;

static float y_min = 1.e30;
static float y_max = -1.e30;

static float z_min = 1.e30;
static float z_max = -1.e30;

/*
 * command line and command file options:
 */

char	command_file[80] = '\0';/* -command=	*/
char    output_prefix[80];	/* -output=	*/
char    slice_filename[80];	/* -input=	*/
float   value = .5;		/* -value=	*/
PIXEL	mask = 0;		/* -mask=	*/
float   scale_x = 1.0;		/* -scale=	*/
float   scale_y = 1.0;
float   scale_z = 1.0;
int     debug = 0;		/* -debug	*/
int     interactive = 1;	/* -go		*/
int     pixels_per_line = 12;	/* resolution=	*/
int     lines_per_slice = 12;
int     number_slices = 1;
int     start_slice = 1;	/* start=	*/
int     end_slice = 0;
int	increment_slice = 1;
int     start_x = 0;
int     end_x = -1;
int     start_y = 0;
int     end_y = -1;
int     text_mode = 0;		/* -text	*/
