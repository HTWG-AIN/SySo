#include <stdio.h>  /* fprintf */
#include <string.h> /* strerror */
#include <stdlib.h>
#include <sys/stat.h> /* mode_t, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH */
#include <fcntl.h>    /* open, O_RDONLY, O_WRONLY, O_CREAT, O_EXCL */
#include <unistd.h>   /* read, write */
#include <errno.h>    /* errno */

#define BUFFER_SIZE 4096

ssize_t read_all(int fd, char *data, size_t count);

static const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* rw-r--r-- */

int main(int argc, char *argv[])
{
	int in, ret;
	char buffer[BUFFER_SIZE];
	
	//memset(buffer, '\0', BUFFER_SIZE);
	
	if (argc != 2)
	{
		printf("Please call %s <destination>", argv[0]);
		return -1;
	}
	
	in = open(argv[1], O_RDWR, mode);
	
	ret = read_all(in, buffer, BUFFER_SIZE);
	
	close(in);
	
	return ret;
}

ssize_t read_all(int fd, char *data, size_t count)
{
	ssize_t bytes_read;
	char *data_ptr = data;
	size_t total = 0;

	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	while (count && bytes_read != 0) 
	{
		bytes_read = read(fd, data_ptr, count);
		
		if (bytes_read == -EAGAIN || bytes_read == -1)
		{
			continue;
		}
		
		/*if (bytes_read == -1)
		{
			fprintf(stderr, "Fehler beim Lesen (errno %d: %s)\n", errno, strerror(errno));
			return -1;
		}*/
		
		data_ptr[bytes_read + 1] = '\0';
		printf("%s\n", data_ptr);
		
		data_ptr += bytes_read;
		count -= bytes_read;
		total += bytes_read;
	}

	return total;
}
