#include        <stdio.h>

#define MAX_CHARS       200
#define ERROR           -1

#define READ            1
#define WRITE           2
#define READ_WRITE      3

extern char *gets ();
extern char *index ();
extern char *strcpy ();


char    last_filename[MAX_CHARS] = "";
char	full_filename[MAX_CHARS] = "";
int     len_response;


/****************
 *              *
 *** OPENFILE ***
 *              *
 ****************/

FILE    *openfile (prompt,extent,mode)
/*
	purpose
		to prompt user for a filename and open it

	parameters
		input
			prompt - user prompt
			extent - default extension
			mmode  - open mode
				 < 0 -- binary
				 > 0 -- formatted

				       | |= 1 read only
				       | |= 2 write
				       | |= 3 read/write
		output
			return - file descriptor
 */
char    prompt[];
char    extent[];
int     mode;
{
	FILE    *fd,*open_it() ;

	while ((fd = open_it(prompt,extent,mode)) == (FILE *) ERROR);
	return (fd);
} /* openfile */

/***************
 *             *
 *** OPEN_IT ***
 *             *
 ***************/

FILE    *open_it (prompt,extent,mode)
/*
 * function : do the work for openfile
 */
char    prompt[];
char    extent[];
int     mode;
{
	char    *to_ptr, *from_ptr;
	char    input_string[80];       /* buffer for user response */
	char    response[MAX_CHARS];    /* user response */
	FILE    *fd, *fopen();
	int     dir_name_offset;        /* offset of directory name */
	char    *extension_offset;       /* offset of extension */
	int     i;                      /* loop control */
	int     len_extent;             /* length of the extension  */
	int     len_filename;
	int     len_response;           /* length of the response   */

/*
 *  prompt user for filename
 */
	printf ("%s (%s.%s) ",prompt,last_filename,extent);
/*
 *  get response, exit if EOF detected
 */
	*input_string = '\0';
	if (gets (input_string) == NULL) return(0);
	*response = '\0';
	sscanf (input_string, "%s", response);
/*
 * use default filename if user typed return only
 */
	if ((len_response = strlen(response)) == 0) {
		strcpy (response, last_filename);
		len_response = strlen(response);
	} /* if */
/*
 * if response was <cr> and there is no last file, return a zero
 */
	if (len_response == 0) return (0);

	len_response = strlen(response);

/*
 * find start of extension
 */
	extension_offset = index (response, '.');
/*
 * if there is no extension then add the default
 */
	if (extension_offset == 0) {
		len_extent = strlen(extent);
		*(response + len_response++) = '.';
		for (i = 0 ; i < len_extent ; i++)
			*(response + len_response + i) = *(extent + i);
		*(response + len_response + i) = '\0';
		len_response += len_extent;
	} /* if */
	if (mode < 0)
		switch  (-mode) {
			case READ :
				fd = fopen(response,"r");
				break;
			case WRITE :
				fd = fopen(response,"w");
				break;
			case READ_WRITE :
				fd = fopen(response,"rw");
				break;
		} /* switch */
	else
		switch  (mode) {
			case READ :
				fd = fopen(response,"r");
				break;
			case WRITE :
				fd = fopen(response,"w");
				break;
			case READ_WRITE :
				fd = fopen(response,"rw");
				break;
		} /* switch */
	if (fd == NULL){
		perror ("openfile");
		fd = (FILE *) ERROR;
	} /* if */
/*
 * update last filename
 */
	if (fd != (FILE *) ERROR) {
		to_ptr = last_filename;
		from_ptr = response;
		while (*from_ptr && *from_ptr != '.') {
			*(to_ptr++) = *(from_ptr++);
		}
		*to_ptr = '\0';
	}
	strcpy (full_filename, response);

	return (fd);
} /* open_file */
