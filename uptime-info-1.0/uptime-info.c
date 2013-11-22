#include <stdio.h>
#include <sys/sysinfo.h>
#include <unistd.h>

int main()
{
	struct sysinfo info;
	
	while(1) {
	    if (!sysinfo(&info)) {
	        printf("Uptime: %ld\n", info.uptime);
	    } else {
		    printf("Error calling sysinfo()!\n");
	    }
	    sleep(1);
	}
	
	return 0;
}
