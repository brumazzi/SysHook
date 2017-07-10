#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <linux/input.h>
#include <signal.h>
#include <stdlib.h>

#define C_FILE "/etc/hook-evt/env_stat"	// keyboard event
#define R_PID "/var/run/hook-evt.pid"	// hook process id file

#define PRINT_OK	"\x1b[32;01m*\x1b[00m"
#define PRINT_ERR	"\x1b[31;01m*\x1b[00m"
#define save_cur_p	"\x1b[s"
#define load_cur_p	"\x1b[u"

void hook_event(struct input_event ie);
void sig_term(int sig);
static int efd = 0;

int main(int argc, const char **argv){
	int uid = getuid();

	// check if is root
	if(uid != 0){
		fprintf(stderr, "You can't be run this command: Permission deined.\n");
		return 1;
	}

	int p_child = fork();
	// hook child process
	if(!p_child){
		//fprintf(stderr, "[ %s  ] Hook service\n", save_cur_p);
		
		// check if hook as running
		int fd = open(R_PID, O_RDONLY);
		if(fd != -1){
			//fprintf(stderr, "%s%s\n", load_cur_p, PRINT_ERR);
			fprintf(stderr, "Service as running.\n");
			close(fd);
			return 3;
		}

		// check if config file exists
		fd = open(C_FILE, O_RDONLY);
		if(fd == -1){
			//fprintf(stderr, "%s%s\n", load_cur_p, PRINT_ERR);
			fprintf(stderr, "\"%s\" file not found!\n", C_FILE);
			return 4;
		}

		// get event file from config file
		struct stat st_buff;
		fstat(fd, &st_buff);

		char *f_evt = malloc(st_buff.st_size+1);
		bzero(f_evt, st_buff.st_size+1);

		read(fd, f_evt, st_buff.st_size);
		close(fd);

		// open event file
		fd = open(f_evt, O_RDONLY);
		// check if event file exists
		if(!fd){
			//fprintf(stderr, "%s%s\n", load_cur_p, PRINT_ERR);
			fprintf(stderr, "\"%s\" file not found!\n", f_evt);
			free(f_evt);
			return 5;
		}
		free(f_evt);
		
		//fprintf(stderr, "%s%s\n", load_cur_p, PRINT_OK);

		struct input_event ie;
		int s_ie = sizeof(struct input_event);

		// set child process id
		FILE *log = fopen(R_PID, "w");
		fprintf(log, "%d", getpid());
		fclose(log);

		efd = fd;
		signal(SIGTERM, sig_term);
		while(read(fd, &ie, s_ie)){
			hook_event(ie);
		}
	}else if(p_child == 1){
		fprintf(stderr, "Fail to create a new process.\n");
		return 2;
	}

	return 0;
}

void sig_term(int sig){
	close(efd);
	exit(0);
}
