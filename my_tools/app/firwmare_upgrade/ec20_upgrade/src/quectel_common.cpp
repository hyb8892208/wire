#include <stdio.h>
#include "quectel_common.h"
#include "platform_def.h"
#include "quectel_log.h"
#include "openvox_version_record.h"


#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
static int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;  /* write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;  //lock the whole file

	return(fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) 
	{
        printf("can't open %s: %m\n", filename);
        exit(1);
	}

	/* �Ȼ�ȡ�ļ��� */
	if (lockfile(fd) == -1) {
        if (errno == EACCES || errno == EAGAIN) 
        {
            printf("file: %s already locked", filename);
            close(fd);
            return 1;
        }
        printf("can't lock %s: %m\n", filename);
        exit(1);
	}
	/* д������ʵ����pid */
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}


int is_emergency_diag_port()
{
	struct dirent* ent = NULL;
	DIR* pDir;
	char dir[255] = "/sys/bus/usb/devices";
	pDir = opendir(dir);
	int ret = 1;
	if(pDir)
	{
		while((ent = readdir(pDir)) != NULL)
		{
			struct dirent* subent = NULL;
			DIR *psubDir;
			char subdir[255];
			char dev[255];
			char idVendor[4 + 1] = {0};
			char idProduct[4 + 1] = {0};
			char number[10] = {0};
			int fd = 0;

			char diag_port[32] = "\0";

            snprintf(subdir, sizeof(subdir), "%s/%s/idVendor", dir, ent->d_name);
            fd = open(subdir, O_RDONLY);
            if (fd > 0) {
                read(fd, idVendor, 4);
                close(fd);
             }else
             {
             	continue;
             }

            snprintf(subdir, sizeof(subdir), "%s/%s/idProduct", dir, ent->d_name);
            fd  = open(subdir, O_RDONLY);
            if (fd > 0) {
                read(fd, idProduct, 4);
                close(fd);
            }else
            {
            	continue;
            }            
            
            if (!strncasecmp(idVendor, "05c6", 4) || !strncasecmp(idVendor, "2c7c", 4))
                ;
            else
            	continue;            
            snprintf(subdir, sizeof(subdir), "%s/%s:1.%d",dir, ent->d_name, 0);   
            if(!strncasecmp(idVendor, "05c6", 4))
            {
            	return 0;
            }                             
		}
		closedir(pDir);
	}else
	{
		return -ENODEV;
	}
	return ret;
}

/*
interface 0 --- > diag port
interface 1 --- > nmea port
interface 2 --- > at port
interface 3 --- > modem port
interface 4 --- > rmnet

*/
static int ttyusb_dev_detect(char** pp_diag_port, int interface)
{
	struct dirent* ent = NULL;
	DIR* pDir;
	char dir[255] = "/sys/bus/usb/devices";
	pDir = opendir(dir);
	int ret = 1;
	if(pDir)
	{
		while((ent = readdir(pDir)) != NULL)
		{
			struct dirent* subent = NULL;
			DIR *psubDir;
			char subdir[255];
			char dev[255];
			char idVendor[4 + 1] = {0};
			char idProduct[4 + 1] = {0};
			char number[10] = {0};
			int fd = 0;

			char diag_port[32] = "\0";

            snprintf(subdir, sizeof(subdir), "%s/%s/idVendor", dir, ent->d_name);
            fd = open(subdir, O_RDONLY);
            if (fd > 0) {
                read(fd, idVendor, 4);
                close(fd);
             }else
             {
             	continue;
             }

            snprintf(subdir, sizeof(subdir), "%s/%s/idProduct", dir, ent->d_name);
            fd  = open(subdir, O_RDONLY);
            if (fd > 0) {
                read(fd, idProduct, 4);
                close(fd);
            }else
            {
            	continue;
            }            
            
            if (!strncasecmp(idVendor, "05c6", 4) || !strncasecmp(idVendor, "2c7c", 4))
                ;
            else
            	continue;            
            //snprintf(subdir, sizeof(subdir), "%s/%s:1.0",dir, ent->d_name);   
            snprintf(subdir, sizeof(subdir), "%s/%s:1.%d",dir, ent->d_name, interface);   
            
            
            psubDir = opendir(subdir);
            if(psubDir == NULL)
            {
            	continue;
            }
            while((subent = readdir(psubDir)) != NULL)
            {
            	if(subent->d_name[0] == '.')
            		continue;            	
            	if(!strncasecmp(subent->d_name, "ttyUSB", 6))
            	{
            		strcpy(diag_port, subent->d_name);
            		break;         
            	}
            }
            closedir(psubDir);    
            if(pp_diag_port != NULL)
            {
            	snprintf(dev, sizeof(dev), "/dev/%s",diag_port);
#if 0 
            	if((fd = open(dev, R_OK)) < 0)
            	{
            		QFLASH_LOGD("%s open failed!\n", dev);            		
            	}else
            	{
            		close(fd);
            	}
#endif
	       		*pp_diag_port = strdup(diag_port);
	       		closedir(pDir);
            	return 0;
            }            

		}
		closedir(pDir);
	}else
	{
		return -ENODEV;
	}
	return ret;
}
static int charsplit(const char *src,char* desc,int n,const char* splitStr)
{
	char* p;
	char*p1;
	int len;
	
	len = strlen(splitStr);
	p = strstr((char*)src,splitStr);
	if(p == NULL)
		return -1;
	p1 = strstr(p,"\n");
	if(p1 == NULL)
		return -1;
	memset(desc,0,n);
	memcpy(desc,p+len,p1-p-len);
	
	return 0;
}
static int  CreateDir(const   char   *sPathName)  
{  
	char DirName[256];  
	strcpy(DirName,sPathName);  
	int  i,len  = strlen(DirName);  
	if(DirName[len-1]!='/')  
	strcat(DirName,   "/");  
	len=strlen(DirName);  
	for(i=1; i<len;i++)  
	{  
		if(DirName[i]=='/')  
		{  
			DirName[i]   =   0;  
			if(opendir(DirName)==0)  
			{  
				if(mkdir(DirName,   0755)==-1)  
				{   
					printf("mkdir %s\n",DirName);   
					return   -1;   
				}  
			}  
			DirName[i]   =   '/';  
		}  
	}  
	return   0;  
} 
int detect_diag_port(char **diag_port)
{
	return ttyusb_dev_detect(diag_port, 0);
}
int detect_modem_port(char **modem_port)
{
	return ttyusb_dev_detect(modem_port, 3);
}

int detect_adb()
{
	int re = 0;
	const char* base = "/sys/bus/usb/devices";
	struct dirent *de;
	char busname[64], devname[64];
	int fd;
	int writable;
	int n;
	DIR *busdir, *devdir;
	char desc[1024];
	char busnum[64],devnum[64],devmajor[64],devminor[64];
	char buspath[128],devpath[128];
	
	busdir = opendir(base);
	if(busdir == 0) 
		return -1;
	while(de = readdir(busdir))
	{
		sprintf(busname, "%s/%s", base, de->d_name);
		devdir = opendir(busname);
		if(devdir == 0) 
			continue;
		while((de = readdir(devdir)) )
		{
			sprintf(devname, "%s/%s", busname, de->d_name);
			if(strstr(devname,"uevent")!=NULL)
			{
				writable = 1;
				if((fd = open(devname, O_RDWR)) < 0)
				{
					writable = 0;
					if((fd = open(devname, O_RDONLY)) < 0)
						continue;
				}
				memset(desc,0,1024);
				n = read(fd, desc, sizeof(desc));
				desc[n-1] = '\n';
				desc[n] = '\0';
				
				if(strstr(desc,"18d1/d00d") != NULL && strstr(desc,"DEVTYPE=usb_device") != NULL)
				{
					if(charsplit(desc,busnum, 64, "BUSNUM=")==-1||
					charsplit(desc,devnum, 64, "DEVNUM=")==-1||
					charsplit(desc,devmajor, 64, "MAJOR=")==-1||
					charsplit(desc,devminor, 64, "MINOR=")==-1)
					{
						printf("[FASTBOOT] Split String Error\n");
						close(fd);
						closedir(devdir);
						closedir(busdir);
						return 2;
					}
					memset(buspath,0,128);
					memset(devpath,0,128);
					sprintf(buspath,"/dev/bus/usb/%s",busnum);
					sprintf(devpath,"/dev/bus/usb/%s/%s",busnum,devnum);

					if(access(devpath, R_OK | W_OK))
					{
						printf("test %s Read/WRITE failed\n", devpath);
						if(access(devpath, F_OK))
						{
							printf("error: %s is not existent\n", devpath);
							if(CreateDir(buspath))
							{
								printf("Create %s failed!\n", buspath);	
								close(fd);
								closedir(devdir);
								closedir(busdir);
								return 2;
							}else
							{
								printf("create adb port direcotry OK\n");
								if((0 != mknod(devpath,  S_IFCHR|0666, makedev(atoi(devmajor),atoi(devminor)))))
								{
									printf("mknod for %s failed, MAJOR = %s, MINOR =%s, errno = %d(%s)\n", devpath, devmajor,devminor, errno, strerror(errno));
									close(fd);
									closedir(devdir);
									closedir(busdir);
									return 3;
								}else
								{
									printf("mknod %s OK\n", devpath);
								}
							}
						}
						if(!chmod(devpath, 0666))
						{
							printf("chmod %s 0666 success\n", devpath);
						}else
						{
							printf("chmod %s 0666 failed\n", devpath);
						}
					}else
					{
						printf("test %s Read/WRITE OK\n", devpath);						
					}					

#if 0
					if (access(buspath, R_OK))
					{
						if(CreateDir(buspath)!=0)
						{
							printf("[FASTBOOT] Create dir[%s] failed\n",buspath);
							close(fd);
							closedir(devdir);
							closedir(busdir);
							return 3;
						}
					}


					if (!access(buspath, R_OK) && access(devpath, F_OK))
					{												
						printf("mknod %s, MAJOR = %s, MINOR =%s\n", devpath, devmajor,devminor);
						if((0 != mknod(devpath,  S_IFCHR|0666, makedev(atoi(devmajor),atoi(devminor)))))
						{
							printf("mknod for %s failed, MAJOR = %s, MINOR =%s, errno = %d(%s)\n", devpath, devmajor,devminor, errno, strerror(errno));
							close(fd);
							closedir(devdir);
							closedir(busdir);
							return 4;
						}
					}
#endif
					close(fd);
					closedir(devdir);
					closedir(busdir);
					return 0;
				}			
				close(fd);
			}
		}
		closedir(devdir);
	}
	closedir(busdir);
	return 1;
}


int checkCPU()
{
   short int test = 0x1234;
   if(*((char *)&test)== 0x12)
   	{
		return 1;
	
   	}
   return 0;
}
int detect_diag_port()
{
	char* diag_port = 0;
	if(ttyusb_dev_detect(&diag_port, 0) == 0)
	{
		free(diag_port);
		return 0;
	}
	return 1;
}

int wait_adb(int timeout)
{
	timeout = timeout * 1000;
	int t = 0;
	do{
		if(detect_adb() == 0)
		{
			printf("Detect Adb port\n");
			return 0;
		}
		usleep(1000 * 5);			// 1 ms
		t++;
		if(t > timeout)
		{
			return 1;
		}
	}while(1);
	return 1;
}

int wait_diag_port_disconnect(int timeout)
{
	int t = 0;
	timeout = timeout * 1000;
	do{
		if(detect_diag_port() != 0)
		{
			return 0;
		}
		usleep(1000 * 5);			// 1 ms
		t++;
		if(t > timeout)
		{
			return 1;
		}
	}while(1);
	return 1;
}

int detect_diag_port_timeout(int timeout)
{
	int t = 0;
	timeout > 1?t = timeout: t = 1;
	do
	{
		sleep(1);
		if(detect_diag_port() == 0)
		{
			//find diag port
			printf("Diagnose port connected.\n");
			return 0;
		}
		t--;
		if(t == 0)
		{
			//timeout
			return 1;
		}
	}while(1);
	
	return 2;
}



int probe_quectel_speed(enum usb_speed* speed)
{
	FILE* fpin1 = 0, *fpin2 = 0;
	FILE* fpin3 = 0;
	char *line = (char*)malloc(MAX_PATH);
	char *command = (char*)malloc(MAX_PATH + MAX_PATH + 32);	
	char *p = 0;
	char devicename[50];
	int ret = 1;

	sprintf(command, "ls /sys/bus/usb/devices/");
	fpin1 = popen(command, "r");
	if(!fpin1)	goto _exit_;

	while(fgets(line, MAX_PATH - 1, fpin1) != NULL)
	{		
		p = line;
		while(*p != '\n' && p != 0) p++;
		*p = 0;			
		sprintf(command, "cat /sys/bus/usb/devices/%s/idVendor", line);
		if(strlen(p) < 49)
		{
			memset(devicename, 0, 50);
			strcpy(devicename, line);
		}
		fpin2 = popen(command, "r");
		if(!fpin2) goto _exit_;
		
		while(fgets(line, MAX_PATH - 1, fpin2) != 0)
		{		
			if(strstr(line, "2c7c") != 0)
			{
				printf("find Quectel device!\n");
				p = line;
				while(*p != '\n' && p != 0) p++;
				*p = 0;	
				sprintf(command, "cat /sys/bus/usb/devices/%s/speed", devicename);
				fpin3 = popen(command, "r");
				if(!fpin3) goto _exit_;
				if(fgets(line, MAX_PATH - 1, fpin3) != 0)
				{ // refer to :http://stackoverflow.com/questions/1957589/usb-port-speed-linux
					printf("speed = %s", line);
					if(strstr(line, "480") != NULL)
					{
						*speed = usb_highspeed;
					}
					else if(strstr(line, "12") != 0 ||
							strstr(line, "1.5") != 0)
					{
						*speed = usb_fullspeed;
					}else
					{
						*speed = usb_superspeed;
					}
					ret = 0;
					goto _exit_;
				}				
			}
			fclose(fpin3); fpin3 = 0;
		}
		fclose(fpin2); fpin2 = 0;
	}
_exit_:
	if(line)
	{
		free(line);line = 0;
	}
	if(command)
	{
		free(command); command = 0;
	}
	if(fpin1)
	{
		fclose(fpin1);
	}
	if(fpin2)
	{
		fclose(fpin2);
	}
	if(fpin3)
	{
		fclose(fpin3);
	}
	return ret;
}



void strToLower(char* src)
{
	for(char* ptr = src; *ptr; ++ptr)
	{
		*ptr = tolower(*ptr);
	}
}

int open_port_once(int ioflush)
{
	extern int openport(int ioflush);
	int ret =  openport(ioflush);
	if(ret != 0)
		transfer_statistics::getInstance()->process_bar((char *)"open port failed!");
	return ret;
}

void upgrade_process(int writesize,int size,int clear)
{
	unsigned long long tmp=(unsigned long long)writesize * 100;
	unsigned int progress = tmp / size;
    if(progress == 100)
    {
        printf( "progress : %d%% finished \n", progress);        
        fflush(stdout);
    }
    else
    {
        printf( "progress : %d%% finished \r", progress);
        //printf("process: %d%% finished, All process = %d%%\r", progress, transfer_statistics::getInstance()->get_percent());
        fflush(stdout);
    }
    
}


int show_user_group_name()
{
	struct passwd* passwd;
	passwd = getpwuid(getuid());
	printf("------------------\n");
	printf("User:\t %s\n",passwd->pw_name);
	struct group* group;
	group = getgrgid(passwd->pw_gid);
	printf("Group:\t %s\n", group->gr_name);
	printf("------------------\n");
	return 0;
}

double get_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000;
}

transfer_statistics* transfer_statistics::instance = 0;


transfer_statistics* transfer_statistics::getInstance()
{
	if(instance == 0)
	{
		instance = new transfer_statistics();
	}
	return instance;
}

void transfer_statistics::set_total(long long all_files_size/*kb*/)
{
	m_all_files_bytes = all_files_size;
}
void transfer_statistics::set_write_bytes(long long transfer_bytes)
{
	m_transfer_bytes += transfer_bytes;
}

int transfer_statistics::get_percent()
{
	if(m_all_files_bytes == 0)
	{
		return 0;
	}else
	{
		if(m_all_files_bytes > m_transfer_bytes)
		{
			return 100 * m_transfer_bytes / m_all_files_bytes;
		}else
		{
			return 100.0f;
		}
	}
	return 0.f;
}



transfer_statistics::transfer_statistics()
{
	m_all_files_bytes = 0;
	m_transfer_bytes = 0;
//	m_channel=channel;
	char filename[128] = {0};
	sprintf(filename, "/www/chn_%d", module_info::getInstance()->get_channel());
	m_logfd = fopen(filename, "w+");
	if(m_logfd < 0)
		m_logfd=stdout;
	m_lastsize=0;
}
transfer_statistics::~transfer_statistics(){
	if(m_logfd){
		fclose(m_logfd);
		m_logfd = NULL;
	}
}
transfer_statistics::transfer_statistics(const transfer_statistics&)
{
	m_all_files_bytes = 0;
	m_transfer_bytes = 0;
}
transfer_statistics& transfer_statistics::operator=(const transfer_statistics&)
{
}
unsigned long get_file_size(const char* filename)
{
	unsigned long size;
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		printf("Open file %s failed.\n", filename);
		return 0;
	}
	fseek(fp, SEEK_SET, SEEK_END);
	size = ftell(fp);
	fclose(fp);
	return size;
}	

void transfer_statistics::process_bar(char *result){
	fseek(m_logfd, 0 - m_lastsize, SEEK_CUR );
	if(!result)//����ǿգ����������
		m_lastsize = fprintf(m_logfd, "%d", (m_transfer_bytes*100/m_all_files_bytes));
	else//�������ʧ�ܻ�ɹ���ԭ��
		m_lastsize = fprintf(m_logfd, "%s", result);
}



