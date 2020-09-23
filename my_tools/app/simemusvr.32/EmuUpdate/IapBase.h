
/****************************************************************************
* 版权信息：
* 系统名称：Sim Bank ( SimRdr,SimEmu)
* 文件名称：IapBase.h 
* 文件说明：在线升级 Sim Bank  相关头文件
* 作    者：Carson 
* 版本信息：v1.0 
* 设计日期：2015/1/7
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

#ifndef _IAPBASE_H
#define _IAPBASE_H

#include "stdlib.h" 
#include <pthread.h>
#include "hidapi.h"
#include "serial.h"
#include <iostream>
#include "ShortBankLinux.h"




//
#define  PAYLOAD_MAX_LENGTH              0x100
#define  VERSION_STRING_MAX_LENGTH       0x40
#define  IAP_BUF_MAX_LENGTH              1024*48
#define  IAP_OBJECT_MAX_NUMBER            8

#define  IAPMODE_IS_SIMULTANEOUS       1
#define  IAPMODE_IS_STEPBYSTEP         0

#define  IAPSLOTMODE_SINGLE            0
#define  IAPSLOTMODE_MULTI             1
#define  IAP_DEPENCE_MCU_NULL         0xff

#define  IAP_MODE_IAP                 0    
#define  IAP_MODE_APP                 1

#define  IAP_STATUS_SUCEESS         1

#define  IAP_FILE_VOLUMES         2

#define  IAPGEN_STATUS_IDLE            0
#define  IAPGEN_STATUS_WORKING         1
#define  IAPGEN_STATUS_SUCEESED        2
#define  IAPGEN_STATUS_HALT            3



#define  MAX_SIMBORAD_VOLUME           40
#define  MAX_CONFIG_LIST_NUM           10

#define  STRINGMAXLENGTH               20
#define  HALT_STATUS_STRING_MAXLENGTH  50


#define  HALT_NULL                         0
#define  HALT_INPUTFILE_OVERFLOW           1
#define  HALT_REPEAT_IAP_REQ               2
#define  HALT_CONGFIG_UNSUPPORT            3
#define  HALT_IAPFILE_MISMATCH             4
#define  HALT_DEPENDDEV_OVERFLOW           5
#define  HALT_RESET_DEPENDDEV_FAIL         6
#define  HALT_DEPENDDEV_RUNNING_INVALID    7
#define  HALT_RESET_IAPDEV_FAIL            8
#define  HALT_IAPDEV_SWITCH_TO_IAP_STATUS_FAIL            9
#define  HALT_IAPDEV_RUNNING_IS_NOT_IAP_STATUS            10
#define  HALT_IAP_FAIL                                    11
#define  HALT_IAPDEV_IS_NOT_APP_STATUS_AFTER_IAP_SUCCESS  12 
#define  HALT_CONGFIG_INVALID           13
#define  HALT_DEV_VOLUME_OVERFLOW       14
#define  HALT_MULTI_DEV_IAP_FAIL        15
#define  HALT_CURRENT_S2M_DEV_DEPENCE_DEV_INVALID  16


#define IAP_FAIL_NULL                                                   0
#define IAP_FAIL_RETRYTHISFRAME_CHECKSUM_ERR                            1
#define IAP_FAIL_RETRYTHISFRAME_LENGTH_ERR                              2
#define IAP_FAIL__RETRYALL_PACAKGENO_ERR                                3
#define IAP_FAIL__RETRYALL_RETRY_OVERFLOW                               4
#define IAP_FAIL__RETRYALL_APP_OVERFLOW                                 5
#define IAP_FAIL__RESPONSE_INVALID                                      6
#define IAP_FAIL__WRITE_FLASH_ERR_RESETMCU                              7
#define IAP_FAIL__RESPONSE_TIMEOUT                                      8

#define  IAP_MAX_RETRY_TIMES                                            5
#define  IAP_MAX_DEV_VOLUME                                             50


#define  IAPMODE_S2S              0   // 单通信口对单设备
#define  IAPMODE_S2M              1   // 单通信口对多设备

#define  MAXPACKAGELEN            320   
#define  WAIT_RESET_DURATION      1500000  

#define  RUNING_APP            0   
#define  RUNING_IAP            1  
#define  RUNING_UNRECOGNIZED   2  

#define  DEBUG_ENABLE    1
//#define  READCOMTIMEOUT    150 //ms
#define  READCOMTIMEOUT    2000 //ms  等待emu处理时间

#define  IAP_S2M__SIMULTANEOUS_IDLE     0
#define  IAP_S2M__SIMULTANEOUS_START    1
#define  IAP_S2M__SIMULTANEOUS_POLLING_START     2



// 固件及适配硬件信息数据结构
/*typedef struct FIRM_HARDWARE_INFO_S
{
	unsigned char ucFirmwareNameString[VERSION_STRING_MAX_LENGTH];				            // 固件名字符串
	unsigned char ucVendorDeviceID ;			              // MCU  ID 
	unsigned char ucFirmwareVersion;        //固件版本
	unsigned char ucIapFirmwareNameString[VERSION_STRING_MAX_LENGTH];	     //对应的IAP 固件名
	unsigned char ucIapCommunicationMode;                 //IAP 通信模式
	
}firm_hardware_info_t;*/
	
// 固件及适配硬件信息数据结构
typedef struct FIRM_HARDWARE_INFO_S
{
	unsigned char ucFirmwareNameString[STRINGMAXLENGTH];				            // 固件内部名称			                                         
	unsigned char ucIapFirmwareNameString[STRINGMAXLENGTH];	                       //对应的IAP 固件名
	unsigned char ucIapCommunicationMode;                                          //IAP 支持的通信模式   1：支持多线程升级 0：Step by Step
    int ucDepenceOrder ;                                                 // 依赖固件表序号 
    unsigned short usMaxFirmwareLength;                                            //最大的固件长度
}firm_hardware_info_t;

typedef struct IAP_FAIL_INFO_S
{
	unsigned char ucPackageOrder;				            // 包号			                                         
	unsigned char ucFailType;	                       //错误类型	
}iap_fail_info_s;

typedef struct IAP_FAIL_LIST_S
{
	unsigned char ucSlotOrder;				            // 通道号      
	int ucDeviceOrder;				            // 设备号   
	iap_fail_info_s ucFailInfo[IAP_MAX_RETRY_TIMES];	    //错误信息	
}iap_fail_list_s;


typedef struct IAP_SLOT_LOOKUP_TABLE_S
{
    unsigned char ucFirmName[STRINGMAXLENGTH];           //固件名
	unsigned char ucIapFirmName[STRINGMAXLENGTH];				           
	unsigned char ucIapSlotMode ;			              // 0 :S2S  1:S2M
	unsigned char ucIapMaxSlotOrder;                      //最大槽号
	unsigned char ucIapSlotVolume;                      //设备数量
	unsigned char ucIapDepenceOrder;                      
	
}iap_slot_look_table_t;


typedef struct IAP_DATASTRUC_T
{
	unsigned char ucCurrentIapStatus;				           
	unsigned char ucIapInputBuf[IAP_BUF_MAX_LENGTH] ;	
	unsigned long ulIapFileLength;
	unsigned char ucIapDescrptBuf[IAP_BUF_MAX_LENGTH] ;	
	unsigned long ulIapDescrptFileLength;
	unsigned char ucIapLastSuccessBuf[IAP_FILE_VOLUMES][IAP_BUF_MAX_LENGTH] ;  
	unsigned long ulIapLastSuccessFileLength[IAP_FILE_VOLUMES];
	
	firm_hardware_info_t struConfigList[MAX_CONFIG_LIST_NUM];
	unsigned long ulIapAcutualFileLength;
	unsigned long ulIapAcutualFileOffset;
	int  ucCurrentIapOrder;
	
	unsigned char ucDebugEnable;	
	iap_fail_list_s iap_fail_list[IAP_MAX_DEV_VOLUME];
	unsigned char ucHaltCode;
	unsigned char ucIapSimutanenousStatus;
}iap_datastruc_s;


typedef class IAPBASE_T
{
   private:
   iap_datastruc_s  iapstruc;

   pthread_cond_t   iapcond;	
   pthread_mutex_t  iapmutex;
   static IAP_SLOT_LOOKUP_TABLE_S platformlook_table[IAP_FILE_VOLUMES];
   static char  halt_status_output_table[][HALT_STATUS_STRING_MAXLENGTH];
   int SetConfig(void);
   void GetAppName(unsigned char platformlook_table_order,unsigned char * AppName);
   void GetIapName(unsigned char platformlook_table_order,unsigned char * IapName);
   public:
   pthread_mutex_t*  GetIapGenMutex(void);
   pthread_cond_t *   GetIapGenCond(void);
   unsigned char GetCurrentIapGenStatus_M(void);
   void SetIapGenStatus_M(int status);
   int  CopyIapFile(unsigned char * inputbuf, unsigned long * pFilelength);
   int CheckSameIap(void);
   void SetIapHaltCode_M(int haltcode);
   void Descrypt(void);
   int CheckIapValid(void);
   int  MatchIapFirmware(unsigned char *ucFilebuf,unsigned long  filelength, unsigned char *ucIapsupportfirmstring);
   unsigned char GetCurrentDepenceOrder(void);
   unsigned char GetMaxSlotNumber(unsigned char look_table_order);
   unsigned char GetSlotVolume(unsigned char look_table_order);
   void  GenHaltCodeAndHaltStatus(int Code);
   int ResetMcu(int fd , unsigned char slot,unsigned char * revbuf ,int length ,pthread_mutex_t* pMutex );
   int GetEmuVersion(int fd ,unsigned char slot,unsigned char * revbuf ,int length ,pthread_mutex_t* pMutex);
   int GetMcuRunningStatus(int fd , unsigned char slot ,unsigned char platformlook_table_order ,unsigned char * revbuf ,int length,pthread_mutex_t* pMutex);
   void SetIapGenDebugMode(unsigned char * debugmode);
   int GetCurrentIapOrder(void);
   int Switch2IapMode(int fd ,unsigned char slot,pthread_mutex_t* pMutex);
   void ResetIapFailList_M(void);
   unsigned short Checksum(unsigned char * DataPtr,int Len );
   int IapOnePackage_Wr( int fd ,unsigned char *sendbuf ,size_t sResult,pthread_mutex_t* pMutex);
   //int Iap( int fd ,unsigned char slot,unsigned char *cIapInputBuf,size_t sResult,pthread_mutex_t* pMutex);
   int Iap( int fd ,unsigned char slot,unsigned char platformlook_table_order ,unsigned char iap_fail_list_order,
	unsigned char *cIapInputBuf,unsigned long sResult,pthread_mutex_t* pMutex);
   void AddIapFailInfo_M(unsigned char iap_fail_list_order,unsigned char ucFailInfoOrder, unsigned char packageNo,unsigned char slot,unsigned char ucFailType);
   unsigned long GetTickCount(void);
   unsigned char* GetCurrentIapBufPtr(void);
   unsigned long GetIapDataLength(void);
   unsigned char GetIapHaltCode_M(void);
   void CopySucceesData(void); 
   unsigned char GetIapGenDebugMode(void);
   void ClearAllFailInfo_M(void);
   int IapSingleDev(int fd,unsigned char slot ,unsigned char iap_fail_list_order,pthread_mutex_t* pMutex ,int * haltcode );
   unsigned char GetCurrentIapSlotMode(unsigned char platformlook_table_order);
  /* int IapOnePackage_WrSimultaneous( int fd ,unsigned char *sendbuf ,size_t sResult,unsigned char* revbuf ,int* revlength ,pthread_mutex_t* pMutex);
   int IapSimultaneous( int fd ,unsigned char slot,unsigned char platformlook_table_order ,unsigned char iap_fail_list_order,
	unsigned char *cIapInputBuf,unsigned long sResult,unsigned char* revbuf ,int* revlength ,pthread_mutex_t* pMutex);
   int IapSingleDevSimultaneous(int fd,unsigned char slot ,unsigned char iap_fail_list_order,unsigned char* revbuf ,int* revlength,pthread_mutex_t* pMutex ,int * haltcode );
   
   int ResetMcuSimultaneous(int fd , unsigned char slot,unsigned char * revbuf ,int *revlength ,pthread_mutex_t* pMutex );
   int GetEmuVersionSimultaneous(int fd , unsigned char slot,unsigned char * revbuf ,int *revlength,pthread_mutex_t* pMutex);
   int GetMcuRunningStatusSimultaneous(int fd ,  unsigned char slot ,unsigned char platformlook_table_order ,unsigned char * revbuf ,int *revlength,pthread_mutex_t* pMutex);
   int Switch2IapModeSimultaneous(int fd ,unsigned char slot,unsigned char* revbuf ,int* revlength ,pthread_mutex_t* pMutex);
   int PollingS2MSimultaneous(int fd ,unsigned char depenceslot, unsigned char maxpollingslot,unsigned char * revbuf ,int *revlength , pthread_mutex_t* pMutex );
  void SetIapGenSimutaneousStatus_M(unsigned char Simutaneousstatus);
  unsigned char GetIapGenSimutaneousStatus_M(void);*/
   IAPBASE_T( );
  
}iapbase_s;


typedef struct IAPINPUT_T
{
	unsigned char *ucpIapBuf;				            // Iap 原始数惧Buf
	unsigned long *ulpIapDataLen ;			            
	unsigned char *ucpIapmode ;			              // Iap mode 1: multi 0:step by step
	unsigned char *ucpDebugEnable ;			          // debug mode 1: enable  0:disable
}iapinput_s;



typedef struct IAPGEN_T
{
	iapinput_s iapinput;				            // Iap 原始数据Buf
	iapbase_s iapgen ;	
	int*       pEmucomportfd;
	pthread_mutex_t*  pEmucomport_mutex;
	hid_device* pSbx_handle;
	pthread_mutex_t*  pSbx_mutex;
	hid_device* pSimboard_handle[MAX_SIMBORAD_VOLUME];
	pthread_mutex_t*  pSimboard_mutex[MAX_SIMBORAD_VOLUME];
		
}iapgen_s;

typedef struct IAP_S2M_MODE_SIMULTANEOUS_S
{
    iapbase_s *pIapobj;
	int fd_s;
	unsigned char slot_s ;
	unsigned char iap_fail_list_order_s;
	unsigned char* pRevbuf_s ;
	int* pRevlength_s;
	pthread_mutex_t* pMutex_s ;
	int * pHaltcode_s;
}iap_s2m_mode_simultaneous_t;

typedef struct IAP_S2M_MODE_SIMULTANEOUS_POLLING_S
{
    iapbase_s *pIapobj;
	int fd_s;
	unsigned char depence_slot_s ;
	unsigned char maxpollingslot_s;
	unsigned char iap_fail_list_order_s;
	unsigned char* pStartRevbuf_s ;
	int* pStartRevlength_s;
	pthread_mutex_t* pMutex_s ;
	int * pHaltcode_s;
}iap_s2m_mode_simultaneous_polling_t;

typedef struct IAP_S2M_SIMULTANEOUS_S
{
    iapbase_s *pIapobj;
	int fd_s;
	unsigned char slot_s ;
	unsigned char iap_fail_list_order_s;
	pthread_mutex_t* pMutex_s ;
}iap_s2m_simultaneous_t;

//unsigned char iapbase_s::GetIapGenMutex(void);

/*
int CheckIapFileLengthValid(unsigned char *IapBufPtr ,size_t Length );
int InitIapGenerator(unsigned char *IapBufPtr ,size_t Length );*/

#endif


