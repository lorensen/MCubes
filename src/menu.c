/*
	menus for cubes
*/

#include        <stdio.h>
#include        "kbmenu.h"
#include	"cubes.h"

#ifdef vms
extern	char	vms_filename[];
#else
extern  char    full_filename[];
#endif
extern char *strcpy ();

extern int      interactive;
extern float	value;
extern PIXEL	mask;
extern FILE	*slice_file;
extern int      debug;
extern int      lines_per_slice;
extern int      number_slices;
extern int      pixels_per_line;
extern int      text_mode;
extern float    scale_x;
extern float    scale_y;
extern float    scale_z;
extern int      start_slice;
extern char	command_file[];
extern char     slice_filename[];
extern char     output_prefix[];

/*
 * routines for changing menu items
 */

int     change_slice ();
int     change_scale ();
int     change_mask ();
int     change_start ();
int     change_output ();
int     change_slices ();
int     change_resolution ();
int     change_value ();

/*
 * strings for dynamic menu items
 */

char output_item[80]    = "";
char resolution_item[80]    = "";
char scale_item[80]    = "";
char mask_item[80]    = "";
char slice_item[80]    = "";
char slices_item[80]    = "";
char start_item[80]    = "";
char value_item[80]    = "";

/*
 * menu
 */

MENU_ITEM cubes_menu[]= {
	"Triangulate using current options", 0,
	slice_item, change_slice,
	output_item, change_output,
	slices_item, change_slices,
	resolution_item, change_resolution,
	scale_item, change_scale,
	value_item, change_value,
	mask_item, change_mask,
	"", 0};


cubes_interactive ()

{
	update_menu ();
	menu (cubes_menu);
}


update_menu ()

{
	sprintf (slice_item, "Slice file name (%s)", slice_filename);
	sprintf (output_item, "Output file prefix (%s)", output_prefix);
	sprintf (slices_item, "Number of slices (%d)", number_slices);
	sprintf (start_item, "Starting slice # (%d)", start_slice);
	sprintf (resolution_item, "X, Y resolution (%d,%d)", pixels_per_line,
							     lines_per_slice);
	sprintf (scale_item, "X,Y,Z scale factors (%f,%f,%f)", scale_x,
							       scale_y,
							       scale_z);
	sprintf (value_item, "Surface value (%f)", value);
	sprintf (mask_item, "Connectivity mask in hex (%x)", mask);
}

change_slice ()
{
	if (slice_file != NULL) fclose (slice_file);

	if (text_mode) {
		slice_file = (FILE *) openfile ("What file contains the slices", "slc", 1);
#ifdef vms
		strcpy (slice_filename, vms_filename);
#else
		strcpy (slice_filename, full_filename);
#endif
	} else {
		printf ("Enter slice file prefix: ");
		scanf ("%s", slice_filename);
	}

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_output ()
{

	printf ("Change output prefix to :");
	scanf ("%s", output_prefix);
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_resolution ()
{
	printf ("Change X,Y resolution to :");
	scanf ("%d%*c%d", &pixels_per_line, &lines_per_slice);
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_slices ()
{
	printf ("Change # of slices to :");
	scanf ("%d", &number_slices);
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_start ()
{
	printf ("Change starting slice # to: ");
	scanf ("%d", &start_slice);
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_mask ()
{
	int	int_mask;

	printf ("Change connectivity mask in hex to :");
	scanf ("%x", &int_mask);
	mask = (PIXEL) int_mask;
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_value ()
{
	printf ("Change surface value to :");
	scanf ("%f", &value);
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}

change_scale ()
{
	printf ("Change x,y,z scale factors to: ");
	scanf ("%f%*c%f%*c%f", &scale_x, &scale_y, &scale_z);
	getchar ();

	/*
	 * update menu item
	 */

	update_menu ();

	return (0);
}


/*++
 *
 * routine cubes_options (argc, argv)

 * functional description:
        Process options from a command line.  The command line is 
        searched for flags prefixed by -'s or /'s.  Multiple 
	flags can appear in any argument including the first
        argument.  

 * formal parameters:
	argc - # of arguments on the command line
	argv - vector of arguments

 * implicit inputs:
	none

 * implicit outputs:

 * routine value:
	Returns the number of arguments processed

 * side effects:

	none
*/

cubes_options (argc, argv)

register int argc;
register char *argv[];

{
	int	int_mask;
	register char *ptr;
	register int i;

	/*
	 * process one argument at a time, starting with command itself
	 */

	for (i=1; i < argc; i++) {
		ptr = argv[i];
		if (*ptr == 0 || (*ptr != '-')) return (i);

		/*
		 * scan argument until you reach end
		 */

		while (*ptr != 0) {

			/*
			 * search for flag delimiter
			 */

			if (*ptr != '-') {
				ptr++;
				continue;
			}

			/*
			 * process flag
			 */

			switch (*(++ptr)) {

		/* c = command file */
			case 'c':
				sscanf (ptr, "%*[^=]%*c%s",
					     command_file);
				break;
				

		/* d = debug */
			case 'd':
				debug++;
				break;

		/* t = text mode */
			case 't':
				text_mode++;
				break;

		/* input = input file */
			case 'i':
				sscanf (ptr, "%*[^=]%*c%[^-]", slice_filename);
				break;

		/* output = output prefix */
			case 'o':
				sscanf (ptr, "%*[^=]%*c%[^-]", output_prefix);
				break;

		/* factor = x,y,z scale factors */
			case 'f':
				sscanf (ptr, "%*[^=]%*c%f%*c%f%*c%f",
					     &scale_x,
					     &scale_y,
					     &scale_z);
				break;

		/* resolution = x,y,#slices */
			case 'r':
				sscanf (ptr, "%*[^=]%*c%d%*c%d%*c%d",
					     &pixels_per_line,
					     &lines_per_slice,
					     &number_slices);
				break;

		/* mask = connectivity mask */
			case 'm':
				sscanf (ptr, "%*[^=]%*c%x",
					     &int_mask);
				mask = (PIXEL) int_mask;
				break;

		/* value = surface value */
			case 'v':
				sscanf (ptr, "%*[^=]%*c%f",
					     &value);
				break;

		/* go = execute flag */
			case 'g':
				interactive = 0;
				break;

		/* h: help */
			case 'h':
				cubes_help();
				exit(0);

			default:
				fprintf (stderr, "cubes: unrecognized flag %s\n", ptr);
				exit(1);
			}
		}
	}
	return (argc);
}

cubes_help () {}
