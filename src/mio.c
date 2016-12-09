/*
 * Copyright (c) 1987 general electric company
 */

/*++

 * Module:      mio.c

 * Version:     4.4 12/19/89 10:47:40

 * Facility:	Medical I/O routine

 * Abstract:	These routines provide i/o support for a variety of medical
		input files.

		Currently supports:
			noh2d - 2d files, noheaders, VAX byte order
			signa - signa files with headers
			ct9800 - ct 9800 files with headers
			xim - xim files with headers

		To support a new format, four routines must be provided:

		1) mio_set_xxx - to set the routines for the new format
		2) xxx_build_filename (prefix, number)
			builds a filename for image "number" given
			the file prefix "prefix"
		3) xxx_read_image (file, x_size, y_size, slice)
			reads a file with stream pointer "file" with
			x, y resolution = (x_size, y_size) into the
			area pointed to by "slice"
		4) xxx_rewrite_image (file, x_size, y_size, slice)
			rewrites a file with stream pointer "file" with
			x, y resolution = (x_size, y_size) from the area
			pointed to by "slice".

 * Environment: Sun Unix

 * Author: W. Lorensen, Creation date: 11/10/87

 * Modified by:
	, : version

 *
 */

/*
 * Include files:
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

/*
 * Macros:
 */

#define NORMAL 1
#define PIPE 2

/*
 * Typedefs
 */

typedef short PIXEL;
typedef struct {
	FILE	*stream;
	int	type;
} MYFILE;

/*
 * Own storage:
 */

#ifndef lint
static	char    *sccs_id = "@(#)mio.c	4.4";
#endif

typedef struct {
	int	header_offset;
	int	header_size;
	int	header_swab;
	char *(*build_filename) ();
	int (*read_image) ();
	int (*write_image) ();
} MIO;

static char *noh2d_build_filename ();
static int noh2d_read_image ();
static int noh2d_write_image ();

static MIO noh2d = {0, 0, 0,
		noh2d_build_filename,
		noh2d_read_image,
		noh2d_write_image};

static char *signa_build_filename ();
static int signa_read_image ();
static int signa_write_image ();

static MIO signa = {0, 14336, 0,
		signa_build_filename,
		signa_read_image,
		signa_write_image};

static char *xim_build_filename ();
static int xim_read_image ();
static int xim_write_image ();

static MIO xim = {0, 512, 1,
		xim_build_filename,
		xim_read_image,
		xim_write_image};

static char *ct9800_build_filename ();
static int ct9800_read_image ();
static int ct9800_write_image ();
static short *ct9800_build_line_table ();
static MIO ct9800 = {0, 3072, 0,
		ct9800_build_filename,
		ct9800_read_image,
		ct9800_write_image};


static void myclose ();
static MYFILE *myopen ();
static void myseek ();

/* set defaults */
static MIO *mio = &noh2d;

static char line[256];

static PIXEL min_pix = 35000;
static PIXEL max_pix = 0;

/*
 * External references:
 */

extern char *strrchr ();
extern char *strcpy ();
extern char *malloc ();
extern char *sprintf ();

/***************************************************
	Public routines
****************************************************/
PIXEL *
mio_read_image (prefix, number, x_size, y_size, image)
    char *prefix;
    int number;
    int x_size, y_size;
    PIXEL *image;
{
	MYFILE	*file;
	char	*file_name;

	/*
	 * if image memory is not provided, allocate some
	 */

	if (image == NULL) {
		image = (PIXEL *) malloc (sizeof (PIXEL) * x_size * y_size);
		if (image == NULL) return (image);
	}

	/*
	 * build file name
	 */

	file_name = (*(mio->build_filename)) (prefix, number);

	/*
	 * open the file for input
	 */

	file = myopen (file_name, "r");
	if (file == NULL) {
		perror ("mio_read_image");
		return ((PIXEL *) -1);
	}

	/*
	 * read the image
	 */

	(*(mio->read_image)) (file, x_size, y_size, image);

	/*
	 * close the file
	 */

	myclose (file);

	return (image);
}

int mio_header_size ()
{
	return (mio->header_size);
}

int mio_read_header (prefix, number, header)
    char *prefix;
    int number;
    caddr_t header;
{
	MYFILE	*file;
	char	*file_name;

	if (mio->header_size == 0) return (0);

	/*
	 * build file name
	 */

	file_name = (*(mio->build_filename)) (prefix, number);

	/*
	 * open the file for input
	 */

	file = myopen (file_name, "r");
	if (file == NULL) {
		perror ("mio_read_header");
		return (-1);
	}

	/*
	 * read the header
	 */

	myseek (file, (long) mio->header_offset, 0);
	if (fread (header, 1, mio->header_size, file->stream) != mio->header_size) {
		perror ("mio_read_header");
	}

	/*
	 * close the file
	 */

	myclose (file);

	return (0);
}

int mio_rewrite_image (prefix, number, x_size, y_size, image)
    char *prefix;
    int number;
    int x_size, y_size;
    PIXEL *image;
{
	MYFILE	*file;
	char	*file_name;
	caddr_t	header = NULL;

	/* build file name */

	file_name = (*(mio->build_filename)) (prefix, number);

	/* open the file for update */

	file = myopen (file_name, "r+");
	if (file == NULL) {
		perror ("mio_rewrite_image");
		return (-1);
	}

	/* if file is really a pipe, get header to write it out */

	if (file->type == PIPE && mio->header_size > 0) {
		header = (caddr_t) malloc (mio->header_size);
		mio_read_header (prefix, number, header);
	}

	/* write the image */

	(*(mio->write_image)) (file, x_size, y_size, image, header);

	/*
	 * close the file
	 */

	myclose (file);

	if (header == NULL) free (header);

	return (0);
}

int mio_write_image (prefix, number, x_size, y_size, image, header)
    char *prefix;
    int number;
    int x_size, y_size;
    PIXEL *image;
    caddr_t header;
{
	MYFILE	*file;
	char	*file_name;

	/*
	 * build file name
	 */

	file_name = (*(mio->build_filename)) (prefix, number);

	/*
	 * open the file for write
	 */

	file = myopen (file_name, "w");
	if (file == NULL) {
		perror ("mio_write_image");
		return (-1);
	}

	/*
	 * write the image and header
	 */

	(*(mio->write_image)) (file, x_size, y_size, image, header);

	/*
	 * close the file
	 */

	myclose (file);

	return (0);
}

mio_set_noh2d () {mio = &noh2d;}
mio_set_signa () {mio = &signa;}
mio_set_ct9800 () {mio = &ct9800;}
mio_set_xim () {mio = &xim;}

/***************************************************
	Private routines
****************************************************/
	/********************
	 * 2d, noheaders    *
	 ********************/
static char *
noh2d_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.%d", prefix, number);
	return (line);
}

static int
noh2d_read_image (file, x_size, y_size, slice)
    MYFILE *file;
    PIXEL *slice;
    int x_size, y_size;
{
	int	status;
	int	size = x_size * y_size;

	status = fread ((char *) slice, sizeof (PIXEL), size, file->stream);
#ifndef vms
	swab ((char *) slice, (char *) slice, size * sizeof (PIXEL));
#endif
	return (status);
}

static int
noh2d_write_image (file, x_size, y_size, slice, header)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *slice;
    caddr_t header;
{
	int	size = x_size * y_size;

#ifndef vms
	swab ((char *) slice, (char *) slice, size * sizeof (PIXEL));
#endif
	return (fwrite ((char *) slice, sizeof (PIXEL), size, file->stream));
}

	/********************
	 * 2d, signa headers *
	 ********************/
static char *
signa_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.%03d", prefix, number);
	return (line);
}

static int
signa_read_image (file, x_size, y_size, image)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *image;
{
	int	status;
	int	size = x_size * y_size;

	myseek (file, (long) (28 * 512), 0);
	status = fread ((char *) image, sizeof (PIXEL), size, file->stream);
#ifdef vms
	swab ((char *) image, (char *) image, size * sizeof (PIXEL));
#endif
	return (status);
}

static int
signa_write_image (file, x_size, y_size, image, header)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *image;
{
	int	size = x_size * y_size;

#ifdef vms
	swab ((char *) image, (char *) image, size * sizeof (PIXEL));
#endif
	if (header != NULL) fwrite ((char *) header, 1, 28 * 512, file->stream);
	else myseek (file, (long) (28 * 512), 0);
	return (fwrite ((char *) image, sizeof (PIXEL), size, file->stream));
}

	/********************
	 * 2d, xim headers *
	 ********************/
static char *
xim_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.%d", prefix, number);
	return (line);
}

static int
xim_read_image (file, x_size, y_size, image)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *image;
{
	int	status;
	int	size = x_size * y_size;
	int	bytes = sizeof (PIXEL) * size;

	myseek (file, (long) 512, 0);
	status = fread (image, bytes, 1, file->stream);
#ifndef vms
	swab ((char *) image, (char *) image, bytes);
#endif
	return (status);
}

static int
xim_write_image (file, x_size, y_size, image, header)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *image;
    caddr_t header;
{
	char	header_temp[512];
	int	size = x_size * y_size;

#ifndef vms
	swab ((char *) image, (char *) image, size * sizeof (PIXEL));
#endif
	if (header != NULL) fwrite ((char *) header, 1, 512, file->stream);

	return (fwrite ((char *) image, sizeof (PIXEL), size, file->stream));
}

	/********************
	 * 2d, 9800 headers *
	 ********************/
static char *
ct9800_build_filename (prefix, number)
    char *prefix;
    int number;
{
	char	*suffix;
	char	local[MAXPATHLEN];

	/*
	 * make a local copy of file prefix
	 */

	strcpy (local, prefix);

	/*
	 * strip off suffix, if there
	 */

	suffix = strrchr (local, '.');
	if (suffix == NULL) {
		suffix = "yp";
	}
	else {
		*suffix = '\0';
		suffix++;
	}

	/*
	 * build file name
	 */

	sprintf (line, "%s%c%01d.%s",
			local,
			number < 100 ? ('0' + number / 10) :
				       ('a' + (number / 10 - 10)),
			number % 10,
			suffix);

	return (line);
}

static int
ct9800_read_image (file, x_size, y_size, image)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *image;
{
	PIXEL	data[256];
	PIXEL	*data_end = data + 256;
	PIXEL	*data_ptr = data + 256;
	PIXEL	*ptr;
	PIXEL	*start, *end;
	int	half_resolution;
	int	i;
	short	*line;
	short	*line_table;
	int	size = x_size * y_size;
	int	status;

	min_pix = 10000;
	max_pix = 0;

	/*
	 * zero memory for image
	 */

#ifndef stellar
	bzero ((char *) image, size * sizeof (PIXEL));
#endif

	/*
	 * build the line table
	 */
	
	line_table = (short*) ct9800_build_line_table (file, x_size);
	line = line_table;

	/*
	 * seek to start of data
	 */

#define CT9_IMAGE_START (6 * 256 * sizeof (short))
 	myseek (file, (long) CT9_IMAGE_START, 0);

	/*
	 * build image
	 */

	half_resolution = x_size / 2;
	ptr = image;
	for (i = 0; i < y_size; i++, line++, ptr += x_size) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) {
			if (data_ptr >= data_end) {
				status = fread ((char *) data, sizeof (PIXEL), 256, file->stream);
				if (status != 256) printf ("bad status: %d\n", status);
#ifdef vms
				swab ((char *) data, (char *) data, 256 * sizeof (PIXEL));
#endif
				data_ptr = data;
			}
			*start = *data_ptr;
#ifdef DEBUG
			if (*start > max_pix) max_pix = *start;
			if (*start > 100 && *start < min_pix) min_pix = *start;
#endif
		}
	}
	free ((char *) line_table);
#ifdef DEBUG
	printf ("min is %d, max is %d\n", (unsigned int) min_pix, (unsigned int) max_pix);
#endif
	return (status);
}

static int
ct9800_write_image (file, x_size, y_size, image, header)
    MYFILE *file;
    int x_size, y_size;
    PIXEL *image;
    caddr_t header;
{
	PIXEL	data[256];
	PIXEL	*data_end = data + 256;
	PIXEL	*data_ptr = data;
	PIXEL	*ptr;
	PIXEL	*start, *end;
	int	half_resolution;
	int	i;
	short	*line;
	short	*line_table;

	/*
	 * build the line table
	 */
	
	line_table = (short*) ct9800_build_line_table (file, x_size);
	line = line_table;

	/*
	 * seek to start of data
	 */

#define CT9_IMAGE_START (6 * 256 * sizeof (short))
	if (header != NULL) fwrite (header, 1, CT9_IMAGE_START, file->stream);
	else myseek (file, (long) CT9_IMAGE_START, 0);

	/*
	 * store compressed image
	 */

	half_resolution = x_size / 2;
	ptr = image;
	for (i = 0; i < y_size; i++, line++, ptr += x_size) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) {
			if (data_ptr >= data_end) {
#ifdef vms
				swab ((char *) data, (char *) data, 256 * sizeof (PIXEL));
#endif
				fwrite ((char *) data, sizeof (PIXEL), 256, file->stream);
				data_ptr = data;
			}
			*data_ptr = *start;
		}
	}

	if (data_ptr != data) {
#ifdef vms
		swab ((char *) data, (char *) data, (data_ptr - data) * sizeof (PIXEL));
#endif
		fwrite ((char *) data, sizeof (PIXEL), data_ptr - data, file->stream);
	}

	free ((char *) line_table);
}

static short *ct9800_build_line_table (file, size)
    MYFILE *file;
    int size;
{
	short	*map;

	map = (short *) malloc ((unsigned) (size * sizeof (short)));

	/*
	 * seek to start of line table
	 */

#define CT9_MAP_START (4 * 256 * sizeof (short))
	myseek (file, (long) CT9_MAP_START, 0);
	if (fread ((char *) map, sizeof (short), size, file->stream) == -1) {
		perror ("ct9800_build_line_table");
		return ((short *) NULL);
	}
#ifdef vms
	swab ((char *) map, (char *) map, size * sizeof (short));
#endif

	return (map);
}

/***** my open, close, seek routine to handle pipes ******/

static MYFILE *myopen (file, mode)
    char *file;
    char *mode;
{
	struct	stat buf;
	MYFILE	*file_ptr;
	FILE	*stream;
	char	*temp_name;
	char	compressed[MAXPATHLEN];
	char	command[MAXPATHLEN + 23];

	/* allocate a file */

	file_ptr = (MYFILE *) malloc (sizeof (MYFILE));

	/* first try the normal file */

	stream = fopen (file, mode);
	if (stream == NULL) {

		/* now see if compressed file exists */

		strcpy (compressed, file);
		strcat (compressed, ".Z");
		if (stat (compressed, &buf) == -1) return (NULL);

		/* open for read or write */

		if (strcmp (mode, "r") == 0) {
			sprintf (command, "/usr/ucb/zcat %s", compressed);
			stream = popen (command, "r");
			if (stream == NULL) {
				free (file_ptr);
				return (NULL);
			}
			file_ptr->type = PIPE;
			file_ptr->stream = stream;
		}
		else if (strcmp (mode, "r+") == 0 ||
			 strcmp (mode, "w") == 0) {
			temp_name = tempnam (".", "C_");
			sprintf (command, "cat >%s; /usr/ucb/compress -f %s; mv %s.Z %s", temp_name, temp_name, temp_name, compressed);
			free (temp_name);
			stream = popen (command, "w");
			if (stream == NULL) {
				free (file_ptr);
				return (NULL);
			}
			file_ptr->type = PIPE;
			file_ptr->stream = stream;
		}
		else {
			fprintf (stderr, "myopen: unsupported mode: %s\n", mode);
			free (file_ptr);
			return (NULL);
		}
	}
	else {
		file_ptr->type = NORMAL;
		file_ptr->stream = stream;
	}

	return (file_ptr);
}

static void myseek (file, offset, mode)
    MYFILE *file;
    int offset;
    int mode;
{
	caddr_t filler;

	if (file->type == NORMAL) {
		fseek (file->stream, offset, mode);
	}
	else {
		filler = (caddr_t) malloc (offset);
		fread (filler, offset, 1, file->stream);
		free (filler);
	}
}

static void myclose (file_ptr)
    MYFILE *file_ptr;
{
	if (file_ptr->type == PIPE) pclose (file_ptr->stream);

	else fclose (file_ptr->stream);

	free (file_ptr);
}
