#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


#define NSEC_OVERFLOW_BORDER 999999999
#define SEC_IN_NSEC 1000000000


void *led_blinkenrasp(void *);
int mygpio_write(int value);
int mygpio_read();


int nssleep(long nsec);

int main()
{
    int trigger_temp = 1;
    pthread_t led_blinkenrasp_thread;
    int led_blinkenrasp_running = 0;


    FILE *mygpio = NULL;


    while(1) {
        char read_text[10];
        size_t read_count;
        int fclose_ret = 0;

        mygpio = fopen("/dev/mygpio", "r");
        if (mygpio == NULL) {
            printf("failed to open gpio");
            return -1;
        }
        read_count = fread(&read_text, sizeof(char), 2, mygpio);
        if(read_count > 0 ) {
            int trigger_state = atoi(read_text);
            if (trigger_temp == 0 && trigger_state == 1)  {
                if (!led_blinkenrasp_running) {
                    if(pthread_create(&led_blinkenrasp_thread, NULL, led_blinkenrasp, NULL) != 0) {
                        printf("creation of blinkenrasp thread failed");
                    }
                    led_blinkenrasp_running = 1;
                    printf("started blinking \n");
                } else {
                    pthread_cancel(led_blinkenrasp_thread);
                    pthread_join(led_blinkenrasp_thread, NULL);
                    led_blinkenrasp_running = 0;
                    printf("stoped blinking\n");

                }

            }
            trigger_temp = atoi(read_text);
        }
        fclose_ret = fclose(mygpio);
        if (fclose_ret == EOF) {
            return -1;
        }

    }
	return 0;
}



void *led_blinkenrasp(void * data) {
    while(1) {
        // on
        mygpio_write(0);
        nssleep(4000000);

        // off
        mygpio_write(1);
        nssleep(4000000);
        
    }
}

int mygpio_write(int value){
    int write_count;
    int fclose_ret;
    char data[2];
    FILE *mygpio = fopen("/dev/mygpio", "w");
    if (mygpio == NULL) {
        printf("failed to open gpio");
        return -1;
    }
    int size = sprintf(data,"%d",value);

    write_count = fwrite(&data, sizeof(char), size, mygpio);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(mygpio);
    if (fclose_ret == EOF) {
        return -1;
    }

    return 1;

}
int mygpio_read(){
    int read_count;
    int fclose_ret;
    int read_int;
    FILE *mygpio = fopen("/dev/mygpio", "r");
    if (mygpio == NULL) {
        printf("failed to open gpio");
        return -1;
    }

    read_count = fread(&read_int, sizeof(int), 1, mygpio);
    if (read_count > 0 ) {
        ;
    }

    fclose_ret = fclose(mygpio);
    if (fclose_ret == EOF) {
    }

    return 1;
}



int nssleep(long nsec) {
    struct timespec sleeptime;

    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = nsec;


    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL) == EINTR );

    return 0;
}
