#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>

#define TRACER_ADD_PROCESS		_IOW(_IOC_WRITE, 42, pid_t)
#define TRACER_REMOVE_PROCESS	_IOW(_IOC_WRITE, 43, pid_t)

int main(int argc, char *argv[])
{
	int rc;
	pid_t pid;
	int fd;

	if (argc != 3) {
		printf("aiurea argumente\n");
		return 1;
	}

	fd = open("/dev/tracer", O_RDONLY);
	pid = atoi(argv[2]);

	if (!strncmp(argv[1], "ADD", 3))
		rc = ioctl(fd, TRACER_ADD_PROCESS, pid);
	else if (!strncmp(argv[1], "DEL", 3))
		rc = ioctl(fd, TRACER_REMOVE_PROCESS, pid);

	if (rc < 0) {
		printf("helper-ul a crapat. n-a putut ioctl()\n");
		return rc;
	}

	return 0;
}