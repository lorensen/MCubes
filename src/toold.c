/*
	Map new cases to old
*/

#include <stdio.h>

char tocase[] = {
0, 1, 1, 2, 1, 3, 2, 5,
1, 2, 3, 5, 2, 5, 5, 8,
1, 2, 3, 5, 4, 7, 7, 14,
3, 5, 6, 9, 7, 11, 12, 5,
1, 3, 2, 5, 3, 6, 5, 9,
4, 7, 7, 11, 7, 12, 14, 5,
2, 5, 5, 8, 7, 12, 11, 5,
7, 14, 12, 5, 10, 7, 7, 2,
1, 4, 3, 7, 2, 7, 5, 11,
3, 7, 6, 12, 5, 14, 9, 5,
3, 7, 6, 12, 7, 10, 12, 7,
6, 12, 13, 6, 12, 7, 6, 3,
2, 7, 5, 14, 5, 12, 8, 5,
7, 10, 12, 7, 11, 7, 5, 2,
5, 11, 9, 5, 14, 7, 5, 2,
12, 7, 6, 3, 7, 4, 3, 1,
1, 3, 4, 7, 3, 6, 7, 12,
2, 5, 7, 14, 5, 9, 11, 5,
2, 5, 7, 11, 7, 12, 10, 7,
5, 8, 12, 5, 14, 5, 7, 2,
3, 6, 7, 12, 6, 13, 12, 6,
7, 12, 10, 7, 12, 6, 7, 3,
5, 9, 14, 5, 12, 6, 7, 3,
11, 5, 7, 2, 7, 3, 4, 1,
2, 7, 7, 10, 5, 12, 14, 7,
5, 11, 12, 7, 8, 5, 5, 2,
5, 14, 12, 7, 11, 7, 7, 4,
9, 5, 6, 3, 5, 2, 3, 1,
5, 12, 11, 7, 9, 6, 5, 3,
14, 7, 7, 4, 5, 3, 2, 1,
8, 5, 5, 2, 5, 3, 2, 1,
5, 2, 3, 1, 2, 1, 1, 15};
  
char *descr[16] = {
	"(outside)", /* 0 */
	"",		/* 1 */
	"",		/* 2 */
	"(ambiguous)",	/* 3 */
	"",		/* 4 */
	"",		/* 5 */
	"(ambiguous)",	/* 6 */
	"(ambiguous)",	/* 7 */
	"",		/* 8 */
	"",		/* 9 */
	"(ambiguous)",	/* 10 */
	"",		/* 11 */
	"(ambiguous)",	/* 12 */
	"(ambiguous)",	/* 13 */
	"",		/* 14 */
	"(inside)"	/* 15 */
  };


main ()
{
	int	*histo;
	int	i;
	int	count;
	int	old;
	int	total = 0;
	int	ambiguous = 0;

	histo = (int *) calloc (16, sizeof (int));

	/* read stdin for histogram */

	while (fscanf (stdin, "%d%*c%d", &old, &count) != EOF) {
		total += count;
		*(histo + tocase[old]) += count;
	}

	printf ("  0: %10d %f %-20s\n", *(histo + 0), (float) *(histo + 0) / (float) total * 100.0, descr[0]);
	printf (" 15: %10d %f %-20s\n", *(histo + 15), (float) *(histo + 15) / (float) total * 100.0, descr[15]);
	total -= *(histo + 0);
	total -= *(histo + 15);

	for (i = 1; i < 15; i++) {
		
		if (strcmp (descr[i], "(ambiguous)") == 0) ambiguous += *(histo + i);
		printf ("%3d: %10d %f %-20s\n", i, *(histo + i), (float) *(histo + i) / (float) total * 100.0, descr[i]);
	}
	printf ("Number of ambiguous voxels %d %f\n", ambiguous, (float) ambiguous / (float) total * 100.0);
}
