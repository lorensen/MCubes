#define MAX_CASES       28
#define DPRINT if (debug) printf

typedef struct {
	int     number;
	char    binary[8];
	char    labels[9];
	int     case_edges[64];
} CASES;

CASES cases[MAX_CASES];

typedef struct {
	int vertices[9];
	int edges[13];
} OPERATOR;

OPERATOR i_operator = { {0, 1, 2, 3, 4, 5, 6, 7, 8},
			{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}
		      };

OPERATOR f_operator = { {0, 2, 3, 4, 1, 6, 7, 8, 5},
			{0, 2, 3, 4, 1, 6, 7, 8, 5, 10, 12, 9, 11}
		      };

OPERATOR u_operator = { {0, 2, 6, 7, 3, 1, 5, 8, 4},
			{0, 10, 6, 12, 2, 9, 8, 11, 4, 1, 5, 3, 7}
		      };

OPERATOR r_operator = { {0, 4, 3, 7, 8, 1, 2, 6, 5},
			{0, 5, 10, 1, 9, 7, 12, 3, 11, 8, 6, 4, 2}
		      };

int number_cases = 0;
int debug = 0;
