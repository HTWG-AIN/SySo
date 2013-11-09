#include <stdio.h>
#include <sys/sysinfo.h>
#include <unistd.h>

int main()
{
	struct sysinfo info;

	if (!sysinfo(&info)) {
		long pagesize = sysconf(_SC_PAGESIZE);

		printf("Hello User world\n");
		printf("Uptime: %ld\n", info.uptime);
		printf("Total RAM: %lu MB \n", info.totalram*info.mem_unit/1024/1024);
		printf("Free RAM: %lu MB\n", info.freeram*info.mem_unit/1024/1024);
		printf("Process count: %d\n", info.procs);
		printf("Page size: %ld Bytes \n", pagesize);
		printf("Mem Unit:%u\n", info.mem_unit);
	}
	else {
		printf("Error calling sysinfo()!\n");
	}
	
	return 0;
}
