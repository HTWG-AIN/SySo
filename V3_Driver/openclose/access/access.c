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

int main(int argc, char *argv[])
{
  int in; /* Dateideskriptor */

  if (argc != 2)
  {
      fprintf(stderr, "Aufruf: %s Ziel\n", argv[0]);
      return 1;
  }

  in = open(argv[1], O_RDONLY);
  if (in == -1)
  {
      fprintf(stderr,
	      "Quelle %s kann nicht geoeffnet werden (errno %d: %s)\n",
	      argv[1], errno, strerror(errno));
      return 1;
  }

  close(in);

  return 0;
}
