#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

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

int detect_diag_port(char **diag_port)
{
	return ttyusb_dev_detect(diag_port, 0);
}

int detect_modem_port(char **modem_port)
{
	return ttyusb_dev_detect(modem_port, 3);
}


