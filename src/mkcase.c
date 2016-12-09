/*
	make cases for cube triangulator
*/

#include <stdio.h>
#define MAX_CASE_LENGTH 20

main (argc, argv)

    int argc;
    char **argv;
{
	int     last = -1;
	int number;
	int	original_case;
	int edge;
	int i;
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
	printf ("       EDGE_LIST edges[%d];\n", MAX_CASE_LENGTH + 1);
	printf ("} POLY_CASES;\n");
	printf (" \n");
	printf ("POLY_CASES poly_cases[] = {\n");
	printf ("{");
	for (i = 0; i < MAX_CASE_LENGTH; i++) printf ("0,");
	printf ("0},\n");

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
	printf ("{");
	for (i = 0; i < MAX_CASE_LENGTH; i++) printf ("0,");
	printf ("0}};\n");
	exit (0);
}


