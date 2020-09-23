
/****************************************************************************
* 版权信息：
* 系统名称：
* 文件名称：
* 文件说明：
* 作    者：Carson 
* 版本信息：v1.0 
* 设计日期：2015/1/7
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
#ifndef __EMUVCARD_G2_H__
#define __EMUVCARD_G2_H__

#include <pthread.h>
#include "Emu_Rdr_Common.h"
#include "card.h"

//#define  MAX_VCARDINDEX                     50
//#define  MAX_VCARDINDEX                     150   //20180620
#define  MAX_VCARDINDEX                     500   //20180620
#define  EMUIATIMEOUT                       10000
#define  NETAPIBUFMAXLENGTH                 0x200
#define  APDUCOMMANDLENGTH                  5
#define  APDUARGUMENTMAXLENGTH              255


#define STATUS_EAE_OK             0
#define STATUS_EAE_ERROR_VCARD_OVERFLOW             1
#define IDENTIFIERDEEPTH                    0x10

#define ADFSESSIONSTATUS_DISABLE       0
#define ADFSESSIONSTATUS_COMMAND_OK       1
#define ADFSESSIONSTATUS_DATA_OK          2
#define ADFSESSIONSTATUS_ENABLE           3
#define ADFSESSIONSTATUS_ADDITONAL_COMMAND_OK          4
#define ADFSESSIONSTATUS__ADDITONAL_DATA_OK           5



#define VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD             0xFFF
#define VCARDINDEXLISTSERIALNUMBER_UNKOWN                           (VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD-1)
#define VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD                (VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD-2)
#define VCARDINDEXLISTSERIALNUMBER_NO_RESPONSEBYTE_IN_VCARD          (VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD-3)
#define VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD          (VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD-4)
#define VCARDINDEXLISTSERIALNUMBER_UPDATE_FORK_FAIL            (VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD-5)

#define TRANSFER2NET_STATUS_SINGLE_ID          0
#define TRANSFER2NET_STATUS_CURRENT_PATH       1

#define EXPECTSYNCLOCAL_REMOTE_DISABLE         0
#define EXPECTSYNCLOCAL_REMOTE_ENABLE          1

#define SYNCLOCAL_REMOTE_NO         0
#define SYNCLOCAL_REMOTE_YES        1
#define SYNC_DIR_LOCAL_REMOTE_YES_FOR_SFI        3

#define CHV1_STATUS_ENABLE             0
#define CHV1_STATUS_DISABLE            1
#define CHV1_STATUS_WAITING_PENDING    2
#define CHV1_STATUS_ENABLE_UNKOWN      3
#define CHV1_STATUS_CHV_RETRYTIMES_PENDING  4

#define PROCESSINDEXSTATUS_NO_PROCESSING  0
#define PROCESSINDEXSTATUS_PROCESSING     1
#define PROCESSINDEXSTATUS_PROCESSED      2


#define LOGICALCHANNELSTATUS_CLOSE      0
#define LOGICALCHANNELSTATUS_OPEN       1

#define SEEKSTATUS_DISABLE              0
#define SEEKSTATUS_ENABLE               1

#define SFI_FOR_UPDATE_UNKNOWN               0
#define SFI_FOR_UPDATE_KNOWN                 1

#define RECORDPRORITYINVCARD_AVAILABLE      0
#define RECORDPRORITYINVCARD_UNAVAILABLE    1

#define CURRENTFILEPRORITY_FORK_UNAVAILABLE      0
#define CURRENTFILEPRORITY_FORK_AVAILABLE      1

#define READRECORDSTATUS_NORMAL              0
#define READRECORDSTATUS_NUMBER_OVERFLOW     1

#define CURRENTSELECTRESPONSESTATUS_NORMAL              0
#define CURRENTSELECTRESPONSESTATUS_NODATA_RETURN       1


typedef struct  EMU_APDU_LOGIC_CHANNEL_S
{
	unsigned char ucLastLogical_Channel_Identifier[IDENTIFIERDEEPTH];
	unsigned char ucLastLogical_Channel_IdentifierLength;
	unsigned char ucCurrentLogical_Channel_Identifier[IDENTIFIERDEEPTH];
	unsigned char ucCurrentLogical_Channel_IdentifierLength;
	unsigned char ucCurrentLogical_Channel_DirIdentifier[IDENTIFIERDEEPTH];
	unsigned char ucCurrentLogical_Channel_DirIdentifierLength;
	unsigned long ucCurrentLogical_Channel_IdentifierProperty;		
	unsigned char ucExpectLogical_Channel_Identifier[IDENTIFIERDEEPTH];
  unsigned char ucExpectLogical_Channel_IdentifierLength;	
  unsigned char ucCurrentLogical_Channel_RidType; //由 select AID 指令决定
  unsigned char ucCurrentLogical_Channel_Type;   //由每次 select 指令决定
  unsigned char LogicalChannelStatus;
  unsigned char LogicalChannelucAdfSessionStatus;
  int           LogicalChanneliCurrentIndexSerialnumber;
  unsigned long LogicalChannelCurrentIdentifierProperty;	
  id_with_path_t CurrentLogical_ChannelIdWithPath;
  alltype_vcard_identifier_t Logical_ChannelId_AllType;
  unsigned char ucLogical_Channel_ExpectRunAuthResLength;
}emu_apdu_logic_channel_t;
/*
typedef struct  RID_INFORM_S
{
	unsigned char rid[5];
	unsigned char ridtype; 
}rid_inform_t;
*/
/*
typedef struct  EMU_LOGIC_CHANNEL_MANAGER_S
{
 emu_apdu_logic_channel_t LogicalChannel[MAX_LOGICAL_CHANNELS_NUMBER];
 unsigned char LogicalChannelStatus;
}emu_logic_channel_manager_t;
*/
// EmuAPDU Engine数据结构
typedef struct  EMU_APDU_ENGINE_S
{
	unsigned char ucEmuEngineStatus;				            
	unsigned char ucRemoteSimCardPlug ;			              
	unsigned char ucLocoalVCardReady ;	
	unsigned char ucEmuSimcardAtrReady ;
	unsigned char ucDeliverAtrToEmu ;
	unsigned char ucEmuDeliverRstIccCommand ;	
	unsigned char ucTransmissionProtocol;
	unsigned char ucEmuApduNetApiStatus;
	
	unsigned char ucFromEmuDataBuf[APDU_BUF_MAX_LENGTH] ; 
	unsigned char ucFromEmuDataProperty;       
	unsigned short usFromEmuDataBufLength; 
	
	unsigned char ucToEmuDataBuf[APDU_BUF_MAX_LENGTH] ;      
	unsigned short usToEmuDataBufLength; 
	
	unsigned char ucToNetDataBuf[NETAPIBUFMAXLENGTH] ; 
	unsigned char ucToNetDataProperty;       
	unsigned short usToNetDataBufLength; 
	
	unsigned char ucFromNetDataBuf[NETAPIBUFMAXLENGTH] ;      
	unsigned short usFromNetDataBufLength; 
	unsigned char ucFromNetDataProperty; 
	
	unsigned long  ucWaitGetInteractiveStartTime;   
	
	unsigned char ucLastSimCardIdentifier[IDENTIFIERDEEPTH];
	unsigned char ucLastSimCardIdentifierLength;
	unsigned char ucCurrentSimCardIdentifier[IDENTIFIERDEEPTH];
	unsigned char ucCurrentSimCardIdentifierLength;
	unsigned char ucCurrentDirIdentifier[IDENTIFIERDEEPTH];
	unsigned char ucCurrentDirIdentifierLength;
	unsigned long ucCurrentSimCardIdentifierProperty;		
	unsigned char ucExpectIdentifier[IDENTIFIERDEEPTH];
  unsigned char ucExpectIdentifierLength;	
  unsigned char ucExpectRunAuthResLength;
  id_with_path_t CurrentIdWithPath;
  id_with_path_t ExpectIdWithPath;
  alltype_vcard_identifier_t CurrentId_AllType;
  alltype_vcard_identifier_t ExpectId_AllType;
  unsigned char ExpectSyncLocal_Remote;   
  unsigned char SyncLocal_Remote; //Emu端和Bank端 是否文件是否同步
  unsigned char ucCurrentSFI;
  unsigned char ucCurrentReadMode;
  unsigned char ucCurrentUpdateMode;
  unsigned char ucCurrentSFIForUpdatePrority;
  
	unsigned char ucCurrentCommand[APDUCOMMANDLENGTH];
	unsigned char ucCurrentCommandLength;
	unsigned char ucLastCommand[APDUCOMMANDLENGTH];
	unsigned char ucLastCommandLength;
	
	unsigned char ucCurrentArgument[APDUARGUMENTMAXLENGTH];
	unsigned char ucCurrentArgumentLength;
	unsigned char ucLastArgument[APDUARGUMENTMAXLENGTH];
	unsigned char ucLastArgumentLength;
	unsigned long ulApduCommandCounter;
	unsigned char ucStatusCode;
	unsigned char ucCurrentEngineStatus;
	unsigned char ucCurrentFheaderFbodyResponsePath;
	unsigned char ucCurrentCatLength;
	unsigned char ucAdfSessionStatus;
	unsigned char ucCurrentEaeCapability;
	unsigned char ucCurrentRecordProrityInVcard;
  
   int   iCurrentVcardIndexListSerialnumber;
  unsigned char CurrentTransfer2Net_Status;
  unsigned char ExpectTransfer2Net_Status; 
 // unsigned char identifierresponse[2];
 // unsigned char fileheaderresponse[2];
 // unsigned char filebodyresponse[2];
 // unsigned char filerecordresponse[2]; 
	unsigned char ucCurrentExpect;
	unsigned char ucCurrentExpectFileHeaderLength;
	
	unsigned char ucPinLength; 
  unsigned char ucPinBuf[8];
  unsigned char ucCurrentChv1Status;
	unsigned char ucDefaultChv1Status; 
	unsigned char ucProcessIndexStatus;   
	unsigned char ucCurrentProcessLogicalChannel;   
	unsigned char ucCurrentRidType;   //由每次 select 指令决定 
	emu_apdu_logic_channel_t Lcm[MAX_LOGICAL_CHANNELS_NUMBER];
	unsigned char ucCurrentSimCardRidType;     //逻辑通道0 的ridtype    由每次 select AID指令决定  
	unsigned char ucMeetSeekVolume;
	unsigned char ucMeetSeekRecordNo[255];
	unsigned char ucCurrentSeekStatus;
 // unsigned char	 ucCurrentLogicChannel;  
  unsigned char ucCurrentFilePrority; 
  unsigned char ucOptimizationMode;  
  unsigned char ucReadRecordStatus; 
  unsigned char ucExpectSelectResponseStatus;
}emu_apdu_engine_t;

#define ENGINESTATUS_SIM_USIM_UIM_ENBALE                   0
#define ENGINESTATUS_SIM_ENBALE_ONLY                       1 
#define ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED  2
#define ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED    3  
#define ENGINESTATUS_UIM_ENBALE_ONLY                       4 

#define FHEADERFBODY_RESPONSE_PATH_FROM_VCARD              0
#define FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT            1         

#define EMU_ENGINE_STOP                                    0
#define EMU_ENGINE_PAUSE                                   1
#define EMU_ENGINE_WORKING                                 2
#define EMU_ENGINE_WAITING_ATR                             3
#define EMU_ENGINE_WAITING_DELIVER_ATR_TO_EMU              4
#define EMU_ENGINE_WAITING_TRANSMISSION_PROTOCOL           5
#define EMU_ENGINE_WAITING_VCARD                           6
#define EMU_ENGINE_WAITING_EMU_DELIVER_RST_ICC             7

#define LOCOAL_VCARD_IS_NOT_READY                      0
#define LOCOAL_VCARD_IS_READY                          1
#define EMU_SIMCARD_ATR_IS_NOT_READY                   0
#define EMU_SIMCARD_ATR_IS_READY                       1
#define DELIVER_ATR_TO_EMU_IS_NOT_READY                   0
#define DELIVER_ATR_TO_EMU_IS_READY                       1
#define EMU_DELIVER_RST_ICC_COMMAND_IS_NOT_READY          0
#define EMU_DELIVER_RST_ICC_COMMAND_IS_READY              1
#define TRANSMISSION_PROTOCOL_NULL                         0
#define TRANSMISSION_PROTOCOL_T0                           1
#define TRANSMISSION_PROTOCOL_T1                           2
#define REMOTE_SIMCARD_IS_PULL_OUT                          0
#define REMOTE_SIMCARD_IS_PLUG_IN                           1
//#define EMU_APDU_NETAPI_IDLE                        0
//#define EMU_APDU_NETAPI_ERROR                       1

#define DATA_PROPERTY_IS_NULL              0
#define DATA_PROPERTY_IS_REQ_RST_ICC       1
#define DATA_PROPERTY_IS_DATA              2
#define DATA_PROPERTY_IS_NETAPI_COMMAND    3
#define DATA_PROPERTY_IS_NETAPI_NODELIVER2NET    4

#define EMU_APDU_NETAPI_STATUS_IDLE                       0
#define EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY     1
#define EMU_APDU_NETAPI_STATUS_DATA_TO_EMU_IS_READY       2
#define EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY       3
#define EMU_APDU_NETAPI_STATUS_DATA_FROM_NET_IS_READY     4
#define EMU_APDU_NETAPI_STATUS_ICC_RST_DELIVER_TO_REMOTE  5
#define EMU_APDU_NETAPI_STATUS_ERROR                      6  
#define EMU_APDU_NETAPI_STATUS_WAITING_FROM_NET_READY     7 

#define OFFSET_CLA       0
#define OFFSET_INS       1
#define OFFSET_P1        2
#define OFFSET_P2        3
#define OFFSET_P3        4

                                                						  /***** CLA  TS102.221   GSM11.11       C.S0023-C(Uim)       ********/                                        
#define APDU_COMMAND_SELECT             0xA4    						  //            0x0         A0  
#define APDU_COMMAND_STATUS             0XF2     						  //            0x80        A0  
#define APDU_COMMAND_READ_BINARY        0XB0    						  //            0x0         A0 
#define APDU_COMMAND_UPDATE_BINARY      0XD6    						  //            0x0         A0 
#define APDU_COMMAND_READ_RECORD        0XB2    						  //            0x0         A0
#define APDU_COMMAND_UPDATE_RECORD      0XDC    						  //            0x0         A0
#define APDU_COMMAND_SEEK               0XA2   						    //            0x0         A0
#define APDU_COMMAND_INCREASE           0X32   						    //            0x80        A0 
#define APDU_COMMAND_VERIFY_CHV         0X20    						  //            0x0         A0  
#define APDU_COMMAND_CHANGE_CHV         0X24    						  //            0x0         A0 
#define APDU_COMMAND_DISABLE_CHV        0X26    						  //            0x0         A0  
#define APDU_COMMAND_ENABLE_CHV         0x28   						    //            0x0         A0   
#define APDU_COMMAND_UNBLOCK_CHV        0X2C    						  //            0x0         A0
#define APDU_COMMAND_INVALIDATE         0X04    						  //            0x0         A0    
#define APDU_COMMAND_REHABILITATE       0X44    						  //            0x0         A0
#define APDU_COMMAND_RUN_GSM_ALGORITHM  0X88   						    //            0x0         A0
#define APDU_COMMAND_SLEEP              0xFA    						  //                        A0  
#define APDU_COMMAND_GET_RESPONSE       0XC0    			 			  //            0x0         A0
#define APDU_COMMAND_TERMINAL_PROFILE   0X10    						  //            0x80        A0 
#define APDU_COMMAND_ENVELOPE           0XC2     						  //            0x80        A0 
#define APDU_COMMAND_FETCH              0X12    						  //            0x80        A0 
#define APDU_COMMAND_TERMINAL_RESPONSE  0X14     						  //            0x80        A0 
#define APDU_COMMAND_GET_CHALLENGE      0X84     						  //            0x0         
#define APDU_COMMAND_TERMINAL_CAPABILITY 0XAA    						  //            0x80
#define APDU_COMMAND_MANAGE_CHANNEL     0X70     						  //            0x0
#define APDU_COMMAND_MANAGE_SECURE_CHANNEL 0X73  						  //            0x0
#define APDU_COMMAND_TRANSACT_DATA      0X75     						  //            0x0

#define APDU_COMMAND_UIM_STORE_ESN_ME  0XDE           			  //                                       A0
#define APDU_COMMAND_UIM_BASE_STATION_CHALLENGE  0X8a         //                                       A0
#define APDU_COMMAND_UIM_CONFIRM_SSD    0X82          			  //                                       A0
#define APDU_COMMAND_UIM_GENERATE_KEY_VPM    0X8e     			  //                                       A0
#define APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION    0X80    //                                       80

// Add  Command in C.S0023-0_v4.0  //20200102
#define APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY    0X50     			  //                                     A0
#define APDU_COMMAND_UIM_KEY_GENERATION_REQUEST 0X52     			  //                                     A0
#define APDU_COMMAND_UIM_CONFIGURATION_REQUEST  0X54     			  //                                     A0
#define APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST   0X56    			  //                                     A0
#define APDU_COMMAND_UIM_OTAPA_REQUEST          0XEE     			  //                                     A0
#define APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST   0XEA      //                                     A0
#define APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST        0XEC  	  //                                     A0
#define APDU_COMMAND_UIM_VALID                  0XCE     			  //                                     A0
#define APDU_COMMAND_UIM_COMMIT                 0XCC    			  //                                     A0    Lc=0

#define NETAPI_LASTCMD_LASTARGU_CURRENTCMD_CURRENTAR             0
#define NETAPI_LASTCMD_LASTARGU_CURRENTCMD                       1
#define NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD               2
#define NETAPI_CURRENTCMD                                        3
#define NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU   4
#define NETAPI_CURRENTCMD_CURRENTARGU                            5
#define NETAPI_SELECT_CURRENTIDENTIFIER_LASTCMD_LASTARGU_CURRENTCMD 6    //2017.5.26
#define NETAPI_SELECT_CURRENTIDENTIFIER_GET_RESPONSE_CURRENTCMD_CURRENTARGU   7       //2017.9.5
#define NETAPI_SELECT_PATH_FIRSTLEVEL_SELECT_CURRENTCMD_CURRENTARGU   8
#define NETAPI_SELECT_PATH_FIRSTLEVEL_SECONDLEVEL_CURRENTCMD_CURRENTARGU   9
#define NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD             10
#define NETAPI_SELECT_ALLTYPEPATH_CURRENTCMD_CURRENTARGU         11
#define NETAPI_SELECT_ALLTYPEPATH_CURRENTCMD                     12
#define NETAPI_SELECT_ALLTYPEPATH__LASTCMD_LASTARGU_CURRENTCMD   13

#define EXPECT_IS_COMMAND               0x0
#define EXPECT_IS_ARGUMENT_DATA         0x1


class Emu_Engine
        {           
           public:
           	  emu_apdu_engine_t eae;
         	    vcard_sort_list_t VcardIndexList[MAX_VCARDINDEX];
         	    vcard_sort_list_t VcardIndexList_Usim[MAX_VCARDINDEX];
         	 protected:    
         	    int GetVcardFileProperty(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
         	    int GetVcardFileProperty_Uim(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
         	    int GetVcardFilePropertyWithPath_Uim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property);
         	    int GetVcardFilePropertyWithPath_Sim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property);
         	    int GetVcardFileProperty_Usim(unsigned char length,unsigned char *identifier  ,\
	                                            vcard_file_property_t *vcard_file_property) ;
              
              int GetVcardFileProperty_WithUsimRidType(unsigned char length,unsigned char *identifier  , unsigned char ridtype, \
	                                            vcard_file_property_t *vcard_file_property);	                                            
         	    int CheckCurrentFileInVcard(unsigned char ucvcardbuf[] , unsigned long *currentoffset);
         	    void  UpdatePath_SimAndUim(unsigned char identifier[2] );
         	    void  UpdatePath_Usim(unsigned char identifierLength, unsigned char* identifier);
         	    void  UpdateExpectPath_SimAndUim();
         	    void  UpdateExpectPath_Usim(unsigned char* ExpectIdWithPathLength,unsigned char* ExpectIdWithPath );
         	    int CheckVcardValidate(unsigned char ucvcardbuf[]);
         	    void  TransferEaeToNetPathAndExpectId( );
         	    void InitVcardIndexList( vcard_sort_list_t *VcardIndexList ,unsigned short ListVolume);
          	  int GetRemoteSimCardPlugStatus();
         	    int VerifyCommandFromEmu();
              int VerifyArgumentFromEmu();
              int VerifyDataFromEmu();
              int CopyApduCommandToEngine();
              int CopyApduArgumentToEngine();
              int TransferEaeToEmu(unsigned char *buf, unsigned short length );
              int TransferEaeToEmu_INS();
              int TransferEaeToNet_NetApi_Select();
              int TransferEaeToNet(int netpaiproperty);
              int TransferEaeToNet_Usim(int netpaiproperty);
              int TransferEaeToNet_NetApi_GetResponse();
              int TransferEaeToNet_NetApi_Read_Binary();
              int TransferEaeToNet_NetApi_Status();
              int TransferEaeToNet_NetApi_Run_Gsm_Algorithm();
              void Transfer2Net_PathDependingAcutualStatus_CurrentCmd();
              void Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();
              void TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
              void Transfer2Net_PathDependingAcutualStatus_LastCmd_LastArgu_CurrentCmd();
              void  TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
              void TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
              void TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
              int TransferEaeToNetSide_NoDeliver(int netpaiproperty);
              int  SetCurrentIdentifierPath(unsigned char identifier[2]);
              int  SetCurrentLogicChannelIdentifierPath(unsigned char lch,unsigned char identifier[2] );
              int GetIdentifierResponse(int vcardindexlistserialnumber, unsigned char (*identifierresponse)[2]);
              int GetIdentifierIndex(unsigned char identifier[2]);
              int GetIdentifierIndexWithPath(unsigned char identifier[2]);
           //   int GetIdentifierIndex_Uim(unsigned char identifier[2]);
              int GetIdentifierIndex_Usim(unsigned char length,unsigned char identifier[IDENTIFIERDEEPTH]);
              int GetIdentifierIndex_UsimWithRidType(unsigned char length,unsigned char identifier[IDENTIFIERDEEPTH], unsigned ridtype);
              
              int GetFileHeaderResponse(int vcardindexlistserialnumber, unsigned char (*fileheaderresponse)[2]);
              int GetFileHeaderContext(int vcardindexlistserialnumber, unsigned char *fileheaderbuf, unsigned char length);
              int GetFileHeaderLength(int vcardindexlistserialnumber, unsigned char *length);
              int GetFileBodyResponse(int vcardindexlistserialnumber, unsigned char (*filebodyresponse)[2]);
              int GetFileBodyContext(int vcardindexlistserialnumber, unsigned char *filebodybuf, unsigned short length);
              int GetFileRecordResponse(int vcardindexlistserialnumber, unsigned char (*filerecordresponse)[2]);
              int GetFileRecord(int vcardindexlistserialnumber,unsigned char recordserialnumber, unsigned char *filerecordbuf, unsigned char length);
              int GetFileRecordWithCurrentFileProperty(int vcardindexlistserialnumber,unsigned char recordserialnumber, unsigned char *filerecordbuf, unsigned char length);
              
              int GetFileBodyLength(int vcardindexlistserialnumber);
              int GetRecordVolume(int vcardindexlistserialnumber);
              int GetRecordBodyLength(int vcardindexlistserialnumber);
              int SetFileBodyContext(int vcardindexlistserialnumber, unsigned short offset,unsigned char *filebodybuf, unsigned short length);  	    
              unsigned long GetTickCount();
              int DirectTransfer_EmuToNet();
              int GetTagLocation_inFCP(unsigned char tag,unsigned char *Buf, int length);
              int IsUsimModeEnable();
              int IsSimModeEnable();
              int IsUimModeEnable();
              void SetCurrentEngineStatusWhenUncertain();
              int IsCurrentIdentifierMF();
              int SubProcessSelectArgumentUim(vcard_file_property_t file1);
              int SubProcessSelectArgumentSim(vcard_file_property_t file1);
              int SubProcessSelectArgumentUsim();
              //int SubProcessGetResponseLastCommandIsSelectUim(vcard_file_property_t file1);
             // int SubProcessGetResponseLastCommandIsSelectSim(vcard_file_property_t file1);
              int SubProcessGetResponseLastCommandIsSelectUimSim();
              int SubProcessGetResponseLastCommandIsSelectUsim();
              int SubProcessGetResponseLastCommandIsStatusUsim();
              int SubProcessGetResponseLastCommandIsSeekUsim();
              int SubProcessStatusCommandUim();
              int SubProcessStatusCommandSim();
              int SubProcessStatusCommandUsim();
              int SubProcessReadBinaryCommandUim();
              int SubProcessReadBinaryCommandSim();
              int SubProcessReadBinaryCommandUsim();
              int SubProcessReadRecordCommandUsim();
              int SubProcessReadRecordCommandSim();
              int SubProcessReadRecordCommandUim();
              int GetCurrentDir(unsigned char *identifier );
              int GenExpectedFidWithSfi_Usim();
              int GenExpectedFidWithSfiAccordingIndexList_Usim();
              int SubProcessReadBinaryWithSfiUsim( );
              int SubProcessReadRecordWithSfiUsim( );
              int IsSameDir(unsigned char IdentifierA_Length,unsigned char * IdentifierA,unsigned char IdentifierB_Length,unsigned char * IdentifierB);
              int GetPin(unsigned char ucvcardbuf[]);
              int IsRightPin(unsigned char inputpinlen,unsigned char * inputpin );
              int IsUnsuitableInputAtEnablePin(unsigned char inputpinlen );
              int char_arrayncmp(unsigned char * array1 ,unsigned char * array2 ,unsigned char length);
              int SeekLocalOk_Usim(unsigned char RecordStartNo,unsigned char* SeekResponse);             
              void DeliverReadCommandWithUnkownSfi_Usim();
              int SubProcessUpdateBinaryWithSfiUsim( );
              void DeliverUpdateCommandArgumentWithUnkownSfi_Usim();
              int SubProcessUpdateBinaryCommandArgumentUsim();
              int SubProcessUpdateBinaryCommandArgumentSimOrUim();
              int SubProcessUpdateRecordCommandArgumentUsim();
              int SubProcessUpdateRecordCommandArgumentSimOrUim();
              int SubProcessUpdateRecordWithSfiUsim( );
              int SetFileRecord(int vcardindexlistserialnumber,unsigned char recordserialnumber, unsigned char *filerecordbuf, unsigned char length); 
              void SubProcessStatusCurrentAidUsim(unsigned char ucCurrentProcessLogicalChannel );
              int EmuApduEngineInit();
              void SetOptimizationMode(unsigned char optimizationmode );
              int PreProcessEngine(unsigned char ucvcardbuf[] );
              int SubProcessStatusCurrentAidDfLengthUsim(unsigned char ucCurrentProcessLogicalChannel );
              int IsValidIdentifier(unsigned char length,unsigned char *identifier  );
             
            public: 
            	int EmuApduEngineInitWithOptimizationMode(unsigned char optimizationmode );
            	Emu_Engine();           
              int GenVcardIndex(unsigned char ucvcardbuf[] , unsigned long maxbuflengh,vcard_sort_list_t *VcardIndexList  ,\
              unsigned char tag_filegroup,unsigned short ListVolume ); 
              unsigned char CheckUiccInVcard(unsigned char ucvcardbuf[]) ;
              unsigned char CheckHandleModeInVcard(unsigned char ucvcardbuf[]) ; //20200330 处理Invaild card
              unsigned short FreshLengthAndReturnValue(unsigned char *SourceArray,unsigned short addlength,unsigned short *returnlength );
              int GenVcardIndexAll(unsigned char ucvcardbuf[]);
			  int GenVcardIndexAll(unsigned char ucvcardbuf[],vcard_sort_list_t *VcardList);
              int PrintVcard(vcard_sort_list_t VcardIndexList[]);   
              int PrintVcardUsim(vcard_sort_list_t VcardIndexList[]) ;          
              int SetRemoteSimCardPlugStatus(unsigned char ucRemoteSimCardPlug);
              int GetEmuEngineStatus();
              int SetEmuEngineStatus(unsigned char ucEmuEngineStatus);
              int GetEmuApduNetApiStatus();
              int SetEmuApduNetApiStatus(unsigned char ucEmuApduNetApiStatus);
              void TestVcardIndex();
              int PreProcessEngineWithOptimizationMode(unsigned char ucvcardbuf[],unsigned char optimizationmode );
             
              int ProcessEmuDataCmd();
              int CopyEaeDataToDeliverNetBuf(unsigned char* DeliverNetBuf, unsigned short* lengthptr,unsigned char* ToNetDataPropertyPtr);
              int CopyNetDataToEaeBuf(unsigned char* netbuf, unsigned short length,unsigned char FromNetDataProperty);
              int SetEmuSimcardAtrReadyStatus(unsigned char ucEmuSimcardAtrReadystatus);	 
              unsigned char GetEmuSimcardAtrReadyStatus();              	
              int SetDeliverAtrToEmuStatus(unsigned char ucDeliverAtrToEmustatus);             	
							int SetEmuTransmissionProtocol(unsigned char ucTransmissionProtocol);
							int SetEmuLocoalVCardReadyStatus(unsigned char ucLocoalVCardReadystatus);
							int SetFromEmuDataProperty(unsigned char ucFromEmuDataProperty);
							unsigned char GetFromEmuDataProperty( );
							int CopyFromEmuDataToEaeBuf(unsigned char* fromemubuf, unsigned short length,unsigned char FromEmuDataProperty);
							unsigned char GetToNetDataProperty( );
							unsigned short GetToEmuDataLength( );
							int CopyEaeDataToEmuBuf(unsigned char* ToEmuBuf, unsigned short* lengthptr);
							unsigned char GetPinValueFromEmu(unsigned char *ptrPinLength,unsigned char* ptrPinBuf);
							int SubProcessFromEmuCommand();
							int SubProcessFromEmu_Argument();
							void SubProcessFromNet();
			void SetDefaultCommandIdentifier();   				
						};
        
#endif         
