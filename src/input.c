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
	mio_read_image (slice_filename, slice_no, pixels_per_line, lines_per_slice, slice->s_ptr);

	fprintf (stderr, "slice #%d at %x\n", slice_no, slice->s_ptr); fflush (stderr);

	return (slice->s_ptr);
}

cubes_set_2d_noheaders ()
{
	mio_set_noh2d ();
}

cubes_set_signa ()
{
	mio_set_signa ();
}

cubes_set_9800 ()
{
	mio_set_ct9800 ();
}

cubes_set_9800_compressed ()
{
}

cubes_set_8800 ()
{
}

cubes_set_xim ()
{
	mio_set_xim ();
}

