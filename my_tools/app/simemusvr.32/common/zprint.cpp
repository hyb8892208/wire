
/*
 *      gcc zhl_print.c -fPIC -shared -o libzhlprint.so
 *      arm-hismall-linux-gcc zhl_print.c -fPIC -shared -o libzhlprint.so
 */


#include "zprint.h"
       
       

#define BUF_LEN 1024


FILE *fp = NULL;

int apdu_switch = 0;
int log_class = INFO | WARN | ERROR;
/*
int main(void)
{
    zhl_print("%s--%c--%d", "abcd", '#', 1234);
    
    return 0;
}
*/



char * get_proc_name(pid_t pid, char *proc_name, int len)
{
    char buf[128];
    char *pbuf = buf;
    int cplen;
    
    memset(buf, 0, sizeof(buf));
	readlink("/proc/self/exe", buf, sizeof(buf));
	pbuf = rindex(buf, '/');
	pbuf +=1;
	cplen = strlen(pbuf);
	cplen = (cplen < len) ? cplen : len;
	cplen = (cplen < 128) ? cplen : 128;
	memcpy(proc_name, pbuf, cplen);
	
    return proc_name;
}

#if 0
char *str_timestamp(time_t tt, char *tstr, int tstrlen)
{
	struct tm t;

	if(!tt)
	{
	    tt = time(NULL);
	}
	localtime_r(&tt, &t);
	strftime(tstr, tstrlen, "%Y-%m-%d %X", &t);
	return tstr;
}
#else
char *str_timestamp(time_t tt, char *tstr, int tstrlen)
{
	struct timeval tv;
	struct tm *ptm = NULL;
	
	gettimeofday(&tv, NULL);
	ptm = localtime(&tv.tv_sec);

	sprintf(tstr, "%04d-%02d-%02d %02d:%02d:%02d'%06d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)tv.tv_usec);
	
	return tstr;
}

#endif

void z_printf(unsigned int flags,const char *file,const int line,const char *format, ...)
{
	if(0 == (flags & log_class)){
		return ;
	}
	
	va_list va_args;
	char buf[BUF_LEN];
	char *pbuf = buf;
	pid_t pid;
	char proc_name[128];
	char buf_time[64];
	
	
    // time
    memset(buf_time, 0, sizeof(buf_time));
    str_timestamp(0, buf_time, sizeof(buf_time));
    
	// proc name
	memset(proc_name, 0, sizeof(proc_name));
	
	pid = getpid();
	get_proc_name(pid, proc_name, sizeof(proc_name));

	va_start(va_args, format);
	sprintf(pbuf, "[%s][%s]{%d}: ", buf_time, proc_name, pid);
	pbuf += strlen(pbuf);
	vsnprintf(pbuf, BUF_LEN, format, va_args);
	pbuf += strlen(pbuf);
	sprintf(pbuf, " {%s->%d}", file, line);
	if(fp){
		fprintf(fp,"%s\n",buf);
	}
	else{
		puts(buf);
	}
	va_end(va_args);
}

int openlogfile(char *filename)
{
	fp = fopen(filename,"ab+");
	if(!fp){
		return -1;
	}
	return 0;
}

int open_apdu_log_file(int bank_nbr,int slot_nbr)
{
	char filename[20]={0};
	sprintf(filename,"/tmp/log/SimEmuSvr/%d",bank_nbr*8 + slot_nbr + 1);
	int fd = open(filename,O_CREAT|O_WRONLY|O_APPEND);
	return fd;
}

int apduInfo(unsigned char *apdu,char *buff)
{
	if(NULL == apdu || NULL == buff){
		return 0;
	}
	if(0xa0 != apdu[0] && 0x00 != apdu[0]){
		return 0;
	}

	int len = 0;
	switch(apdu[1])
	{
		case 0xA4:
			len = sprintf(buff,"SELECT command\n");
			break;
		case 0xF2:
			len = sprintf(buff,"STATUS command,length=%d\n",apdu[4]);
			break;
		case 0xB0:
			len = sprintf(buff,"READ BINARY,offset high %d bits,offset low %d bits,length=%d\n",apdu[2],apdu[3],apdu[4]);
			break;
		case 0xD6:
			len = sprintf(buff,"UPDATE BINARY,offset high %d bits,offset low %d bits,length=%d\n",apdu[2],apdu[3],apdu[4]);
			break;
		case 0xB2:
			len = sprintf(buff,"READ RECORD,RECORD No.=0x%x,mode=%d,length=%d\n",apdu[2],apdu[3],apdu[4]);
			break;
		case 0xDC:
			len = sprintf(buff,"UPDATE RECORD,RECORD No.=0x%x,mode=%d,length=%d\n",apdu[2],apdu[3],apdu[4]);
			break;
		case 0xA2:
			len = sprintf(buff,"SEEK command,mode=%d,length=%d\n",apdu[3],apdu[4]);
			break;
		case 0x32:
			len = sprintf(buff,"INCREASE command\n");
			break;
		case 0x20:
			len = sprintf(buff,"VERIFY CHV,CHV = 0x%x\n",apdu[3]);
			break;
		case 0x24:
			len = sprintf(buff,"CHANGE CHV,CHV = 0x%x\n",apdu[3]);
			break;
		case 0x26:
			len = sprintf(buff,"DISABLE CHV command\n");
			break;
		case 0x28:
			len = sprintf(buff,"ENABLE CHV command\n");
			break;
		case 0x2C:
			len = sprintf(buff,"UNLOCK CHV command\n");
			break;
		case 0x04:
			len = sprintf(buff,"INVALIDATE command\n");
			break;
		case 0x44:
			len = sprintf(buff,"REHABILITATE command\n");
			break;
		case 0x88:
			len = sprintf(buff,"RUN GSM ALGORITHM command\n");
			break;
		case 0xFA:
			len = sprintf(buff,"SLEEP command\n");
			break;
		case 0xC0:
			len = sprintf(buff,"GET RESPONSE command,lenth=%d\n",apdu[4]);
			break;
		case 0x10:
			len = sprintf(buff,"TERMINAL PROFILE command,lenth=%d\n",apdu[4]);
			break;
		case 0xC2:
			len = sprintf(buff,"ENVELOPE command,length = %d\n",apdu[4]);
			break;
		case 0x12:
			len = sprintf(buff,"FETCH command,length = %d\n",apdu[4]);
			break;
		case 0x14:
			len = sprintf(buff,"TERMINAL RESPONSE command,length = %d\n",apdu[4]);
			break;
		default:
			break;
		
	}
	return len;
}


int logDataToHex(int fd,unsigned char *buff, unsigned short buff_len,const char *dire)
{
	if(0 == apdu_switch || fd <= 0){
		return 0;
	}
	
	unsigned short i = 0;
	char buff_msg[1024] = {0};
	
	char buf_time[64] = {0};
	int len = 0;

	if(0 == strcmp(dire,"[CommHdlTask ==> SlotHdlTask]") && buff_len > 4){
		len += apduInfo(buff + 1,buff_msg);
	}
	len += sprintf(buff_msg + len, "[%s]",str_timestamp(0, buf_time, sizeof(buf_time)));
	strcpy(buff_msg+len, dire);
	len += strlen(dire);
	//printf("[%02d-%02d-%02d]", usb_nbr, bank_nbr, sim_nbr);
	while (i < buff_len)
	{
		//printf("%02x ", buff[i]);
		sprintf(buff_msg+len, "%02x ", buff[i]);
		len += 3;
		i++;
	}
	//printf("\n");
	buff_msg[len] = '\n';
	write(fd,buff_msg,len + 1);
	return 0;
}

void * logMonHdl(void *pParam)
{
	log_mon_dhl_param_t *param = (log_mon_dhl_param_t *)pParam;
    struct stat st;
    char cmd[128] = {0};
	
	while (param->stop == 0)
	{
	    if (stat(param->file_name, &st) == 0)
	    {
	        if (st.st_size >= param->file_size * 1024 * 1024)
	        {
	            //unlink(param->file_name);
				sprintf(cmd, "/bin/echo \"\" > %s", param->file_name);
	            system(cmd);
	        }
	    }

		usleep(param->intv * 1000 * 1000);
	}
	if(fp){
		fclose(fp);
	}
	return NULL;
}
