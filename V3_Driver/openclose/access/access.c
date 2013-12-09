/**
 * filecopy.c
 * @author: Stefano Di Martino
 * @created: 08.01.2012
 */

#include <stdio.h>  /* fprintf */
#include <string.h> /* strerror */
#include <stdlib.h>

#include <fcntl.h>    /* open, O_RDONLY, O_WRONLY, O_CREAT, O_EXCL */
#include <unistd.h>   /* read, write */
#include <errno.h>    /* errno */

static int open_device(int *in, char *file_name)
{
	*in = open(file_name, O_RDONLY);
	
	if (*in == -EBUSY)
	{
		fprintf(stderr,
		      "Quelle %s kann nicht geoeffnet werden (errno %d: %s)\n",
		      file_name, errno, strerror(errno));
	}
	
	return *in;
}


int write_values(int *fildes)
{
	write(fildes, "Hi", 2);


}

int main(int argc, char *argv[])
{
	int in1, in2, return_value; /* Dateideskriptor */

	if (argc != 2)
	{
		fprintf(stderr, "Aufruf: %s Ziel\n", argv[0]);
		return 1;
	}

	return_value = open_device(&in1, argv[1]);
	
	if (return_value < 0) {
		return return_value;
	}
	
	write(in1, "Hi", 2);


	return_value = open_device(&in2, argv[1]);

	if (return_value < 0) {
		close(in1);
		return return_value;
	}
	
	close(in2);

	return 0;
}
