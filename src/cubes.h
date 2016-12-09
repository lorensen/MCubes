/*
 * copyright (c) 1985 general electric company
 */

/*++

 * module:      cubes.h

 * version:     1.5 02/18/92 08:23:52

 * facility:
		Marching Cubes
 * abstract:
	This header file contains general typedefs and macros for Marching
	Cubes.

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

#ifdef vms
#define LOCAL
#else
#define LOCAL static
#endif

#define MAX_NODES 1
#define MAX_CONNECTIVITY 15
#define DPRINT if (debug) printf

#define WRITE_VERTEX(vertex)\
	output_vertex = *(vertex);\
	output_vertex.x *= scale_x;\
	output_vertex.y *= scale_y;\
	output_vertex.z *= scale_z;\
	if (output_vertex.x < x_min) x_min = output_vertex.x;\
	if (output_vertex.x > x_max) x_max = output_vertex.x;\
	if (output_vertex.y < y_min) y_min = output_vertex.y;\
	if (output_vertex.y > y_max) y_max = output_vertex.y;\
	if (output_vertex.z < z_min) z_min = output_vertex.z;\
	if (output_vertex.z > z_max) z_max = output_vertex.z;\
	fwrite ((char *) &output_vertex, sizeof(output_vertex), 1, vertex_file)

/*
 * typedefs:
 */

typedef int EDGE_LIST;

typedef struct {
       EDGE_LIST edges[65];
} POLY_CASES;
 
typedef short PIXEL;

typedef int CONNECTIVITY;

typedef struct {
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
} VERTEX;

typedef int (*INDEX_PROCEDURE) ();
typedef VERTEX (*EDGE_PROCEDURE)();

typedef	int	*OBJECT;

typedef struct solid {
	struct solid *solid_next;	/* next solid */
	char	*solid_name;		/* name of solid */
	char	*solid_file;		/* name of file to hold triangles */
	FILE	*solid_file_ptr;	/* file pointer to file */
	int	solid_visible;		/* visibility flag */
	INDEX_PROCEDURE solid_index;	/* build index procedure */
	EDGE_PROCEDURE *solid_interpolate;/* interpolation procedure */
	int	(*solid_inside) ();	/* inside procedure */
	VERTEX  *(*solid_intersect) ();	/* solid intersection procedure */
	OBJECT	*solid_object;		/* pointer to object decription */
} SOLID;

typedef struct {
	SOLID	*solid;			/* pointer to surviving solid */
	int	index;			/* index for this survivor */
	int	vertex_count;		/* number of verticies for survivor */
	VERTEX	vertices[50];		/* vertices for sruvivor */
} SURVIVOR;

/*
 * edge data structures
 */

typedef struct {
	VERTEX	cube_edge_4;
	VERTEX	cube_edge_8;
	VERTEX	cube_edge_9;
	VERTEX	cube_edge_11;
} CUBE_EDGES;

typedef struct {
	VERTEX	line_edge_1;
	VERTEX	line_edge_5;
	VERTEX	line_edge_9;
	VERTEX	line_edge_10;
} LINE_EDGES;

/*
 * own storage:
 */

/*
 * external references:
 */

extern char *malloc ();
extern char *strcpy ();
