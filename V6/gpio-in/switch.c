#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>



void sigint_handler(int);


int main()
{
    int trigger_count = 0;
    printf("activating sighandler\n");
    struct sigaction  sa;
    int fclose_ret = 0;
    int trigger_temp = 1;
    FILE *gpio17 = NULL;

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf("starting trigger logger\n");
    perror("");

    perror("");



    while(1) {
        char read_text[10];
        size_t read_count;

        gpio17 = fopen("/sys/class/gpio/gpio17/value", "r");
        if (gpio17 == NULL) {
            printf("failed to open gpio");
            return -1;
        }
        read_count = fread(&read_text, sizeof(char), 2, gpio17);
        if(read_count > 0 ) {
            int trigger_state = atoi(read_text);
            if (trigger_temp == 0 && trigger_state == 1){
                trigger_count++;
                printf("trigger pushed %d times\n",trigger_count);
            }
            trigger_temp = atoi(read_text);
        }
        fclose_ret = fclose(gpio17);
        if (fclose_ret == EOF) {
            return -1;
        }

    }


	return 0;
}

void sigint_handler(int signum) {
    printf("Programm interrupted with signum: %d\n", signum);
    exit(0);
}
