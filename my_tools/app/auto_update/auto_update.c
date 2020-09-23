#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define OPENSSL_PASSWORD            "O7r8yGv0cdlNM4lQ"
#define CHECK_FILE_ERROR_BASE		140
#define DO_FETCH_FILE_ERROR_BASE    240

#define HEADFLAG_NEW 0x2902871A       /* 新版本标志 */

/* 新版本文件头格式(32字节) */
struct openvox_head
{
	char  magic[4];        // flag of image 
    short version;         // version of image format
    short major_hwid;      // hardware type major id 
    short minor_hwid;      // hardware type minor id 
    char  major_ver;       // major firmware version id 
    char  minor_ver;       // minor firmware version id 
    short patch_ver;       // patch firmware version id software version is major_ver.minor_ver.patch_ver.buildnumber 
    short build_num;       // firmware build number.
    int   reserved[4];     // reserved space 
};

int check_file(char *file_name);
int do_fetch_file(char *src_file, char *dst_file);

static void opt_help()
{
	printf("===================auto_update help===================\n");
	printf("-h: help\n");
	printf("-c: check file [-c file]\n");
	printf("-o: use Openssl fetch file [-o src_file -f dst_file]\n");
	printf("=================================================\n");
	printf("\n");
}

int main(int argc,char **argv)
{
	int ret=0;
	int ch;
	int check_flag=0;
	int openssl_flag = 0;
	char *src_file = NULL;
	char *dst_file = NULL;

	while ((ch = getopt(argc, argv, "hc:f:o:")) != -1)
	{
		switch (ch) 
		{
			case 'c':
				check_flag = 1;
				src_file=optarg;
				break;

			case 'f':
				dst_file=optarg;
				break;

			case 'o':
				openssl_flag = 1;
				src_file = optarg;
				break;
			case 'h':
			default:
				opt_help();
				return 0;
		}
	}

	if ( check_flag )
	{
	    ret = check_file(src_file);
		if ( 1 == ret )
		{
			printf("file type: new\n");
		}
		else
		{
		    printf("file type: check fail or unknown file type\n");
			printf("ret = %d\n", ret);
		}
		return ret;
	}

	if ( openssl_flag )
	{
	    ret = do_fetch_file(src_file, dst_file);

		if(ret)
		{
			printf("fetch file %s failed:%d\n", src_file, ret);
			ret=abs(ret);
		}
		else
		{
			printf("fetch file success\n");
		}
		return ret;
	}

	return 0;
}

/******************************************************* 
功能描述: 检查指定文件，返回文件类型
输入参数: file_name -- 指定文件名
输出参数: 无
返回值  : =0 代表老或者中间版本升级文件，=1代表新版本文件或者未知类型，其它代表出错
备注    : 无
********************************************************/
int check_file(char *file_name)
{
	int fd = -1;
	int error_status=0;
	struct openvox_head head;

	if ( file_name == NULL )
	{
		error_status = -(CHECK_FILE_ERROR_BASE + 0);
		return error_status;
	}

	fd=open(file_name, O_RDONLY);
	if(fd<=0)
	{
		error_status=-(CHECK_FILE_ERROR_BASE+1);
		return error_status;
	}

	if ( lseek(fd, 0, SEEK_SET) == -1 ) 
	{
		error_status=-(CHECK_FILE_ERROR_BASE+2);
		goto failed_mmap;
	}

    memset(&head, 0, sizeof(head));

    if ( read(fd, &head, sizeof(head)) != sizeof(head) ) 
	{
		error_status=-(CHECK_FILE_ERROR_BASE+3);
		goto failed_mmap;
	}

	if ( HEADFLAG_NEW == *(unsigned int*)&(head.magic[0]) )
	{
	    error_status = 1;
	}
	else
	{
	    error_status = 2;
	}
	
#if 1
    printf("file head info:\n");
    printf("\t magic        : 0x%08x\n", *(unsigned int*)&(head.magic[0]));
	printf("\t version      : 0x%x\n", head.version);
	printf("\t major_hwid   : 0x%x\n", head.major_hwid);
	printf("\t minor_hwid   : 0x%x\n", head.minor_hwid);
	printf("\t major_ver    : 0x%x\n", head.major_ver);
	printf("\t minor_ver    : 0x%x\n", head.minor_ver);
	printf("\t patch_ver    : 0x%x\n", head.patch_ver);
	printf("\t build_num    : 0x%x\n", head.build_num);
#endif

failed_mmap:	
	close(fd);
	return error_status;
 }
 
 /******************************************************* 
功能描述: 使用openssl解密指定文件
输入参数: src_file -- 要解密的文件
          dst_file -- 已解密的文件
输出参数: 无
返回值  : =0 代表成功，非0代表出错
备注    : 无
********************************************************/
int do_fetch_file(char *src_file, char *dst_file)
{
    char str[256];

    if ( (NULL == src_file) || (NULL == dst_file) )
	{
	    printf("invalid param, file = NULL\r\n");
	    return -(DO_FETCH_FILE_ERROR_BASE + 0);
	}

	if ( access(src_file, 0) != 0 )
	{
	    printf("can not access file %s.\r\n", src_file);
	    return -(DO_FETCH_FILE_ERROR_BASE + 1);
	}

    /* 目标文件已存在就删除 */
	if ( 0 == access(dst_file, 0) )
	{
	    remove(dst_file);
	}

	memset(str, '\0', sizeof(str));
	snprintf(str, sizeof(str), "openssl enc -aes-256-cbc -d -in %s -pass pass:%s -out %s", src_file, OPENSSL_PASSWORD, dst_file);
	//printf("src: %s\n",str);
	(void)system(str);	

    return 0;
}

 
