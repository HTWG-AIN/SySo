#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>



void sigint_handler(int);


int main()
{
    printf("activating sighandler\n");
    struct sigaction  sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);




	return 0;
}

void sigint_handler(int signum) {
    printf("Programm interrupted with signum: %d\n", signum);
    exit(0);
}
