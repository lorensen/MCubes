/*
	permute edge lists based upon symmetry operations
*/

#include <stdio.h>
#include <ctype.h>
#include "permute.h"

main (argc, argv)

    int argc;
    char **argv;
{
	char line[255];
	int     i;

	/*
	 * read cases
	 */

	read_cases (argv[1]);

	/*
	 * for each permutation operator, permute the cases
	 */

	while (fgets (line, 255, stdin) != NULL) {

		for (i = 0; i < number_cases; i++) {
			DPRINT ("Case before permutation\n");
			if (debug) output_case (cases + i);
			permute_cases (line, cases + i);
		}
	}
	exit (0);
}

read_cases (file)
    char *file;
{
	CASES   *a_case;
	char    line[255];
	FILE    *case_file;
	int     edge_no;
	int     i;
	int     number;

	case_file = fopen (file, "r");
	a_case = cases;

	while (fscanf (case_file, "%d", &a_case->number) != EOF) {

		edge_no = 0;

		fscanf (case_file, "%*[^01]");
		for (i = 0; i < 8; i++) {
			fscanf (case_file, "%c", &a_case->binary[i]);
		}

		sprintf (a_case->labels, "12345678");

		for (edge_no = 0; fgetc (case_file) != '\n'; edge_no++) {

		    fscanf (case_file, "%d", &a_case->case_edges[edge_no]);
		}

		a_case++;
		number_cases++;
	}
}

permute_cases (line, a_case)

    char *line;
    CASES *a_case;
{
	CASES   new_case;
	int     i;
	int     times;
	char    *char_ptr;
	char    *char_end;
	OPERATOR *permutation;

	new_case = *a_case;

	/*
	 * decode permutation
	 */

	char_ptr = line;
	char_end = line + strlen(line) - 1;

	for (char_ptr = line; char_ptr < char_end; char_ptr++) {

		times = 1;

		switch (*char_ptr) {

		case 'f':
			permutation = &f_operator;
			break;

		case 'u':
			permutation = &u_operator;
			break;

		case 'i':
			permutation = &i_operator;
			break;

		case 'r':
			permutation = &r_operator;
			break;

		default:
			printf ("unrecognized permutation = |%c|\n", *char_ptr);
			exit (0);

		}

		if (isdigit(*(char_ptr+1))) {
			char_ptr++;
			times = atoi (char_ptr);
		}


		for (i = 0; i < times; i++) {
			permute_it (&new_case, permutation);
		}

	}
	output_case (&new_case);
}

permute_it (a_case, operator)

    CASES *a_case;
    OPERATOR *operator;
{
	CASES temp_case;
	int     test_bit;
	int     i;

	/*
	 * permute bits to form new number
	 */

	temp_case = *a_case;

	for (i = 0; i < 8; i++) {

		temp_case.labels[i] = a_case->labels[operator->vertices[i+1] - 1];
		temp_case.binary[i] = a_case->binary[operator->vertices[i+1] - 1];
	}

	/*
	 * permute edges to form new edge list
	 */

	for (i = 0; i < 64; i++) {
		temp_case.case_edges[i] = operator->edges[a_case->case_edges[i]];
	}

	*a_case = temp_case;
}

output_case (a_case)

    CASES *a_case;
{
	int     number;
	int     i;

	/*
	 * build case number from binary
	 */

	number = 0;
	for (i = 0; i <8; i++) {

		if (a_case->binary[i] == '1') number += (1 << (7 - i));
	}

	printf ("%3d %3d: %s ", number, a_case->number, a_case->labels);

	for (i = 0; i < 8; i++) printf ("%c", a_case->binary[i]);

	for (i = 0; i < 64; i++) {
		printf ("%3d", a_case->case_edges[i]);
	}

	printf ("\n");
}
