
#ifndef __EMU_RDR_COMMON_H__
#define __EMU_RDR_COMMON_H__


#define  MAX_RECORD_VOLUME_PER_LCFILE       254
#define  APDU_BUF_MAX_LENGTH             0x103
#define  READ_BIANARY_BUF_MAX_LENGTH             0x10000  //64K 
#define  BANK_FRAME_MAX_LENGTH             320

#define EMU_ADAPTOR_SPEED_LEVEL_NORMAL    0
#define EMU_ADAPTOR_SPEED_LEVEL_F512D8    1
#define EMU_ADAPTOR_SPEED_LEVEL_F512D16   2

// TS.102.221_11.1.3.2
#define READ_UPDATE_BINARY_P1_SFI_SUPPORT_B8_B7_B6        ((1<<2)|(0<<1)|(0<<0))
#define READMODE_COMMON        0
#define READMODE_SFI           1
#define UPDATEMODE_COMMON        READMODE_COMMON
#define UPDATEMODE_SFI           READMODE_SFI



#define EMU_TRANSLAOTR_OPTIMIZATION_NULL                                      0
#define EMU_TRANSLAOTR_OPTIMIZATION_FORK_GET_RESPONSE_WITH_RUNAUTH            1<<0

typedef struct  RID_INFORM_S
{
	unsigned char rid[5];
	unsigned char ridtype; 
}rid_inform_t;

typedef struct LINEARFIXED_CYCLIC_RECORD_S
{
	unsigned char* ptr_recordserialnumber;				                      
	unsigned char* ptr_recordlength;				                           
	unsigned char* ptr_recordcontext ;			                 
}linearfixed_cyclic_record_t;


typedef  struct VCARD_SORT_LIST_S
{
	unsigned char* ptr_identifier;				                       // 文件标识符ptr
	unsigned char* ptr_identifier_length;				                 // 文件标识符长度ptr
	unsigned char* ptr_access_flag;				                           // 文件access属性
	unsigned char* ptr_fileheaderlength ;	
	unsigned char* ptr_fileheaderresponse ;	
	unsigned char* ptr_fileheader ;	
	unsigned char* ptr_filebodylength ;	
	unsigned char* ptr_filebodyresponse ;	
	unsigned char* ptr_filebody ;	
	unsigned char* ptr_recordresponse ;	
	linearfixed_cyclic_record_t ptr_recordbody[MAX_RECORD_VOLUME_PER_LCFILE]; 	
	unsigned char ucCurrentEFType ;   
  unsigned char ucSfi ;             
}vcard_sort_list_t;

typedef struct  ID_WITH_PATH_S
{
	unsigned char mfid[2];
	unsigned char firstlevelid[2];
	unsigned char secondlevelid[2];
	unsigned char thirdlevelid[2];
	
}id_with_path_t;

// 虚拟卡中文件属性数据结构
typedef struct ALLTYPE_VCARD_IDENTIFIER_S
{
	unsigned char length;				                           // identifier数量
	unsigned char identifier1[2];				                       // 文件标识符
	unsigned char identifier2[2];				                       // 文件标识符
	unsigned char identifier3[2];				                       // 文件标识符
 	unsigned char ridtype;                                    //rid类型， 用于区分不同aid 下的同名文件
}alltype_vcard_identifier_t;

#endif