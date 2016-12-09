#include <stdio.h>
#include <math.h>

typedef short PIXEL;
typedef struct {
	PIXEL	*s_ptr;
	FILE 	*s_fileptr;
} SLICE;

#ifdef vms
extern  int     SS$_NORMAL();
extern  int     map_channel;
extern  int     section_start;
extern  int     section_end;
static	int	status;
#endif
extern	char slice_filename[];
extern int lines_per_slice;
extern int pixels_per_line;

static char *cubes_2d_build_filename ();
static void cubes_2d_read_slice ();
static char *cubes_signa_build_filename ();
static void cubes_signa_read_slice ();
static char *cubes_xim_build_filename ();
static void cubes_xim_read_slice ();
static char *cubes_9800_build_filename ();
static void cubes_9800_read_compressed_slice ();
static void cubes_9800_read_slice ();
static short *ct9800_build_line_table ();
static char *cubes_8800_build_filename ();
static void cubes_8800_read_slice ();
static short *ct8800_build_line_table ();

static void (*cubes_read_slice) () = cubes_2d_read_slice;
static char *(*cubes_build_filename) () = cubes_2d_build_filename;

static int use_1 = 1;
static char line[256];


PIXEL *
cubes_get_slice (slice_no)
    int slice_no;
{
	PIXEL   *image_ptr;
static	SLICE	slice_0 = {0, NULL};
static	SLICE	slice_1 = {0, NULL};
static	SLICE	slice_2 = {0, NULL};
static	SLICE	slice_3 = {0, NULL};
	SLICE   *slice;
	char	*file_name;

	/*
	 * build file name
	 */

	file_name = (*cubes_build_filename) (slice_filename, slice_no);

	/*
	 * select one of four buffers
	 */

	switch (use_1) {

	case 1:
		slice = &slice_0;
		use_1 = 2;
		break;
	case 2:
		slice = &slice_1;
		use_1 = 3;
		break;
	case 3:
		slice = &slice_2;
		use_1 = 4;
		break;
	case 4:
		slice = &slice_3;
		use_1 = 1;
		break;
	}

	/*
	 * get memory for new slice
	 */

	if (slice->s_ptr == NULL) {
		slice->s_ptr = (PIXEL *) malloc ((unsigned) (sizeof (PIXEL) * lines_per_slice * pixels_per_line));	
		if (slice->s_ptr == NULL) {
			fprintf (stderr, "cubes: cannot allocate slice memory\n");
			exit (1);
		}
	}

	/*
	 * read the file
	 */

	(*(cubes_read_slice)) (slice->s_ptr, lines_per_slice * pixels_per_line, file_name, slice_no);

	printf ("slice #%d at %x\n", slice_no, slice->s_ptr);

	return (slice->s_ptr);
}

cubes_set_2d_noheaders ()
{
	cubes_build_filename = cubes_2d_build_filename;
	cubes_read_slice = cubes_2d_read_slice;
}

cubes_set_signa ()
{
	cubes_build_filename = cubes_signa_build_filename;
	cubes_read_slice = cubes_signa_read_slice;
}

cubes_set_9800 ()
{
	cubes_build_filename = cubes_9800_build_filename;
	cubes_read_slice = cubes_9800_read_slice;
}

cubes_set_8800 ()
{
	cubes_build_filename = cubes_8800_build_filename;
	cubes_read_slice = cubes_8800_read_slice;
}

cubes_set_9800_compressed ()
{
	cubes_build_filename = cubes_9800_build_filename;
	cubes_read_slice = cubes_9800_read_compressed_slice;
}

cubes_set_xim ()
{
	cubes_build_filename = cubes_xim_build_filename;
	cubes_read_slice = cubes_xim_read_slice;
}


/*******************************************************
 * file name builders and input routines
 *******************************************************/

	/********************
	 * 2d, noheaders    *
	 ********************/
static char *
cubes_2d_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.%d", prefix, number);
	return (line);
}

static void
cubes_2d_read_slice (slice, size, file_name)
    PIXEL *slice;
    int size;
    char *file_name;
{
#ifndef vms
	FILE	*file_ptr;
	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "cubes_2d_read_slice: cannot open %s\n", file_name);
		perror ("cubes_2d_read_slice");
		exit ();
	}
	else {
		fread ((char *) slice, sizeof (PIXEL), size, file_ptr);
		swab ((char *) slice, (char *) slice, size * sizeof (PIXEL));
		fclose (file_ptr);
	}
#else
	PIXEL	*start, *end, *pixel, *ptr;

	start = (PIXEL *) map_section (file_name, 0, "read", 0, 0, 0, 0, 0);
	if (start == NULL) {
		status = map_error_status ();
		print_sys_msg ("CUBES:", status);
		fprintf (stderr, "cubes: cannot map file %s\n", file_name);
		exit (1);
	}

	pixel = start;
	end = pixel + size;

	/*
	 * copy image
	 */

	for (ptr = slice; pixel < end; ) *ptr++ = *pixel++;

	/*
	 * delete section
	 */

	delete_section ();
#endif
}

	/********************
	 * 2d, signa headers *
	 ********************/
static char *
cubes_signa_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.%03d", prefix, number);
	return (line);
}

static void
cubes_signa_read_slice (slice, size, file_name)
    PIXEL *slice;
    int size;
    char *file_name;
{
#ifndef vms
	FILE	*file_ptr;

	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "cubes_signa_read_slice: cannot open %s\n", file_name);
		perror ("cubes_signa_read_slice");
		exit ();
	}
	else {
		fseek (file_ptr, 28 * 512, 0);
		fread ((char *) slice, sizeof (PIXEL), size, file_ptr);
		fclose (file_ptr);
	}
#else
	PIXEL	*start, *end, *pixel, *ptr;

	start = (PIXEL *) map_section (file_name, 0, "read", 0, 0, 0, 0, 0);
	if (start == NULL) {
		status = map_error_status ();
		print_sys_msg ("CUBES:", status);
		fprintf (stderr, "cubes: cannot map file %s\n", file_name);
		exit (1);
	}

	pixel = start + 28 * 256;
	end = pixel + size;

	/*
	 * copy image
	 */

	for (ptr = slice; pixel < end; ) *ptr++ = *pixel++;

	/*
	 * swap bytes
	 */

	swap_byte (slice, size);

	/*
	 * delete section
	 */

	delete_section ();
#endif
}

	/********************
	 * 2d, xim headers *
	 ********************/
static char *
cubes_xim_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.%d", prefix, number);
	return (line);
}

static void
cubes_xim_read_slice (slice, size, file_name)
    PIXEL *slice;
    int size;
    char *file_name;
{
#ifndef vms
	FILE	*file_ptr;
	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "cubes_xim_read_slice: cannot open %s\n", file_name);
		perror ("cubes_xim_read_slice");
		exit ();
	}
	else {
		fseek (file_ptr, 512, 0);
		fread ((char *) slice, sizeof (PIXEL), size, file_ptr);
		swab ((char *) slice, (char *) slice, size * sizeof (PIXEL));
		fclose (file_ptr);
	}
#else
	PIXEL	*start, *end, *pixel, *ptr;

	start = (PIXEL *) map_section (file_name, 0, "read", 0, 0, 0, 0, 0);
	if (start == NULL) {
		status = map_error_status ();
		print_sys_msg ("CUBES:", status);
		fprintf (stderr, "cubes: cannot map file %s\n", file_name);
		exit (1);
	}

	pixel = start + 256;
	end = pixel + size;

	/*
	 * copy image
	 */

	for (ptr = slice; pixel < end; ) *ptr++ = *pixel++;

	/*
	 * delete section
	 */

	delete_section ();
#endif
}

	/********************
	 * 2d, 9800 headers *
	 ********************/
static char *
cubes_9800_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s%c%01d.yp",
		prefix,
		number < 100 ? ('0' + number / 10) :
			       ('a' + (number / 10 - 10)),
		number % 10);

	return (line);
}

static void
cubes_9800_read_slice (slice, size, file_name)
    PIXEL *slice;
    int size;
    char *file_name;
{

	PIXEL	*ptr;
	PIXEL	*start, *end;
	int	half_resolution;
	int	i;
	short	*line;
	short	*line_table;
#ifndef vms | stellar | hp
	PIXEL	data[256];
	PIXEL	*data_end = data + 256;
	PIXEL	*data_ptr = data + 256;
	FILE	*file_ptr;
#else
	PIXEL	*data_ptr;
#endif
#if !(vms || stellar || hp)
	/*
	 * zero memory for image
	 */

	bzero (slice, size * sizeof (PIXEL));
#else
	end = slice + size;
	for (ptr = slice; ptr < end; ) *ptr++ = 0;
#endif

	/*
	 * build the line table
	 */
	
	line_table = (short*) ct9800_build_line_table (file_name, lines_per_slice);
	line = line_table;

#ifndef vms
	/*
	 * seek to start of data
	 */

#define CT9_IMAGE_START (6 * 256 * sizeof (short))
	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "cubes_9800_read_slice: cannot open %s\n", file_name);
		perror ("cubes_9800_read_slice");
		exit ();
	}
	fseek (file_ptr, CT9_IMAGE_START, 0);

	/*
	 * build image
	 */

	half_resolution = pixels_per_line / 2;
	ptr = slice;
	for (i = 0; i < lines_per_slice; i++, line++, ptr += pixels_per_line) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) {
			if (data_ptr >= data_end) {
				fread (data, sizeof (PIXEL), 256, file_ptr);
				data_ptr = data;
			}
			*start = *data_ptr;
		}
	}
	fclose (file_ptr);
#else
	data_ptr = (PIXEL *) map_section (file_name, 0, "read", 0, 0, 0, 0, 0);
	if (data_ptr == NULL) {
		status = map_error_status ();
		print_sys_msg ("CUBES:", status);
		fprintf (stderr, "cubes: cannot map file %s\n", file_name);
		exit (1);
	}

	data_ptr = data_ptr + 6 * 256;

	/*
	 * build image
	 */

	half_resolution = pixels_per_line / 2;
	ptr = slice;
	for (i = 0; i < lines_per_slice; i++, line++, ptr += pixels_per_line) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) *start = *data_ptr;
	}

	/*
	 * swap bytes
	 */

	swap_byte (slice, size);

	/*
	 * delete section
	 */

	delete_section ();

#endif
	free (line_table);
}

static void
cubes_9800_read_compressed_slice (slice, size, file_name)
    PIXEL *slice;
    int size;
    char *file_name;
{

	PIXEL	*ptr;
	PIXEL	*start, *end;
	int	half_resolution;
	int	i;
	short	*line;
	short	*line_table;
#ifndef vms | stellar | hp
	char	data[512];
	char	*data_end = data + 512;
	char	*data_ptr = data + 512;
	char	pixel;
	short	last_pixel;
	FILE	*file_ptr;
#else
	PIXEL	*data_ptr;
#endif
#if !(vms || stellar || hp)
	/*
	 * zero memory for image
	 */

	bzero (slice, size * sizeof (PIXEL));
#else
	end = slice + size;
	for (ptr = slice; ptr < end; ) *ptr++ = 0;
#endif

	/*
	 * build the line table
	 */
	
	line_table = (short*) ct9800_build_line_table (file_name, lines_per_slice);
	line = line_table;

#ifndef vms
	/*
	 * seek to start of data
	 */

#define CT9_IMAGE_START (6 * 256 * sizeof (short))
	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "cubes_9800_read_slice: cannot open %s\n", file_name);
		perror ("cubes_9800_read_slice");
		exit ();
	}
	fseek (file_ptr, CT9_IMAGE_START, 0);

	/*
	 * build image
	 */

	half_resolution = pixels_per_line / 2;
	ptr = slice;
	for (i = 0; i < lines_per_slice; i++, line++, ptr += pixels_per_line) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) {
			if (data_ptr >= data_end) {
				fread (data, sizeof (char), 512, file_ptr);
				data_ptr = data;
			}

			/*
			 * see if pixel is compressed
			 */

			if (*data_ptr & 0x80) {

				if (*data_ptr & 0x40) {
					pixel = *data_ptr;
				}
				else {
					pixel = *data_ptr & 0x3f;
				}
				last_pixel += pixel;
			}
			else {
				last_pixel = *data_ptr << 8;
				data_ptr++;
				if (data_ptr >= data_end) {
					fread (data, sizeof (char), 512, file_ptr);
					data_ptr = data;
				}
				last_pixel |= (unsigned char) *data_ptr;
				last_pixel += 2048;
			}
			*start = last_pixel;
		}
	}
	fclose (file_ptr);
#else
	data_ptr = (PIXEL *) map_section (file_name, 0, "read", 0, 0, 0, 0, 0);
	if (data_ptr == NULL) {
		status = map_error_status ();
		print_sys_msg ("CUBES:", status);
		fprintf (stderr, "cubes: cannot map file %s\n", file_name);
		exit (1);
	}

	data_ptr = data_ptr + 6 * 256;

	/*
	 * build image
	 */

	half_resolution = pixels_per_line / 2;
	ptr = slice;
	for (i = 0; i < lines_per_slice; i++, line++, ptr += pixels_per_line) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) *start = *data_ptr;
	}

	/*
	 * swap bytes
	 */

	swap_byte (slice, size);

	/*
	 * delete section
	 */

	delete_section ();

#endif
	free (line_table);
}

static short *ct9800_build_line_table (file_name, resolution)
    char *file_name;
    int resolution;
{
	short	*map;
#ifndef vms
	FILE	*file_ptr;
#else
	short	*start, *end, *pixel, *ptr;
#endif

	map = (short *) malloc (resolution * sizeof (short));

	/*
	 * seek to start of line table
	 */

#ifndef vms
#define CT9_MAP_START (4 * 256 * sizeof (short))
	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "ct9800_build_line_table: cannot open %s\n", file_name);
		perror ("ct9800_build_line_table");
		exit ();
	}
	else {
		fseek (file_ptr, CT9_MAP_START, 0);
		if (fread (map, sizeof (short), resolution, file_ptr) == -1) {
			perror ("ct9800_build_line_table");
			exit ();
		}
		fclose (file_ptr);
	}
#else
	start = (PIXEL *) map_section (file_name, 0, "read", 0, 0, 0, 0, 0);
	if (start == NULL) {
		status = map_error_status ();
		print_sys_msg ("CUBES:", status);
		fprintf (stderr, "cubes: cannot map file %s\n", file_name);
		exit (1);
	}

	pixel = start + 4 * 256;
	end = pixel + resolution;

	/*
	 * copy line table
	 */

	for (ptr = map; pixel < end; ) *ptr++ = *pixel++;

	/*
	 * swap bytes
	 */

	swap_byte (map, resolution);

	/*
	 * delete section
	 */

	delete_section ();
#endif
	return (map);
}

	/********************
	 * 2d, 8800 headers *
	 ********************/
static char *
cubes_8800_build_filename (prefix, number)
    char *prefix;
    int number;
{
	sprintf (line, "%s.8800", prefix);
	return (line);
}

static void
cubes_8800_read_slice (slice, size, file_name, number)
    PIXEL *slice;
    int size;
    char *file_name;
    int number;
{

	int	offset;
	PIXEL	*ptr;
	PIXEL	*start, *end;
	int	half_resolution;
	int	i;
	short	*line;
	short	*line_table;
	PIXEL	data[256];
	PIXEL	*data_end = data + 256;
	PIXEL	*data_ptr = data + 256;
	FILE	*file_ptr;

#if !(vms || stellar || hp)
	/*
	 * zero memory for image
	 */

	bzero (slice, size * sizeof (PIXEL));
#else
	end = slice + size;
	for (ptr = slice; ptr < end; ) *ptr++ = 0;
#endif
	/*
	 * build the line table
	 */
	
	line_table = (short*) ct8800_build_line_table (file_name, lines_per_slice);
	line = line_table;

	/*
	 * seek to start of data
	 */

	file_ptr = fopen (file_name, "r");
	if (file_ptr == NULL) {
		fprintf (stderr, "cubes_9800_read_slice: cannot open %s\n", file_name);
		perror ("cubes_9800_read_slice");
		exit ();
	}
	offset = 1024 + (number - 1) * 316 * 512;
	fseek (file_ptr, offset, 0);

	/*
	 * build image
	 */

	half_resolution = pixels_per_line / 2;
	ptr = slice;
	for (i = 0; i < lines_per_slice; i++, line++, ptr += pixels_per_line) {
		start = ptr + (half_resolution - *line);
		end = start + (*line * 2);
		for (; start < end; start++, data_ptr++) {
			if (data_ptr >= data_end) {
				fread (data, sizeof (PIXEL), 256, file_ptr);
				data_ptr = data;
			}
			*start = *data_ptr;
		}
	}
	fclose (file_ptr);
	free (line_table);
}

static short *ct8800_build_line_table (file_name, resolution)
    char *file_name;
    int resolution;
{
	int	i, j;
	int	llflg;
	float	ydiam, xdiam;
	float	pixsz;
	int	yhigh, xhigh;
	short	*map;

	ydiam = 25.599991;
	xdiam = 25.599991;
	pixsz = 0.8;
	llflg = 1;

	map = (short *) malloc (resolution * sizeof (short));

	if (llflg == 0) {
		xdiam = 43.15;
		ydiam = 43.15;
		pixsz = 1.3485336;
	}

	/*
	 * convert head and length to number of pixels
	 */

	yhigh = 5.0 * ydiam / pixsz + .5;
	xhigh = 5.0 * xdiam / pixsz + .5;

	if (xhigh > 160) xhigh = 160;
	if (yhigh > 160) yhigh = 160;

	/*
	 * for lines with no reconstruction, fill in with 1's
	 */

	for (i = 160; i > yhigh; i++) *(map + 160 - i) = 1;

	/*
	 * fill in image with ellipse formula
	 */

	for (i = yhigh; i > 0; i--) {
		j = 160 - i;
		
		*(map + j) = xhigh * sqrt (1.0 - pow (((float) i - .5) / (float) yhigh), 2.0) + .5;
	}

	/*
	 * fill in bottom half
	 */

	for (i = 160; i < 2 * 160; i++) *(map + i) = *(map + 2 * 160 - 1 - i);
		
	return (map);
}

