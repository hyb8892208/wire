#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

const char *str_replace(char *path)
{
	int i;
	int len = strlen(path);
	for(i=0; i<len; i++) {
		if(path[i] == '/') {
			path[i] = '_';
		}
	}
	return path;
}

int main(int argc, char* argv[])
{
	int fd;

	const char *file_path;
	char temp_path[256];
	char lock_path[256];
	char *content;

	if(argc != 3) {
		printf("Usage: safe_append_to_file file_path content\n");
		return -1;
	}
	
	file_path=argv[1];
	content=argv[2];

	snprintf(temp_path,256,"%s",file_path);
	snprintf(lock_path,256,"/tmp/lock/%s.lock",str_replace(temp_path));

	fd = open(lock_path, O_WRONLY|O_CREAT);
	if(fd < 0) {
		printf("open error\n");
		return -1;
	}

	//Lock
	flock(fd, LOCK_EX);

	FILE *fp;
	fp = fopen(file_path,"a+");
	if(fp != NULL) {
		fprintf(fp,"%s\n",content);
		fclose(fp);
	}

	//UnLock
	flock(fd,LOCK_UN);
	close(fd);

	return 0;
}

