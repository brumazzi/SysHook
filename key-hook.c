#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <limits.h>

#define EVENT_VAR "/var/log/hook-key/evt-fd"
#define RUNNING_VAR "/var/log/hook-key/running"

int open_input(){
	int evar = open(EVENT_VAR, O_RDONLY);
	if(!evar)
		return 0;

	char evt_fdp[PATH_MAX];
	struct stat buff;
	off_t fsize;

	fstat(evar, &buff);
	fsize = buff.st_size;

	read(evar, evt_fdp, fsize);
	evt_fdp[fsize] = 0;
	close(evar);
	
	int efd = open(evt_fdp, O_RDONLY);

	return efd;
}

int running(){
	int run_fd = open(RUNNING_VAR, O_RDONLY);
	int running = 0;

	read(run_fd, &running, 1);
	close(run_fd);

	return running-48;
}
