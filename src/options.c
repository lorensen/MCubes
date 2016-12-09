#include <stdio.h>

extern int debug;
extern char command_file[];


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
