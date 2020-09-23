#include "serial.h"
#include <iostream>
#include "stdlib.h" 
#include "string"
#include <errno.h>
#include <fstream>
#include "config.h"
#include "zprint.h"
#include <time.h>
#include <termios.h> 
#include "IapBase.h"
#include "IapSimEmu.h"
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


#include <stdio.h>
#include "ShortBankLinux.h"


  



using namespace std;

//iapbase_s iap;
pthread_t iapthread_t;
pthread_attr_t attr; 

//iapinput_s  iapinput;
iapgen_s  iapgennerator;

//平台相关

pthread_mutex_t serial_mutex=PTHREAD_MUTEX_INITIALIZER;
//平台相关








//线程函数  
/*
void *IapProxyThread(void *arg){
   
} 
*/

void Gohelp(void){
	printf("Usage: ProgramName UpdateFileName SerialDeviceName BaudRate UpdateMode DebugMode\n");
	printf("Default : SerialDeviceName: /dev/ttyS1; BaudRate:115200; UpdateMode:1  DebugMode: 1\n");
	printf("Valid :  UpdateMode:1(SIMULTANEOUS), 0(STEPBYSTEP)  DebugMode: 1(Open) 0(Close)\n");
	printf("Example : Update SimEmu.bin  \n");
	printf("Example : Update SimEmu.bin /dev/ttyUSB0 \n");
	printf("Example : Update SimEmu.bin /dev/ttyUSB0 115200\n");
	printf("Example : Update SimEmu.bin /dev/ttyUSB0 115200 0\n");
	printf("Example : Update SimEmu.bin /dev/ttyUSB0 115200 0 1\n");
	}
// program filename serialdev baudrate Updatemode debugmode

int checkProcessExist(const char *proc_name)
{
    int fd = open(proc_name,O_RDONLY|O_CREAT);
    if(-1 != fd){
        if(-1 ==  flock(fd,LOCK_EX|LOCK_NB)){
            fprintf(stderr,"%s is running...\n",proc_name);
            close(fd);
            return -1;
        }
    }
    else{
        fprintf(stderr,"%s::No such file or directory !!!\n",proc_name);
        return -1;
    }
    return 0;
}

int main(int argc,char* argv[])  
{
	
	if(checkProcessExist("/tmp/lock/emu_update.lock")){
		return -1;
	}

  FILE  *pFilePtr;
	unsigned char cIapInputBuf[IAP_BUF_MAX_LENGTH];
	char cFileName[80];
	size_t sResult;
	unsigned long   lSize,lDecryptionSize,tmpstatus ;
	unsigned char mode=IAPMODE_IS_SIMULTANEOUS;
	unsigned char debugmode=1;
	int serial_fd = -1;
	char key;
	// update
	char SerialDevice[30]="/dev/ttyUSB0";
	uint32 rate=115200;
	if (argc<2){
		printf("Loss Argument\n");
		Gohelp();
		goto quit;
	}
	for (int temparg=1;temparg<argc;temparg++){
		switch (temparg){
			case 1:
				     strcpy(cFileName,argv[temparg]);
				     break;
		  case 2:
				     strcpy(SerialDevice,argv[temparg]);
				     break;		 
			case 3:
				     rate= (uint32)atoi(argv[temparg]);
				     if (!((rate==115200)||(rate==921600)||(rate==460800)||(rate==230400))){
				     	  printf("Baudrate Error\n" ); 
				     	  Gohelp();
				     	  goto quit;
				     	}			     	 
				     break;		 
			case 4:
				     mode= (unsigned char)atoi(argv[temparg]);
				     if (!((mode==IAPMODE_IS_SIMULTANEOUS)||(mode==IAPMODE_IS_STEPBYSTEP))){
				     	  printf("Updatemode Error\n" ); 
				     	  Gohelp();
				     	  goto quit;
				     	}			     	  
				     break;		
			case 5:
				     debugmode= (unsigned char)atoi(argv[temparg]);
				     if (!((debugmode==0)||(debugmode==1))){
				     	  printf("Debugmode Error\n" ); 
				     	  Gohelp();
				     	  goto quit;
				     	}	
				     break;		  	                  
		  default:
		  	     break;		     		
			}
		
		}
	
	//open com
	////平台相关


	serial_fd = open_serial(SerialDevice, rate, 0, 8, 1, 0);
	
	if (serial_fd < 0)
	{
		printf("open com error\n");
		//return -1;
		goto quit;
	}
	printf("open com succ\n" ); 
 //平台相关
 //交互接口相关   
	iapgennerator.iapinput.ucpIapBuf=cIapInputBuf;
	iapgennerator.iapinput.ulpIapDataLen=&lSize;
	iapgennerator.iapinput.ucpIapmode=&mode;
	iapgennerator.iapinput.ucpDebugEnable=&debugmode;
 //交互接口相关   	
////平台相关
	iapgennerator.pEmucomport_mutex=&serial_mutex;
	iapgennerator.pEmucomportfd=&serial_fd;
//平台相关	
   //despatch thread
    pthread_attr_init( &attr ); 
    pthread_attr_setdetachstate(&attr,1); 
    pthread_create(&iapthread_t, &attr, IapEmuThread, (void *)&iapgennerator);  
   
	while (1){
		   
         pFilePtr= fopen(cFileName,"rb");
	        if (pFilePtr==NULL){
		        std::cout<<RED<<"Can not open the file"<<RESET<<std::endl<<std::flush;
		         goto quit;  
		      }	    
        fseek(pFilePtr,0,SEEK_END);	 
        lSize=ftell(pFilePtr);
        fseek (pFilePtr,0,SEEK_SET);
        if (lSize>IAP_BUF_MAX_LENGTH){
		        std::cout<<RED<<"File length overflow the buf"<<RESET<<std::endl<<std::flush;
			      fclose(pFilePtr);
		        goto quit;
    	  }	
        memset(cIapInputBuf,0,  IAP_BUF_MAX_LENGTH);	
        sResult=fread(	cIapInputBuf,1,lSize, pFilePtr);
		    fclose(pFilePtr);
        if (sResult!=lSize){
           std::cout<<RED<<"File read error"<<RESET<<std::endl<<std::flush;	  
           goto quit;
        }
        if (iapgennerator.iapgen.GetCurrentIapGenStatus_M()!=IAPGEN_STATUS_WORKING){
		 	        pthread_mutex_lock(iapgennerator.iapgen.GetIapGenMutex());
              pthread_cond_signal(iapgennerator.iapgen.GetIapGenCond());
			        pthread_mutex_unlock(iapgennerator.iapgen.GetIapGenMutex());
		}		 	
        
		do{
            		
            usleep(100);
			//判断线程是否存活
			if (pthread_kill(iapthread_t,0)==ESRCH)
			 	{
                      std::cout<<RED<<"Thread Iap has exit!"<<RESET<<std::endl<<std::flush;
					  goto quit;
			}
    		 tmpstatus=iapgennerator.iapgen.GetCurrentIapGenStatus_M();	
		 }while (tmpstatus==IAPGEN_STATUS_WORKING);
		 if(tmpstatus==IAPGEN_STATUS_HALT){
		 	std::cout<<RED<<"Halt Code:"<<(int) iapgennerator.iapgen.GetIapHaltCode_M()<<RESET<<std::endl<<std::flush;
			goto quit;
		 }
		 if(tmpstatus==IAPGEN_STATUS_SUCEESED){
		 	std::cout<<GREEN<<"IAP Success!"<<RESET<<std::endl<<std::flush;
		 }
		 	
			return 0;
	}
   
quit:
	  return 1;
   
}

	
