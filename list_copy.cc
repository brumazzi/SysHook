#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "key-hook.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

static const char *root_check = "\
if [ \"$EUID\" -eq \"0\" ]; then\n\
	exit 1\n\
else\n\
	exit 0\n\
fi";

#define IS_ROOT system(root_check)

int main(int argc, const char **argv){
	if(!IS_ROOT){
		write(1, "This service required root privileges.\n", 39);
		char prog[512] = {0};
		sprintf(prog,"gksu %s", argv[0]);
		system(prog);
		return 1;
	}
	int efd = open_input();

	if(!efd){
		write(1, "Can't be possible open key event.\n", 34);
		return 2;
	}

	char *buff;
	struct input_event ie;
	char KEY[256];
	bool lcopy = false;
	bzero(KEY, 256);

	vector<const char *> vec;

	while(read(efd, &ie, sizeof(struct input_event))){
		if(ie.value == 1)
			KEY[ie.code] = 1;
		else if(ie.value == 0)
			KEY[ie.code] = 0;
		else if(ie.value != 2){
			if(lcopy){
				if(KEY[KEY_LEFTCTRL] && (KEY[KEY_C] || KEY[KEY_X])){
					bzero(KEY, 256);
					system("xclip -o -d \"$DISPLAY\" > /tmp/xcopy");

					int fd = open("/tmp/xcopy", O_RDONLY);
					struct stat st;
					fstat(fd, &st);

					buff = new char[st.st_size];
					read(fd, buff, st.st_size);
					close(fd);

					printf("Copy<%s>\n", buff);

					vec.insert(vec.end(),(const char *) buff);
				}else if(KEY[KEY_LEFTCTRL] && KEY[KEY_V]){
					bzero(KEY, 256);
					if(vec.size() == 0)
						continue;
					string comm = "printf '";
					comm += vec[0];
					comm += "' | xsel -b";
					system(comm.c_str());

					char *del_bf = (char *) vec[0];
					vec.erase(vec.begin());
					delete del_bf;
				}
			}
			if((KEY[KEY_LEFTCTRL] && KEY[KEY_LEFTSHIFT] && KEY[KEY_LEFTALT]) && KEY[KEY_UP]){
				puts("UP");
				lcopy = true;
			}else if((KEY[KEY_LEFTCTRL] && KEY[KEY_LEFTSHIFT] && KEY[KEY_LEFTALT]) && KEY[KEY_DOWN]){
				puts("DOWN");
				lcopy = false;
				while(vec.size() > 0){
					char *del = (char *) vec[0];
					vec.erase(vec.begin());
					delete del;
				}
			}
		}
	}
}
