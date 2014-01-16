#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sched.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#define NSEC_OVERFLOW_BORDER 999999999
#define SEC_IN_NSEC 1000000000
#define LOOPS 100000


int mygpio_write(char *value);
int mygpio_read();
struct timespec diff(struct timespec, struct timespec);


int nssleep(long nsec);

int main()
{

    
    long long nsec_average = 0;
    mygpio_write(1);

    pid_t pod = getpid();
    struct sched_param param;

    sched_setscheduler(pod, SCHED_FIFO, &param);

    int i = 0;

    struct timespec res_start, res_stop, diff_stamp;

    for(i = 0; i < LOOPS; i++) {
        clock_gettime(CLOCK_REALTIME, &res_start);
        mygpio_write("0");
        mygpio_write("1");
        clock_gettime(CLOCK_REALTIME, &res_stop);

        
        diff_stamp = diff(res_start, res_stop);
        
        //printf("diff_time = %lld.%.9ld, average = %ld\n", (long long) diff_stamp.tv_sec, diff_stamp.tv_nsec, nsec_average);

        if (nsec_average == 0 ) {
            nsec_average = diff_stamp.tv_nsec;
        } else {
            nsec_average = (nsec_average + diff_stamp.tv_nsec) / 2;
        }

        
    }

    double nseconds = (double)nsec_average * pow(10, -9);
    printf("Nsec Avarage = %lld 10^-9=%lf nseconds =%lf\n", nsec_average, pow(10,-9), nseconds);


    printf("kHZ = %lf\n", (1.0 / nseconds) / 1000.0);


	return 0;
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

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}


