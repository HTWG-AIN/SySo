#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>


#define NSEC_OVERFLOW_BORDER 999999999
#define SEC_IN_NSEC 1000000000
#define HIGH "1"
#define LOW "0"

long long blinken_states[] = { 
    100000000,  // 5 HZ
    50000000,   // 10 HZ
    25000000,   // 20 HZ
    10000000,   // 50 HZ
    5000000,    // 100 HZ
    2000000,    // 250 HZ
};

enum { HZ_5, HZ_10, HZ_20, HZ_50, HZ_100, HZ_250, OFF};



void *led_blinkenrasp(void *);
int mygpio_write(char* value);
int mygpio_read();
void blink(long long sleeptime);
int frequencyToHZ(long long frq);

void sigint_handler(int signum);


int nssleep(long nsec);

int main()
{
    int trigger_temp = 1;
    pthread_t led_blinkenrasp_thread;
    int selected_state = 0;

    FILE *mygpio = NULL;

    mygpio_write(HIGH);


    struct sigaction  sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    int blinkenrasp_running = 0;




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
                if(selected_state != OFF) {
                    if (blinkenrasp_running){
                        pthread_cancel(led_blinkenrasp_thread);
                        pthread_join(led_blinkenrasp_thread, NULL);
                    }

                    long long data =  blinken_states[selected_state];
                    if(pthread_create(&led_blinkenrasp_thread, NULL, led_blinkenrasp, (void *) &data) != 0) {
                        printf("creation of blinkenrasp thread failed with frequency %lld", data);
                    }
                    blinkenrasp_running=1;
                    selected_state++;

                } else {
                    pthread_cancel(led_blinkenrasp_thread);
                    pthread_join(led_blinkenrasp_thread, NULL);
                    mygpio_write(HIGH);
                    printf("stoped blinking\n");
                    selected_state = 0;
                    blinkenrasp_running=0;

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

    long long frequency = *(long long  *)data;

    printf("starting of blinkenrasp thread with %dHZ\n", frequencyToHZ(frequency*2));

    while(1) {
        blink(frequency);
    }

    return 0;
}

int frequencyToHZ(long long frq) {
    
    return (1/(frq * pow(10, -9))) + 1;

}

void sigint_handler(int signum) {
    printf("Programm interrupted with signum: %d\n", signum);
    mygpio_write(HIGH);
    exit(0);
}

void blink(long long sleeptime) {
    // on
    mygpio_write(LOW);
    nssleep(sleeptime);

    // off
    mygpio_write(HIGH);
    nssleep(sleeptime);
}


int mygpio_write(char *value){
	int led_file = open("/dev/mygpio", O_NONBLOCK | O_RDWR);

    write(led_file, value, 2);

	if (close(led_file) != 0) {
	}

    return 1;

}

int mygpio_read(){
    int read_count;
    int fclose_ret;
    int read_int;
	int mygpio = open("/dev/mygpio", O_NONBLOCK | O_RDWR);
    if (mygpio == -1) {
        printf("failed to open gpio");
        return -1;
    }

    read_count = read(mygpio, &read_int, sizeof(int));
    if (read_count > 0 ) {
        ;
    }

    fclose_ret = close(mygpio);
    if (fclose_ret == 0) {
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
