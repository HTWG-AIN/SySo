#include <stdio.h>  /* fprintf */
#include <string.h> /* strerror */
#include <stdlib.h>
#include <sys/stat.h> /* mode_t, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH */
#include <fcntl.h>    /* open, O_RDONLY, O_WRONLY, O_CREAT, O_EXCL */
#include <unistd.h>   /* read, write */
#include <errno.h>    /* errno */

ssize_t write_all(int fd, void *data, size_t count);

static const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* rw-r--r-- */

int main(int argc, char *argv[])
{
	int out, ret;
	
	if (argc != 3)
	{
		printf("Please call %s <message> <destination>", argv[0]);
		return -1;
	}
	
	out = open(argv[2], O_RDWR, mode);
	
	ret = write_all(out, argv[1], strlen(argv[1]));
	
	close(out);
	
	return ret;
}

ssize_t write_all(int fd, void *data, size_t count) 
{
	ssize_t bytes_written;
	char *data_ptr = data;
	size_t total = 0;

	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	while (count) 
	{
		bytes_written = write(fd, data_ptr, count);
		
		if (bytes_written == -EAGAIN)
		{
			continue;
		}
		
		data_ptr += bytes_written;
		count -= bytes_written;
		total += bytes_written;
	}

	return total;
}
