#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
* set_config path get context_sum													4
* set_config path get context_name <context_number>									5
* set_config path get option_value <context_name> <option_name> 					6
* set_config path set option_value <context_name> <option_name> <option_value>		7 
* set_config path add context <context_name>										5
* set_config path del context <context_name>										5 
* set_config path rename context <old_context_name> <new_context_name>				6 
* set_config path del option <context_name> <option_name>							6
*/


#define PRINTF printf
#define FPRINTF fprintf

#define LINE_SIZE 1024
#define NAME_SIZE 256
#define PATH_SIZE 256
#define BUF_SIZE 1024

void copy_file(char* src, char* dst);
void delete_file(char* path);
char* create_tmp_file_path(char* path, int size);

int get_context_sum(char* file_path);
int get_context_name(char* file_path, int number);
int add_context(char* file_path, char* name);
int del_context(char* file_path, char* context_name);
int get_option_value(char* file_path, char* context_name, char* option_name);
int del_option(char* file_path, char* context_name, char* option_name);
int set_option_value(char* file_path, char* context_name, char* option_name, char* option_value);
int rename_context(char* file_path, char* old_context, char* new_context);

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


int lock_file(const char* path)
{
	char temp_path[256];
	char lock_path[256];
	int fd;

	snprintf(temp_path,256,"%s",path);
	snprintf(lock_path,256,"/tmp/lock/%s.lock",str_replace(temp_path));

	fd = open(lock_path, O_WRONLY|O_CREAT);
	if(fd <= 0) {
		printf("open error\n");
		return -1;
	}

	//Lock
	flock(fd, LOCK_EX);

	return fd;
}

int unlock_file(int fd)
{
	if(fd <= 0) {
		return -1;
	} else {
		//UnLock
		flock(fd,LOCK_UN);
		close(fd);
		return 0;
	}
}

void usage()
{
	PRINTF("\nUsage:\n");
	PRINTF("set_config path get context_sum\n");
	PRINTF("set_config path get context_name <context_number>\n");
	PRINTF("set_config path get option_value <context_name> <option_name>\n");
	PRINTF("set_config path set option_value <context_name> <option_name> <option_value>\n"); 
	PRINTF("set_config path add context <context_name>\n");
	PRINTF("set_config path del context <context_name>\n");
	PRINTF("set_config path rename context <old_context_name> <new_context_name>\n");
	PRINTF("set_config path del option <context_name> <option_name>\n");
}

int main(int argc, char* argv[])
{
	if(argc<4 || argc >7) {
		FPRINTF(stderr,"parameter error(short)\n");
		usage();
		return -1;
	}

	switch(argc) {
	case 4:	//set_config path get context_sum
		if( (0==strcmp(argv[2],"get")) && (0==strcmp(argv[3],"context_sum")) ) {
			get_context_sum(argv[1]);
		} else {
			FPRINTF(stderr,"parameter error(invalid)\n");
			usage();
			return -1;
		}
		break;
	case 5:	//set_config path get context_name <context_number>
			//set_config path add context <context_name>
			//set_config path del context <context_name>
		if( (0==strcmp(argv[2],"get")) && (0==strcmp(argv[3],"context_name")) ) {
			get_context_name(argv[1],atoi(argv[4]));
		} else if( (0==strcmp(argv[2],"add")) && (0==strcmp(argv[3],"context")) ) {
			add_context(argv[1],argv[4]);
		} else if( (0==strcmp(argv[2],"del")) && (0==strcmp(argv[3],"context")) ) {
			del_context(argv[1],argv[4]);
		} else {
			FPRINTF(stderr,"parameter error(invalid)\n");
			usage();
			return -1;
		}

		break;
	case 6:	//set_config path get option_value <context_name> <option_name>
			//set_config path del option <context_name> <option_name>
			//set_config path rename context <old_context_name> <new_context_name>				6 
		if( (0==strcmp(argv[2],"get")) && (0==strcmp(argv[3],"option_value")) ) {
			get_option_value(argv[1],argv[4],argv[5]);
		} else if( (0==strcmp(argv[2],"del")) && (0==strcmp(argv[3],"option")) ) {
			del_option(argv[1],argv[4],argv[5]);
		} else if( (0==strcmp(argv[2],"rename")) && (0==strcmp(argv[3],"context")) ) {
			rename_context(argv[1],argv[4],argv[5]);
		} else {
			FPRINTF(stderr,"parameter error(invalid)\n");
			usage();
			return -1;
		}
		break;
	case 7:	//set_config path set option_value <context_name> <option_name> <option_value>
		if( (0==strcmp(argv[2],"set")) && (0==strcmp(argv[3],"option_value")) ) {
			set_option_value(argv[1],argv[4],argv[5],argv[6]);
		} else {
			FPRINTF(stderr,"parameter error(invalid)\n");
			usage();
			return -1;
		}
		break;
	default:
		break;
	}

	return 0;
}

int get_context_sum(char* file_path)
{
	char buf[LINE_SIZE];
	int s;
	int sum=0;
	int len;
	int out;
	int i;
	FILE* fp;
	int lock;

	lock=lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
				case '#':
				case '\r':
				case '\n':
					out=1;
					break;
				case '[':
					s=i;
					break;
				case ']':
					if( s != -1 )
						sum++;
					break;
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	unlock_file(lock);

	PRINTF("%d",sum);
}

int get_context_name(char* file_path, int number)
{
	char buf[LINE_SIZE];
	int s;
	int sum=0;
	int len;
	int out;
	int i;
	char name[NAME_SIZE];
	FILE* fp;
	int lock;

	lock=lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					sum++;
					if(sum == number) {
						memcpy(name,buf+s+1,i-s-1);
						name[i-s-1] = '\0';

						PRINTF("%s",name);
						fclose(fp);
						return 1;
					}
				}
				break;
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	unlock_file(lock);

	return 0;
}

int add_context(char* file_path, char* name)
{
	FILE* fp;
	int lock;

	lock=lock_file(file_path);

	if( NULL == (fp=fopen(file_path,"a+")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		return -1;
	}

	fseek(fp,0,SEEK_END);
	fprintf(fp,"[%s]\n",name);

	unlock_file(lock);

	fclose(fp);
}

int del_context(char* file_path,char* context_name)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int exist = 0;
	char name[NAME_SIZE];
	FILE *fp, *fp_tmp;
	char tmpfile[PATH_SIZE];
	int lock;

	create_tmp_file_path(tmpfile,PATH_SIZE);

	lock=lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		return -1;
	}

	if( NULL == (fp_tmp=fopen(tmpfile,"w+")) ) {
		FPRINTF(stderr,"Can't open %s\n",tmpfile);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						exist = 1;
						out = 1;
						break;
					} else {
						exist = 0;
					}
				}
				break;
			}
			if(out)
				break;
		}

		if(!exist) {
			fprintf(fp_tmp,"%s",buf);
		}

	}

	fclose(fp);
	fclose(fp_tmp);
	copy_file(tmpfile,file_path);
	unlock_file(lock);
	
	delete_file(tmpfile);

	return 0;
}


int get_option_value(char* file_path, char* context_name, char* option_name)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	char name[NAME_SIZE];
	FILE* fp;
	int lock;

	lock = lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						memcpy(name,buf+i+1,len-i-1-1);
						name[len-i-1-1] = '\0';
						PRINTF("%s",name);
						fclose(fp);
						return 1;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	unlock_file(lock);
	return 0;
}

int del_option(char* file_path,char* context_name,char* option_name)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int exist = 0;
	int finded = 0;
	int finish = 0;
	char name[NAME_SIZE];

	FILE *fp, *fp_tmp;
	char tmpfile[PATH_SIZE];
	int lock;

	create_tmp_file_path(tmpfile,PATH_SIZE);

	lock = lock_file(file_path);

	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	if( NULL == (fp_tmp=fopen(tmpfile,"w+")) ) {
		FPRINTF(stderr,"Can't open %s\n",tmpfile);
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0 == strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						exist = 1;
						out = 1;
						break;
					}
				}
			}
			if(out)
				break;
		}

		if(!exist) {
			fprintf(fp_tmp,"%s",buf);
		} else {
			exist = 0;
		}
	}

	fclose(fp);
	fclose(fp_tmp);
	copy_file(tmpfile,file_path);
	unlock_file(lock);
	delete_file(tmpfile);

	return 0;
}

// modify history: 
// 1.	当context下的无此option时，添加新的option_name=option_value
// 		date: 2016/3/10
int set_option_value(char* file_path, char* context_name, char* option_name, char* option_value)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	int exist = 0;
	char name[NAME_SIZE];
	FILE *fp, *fp_tmp;
	char tmpfile[PATH_SIZE];
	int lock;
	int over = 0;
	int new_option_added = 0;

	create_tmp_file_path(tmpfile,PATH_SIZE);

	lock = lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	if( NULL == (fp_tmp=fopen(tmpfile,"w+")) ) {
		FPRINTF(stderr,"Can't open %s\n",tmpfile);
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				// 遇到了[标志，但此时已经找到了contex并且没有找到option
				// 则添加新的option
				if (finded && !over) {
					fprintf(fp_tmp, "%s=%s\n", option_name, option_value);
					new_option_added = 1;
					out = 1;	
				}
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					finded = 0;
					if( 0==strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded && !over) { //new
							finish = 1;
							finded = 0;

							fprintf(fp_tmp,"%s=%s\n",option_name,option_value);
							out=1;
							break;
						}
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) { //exists
						exist = 1;
						over = 1;
						fprintf(fp_tmp,"%s=%s\n",option_name,option_value);
						out=1;
						break;
					}
				}
			}
			if(out)
				break;
		}

		if(!exist) {
			fprintf(fp_tmp,"%s",buf);
		} else {
			exist = 0;
		}
	}

	// 如果存在contex，不存在option，且没有新添加此option，则添加
	if (finded && !over && !new_option_added) {
		fprintf(fp_tmp, "%s=%s\n", option_name, option_value);
	}
	fclose(fp);
	fclose(fp_tmp);
	copy_file(tmpfile,file_path);
	unlock_file(lock);
	delete_file(tmpfile);

	return 0;
}

int rename_context(char* file_path, char* old_context, char* new_context)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int exist = 0;
	char name[NAME_SIZE];
	FILE *fp, *fp_tmp;
	char tmpfile[PATH_SIZE];
	int lock;

	create_tmp_file_path(tmpfile,PATH_SIZE);

	lock = lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		FPRINTF(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	if( NULL == (fp_tmp=fopen(tmpfile,"w+")) ) {
		FPRINTF(stderr,"Can't open %s\n",tmpfile);
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,old_context) ) {
						fprintf(fp_tmp,"[%s]\n",new_context);
						exist = 1;
						out = 1;
						break;
					}
				}
				break;
			}
			if(out)
				break;
		}

		if(!exist) {
			fprintf(fp_tmp,"%s",buf);
		} else {
			exist = 0;
		}

	}

	fclose(fp);
	fclose(fp_tmp);
	copy_file(tmpfile,file_path);
	unlock_file(lock);
	delete_file(tmpfile);

	return 0;
}

void copy_file(char* src, char* dst)
{
	char buf[BUF_SIZE];

	snprintf(buf,BUF_SIZE,"cp %s %s",src,dst);

	system(buf);
}

void delete_file(char* path)
{
	char buf[BUF_SIZE];
	snprintf(buf,BUF_SIZE,"rm %s",path);
	system(buf);
}

char* create_tmp_file_path(char* path, int size)
{
	snprintf(path,size,"/tmp/%ld",time(NULL));

	return path;
}

