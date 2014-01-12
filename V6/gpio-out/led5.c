#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


#define GPIO_17 "17"
#define GPIO_18 "18"
#define GPIO_IN "in"
#define GPIO_OUT "out"
#define NSEC_OVERFLOW_BORDER 999999999
#define SEC_IN_NSEC 1000000000

void sigint_handler(int);

void *led_blinkenrasp(void *);

int gpio_export_port(char *port);
int gpio_unexport_port(char *port);
int gpio_set_direction(char *port, char *direction);
int gpio_set_value(char *port, int value);

int nssleep(long nsec);

int main()
{
    printf("activating sighandler\n");
    struct sigaction  sa;

    int trigger_temp = 1;
    pthread_t led_blinkenrasp_thread;
    int led_blinkenrasp_running = 0;


    FILE *gpio17 = NULL;

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    gpio_export_port(GPIO_17);
    gpio_export_port(GPIO_18);
    printf("setting gpio 17 to in\n");
    gpio_set_direction(GPIO_17, GPIO_IN);
    printf("setting gpio 18 to out\n");
    gpio_set_direction(GPIO_18, GPIO_OUT);
    gpio_set_value(GPIO_18, 1);


    while(1) {
        char read_text[10];
        size_t read_count;
        int fclose_ret = 0;

        gpio17 = fopen("/sys/class/gpio/gpio17/value", "r");
        if (gpio17 == NULL) {
            printf("failed to open gpio");
            return -1;
        }
        read_count = fread(&read_text, sizeof(char), 2, gpio17);
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
                    gpio_set_value(GPIO_18, 1);
                    led_blinkenrasp_running = 0;
                    printf("stoped blinking\n");

                }

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



void *led_blinkenrasp(void * data) {
    while(1) {
        // on
        gpio_set_value(GPIO_18, 0);
        nssleep(100000000);

        // off
        gpio_set_value(GPIO_18, 1);
        nssleep(100000000);
        
    }
}

void sigint_handler(int signum) {
    printf("Programm interrupted with signum: %d\n", signum);
    gpio_set_value(GPIO_18, 1);
    gpio_unexport_port(GPIO_17);
    gpio_unexport_port(GPIO_18);
    exit(0);
}


int gpio_set_value(char *port, int value){
    FILE *gpio_port = NULL;
    size_t write_count;
    int fclose_ret = 0;
    char value_string_path[100];
    char int_string_value[4];
    sprintf(int_string_value, "%d", value);
    sprintf(value_string_path, "/sys/class/gpio/gpio%s/value", port);
    gpio_port = fopen(value_string_path, "w");
    if (gpio_port == NULL) {
        printf("failed get write permission to set port %s's value\n", port);
        return -1;
    }
    write_count = fwrite(&int_string_value, sizeof(char), 1, gpio_port);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(gpio_port);
    if (fclose_ret == EOF) {
        printf("failed to close value file\n");
        return -1;
    }

    return 1;
}


int gpio_set_direction(char *port, char *direction) {
    FILE *gpio_port = NULL;
    size_t write_count;
    int fclose_ret = 0;
    char direction_string_path[100];
    sprintf(direction_string_path, "/sys/class/gpio/gpio%s/direction", port);
    gpio_port = fopen(direction_string_path, "w");
    if (gpio_port == NULL) {
        printf("failed get write permission to set port %s's direction", port);
        return -1;
    }
    write_count = fwrite(direction, sizeof(char), strlen(direction) + 1, gpio_port);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(gpio_port);
    if (fclose_ret == EOF) {
        return -1;
    }

    return 1;

}
int gpio_export_port(char *port) {
    FILE *export = NULL;
    size_t write_count;
    int fclose_ret = 0;
    export = fopen("/sys/class/gpio/export", "w");
    if (export == NULL) {
        printf("failed get write permission to export port %s", port);
        return -1;
    }
    write_count = fwrite(port, sizeof(char), strlen(port) + 1, export);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(export);
    if (fclose_ret == EOF) {
        return -1;
    }

    return 1;
}

int gpio_unexport_port(char *port) {
    FILE *unexport = NULL;
    size_t write_count;
    int fclose_ret = 0;
    unexport = fopen("/sys/class/gpio/unexport", "w");
    if (unexport == NULL) {
        printf("failed get write permission to unexport port %s", port);
        return -1;
    }
    write_count = fwrite(port, sizeof(char), strlen(port) + 1, unexport);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(unexport);
    if (fclose_ret == EOF) {
        return -1;
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
