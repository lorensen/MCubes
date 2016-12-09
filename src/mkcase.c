/*
	make cases for cube triangulator
*/

#include <stdio.h>

main (argc, argv)

    int argc;
    char **argv;
{
	int     last = -1;
	int number;
	int	original_case;
	int edge;
	char    line[255];

	/*
	 * read a line and write structure
	 */
	printf ("/*\n");
	printf ("       cases for polygoning a cube\n");
	printf ("*/\n");
	printf (" \n");
	printf ("typedef int EDGE_LIST;\n");
	printf ("typedef struct {\n");
	printf ("       EDGE_LIST edges[65];\n");
	printf ("} POLY_CASES;\n");
	printf (" \n");
	printf ("POLY_CASES poly_cases[] = {\n");
	printf ("{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},\n");

	while (fscanf (stdin, "%d", &number) != EOF) {

		if (number == last) {
			fgets (line, 255, stdin);
			continue;
		}

		last = number;
		printf ("{");

		fscanf (stdin, "%d", &original_case);
		fscanf (stdin, "%*[^01]");
		fscanf (stdin, "%*d%*d");
		for (; fgetc (stdin) != '\n';) {

		    fscanf (stdin, "%d", &edge);
		    printf ("%2d, ", edge);
		}

		printf ("0}, /* %d %d */\n", number, original_case);
	}
	printf ("{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};\n");
	exit (0);
}


