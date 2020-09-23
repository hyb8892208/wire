
#include "file.h"
#include "download.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_ANDROID)
#include "os_linux.h"
#include <string.h>
#endif 

/******读取Image文件**********/
byte *nprghex = NULL;
int nprghex_length = 0;
byte *partition = NULL;
int partition_length = 0;
byte *qcsblhd_cfgdata = NULL;
int qcsblhd_cfgdata_length = 0;
byte *qcsbl = NULL;
int qcsbl_length =0;
byte *pbl = NULL;
int pbl_length = 0;
byte *oemsblhd = NULL;
int oemsblhd_length = 0;
byte *oemsbl = NULL;
int oemsbl_length = 0;
byte *amsshd = NULL;
int amsshd_length = 0;
byte *amss = NULL;
int amss_length = 0;
byte *dbl = NULL;
int dbl_length = 0;
byte *fsbl = NULL;
int fsbl_length = 0;
byte *osbl = NULL;
int osbl_length = 0;

//声明函数
void nprghex_handle(void);

void rtrim(char *inbuff,int i)
{   
    int l;   
    char *p=inbuff;   
	char outbuff[255] = {0};
    p+=(strlen(inbuff)-1);   
    while(p>inbuff)   
    {   
        if(32!=(char)*p)
			break;   
        p--;   
    }   
    strcpy(outbuff,inbuff);   
    outbuff[p-inbuff+1]='\0';   
	memset(inbuff,0,sizeof(inbuff));
	memcpy(inbuff,outbuff,sizeof(outbuff));
} 

int get_cfg_file_data(char * aszSession,        
                        char * aszKey,        
                        char * aszDefault,        
                        char *  aszText,  
                        int   nSize,            
                        char * aszFileName)
{
        FILE * fp;
        char szLineBuf[255] = "";        
        int  iInSession = 0; 
        int  iHaveKey = 0;
        int  i ,j;
        int  iLen = 0;
        int  iBegin = 0;
        char szSession[40] = "";
        char szKey[40] = "";
        char szText[255] = "";
        strcpy(aszText,aszDefault);
        if ((fp = fopen(aszFileName,"r")) == NULL) return FALSE;
        while(fgets(szLineBuf,254,fp) != NULL)
        {
                /*分析输入的字符行*/
                iLen = strlen(szLineBuf);
                /*取第一\u017e鲎址*/
                i = 0;
                for (;i < iLen && szLineBuf[i] == ' ' && szLineBuf[i] < '!' ;i ++ );
                if (i == iLen ) continue;
                if (szLineBuf[i] == '#') continue;
                if (szLineBuf[i] == '[')
                {
                        if (iInSession)                
                        {
                                fclose(fp);
                                return FALSE;
                        }
                        /*取Session*/
                        for (j = i + 1;(j < iLen && szLineBuf[j] != ']');j ++);
                        memcpy(szSession,szLineBuf + i + 1,j - i - 1);
                        szSession[j - i - 1] = '\0';	
                       // if (strcmp(szSession,aszSession) == 0) iInSession = 1;
                        if (strcmp(szSession,aszKey) == 0) iInSession = 1;
                        continue;
                }
                else if (iInSession)        
                {
                /*取Key*/
		                     for (j = i ;(j < iLen && (szLineBuf[j] != ' ' && szLineBuf[j] != '=' ) && szLineBuf[j] >= '0');j ++);
							memcpy(szKey,szLineBuf + i ,j - i );
							szKey[j - i ] = '\0';
							i = j;
							//maxx
						 	//if (strcmp(szKey,aszKey) == 0)        
						 if (strcmp(szKey,"enable") == 0)        
							{        
									for (;i < iLen && ((unsigned char)szLineBuf[i] < '-' || szLineBuf[i] == '=') ;i ++ );

									for (j = i ;j < iLen && (szLineBuf[j] != '#') ;j ++);
									if (j <=i)
									{
											strcpy(aszText,"");
									}
									else
									{
											memcpy(szText,szLineBuf + i ,j - i - 1);
											rtrim(szText,0);
											szText[j - i - 1] = '\0';
											strcpy(aszText,szText);
									}
									iHaveKey = 1;
									break;
							}
                }
                else
                {
                        continue;
                }
        }
        fclose(fp);
        rtrim(aszText,0);
        if (iHaveKey)  
		{
			return TRUE;
			}
        else                             
			return FALSE;
}

byte * open_file(char *path,int *length)
{
	FILE *fd = NULL;
	int size;
	byte *pbuf = NULL;

	fd = fopen(path,"rb");
	if(fd == NULL){
		return NULL;
	}

	fseek( fd, 0, SEEK_END  );

	size = ftell(fd);

	fseek( fd, 0, SEEK_SET);

	if(size == 0){
		fclose(fd);
		return NULL;
	}
	pbuf = (byte*)malloc(size + 128);
	if(pbuf == NULL){
		fclose(fd);
		return NULL;
	}

	*length = fread(pbuf,1,size,fd);

	fclose(fd);

	return pbuf;
}

void free_file(void)
{
	if(nprghex!= NULL)
	{
		free(nprghex);
		nprghex = NULL;
	}
	if(partition != NULL)
	{

		free(partition);
		partition = NULL;
	}
	if(qcsblhd_cfgdata != NULL)
	{
		free(qcsblhd_cfgdata);
		qcsblhd_cfgdata = NULL;
	}
	if(qcsbl != NULL)
	{
		free(qcsbl);
		qcsbl=NULL;
	}
	if(pbl != NULL)
	{
		free(pbl);
		pbl=NULL;
	}
	if(oemsblhd != NULL)
	{
		free(oemsblhd);
		oemsblhd=NULL;
	}
	if(oemsbl != NULL)
	{
		free(oemsbl);
		oemsbl=NULL;
	}	  
	if(amsshd != NULL)
	{
		free(amsshd);
		amsshd=NULL;
	}
	if(amss != NULL)
	{
		free(amss);
		amss=NULL;
	}	
	if(dbl != NULL)
	{
		free(dbl);
		dbl=NULL;
	}	
	if(fsbl != NULL)
	{
		free(fsbl);
		fsbl=NULL;
	}	
	if(osbl != NULL)
	{
		free(osbl);
		osbl=NULL;
	}
}


extern void qdl_pre_download(void);

extern void qdl_post_download(void);

/*根据不同平台，读取相应的文件，平台信息和路径从参数获得*/
int image_read(dload_cfg_type * pdload)
{
	//strcat(pdload->ImagePath,"/NPRG6280.hex");
	int result = 1;
	char pathtemp[MAX_PATH];
	getcwd(pathtemp,256);
	chdir(pdload->ImagePath);

	switch (pdload->TargetPlatform)
	{
	case TARGET_PLATFORM_6270_SIM5215:
	case TARGET_PLATFORM_6270_SIM5320:
		nprghex = open_file("NPRG6270.hex",&nprghex_length);
			if(nprghex == NULL)
			{
				free_file();
				result = 0;
			}
		dbl = open_file("dbl.mbn",&dbl_length);
			if(dbl == NULL)
			{
				free_file();
				return 0;
			}
		fsbl = open_file("fsbl.mbn",&fsbl_length);
			if(fsbl == NULL)
			{
				free_file();
				return 0;
			}
		osbl = open_file("osbl.mbn",&osbl_length);
			if(osbl == NULL)
			{
				free_file();
				return 0;
			}
		amss = open_file("amss.mbn",&amss_length);
			if(amss == NULL)
			{
				free_file();
				return 0;
			}
	break;
	case TARGET_PLATFORM_6290:
		nprghex = open_file("NPRG6280.hex",&nprghex_length);
			if(nprghex == NULL)
			{
				free_file();
				return 0;
			}
		qcsblhd_cfgdata = open_file("qcsblhd_cfgdata.mbn",&qcsblhd_cfgdata_length);
			if(qcsblhd_cfgdata == NULL)
			{
				free_file();
				return 0;
			}
		qcsbl = open_file("qcsbl.mbn",&qcsbl_length);
			if(qcsbl == NULL)
			{
				free_file();
				return 0;
			}
		oemsbl = open_file("oemsbl.mbn",&oemsbl_length);
			if(oemsbl == NULL)
			{
				free_file();
				return 0;
			}
		oemsblhd = open_file("oemsblhd.mbn",&oemsblhd_length);
			if(oemsblhd == NULL)
			{
				free_file();
				return 0;
			}
		amsshd = open_file("amsshd.mbn",&amsshd_length);
			if(amsshd == NULL)
			{
				free_file();
				return 0;
			}
		amss = open_file("amss.mbn",&amss_length);
			if(amss == NULL)
			{
				free_file();
				return 0;
			}
			break;
	case TARGET_PLATFORM_6085:
		nprghex = open_file("NPRG60X5.hex",&nprghex_length);
			if(nprghex == NULL)
			{
				free_file();
				return 0;
			}
		qcsblhd_cfgdata = open_file("qcsblhd_cfgdata.mbn",&qcsblhd_cfgdata_length);
			if(qcsblhd_cfgdata == NULL)
			{
				free_file();
				return 0;
			}
		qcsbl = open_file("qcsbl.mbn",&qcsbl_length);
			if(qcsbl == NULL)
			{
				free_file();
				return 0;
			}
		oemsbl = open_file("oemsbl.mbn",&oemsbl_length);
			if(oemsbl == NULL)
			{
				free_file();
				return 0;
			}
		oemsblhd = open_file("oemsblhd.mbn",&oemsblhd_length);
			if(oemsblhd == NULL)
			{
				free_file();
				return 0;
			}
		amsshd = open_file("amsshd.mbn",&amsshd_length);
			if(amsshd == NULL)
			{
				free_file();
				return 0;
			}
		amss = open_file("amss.mbn",&amss_length);
			if(amss == NULL)
			{
				free_file();
				return 0;
			}
		break;
	}

	partition= open_file("partition.mbn",&partition_length);
	if(partition == NULL)
	{
		free_file();
		return 0;
	}

	nprghex_handle();

	chdir(pathtemp);
	return result;
}

int image_close()
{
	free_file();

	return 1;
}
int image_size(){//只用了以下5个固件
	return  qcsblhd_cfgdata_length + partition_length + qcsbl_length + oemsbl_length + amss_length;
}
//转换hex文件
int convert_asc_to_hex(unsigned char asc1,unsigned char asc2)
{
	int hex1=0,hex2=0;
	int ret_val=0;
	
	if(asc1>=0x30 && asc1<=0x39)
	{
		hex1 = asc1 - 0x30;
	}
	else if(asc1>=0x41 && asc1<=0x46)
	{
		hex1 = asc1 - 0x41 + 0x0A;
	}
	else if(asc1>=0x61 && asc1<=0x66)
	{
		hex1 = asc1 - 0x61 + 0x0A;
	}
	else
	{
		return 9999;
	}
	
	if(asc2>=0x30 && asc2<=0x39)
	{
		hex2 = asc2 - 0x30;
	}
	else if(asc2>=0x41 && asc2<=0x46)
	{
		hex2 = asc2 - 0x41 + 0x0A;
	}
	else if(asc1>=0x61 && asc1<=0x66)
	{
		hex2 = asc2 - 0x61 + 0x0A;
	}
	else
	{
		return 9999;
	}
	
	ret_val = ((hex1<<4) & 0xF0) + hex2;
	
	return ret_val;
	
}

int	g_hex_start_addr = 0; 

int decode_hexfile(unsigned char* srcHexFile, int srcFileSize, unsigned char** pbinfile )
{
	int i=0,j;
	int addr1,addr2,len=0;
	int tmpbuf;
	int nbinfile = 0;
	
	*pbinfile = (unsigned char*)malloc(srcFileSize/2);  
	
	while(i<=srcFileSize)
	{
		if(srcHexFile[i]==0x3a)
		{
			if(srcHexFile[i+7]==0x30 && srcHexFile[i+8]==0x34)
			{
				if((addr1 = convert_asc_to_hex(srcHexFile[i+9],srcHexFile[i+10]))==9999)
					return -1;
				if((addr2 = convert_asc_to_hex(srcHexFile[i+11],srcHexFile[i+12]))==9999)
					return -1;
				g_hex_start_addr = ((addr1<<24) & 0xFF000000) + ((addr2<<16) & 0x00FF0000);
			}
			else if(srcHexFile[i+7]==0x30 && srcHexFile[i+8]==0x30)
			{
				if((len = convert_asc_to_hex(srcHexFile[i+1],srcHexFile[i+2]))==9999)
					return -1;
				//				printf("\n");
				for(j=0;j<len;j++)
				{
					if((tmpbuf = convert_asc_to_hex(srcHexFile[i+9+j*2],srcHexFile[i+10+j*2]))==9999)
						return -1;
					(*pbinfile)[nbinfile++] = tmpbuf;
					//					printf("%02X ",tmpbuf);
				}
			}
			else if(srcHexFile[i+7]==0x30 && srcHexFile[i+8]==0x31)
			{
				
			}
		}
		i++;
	}
	
	
	return nbinfile;
}
void nprghex_handle(void)
{
		byte *tempbuf = NULL;
		int temp_length = 0;
		
		temp_length = decode_hexfile( (unsigned char*)nprghex, nprghex_length, &tempbuf ); 
		free(nprghex);
		
		nprghex =tempbuf;
		nprghex_length = temp_length ;
}


