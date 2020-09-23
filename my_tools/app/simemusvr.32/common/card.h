
/****************************************************************************
* 版权信息：
* 系统名称：SimRdrSvr
* 文件名称：card.h 
* 文件说明：SimRdrSvr端虚拟Sim Card 相关头文件
* 作    者：Carson 
* 版本信息：v1.0 
* 设计日期：2015/1/7
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
#ifndef __CARD_G2_H__
#define __CARD_G2_H__

#include <pthread.h>
#include "Emu_Rdr_Common.h"

#define  FILEOPERATIONMAX           0x100
#define  FILEMAXINVCARD             0x30
#define  SIMFILEPATHDEEPTH          3
#define  GETIATIMEOUT               6000
//#define  GETIATIMEOUT               60000000
#define  FILE_MAX_IN_EXTEND_VCARD   0X20
//#define APPICAITON_NUMBER_MAX        2
#define APPICAITON_NUMBER_MAX        5

#define CARD_IDENTIFIERDEEPTH                    0x10
#define MAX_LOGICAL_CHANNELS_NUMBER      (3+1)  //TS 102.221 V12 Table 10.3

/****************************************************************************
 ATR中判断卡类型的字节--- 可能会影响模块的工作模式（如Sim 或Usim）
****************************************************************************/
#define HANDLE_ATR_FISRT_TA_FOR_T_EUQAL15              1

/****************************************************************************
 修改ATR时只允许修改 TA1 ，其余保持不变
****************************************************************************/
#define KEEP_ATR_EXCLUDE_TA1              1


#define PIN_ERROR_OR_NO_SET              0


/****************************************************************************
文件类型
****************************************************************************/

#define FILE_TYPE_MF               1
#define FILE_TYPE_DF               1<<1
#define FILE_TYPE_EF               1<<2


// 为运营商自定义ID识别用
#define FILE_1ST_ID_MF                   0x3F 
#define FILE_1ST_ID_UNDER_MF_EF          0x2F 
#define FILE_1ST_ID_1LEVEL_DF            0x7F 
#define FILE_1ST_ID_UNDER_1LEVEL_DF_EF   0x6F 
#define FILE_1ST_ID_2LEVEL_DF            0x5F
#define FILE_1ST_ID_UNDER_2LEVEL_DF_EF   0x4F


#define VCARD_FILE_PROPERTY_LIST_MODE_SIM   0
#define VCARD_FILE_PROPERTY_LIST_MODE_UIM   1

/****************************************************************************
EF文件结构类型
****************************************************************************/
#define EF_STRUCTURE_TRANSPARENT        1<<3
#define EF_STRUCTURE_LINEARFIXED        1<<4
#define EF_STRUCTURE_CYCLIC             1<<5

/****************************************************************************
文件更新程度属性
****************************************************************************/
#define FILE_UPDATE_ACTIVITY_LOW         1<<6
#define FILE_UPDATE_ACTIVITY_HIGH        1<<7

/****************************************************************************
文件必要程度
****************************************************************************/
#define FILE_MANDATORY                   1<<8
#define FILE_OPTIONAl                    1<<9



/****************************************************************************
允许Vcard 不记录 线性定长/循环文件中的缺省数据
****************************************************************************/
#define SPARE_MODE_LINEARFIXED_CYCLIC    1<<10

/****************************************************************************
线性定长/循环文件中的缺省数据值
****************************************************************************/
#define DEFAULT_FF_LINEARFIXED_CYCLIC    1<<11
#define DEFAULT_0_LINEARFIXED_CYCLIC     1<<12

/****************************************************************************
从线性定长/循环文件中的记录中第几位开始计算缺省数据
****************************************************************************/
#define STARTBYTE_1_LINEARFIXED_CYCLIC    1<<13

/****************************************************************************
文件訪問方式
****************************************************************************/
#define FILE_ACESS_AID_ONLY                   1<<14
#define FILE_ACESS_FID_PATH_SFI               1<<15

/****************************************************************************
會話激活條件
****************************************************************************/
#define FILE_SESSION_ACTIVATE_AFTER_AID_ACCESS 1<<16
#define FILE_SESSION_ACTIVATE_NOLIMITE         1<<17

/****************************************************************************
重名EF FID
****************************************************************************/
#define EXIST_SAME_EF_NAME                      1<<18
/****************************************************************************
重名DF FID
****************************************************************************/
#define EXIST_SAME_DF_NAME                      1<<19

/****************************************************************************
太长的透明文件可以截除后面的0
****************************************************************************/
#define LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0                      1<<20    //只用于过长的透明文件

/****************************************************************************
//只读透明文件的文件头，用于读取High Active文件头，或标准中未明确定义的文件的文件头
****************************************************************************/
#define EF_READ_HEADER_ONLY                      1<<21   

/****************************************************************************
//从文件头查找EF文件类型（EF 透明，线性，循环）
****************************************************************************/
#define GET_EF_TYPE_FROM_HEADER                      1<<22  

/****************************************************************************
//文件可能被Update，如果被Update， 允许EMU侧Vcard 和 True Card 对此文件不同步
****************************************************************************/
#define CAN_FORK                      1<<23  


/****************************************************************************
从线性定长/循环文件中的记录中第2位开始计算缺省数据
****************************************************************************/
#define STARTBYTE_2_LINEARFIXED_CYCLIC    1<<24

#define DEFAULT_UNKOWN_LINEARFIXED_CYCLIC     1<<25

#define SELECTFILESTATUS_SUCCESS           0
#define SELECTFILESTATUS_FAIL              1

#define CURRENT_FILE_BODY_LENGTH_STATUS_NORMAL             0
#define CURRENT_FILE_BODY_LENGTH_STATUS_MORETHAN255B       1

#define LCEFREORDDEAULT_UNKNOWN                            0
#define LCEFREORDDEAULT_KNOWN                              1


#define VCARD_FLAG_LCEFREORDDEAULT_FF                       1<<3
#define VCARD_FLAG_LCEFREORDDEAULT_0                       1<<4

#define INNERDATAPROCESSSTATUS_NORMAL                      0
#define INNERDATAPROCESSSTATUS_IGNORE                     1

/****************************************************************************
文件标识符
****************************************************************************/
#define  MF_IDENTIFIER                   {0x3F,0x00}

#define  DFTELECOM_IDENTIFIER            {0x7F,0x10}
#define  DFGSM_IDENTIFIER                {0x7F,0x20}
#define  DFDCS1800_IDENTIFIER            {0x7F,0x21}
#define  USIMADF_IDENTIFIER              {0x7F,0xff}
#define  DFIS_41_IDENTIFIER              {0x7F,0x23}

/*********************************
EF --Parent is MF 
**********************************/
#define  EFICCID_IDENTIFIER              {0x2F,0xE2}
#define  EFELP_IDENTIFIER                {0x2f,0x05}
#define  EFDIR_IDENTIFIER                {0x2f,0x0}
#define  PMF_EFARR_IDENTIFIER            {0x2f,0x06}

/********************************* SIM EF 文件空间****************************************/

/*********************************
EF --Parent is DFGSM 
**********************************/
#define  EFLP_IDENTIFIER                 {0x6f,0x05}   //有重名FID
#define  PGSM_EFIMSI_IDENTIFIER          {0x6F,0x07}   //有重名FID
#define  EFKc_IDENTIFIER                 {0x6f,0x20}
#define  EFPLMNsel_IDENTIFIER            {0x6f,0x30}
#define  EFHPLMN_IDENTIFIER              {0x6f,0x31}  //有重名FID
#define  EFACMmax_IDENTIFIER             {0x6f,0x37}  //有重名FID
#define  EFSST_IDENTIFIER                {0x6f,0x38} //有重名FID
#define  EFACM_IDENTIFIER                {0x6f,0x39}//有重名FID
#define  EFGID1_IDENTIFIER               {0x6f,0x3e} //有重名FID
#define  EFGID2_IDENTIFIER               {0x6f,0x3f} //有重名FID
#define  EFSPN_IDENTIFIER                {0x6f,0x46} //有重名FID
#define  EFPUCT_IDENTIFIER               {0x6f,0x41} //有重名FID
#define  EFCBMI_IDENTIFIER               {0x6f,0x45} //有重名FID
#define  EFBCCH_IDENTIFIER               {0x6f,0x74}
#define  EFACC_IDENTIFIER                {0x6f,0x78}//有重名FID
#define  EFFPLMN_IDENTIFIER              {0x6f,0x7b} //有重名FID
#define  EFLOCI_IDENTIFIER               {0x6f,0x7e} //有重名FID
#define  EFAD_IDENTIFIER                 {0x6f,0xad} //有重名FID
#define  EFPhase_IDENTIFIER              {0x6f,0xAE}

#define  EFVGCS_IDENTIFIER               {0x6f,0xB1}//有重名FID
#define  EFVGCSS_IDENTIFIER              {0x6f,0xB2}//有重名FID
#define  EFVBS_IDENTIFIER                {0x6f,0xB3}//有重名FID
#define  EFVBSS_IDENTIFIER               {0x6f,0xB4}//有重名FID
#define  EFEmlpp_IDENTIFIER              {0x6f,0xB5}//有重名FID
#define  EFAAeM_IDENTIFIER               {0x6f,0xB6}//有重名FID
#define  EFCBMID_IDENTIFIER              {0x6f,0x48}//有重名FID
#define  EFECC_IDENTIFIER                {0x6F,0xB7}//有重名FID
#define  EFCBMIR_IDENTIFIER              {0x6f,0x50}//有重名FID
#define  EFDCK_IDENTIFIER                {0x6f,0x2c}//有重名FID
#define  EFCNL_IDENTIFIER                {0x6f,0x32}//有重名FID
#define  EFNIA_IDENTIFIER                {0x6f,0x51}
#define  EFKcGPRS_IDENTIFIER             {0x6f,0x52}
#define  EFLOCIGPRS_IDENTIFIER           {0x6f,0x53}
#define  EFSUME_IDENTIFIER               {0x6f,0x54}
#define  EFPLMNwAcT_IDENTIFIER           {0x6F,0x60}//有重名FID     3GPP TS11.11
#define  EFOPLMNwAcT_IDENTIFIER          {0x6F,0x61}//有重名FID

//6F1X 用于管理用途 
#define  PFGSM_EF_6F11                       {0x6F,0x11}
#define  PFGSM_EF_6F13                       {0x6F,0x13}
#define  PFGSM_EF_6F14                       {0x6F,0x14}
#define  PFGSM_EF_6F15                       {0x6F,0x15}
#define  PFGSM_EF_6F16                       {0x6F,0x16}
#define  PFGSM_EF_6F17                       {0x6F,0x17}
#define  PFGSM_EF_6F18                       {0x6F,0x18}
#define  PFGSM_EF_6F19                       {0x6F,0x19}

//未在GSM标准中
#define  PFGSM_EF_6F60          {0x6F,0x60}//有重名FID   
#define  PFGSM_EF_6F61          {0x6F,0x61}//有重名FID
#define  PFGSM_EF_6F62          {0x6F,0x62}//有重名FID
#define  PFGSM_EF_6FC5          {0x6F,0xC5}
#define  PFGSM_EF_6FC6          {0x6F,0xC6}
#define  PFGSM_EF_6FCB          {0x6F,0xCB}
#define  PFGSM_EF_6FCD          {0x6F,0xCD}




/*********************************
EF --Parent is DFTELECOM 
**********************************/
#define  EFADN_IDENTIFIER                {0x6f,0x3a}
#define  EFFDN_IDENTIFIER                {0x6f,0x3b}//有重名FID
#define  EFSMS_IDENTIFIER                {0x6F,0x3C}//有重名FID
#define  EFCCP_IDENTIFIER                {0x6F,0x3d}
#define  EFMSISDN_IDENTIFIER             {0x6f,0x40}//有重名FID
#define  EFSMSP_IDENTIFIER               {0x6f,0x42} //有重名FID
#define  EFSMSS_IDENTIFIER               {0x6f,0x43}//有重名FID
#define  EFLND_IDENTIFIER                {0x6f,0x44}
#define  EFSMSR_IDENTIFIER               {0x6f,0x47}//有重名FID

#define  EFSDN_IDENTIFIER                {0x6f,0x49} //有重名FID
#define  EFEXT1_IDENTIFIER               {0x6f,0x4a} 
#define  EFEXT2_IDENTIFIER               {0x6f,0x4b}
#define  EFEXT3_IDENTIFIER               {0x6f,0x4c}//有重名FID
#define  EFBDN_IDENTIFIER                {0x6f,0x4d}//有重名FID
#define  EFEXT4_IDENTIFIER               {0x6f,0x4e}//有重名FID
#define  PTELECOM_EFARR_IDENTIFIER       {0x6f,0x06}  //Not Exist in GSM //有重名FID

/*********************************
DF --Parent is DFTELECOM 
**********************************/
#define  PTELECOMDF_DFGRAPHICS_IDENTIFIER {0x5F,0x50}  //有重名FID
/********************************* SIM EF 文件空间****************************************/

/********************************* USIM EF 文件空间****************************************/


/*********************************
EF --Parent is ADFUSIM    Based TS31.102
**********************************/
#define  PADF_EFLI_IDENTIFIER            {0x6f,0x05}  //有重名FID
#define  PADF_EFARR_IDENTIFIER           {0x6f,0x06}  //有重名FID
#define  PADF_EFIMSI_IDENTIFIER          {0x6F,0x07}  //有重名FID
#define  PADF_EFKeys_IDENTIFIER          {0x6F,0x08}
#define  PADF_EFKEYPS_IDENTIFIER         {0x6F,0x09}
#define  PADF_EFDCK_IDENTIFIER           {0x6F,0x2C} //有重名FID
#define  PADF_EFHPPLMN_IDENTIFIER        {0x6F,0x31} //有重名FID
#define  PADF_EFCNL_IDENTIFIER           {0x6F,0x32} //有重名FID
#define  PADF_EFACMmax_IDENTIFIER        {0x6F,0x37} //有重名FID
#define  PADF_EFUST_IDENTIFIER           {0x6F,0x38}//有重名FID
#define  PADF_EFACM_IDENTIFIER           {0x6F,0x39}//有重名FID
#define  PADF_EFFDN_IDENTIFIER           {0x6F,0x3B} //有重名FID
#define  PADF_EFSMS_IDENTIFIER           {0x6F,0x3C} //有重名FID
#define  PADF_EFGID1_IDENTIFIER          {0x6F,0x3E} //有重名FID
#define  PADF_EFGID2_IDENTIFIER          {0x6F,0x3F} //有重名FID
#define  PADF_EFMSISDN_IDENTIFIER        {0x6F,0x40} //有重名FID
#define  PADF_EFPUCT_IDENTIFIER          {0x6F,0x41}//有重名FID
#define  PADF_EFSMSP_IDENTIFIER          {0x6F,0x42} //有重名FID
#define  PADF_EFSMSS_IDENTIFIER          {0x6F,0x43} //有重名FID
#define  PADF_EFCBMI_IDENTIFIER          {0x6F,0x45} //有重名FID
#define  PADF_EFSPN_IDENTIFIER           {0x6F,0x46} //有重名FID
#define  PADF_EFSMSR_IDENTIFIER          {0x6F,0x47} //有重名FID
#define  PADF_EFCBMID_IDENTIFIER         {0x6F,0x48} //有重名FID
#define  PADF_EFSDN_IDENTIFIER           {0x6F,0x49} //有重名FID
#define  PADF_EFEXT2_IDENTIFIER          {0x6F,0x4B}
#define  PADF_EFEXT3_IDENTIFIER          {0x6F,0x4C} //有重名FID
#define  PADF_EFBDN_IDENTIFIER           {0x6F,0x4D} //有重名FID
#define  PADF_EFEXT5_IDENTIFIER          {0x6F,0x4E} //有重名FID
#define  PADF_EFCCP2_IDENTIFIER          {0x6F,0x4F}
#define  PADF_EFCBMIR_IDENTIFIER         {0x6F,0x50} //有重名FID
#define  PADF_EFEXT4_IDENTIFIER          {0x6F,0x55} 
#define  PADF_EFEST_IDENTIFIER           {0x6F,0x56}
#define  PADF_EFACL_IDENTIFIER           {0x6F,0x57}
#define  PADF_EFCMI_IDENTIFIER           {0x6F,0x58}
#define  PADF_EFSTART_HFN_IDENTIFIER     {0x6F,0x5B}
#define  PADF_EFTHRESHOLD_IDENTIFIER     {0x6F,0x5C}
#define  PADF_EFPLMNwAcT_IDENTIFIER      {0x6F,0x60}//有重名FID
#define  PADF_EFOPLMNwAcT_IDENTIFIER     {0x6F,0x61}//有重名FID
#define  PADF_EFHPLMNwAcT_IDENTIFIER     {0x6F,0x62}
#define  PADF_EFPSLOCI_IDENTIFIER        {0x6F,0x73}
#define  PADF_EFACC_IDENTIFIER           {0x6F,0x78} //有重名FID
#define  PADF_EFFPLMN_IDENTIFIER         {0x6F,0x7B} //有重名FID
#define  PADF_EFLOCI_IDENTIFIER          {0x6F,0x7E} //有重名FID
#define  PADF_EFICI_IDENTIFIER           {0x6F,0x80}
#define  PADF_EFOCI_IDENTIFIER           {0x6F,0x81}
#define  PADF_EFICT_IDENTIFIER           {0x6F,0x82}
#define  PADF_EFOCT_IDENTIFIER           {0x6F,0x83}
#define  PADF_EFAD_IDENTIFIER            {0x6F,0xAD}//有重名FID
#define  PADF_EFVGCS_IDENTIFIER          {0x6F,0xB1} //有重名FID
#define  PADF_EFVGCSS_IDENTIFIER         {0x6F,0xB2} //有重名FID
#define  PADF_EFVBS_IDENTIFIER           {0x6F,0xB3} //有重名FID
#define  PADF_EFVBSS_IDENTIFIER          {0x6F,0xB4} //有重名FID
#define  PADF_EFeMLPP_IDENTIFIER         {0x6F,0xB5}//有重名FID
#define  PADF_EFAAeM_IDENTIFIER          {0x6F,0xB6}//有重名FID
#define  PADF_EFEFECC_IDENTIFIER         {0x6F,0xB7}//有重名FID
#define  PADF_EFHiddenkey_IDENTIFIER     {0x6F,0xC3}
#define  PADF_EFNETPAR_IDENTIFIER        {0x6F,0xC4}
#define  PADF_EFPNN_IDENTIFIER           {0x6F,0xC5}
#define  PADF_EFOPL_IDENTIFIER           {0x6F,0xC6}
#define  PADF_EFMBDN_IDENTIFIER          {0x6F,0xC7}
#define  PADF_EFEXT6_IDENTIFIER          {0x6F,0xC8}
#define  PADF_EFMBI_IDENTIFIER           {0x6F,0xC9}
#define  PADF_EFMWIS_IDENTIFIER          {0x6F,0xCA}
#define  PADF_EFCFIS_IDENTIFIER          {0x6F,0xCB}
#define  PADF_EFEXT7_IDENTIFIER          {0x6F,0xCC}
#define  PADF_EFSPDI_IDENTIFIER          {0x6F,0xCD}
#define  PADF_EFMMSN_IDENTIFIER          {0x6F,0xCE}
#define  PADF_EFEXT8_IDENTIFIER          {0x6F,0xCF}
#define  PADF_EFMMSICP_IDENTIFIER        {0x6F,0xD0}
#define  PADF_EFMMSUP_IDENTIFIER         {0x6F,0xD1}
#define  PADF_EFMMSUCP_IDENTIFIER        {0x6F,0xD2}
#define  PADF_EFNIA_IDENTIFIER           {0x6F,0xD3}
#define  PADF_EFVGCSCA_IDENTIFIER        {0x6F,0xD4}
#define  PADF_EFVBSCA_IDENTIFIER         {0x6F,0xD5}
#define  PADF_EFGBAP_IDENTIFIER          {0x6F,0xD6}
#define  PADF_EFMSK_IDENTIFIER           {0x6F,0xD7}
#define  PADF_EFMUK_IDENTIFIER           {0x6F,0xD8}
#define  PADF_EFEHPLMN_IDENTIFIER        {0x6F,0xD9}
#define  PADF_EFGBANL_IDENTIFIER         {0x6F,0xDA}
#define  PADF_EFEHPLMNPI_IDENTIFIER      {0x6F,0xDB}
#define  PADF_EFLRPLMNSI_IDENTIFIER      {0x6F,0xDC}
#define  PADF_EFNAFKCA_IDENTIFIER        {0x6F,0xDD}
#define  PADF_EFSPNI_IDENTIFIER          {0x6F,0xDE}
#define  PADF_EFPNNI_IDENTIFIER          {0x6F,0xDF}
#define  PADF_EFNCP_IP_IDENTIFIER        {0x6F,0xE2}
#define  PADF_EFEPSLOCI_IDENTIFIER       {0x6F,0xE3}
#define  PADF_EFEPSNSC_IDENTIFIER        {0x6F,0xE4}

//6F1X 用于管理用途 （3GPP TS 31.102）
#define  PADF_6F14                       {0x6F,0x14}
#define  PADF_6F15                       {0x6F,0x15}
#define  PADF_6F16                       {0x6F,0x16}
#define  PADF_6F17                       {0x6F,0x17}
#define  PADF_6F18                       {0x6F,0x18}

//6F65 用于早期版本，未来不被分配 （3GPP TS 31.102）
#define  PADF_6F65                       {0x6F,0x65}

/*********************************
DF --Parent is ADFUSIM    Based TS31.102
**********************************/
#define  PADF_DFPHONEBOOK_IDENTIFIER     {0x5F,0x3A}
#define  PADF_DFGSM_ACCESS_IDENTIFIER    {0x5F,0x3B}
#define  PADF_DFMExE_IDENTIFIER          {0x5F,0x3C}
#define  PADF_DFSoLSA_IDENTIFIER         {0x5F,0x70}
#define  PADF_DFWLAN_IDENTIFIER          {0x5F,0x40}
#define  PADF_DFHNB_IDENTIFIER           {0x5F,0x50} //有重名FID

/*********************************
EF --Parent is DFPHONEBOOK    Based TS31.102
**********************************/
#define  PDFPHONEBOOK_EFPSC_IDENTIFIER   {0x4F,0x22}
#define  PDFPHONEBOOK_EFCC_IDENTIFIER    {0x4F,0x23}
#define  PDFPHONEBOOK_EFPUID_IDENTIFIER  {0x4F,0x24}
#define  PDFPHONEBOOK_EFPBR_IDENTIFIER   {0x4F,0x30}  //有重名FID
#define  PDFPHONEBOOK_EFUID1_IDENTIFIER  {0x4F,0x20}  //有重名FID

#define  PDFPHONEBOOK_EFUID_IDENTIFIER   {0x4F,0x21} //4fxx


//#define  PDFPHONEBOOK_EFCCP1_IDENTIFIER {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFIAP_IDENTIFIER  {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFADN_IDENTIFIER {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFEXT1_IDENTIFIER  {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFGRP_IDENTIFIER {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFEAAS_IDENTIFIER  {0x4F,0xxx}
 #define  PDFPHONEBOOK_EFGAS_IDENTIFIER {0x4F,0x4c}
//#define  PDFPHONEBOOK_EFEANR_IDENTIFIER  {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFSNE_IDENTIFIER {0x4F,0xxx}
//#define  PDFPHONEBOOK_EFEMAIL_IDENTIFIER  {0x4F,0xxx}

/*********************************
EF --Parent is DFGSM_ACCESS    Based TS31.102
**********************************/

#define PDFGSM_ACCESS_EFKcGPRS_IDENTIFIER {0x4F,0x52}
#define PDFGSM_ACCESS_EFCPBCCH_IDENTIFIER {0x4F,0x63}
#define  PDFGSM_ACCESS_EFinvSCAN_IDENTIFIER   {0x4F,0x64}
#define  PDFGSM_ACCESS_EFKc_IDENTIFIER    {0x4F,0x20}  //有重名FID

/*********************************
EF --Parent is DFMExE    Based TS31.102
**********************************/
#define  PDFDFMExE_EFMExE_ST_IDENTIFIER   {0x4F,0x40}
#define  PDFDFMExE_EFORPK_IDENTIFIER      {0x4F,0x41} //有重名FID
#define  PDFDFMExE_EFARPK_IDENTIFIER      {0x4F,0x42} //有重名FID
#define  PDFDFMExE_EFTPRK_IDENTIFIER      {0x4F,0x43} //有重名FID



//#define  PDFDFMExE_EFTKCDF_IDENTIFIER      {0x4F,0xxx}

/*********************************
EF --Parent is DFSoLSA    Based TS31.102
**********************************/

#define  PDFDFSoLSA_EFSLL_IDENTIFIER      {0x4F,0x31}
#define  PDFDFSoLSA_EFSAI_IDENTIFIER      {0x4F,0x30} //有重名FID

/*********************************
EF --Parent is DFWLAN    Based TS31.102
**********************************/


#define  PDFDFWLAN_EFPseudo_IDENTIFIER    {0x4F,0x41} //有重名FID
#define  PDFDFWLAN_EFUPLMNWLAN_IDENTIFIER {0x4F,0x42} //有重名FID
#define  PDFDFWLAN_EF0PLMNWLAN_IDENTIFIER {0x4F,0x43} //有重名FID
#define  PDFDFWLAN_EFUWSIDL_IDENTIFIER    {0x4F,0x44}
#define  PDFDFWLAN_EFOWSIDL_IDENTIFIER    {0x4F,0x45}
#define  PDFDFWLAN_EFWRI_IDENTIFIER       {0x4F,0x46}
#define  PDFDFWLAN_EFHWSIDL_IDENTIFIER    {0x4F,0x47}
#define  PDFDFWLAN_EFWEHPLMNPI_IDENTIFIER {0x4F,0x48}
#define  PDFDFWLAN_EFWHPI_IDENTIFIER      {0x4F,0x49}
#define  PDFDFWLAN_EFWLRPLMN_IDENTIFIER   {0x4F,0x4A}
#define  PDFDFWLAN_EFHPLMNDAI_IDENTIFIER  {0x4F,0x4B}

/*********************************
EF --Parent is DFHNB    Based TS31.102
**********************************/
#define  PDFDFHNB_EFACSGL_IDENTIFIER      {0x4F,0x81}
#define  PDFDFHNB_EFCSGI_IDENTIFIER       {0x4F,0x82}
#define  PDFDFHNBN_EFACSGL_IDENTIFIER     {0x4F,0x83}


/*********************************
EF --FID is not Unique, attach parent ID   Based TS31.102
**********************************/
/*
#define  PDFPHONEBOOK_EFPBR_IDENTIFIER6    {0x7f,0xff,0x5F,0x3A,0x4F,0x30}
#define  PDFDFSoLSA_EFSAI_IDENTIFIER6      {0x7f,0xff,0x5F,0x70,0x4F,0x30}
#define  PDFPHONEBOOK_EFUID1_IDENTIFIER6   {0x7f,0xff,0x5F,0x3A,0x4F,0x20}
#define  PDFGSM_ACCESS_EFKc_IDENTIFIER6    {0x7f,0xff,0x5F,0x3B,0x4F,0x20}
#define  PDFDFMExE_EFORPK_IDENTIFIER6      {0x7f,0xff,0x5F,0x3C,0x4F,0x41}
#define  PDFDFWLAN_EFPseudo_IDENTIFIER6    {0x7f,0xff,0x5F,0x40,0x4F,0x41}
#define  PDFDFMExE_EFARPK_IDENTIFIER6      {0x7f,0xff,0x5F,0x3C,0x4F,0x42}
#define  PDFDFWLAN_EFUPLMNWLAN_IDENTIFIER6 {0x7f,0xff,0x5F,0x40,0x4F,0x42}
#define  PDFDFMExE_EFTPRK_IDENTIFIER6      {0x7f,0xff,0x5F,0x3C,0x4F,0x43}
#define  PDFDFWLAN_EF0PLMNWLAN_IDENTIFIER6 {0x7f,0xff,0x5F,0x40,0x4F,0x43}
*/
/*********************************
EF --Parent is TELECOM    Based TS31.102
**********************************/
#define  PTELECOM_DFPHONEBOOK_IDENTIFIER     {0x5F,0x3A}
#define  PTELECOM_DF5F3C                     {0x5F,0x3C}
#define  PPTELECOM_PDFPHONEBOOK_EFPBC_IDENTIFIER   {0x4F,0x09}
#define  PPTELECOM_PDFPHONEBOOK_EFPSC_IDENTIFIER   {0x4F,0x22}
#define  PPTELECOM_PDFPHONEBOOK_EFGRP_IDENTIFIER {0x4F,0x26}
#define  PPTELECOM_PDFPHONEBOOK_EFEXT1_IDENTIFIER   {0x4f,0x4a} 
#define  PPTELECOM_PDFPHONEBOOK_EFAAS_IDENTIFIER   {0x4f,0x4b}
#define  PPTELECOM_PDFPHONEBOOK_EFEMAIL_IDENTIFIER   {0x4f,0x50} 
#define  PPTELECOM_PDFPHONEBOOK_EFADN_IDENTIFIER   {0x4f,0x3a} 
#define  PPTELECOM_PDFPHONEBOOK_EFADN1_IDENTIFIER   {0x4f,0x3B}  //20180927

#define  PPTELECOM_PDFPHONEBOOK_EFANRA_IDENTIFIER   {0x4f,0x11}  //20181022
#define  PPTELECOM_PDFPHONEBOOK_EFGRP1_IDENTIFIER   {0x4f,0x25}  //20181022
#define  PPTELECOM_PDFPHONEBOOK_EFANRB_IDENTIFIER   {0x4f,0x13}  //20181022
#define  PPTELECOM_PDFPHONEBOOK_4F34   {0x4f,0x34}  //20181022
#define  PPTELECOM_PDFPHONEBOOK_4F33   {0x4f,0x33}  //20181022
#define  PPTELECOM_PDFPHONEBOOK_4F39   {0x4f,0x39}  //20181022
#define  PPTELECOM_PDFPHONEBOOK_EFPBC1_IDENTIFIER   {0x4f,0x0a}  //20181022

#define  PPTELECOM_PDFPHONEBOOK_4F31   {0x4f,0x31}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F41   {0x4f,0x41}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F51   {0x4f,0x51}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F52   {0x4f,0x52}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F5A   {0x4f,0x5a}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F61   {0x4f,0x61}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F72   {0x4f,0x72}  //20180927

#define  PPTELECOM_PDFPHONEBOOK_4F32   {0x4f,0x32}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F42   {0x4f,0x42}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F62   {0x4f,0x62}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F71   {0x4f,0x71}  //20180927
#define  PPTELECOM_PDFPHONEBOOK_4F5B   {0x4f,0x5B}  //20180927

#define  PPTELECOM_PDF5F3C_4F21   {0x4F,0x21} //4fxx
#define  PPTELECOM_PDF5F3C_4F20   {0x4F,0x20} //4fxx


/********************************* USIM EF 文件空间****************************************/


/********************************* CDMA 文件空间****************************************/
#define  DFCDMA_IDENTIFIER                {0x7F,0x25}
#define  PADF_6F06_CDMA                   {0x6f,0x06}  
#define  EFCOUNT_CDMA                     {0x6F,0x21}
#define  EFIMSI_M_CDMA                    {0x6F,0x22}
#define  EFIMSI_T_CDMA                    {0x6F,0x23}
#define  EFTMSI_CDMA                       {0x6F,0x24}
#define  EFAH_CDMA                         {0x6F,0x25}
#define  EFAOP_CDMA                        {0x6F,0x26}

#define   EFALOC_CDMA                      {0x6F,0x27}
#define  EFCDMAHOME_CDMA                   {0x6F,0x28}
#define  EFZNREGI_CDMA                     {0x6F,0x29}
#define  EFSNREGI_CDMA                     {0x6F,0x2A}
#define  EFDISTREGI_CDMA                   {0x6F,0x2B}
#define  EFACCOLC_CDMA                     {0x6F,0x2C}

#define   EFTERM_CDMA                      {0x6F,0x2D}
#define  EFSSCI_CDMA                       {0x6F,0x2E}
#define  EFACP_CDMA                        {0x6F,0x2F}
#define  EFPRL_CDMA                        {0x6F,0x30}
#define  EFRUIMID_CDMA                     {0x6F,0x31}
#define  EFCST_CDMA                        {0x6F,0x32}

#define  EFTSPC_CDMA                       {0x6F,0x33}
#define  EFOTAPASPC_CDMA                   {0x6F,0x34}
#define  EFNAMLOCK_CDMA                    {0x6F,0x35}
#define  EFOTA_CDMA                        {0x6F,0x36}
#define  EFSP_CDMA                         {0x6F,0x37}
#define  EFESNME_CDMA                      {0x6F,0x38}

#define  EFREVISION_CDMA                   {0x6F,0x39}
#define  EFPL_CDMA                         {0x6F,0x3A}
#define  EFFDN_CDMA                        {0x6F,0x3B}
#define  EFSMS_CDMA                        {0x6F,0x3C}
#define  EFSMSP_CDMA                        {0x6F,0x3D}
#define  EFSMSS_CDMA                         {0x6F,0x3E}
#define  EFESSFC_CDMA                      {0x6F,0x3F}

#define  EFSPN_CDMA                         {0x6F,0x41}
#define  EFUSGINDN_CDMA                     {0x6F,0x42}
#define  EFAD_CDMA                         {0x6F,0x43}
#define  EFMDN_CDMA                         {0x6F,0x44}
#define  EFMAXPRL_CDMA                       {0x6F,0x45}
#define  EFSPCS_CDMA                         {0x6F,0x46}
#define  EFECC_CDMA                         {0x6F,0x47}
#define  EFME3GPDOPC_CDMA                   {0x6F,0x48}
#define  EF3GPDOPM_CDMA                     {0x6F,0x49}
#define  EFSIPCAP_CDMA                      {0x6F,0x4a}
#define  EFMIPCAP_CDMA                       {0x6F,0x4B}
#define  EFSIPUPP_CDMA                      {0x6F,0x4C}
#define  EFMIPUPP_CDMA                      {0x6F,0x4D}
#define  EFSIPSP_CDMA                       {0x6F,0x4E}
#define  EFMIPSP_CDMA                       {0x6F,0x4F}


#define  EFSIPPAPSS_CDMA                      {0x6F,0x50}
#define  EFPUZL_CDMA                          {0x6F,0x53}
#define  EMAXPUZL_CDMA                        {0x6F,0x54}
#define  EFMECRP_CDMA                         {0x6F,0x55}
#define  EFHRPDCAP_CDMA                       {0x6F,0x56}
#define  EFHRPDUPP_CDMA                       {0x6F,0x57}
#define  EFCSSPR_CDMA                         {0x6F,0x58}
#define  EFATC_CDMA                           {0x6F,0x59}
#define  EFEPRL_CDMA                          {0x6F,0x5A}
#define  EFBCSMSCFG_CDMA                      {0x6F,0x5B}
#define  EFBCSMSPREF_CDMA                     {0x6F,0x5C}
#define  EFBCSMSTABLE_CDMA                    {0x6F,0x5D}
#define  EFBCSMSP_CDMA                        {0x6F,0x5E}
#define  EFIMPI_CDMA                          {0x6F,0x5F}

#define  EFDOMAIN_CDMA                        {0x6F,0x60}
#define  EFIMPU_CDMA                          {0x6F,0x61}
#define  EFPCSCF_CDMA                         {0x6F,0x62}
#define  EFBAKPARA_CDMA                       {0x6F,0x63}
#define  EFUpBAKPARA_CDMA                     {0x6F,0x64}
#define  EFMMSN_CDMA                          {0x6F,0x65}
#define  EFEXT8_CDMA                          {0x6F,0x66}
#define  EFMMSICP_CDMA                        {0x6F,0x67}
#define  EFMMSUP_CDMA                         {0x6F,0x68}
#define  EFMMSUCP_CDMA                        {0x6F,0x69}
#define  EFAUTHCAPABILITY_CDMA                {0x6F,0x6A}
#define  EF3GCIK_CDMA                        {0x6F,0x6B}
#define  EFDCK_CDMA                           {0x6F,0x6C}
#define  EFGID1_CDMA                          {0x6F,0x6D}
#define  EFGID2_CDMA                          {0x6F,0x6E}
#define  EFCDMACNL_CDMA                       {0x6F,0x6F}

#define  EFHOME_TAG_CDMA                      {0x6F,0x70}
#define  EFGROUP_TAG_CDMA                     {0x6F,0x71}
#define  EFSPECIFIC_TAG_CDMA                  {0x6F,0x72}
#define  EFCALL_PROMPT_CDMA                   {0x6F,0x73}
#define  EFSF_EUIMID_CDMA                     {0x6F,0x74}

#define  EF_6F85_CDMA                         {0x6F,0x85}
#define  EF_6F86_CDMA                         {0x6F,0x86}
//C.S0065
#define  EFEST_CDMA                           {0x6F,0x75}
#define  EFHIDDENKEY_CDMA                     {0x6F,0x76}
#define  EFEXT2_CDMA                          {0x6f,0x7a}
#define  EFICI_CDMA                           {0x6F,0x7c}
#define  EFOCI_CDMA                           {0x6F,0x7d}
#define  EFEXT5_CDMA                          {0x6F,0x7e}
#define  EFCCP2_CDMA                          {0x6F,0x7f}

#define  PADF_6F85_CDMA                     {0x6F,0x85}
#define  PADF_6F80_CDMA                     {0x6F,0x80}
#define  PADF_6F81_CDMA                     {0x6F,0x81}
#define  PADF_6F83_CDMA                     {0x6F,0x83}
#define  PADF_6F86_CDMA                     {0x6F,0x86}
#define  PADF_6F89_CDMA                     {0x6F,0x89}
#define  PADF_6F8F_CDMA                     {0x6F,0x8F}
#define  PADF_6F90_CDMA                     {0x6F,0x90}
//TODO ID Under DFTELECOM_DFPHONEBOOK 


/****************************************************************************
****************************************************************************/

#define  FILE_HEADER_MAX_LENGTH          0x100
#define  FILE_RECORD_MAX_LENGTH          0x100
//#define  FILE_BODY_MAX_LENGTH            0x100
#define  FILE_BODY_MAX_LENGTH            0x10000   //64K
#define  VCARD_MAX_LENGTH                0x10000-1              //64K   为简化设计 Vcard 大小不能超过64K-1

#define CARDMODE_UNKOWN                 0
#define CARDMODE_SIM                    1<<0
#define CARDMODE_USIM                   1<<1   
#define CARDMODE_UIM                    1<<2  

#define CARDMODE_USIM_SESSIONDISACTIVE  1<<3
#define CARDMODE_USIM_SESSIONACTIVE     1<<4

#define CHVANDUNBLOCK_CHECK_CHV_NO                       0
#define CHVANDUNBLOCK_CHECK_CHV_RETRYTIMES               1
#define CHVANDUNBLOCK_CHECK_GLOBLE_CHV_COMMAND           2
#define CHVANDUNBLOCK_CHECK_GLOBLE_CHV_ARGU              3
#define CHVANDUNBLOCK_CHECK_SPECIFIC_CHV_COMMAND         4
#define CHVANDUNBLOCK_CHECK_SPECIFIC_CHV_ARGU            5
#define CHVANDUNBLOCK_CHECK_OK                           6
#define CHVANDUNBLOCK_CHECK_GLOBLE_CHV_REQUEST_PIN       7

#define  RIDTYPE_NONEEDAID                               0
#define  RIDTYPE_3GPP                                    1
#define  RIDTYPE_3GPP2                                   2
#define  RIDTYPE_ETSI                                    3

#define USIMMODE_INVALID_3GPP                           (1<<0)
#define USIMMODE_INVALID_3GPP2                          (1<<1)

#define USIMMODE_3GPP_FILE_EXIST                        (1<<4)
#define USIMMODE_3GPP2_FILE_EXIST                       (1<<5)

#define  RIDTYPE_UNRECOGNIZED                            7   //RIDTYPE 最大不超过 7

// 复合卡中最多应用数
#define   MAXRIDITEM        5

#define TS_STATUS_NORMAL                         0
#define TS_STATUS_GEN_GETSTATUS_FOR_RUNAUTH      1
#define TS_STATUS_GEN_GETSTATUS_RESPONSELENGTH_ENBALE      2

#define CURRENTAPDUMODE_UNKOWN                 0
#define CURRENTAPDUMODE_USIM                   1
#define CURRENTAPDUMODE_SIM_UIM                2

// 虚拟卡中文件属性数据结构
typedef struct VCARD_FILE_PROPERTY_S
{
	unsigned char identifier[2];				                       // 文件标识符
//	unsigned short property;				                           // 文件属性
  unsigned long property;				                           // 文件属性
	unsigned char parent_identifier[2];				                 // 父文件标识符
	unsigned char grandfather_identifier[2];				                 // 祖父文件标识符
	unsigned char great_grandfather_identifier[2];				                 // 曾祖父文件标识符
	unsigned char sfi;
	unsigned char application_id_type;   //20180705
}vcard_file_property_t;

typedef struct VCARD_FILE_HIERARCHY_S
{
	unsigned char key_identifier;				                       
  unsigned char key_parent_identifier;				                           
	unsigned char key_grandfather_identifier;				                			                 
	unsigned char key_great_grandfather_identifier;				                 
}vcard_file_hierarchy_t;



// 虚拟卡文件操作列表
typedef struct VCARD_FILE_OPERATION_LIST_S
{
	unsigned char identifier[2];				                      // 文件标识符
	unsigned short operation_property;				                // 文件操作属性
	alltype_vcard_identifier_t  identifier_to_vcard;
}vcard_file_operation_list_t;




// 虚拟卡文件操作列表
typedef struct USIM_VCARD_FILE_OPERATION_LIST_S
{
	unsigned char length;	
	unsigned char identifier[6];				                      // 文件标识符
	unsigned char ridtype;                               //20180706 
	unsigned char nextridtype;                               //20180706 
	unsigned short operation_property;				                // 文件操作属性
	unsigned char select_command[5];				                  // 文件选择命令
	
}usim_vcard_file_operation_list_t;

// 虚拟卡命令操作列表
typedef struct VCARD_COMMAND_OPERATION_LIST_S     //20181018
{
	unsigned char tag;	
	unsigned char ApduOperation;				                     
	unsigned char ApduCommand[5];                              
	unsigned char ApduArgumentLength;                             
	unsigned char ApduArgument[APDU_BUF_MAX_LENGTH];				                  
	
}vcard_comand_operation_list_t;


//AID 结构
#define MAX_AID_LENGTH                    0x10

typedef struct VCARD_AID_LIST_S
{
	unsigned char length;	
	unsigned char aid[MAX_AID_LENGTH];				                          // 文件标识符	
  unsigned char ridtype;  
}vcard_aid_list_t;

// 虚拟卡生成器数据结构
typedef struct VCARD_GENERATOR_S
{
	unsigned char ucApduInteractiveStatus;				            // Apdu 交互状态
	unsigned char ucVcardGeneratorStatus ;			              // 虚拟卡生成器状态
	unsigned char ucApduSendBuf[APDU_BUF_MAX_LENGTH] ;        // Apdu Send Buf
	unsigned short usApduSendLength;                          //Apdu Send Buf实际数据长度
	unsigned char ucEarlyApduSendBuf[5] ;        // 只用于 保存 read 
	unsigned short usEarlyApduSendLength;                          //Apdu Send Buf实际数据长度
	unsigned char ucApduReceiveBuf[APDU_BUF_MAX_LENGTH] ;			// Apdu Rev Buf
	unsigned short usApduReceiveLength;                       //Apdu Rev Buf实际数据长度
	unsigned char ucVcardBuf[VCARD_MAX_LENGTH] ;			        // VcardBuf
	unsigned long  ucWaitGetInteractiveStartTime;            
	vcard_file_operation_list_t  file_operation_list[FILEOPERATIONMAX]; 
	vcard_file_property_t  CurrentOperationFileProperty;
	usim_vcard_file_operation_list_t  Usim_file_operation_list[FILEOPERATIONMAX];
	unsigned char  ucCurrentHandleCardMode;                    //当前处理的卡模式
	unsigned char  ucHandleMode;                               //可以处理的卡模式
	unsigned char  ucCurrentApduOperation;                    //当前APDU命令/数据
	unsigned char  EarlyApduOperation;
	unsigned char  ucCurrentFileSerialNumber;   
	unsigned char  ucCurrentUsimFileSerialNumber;   
	unsigned char  ucCurrentCommandOrData;
	unsigned char  ucCurrentFileHeaderLength; 
	unsigned char  ucCurrentFileHeader[FILE_HEADER_MAX_LENGTH];
	unsigned char  ucCurrentFileHeaderReponse[2];
	unsigned short usCurrentFileBodyLength; 
	unsigned short usCurrentFileBodyEndOffset2ReadBinary; 
	unsigned short usCurrentFileBodyStartOffset2ReadBinary; 
//	unsigned char  usCurrentFileBodyLengthStatus;
	unsigned char  ucCurrentFileBody[FILE_BODY_MAX_LENGTH]; 
	unsigned char  ucCurrentFileBodyReponse[2]; 
	unsigned char  ucCurrentRecordLength;
	unsigned char  ucCurrentRecord[FILE_RECORD_MAX_LENGTH];
	unsigned char  ucCurrentRecordSerialNumber;
	unsigned char  ucVcardSource;
	unsigned char  ucStatusCode;
	unsigned char  ucSimcardTransmissionProtocol;
	
	unsigned long  ulOffset_VcardCurrentFileLength;
	unsigned long  ulOffset_VcardCurrentFileHeaderLength;
	unsigned long  ulOffset_VcardCurrentFileBodyLength;
	unsigned long  ulOffset_VcardCurrentRecordSerialNumber;
  unsigned long  ulOffset_VcardCurrentUsimGroupTag;
  unsigned long  ulOffset_VcardCurrentSimGroupTag;
  
  unsigned char ucCurrentChvAndUnblockOperation;
  unsigned char ucCurrentPinLength; 
 // unsigned char ucCurrentPinBuf[APDU_BUF_MAX_LENGTH];
  unsigned char ucCurrentPinBuf[8];
  unsigned char  ucUsimSessionActiveStatus;
  vcard_aid_list_t     aid_list[APPICAITON_NUMBER_MAX];
  unsigned char        ucCurrentProcessAidNumber;
  unsigned char        ucCurrentAidNumber;
  unsigned char        ucCurrentRidType;
  unsigned char        ucExpectRidType;
  unsigned char        ucCurrentEFType; 
  unsigned char        ucCurrentLCEFReordDefault;
  
 // vcard_aid_property_t  AvailableAid_array[MAXRIDITEM];
 //20200330
  unsigned char      ucCurrentUsimMode_Invalid;  
}vcard_generator_t;


/****************************************************************************
APDU 数据交互状态
****************************************************************************/

#define  AIAS_SENDDATAISREADY       0
#define  AIAS_IDLE                  1
#define  AIAS_SEND                  2
#define  AIAS_SENDSUCCESS           3
#define  AIAS_WAITGETDATA           4
#define  AIAS_GETDATAISREADY        5
#define  AIAS_GET                   6
#define  AIAS_GETSUCCESS            7
#define  AIAS_STOP                  8
#define  AIAS_RESET_CARD_COMPLETED  9
#define  AIAS_WAIT_PIN_INPUT        10

#define  OPERATION_PROPERTY_NO_OPERATION           0
#define  OPERATION_PROPERTY_GENERATION_VCARD       1
#define  OPERATION_PROPERTY_CHANGE_DIR             2


/****************************************************************************
APDU 命令偏置位
****************************************************************************/
#define  APDU_CMD_OFFSET_CLA                 0
#define  APDU_CMD_OFFSET_INS                 1
#define  APDU_CMD_OFFSET_P1                  2
#define  APDU_CMD_OFFSET_P2                  3
#define  APDU_CMD_OFFSET_P3                  4


/****************************************************************************
虚拟卡生成器状态
****************************************************************************/
#define  VCARD_GENERATOR_STATUS_PROCESSING           0
#define  VCARD_GENERATOR_STATUS_SUCCESS              1
#define  VCARD_GENERATOR_STATUS_ERROR                2
#define  VCARD_GENERATOR_STATUS_UNSUITABLE           3
#define  VCARD_GENERATOR_STATUS_FROM_DISK_PROCESSING 4
#define  VCARD_GENERATOR_STATUS_WAITING_FAST_RESET_FOR_SWITCH2SIM   5
#define  VCARD_GENERATOR_STATUS_WAITING_FAST_RESET_FOR_CARD_INITIAL_STATUS   6
#define  VCARD_GENERATOR_STATUS_PROCESSING_REQUEST_PIN   7

/****************************************************************************
虚拟卡数据来源
****************************************************************************/
#define  VCARD_SOURCE_SIMCARD           0
#define  VCARD_SOURCE_DISK              1

/****************************************************************************
Sim卡传输层协议类型
****************************************************************************/
#define  SIMCARD_TRANSMISSION_PROTOCOL_T0           0
#define  SIMCARD_TRANSMISSION_PROTOCOL_T1           1

/****************************************************************************
当前文件操作状态
****************************************************************************/
#define  APDU_SELECT_COMMAND                        0
#define  APDU_SELECT_DATA                           1
#define  APDU_GET_RESPONSE                          2
#define  APDU_READ                                  3
#define  APDU_READ_RECORD                           4
#define  APDU_IDLE                                  5
#define  APDU_VERIFY_CHV                            6
#define  APDU_UNBLOCK_GLOBLE_USIM                   7

/***********************
状态位
/***********************/
#define SW1_90     0x90
#define SW1_91     0x91
#define SW1_9F  	 0x9F
#define SW1_94  	 0x94
#define SW1_98  	 0x98
#define SW2_00 		 0x0
#define SW2_04 		 0x04
#define SW2_08 		 0x08

#define SW1_61 		 0x61 
#define SW1_6A 		 0x6A 
#define SW2_82 		 0x82
#define SW1_63 		 0x63
#define SW1_69 		 0x69
#define SW1_6C 		 0x6C  //instruct the terminal to immediately resend the previous command header setting P3 = 'xx'.

//TS 102.221
/***********************
  Tag in FCP
/***********************/
#define TAG_FCP_TEMPLATE                              0x62
#define TAG_FILE_DESCRIPTOR_WITH_TEMPLATE             0x82
#define TAG_FILE_IDENTIFIER_WITH_TEMPLATE  	          0x83
#define TAG_LIFE_CYCLE_STATUS_INTEGER_WITH_TEMPLATE   0x8A
#define TAG_SECURITY_ATTRIBUTES_WITH_TEMPLATE 		    0x8B
#define TAG_EF_FILE_SIZE_WITH_TEMPLATE_WITH_TEMPLATE  0x80
#define TAG_RESONSE_DF_NAME_WITH_FCP_TEMPLATE         0x84
#define TAG_SFI_WITH_FCP_TEMPLATE                     0x88

/***********************
  Tag in APPLICATION TEMPLATE
/***********************/
#define TAG_ACP_TEMPLATE                              0x61
#define TAG_AID                                       0x4F

/***********************
  Tag in VCARD
/***********************/
#define TAG_VCARDCURRENTUSIMGROUP                    0xC1
#define TAG_VCARDCURRENTSIMGROUP                     0xC2
#define TAG_VCARDCURRENTUIMGROUP                     0xC3
#define TAG_PIN                                      0xB1
#define TAG_UNBLOCK_GLOBLE_USIM                      0xB2
#define TAG_UNBLOCK_ADF_USIM                         0xB3
#define TAG_VERIFY_GLOBLE_USIM                       0xB4
#define TAG_VERIFY_ADF_USIM                          0xB5

#define SW1_61 		 0x61 
#define SW1_6A 		 0x6A 
#define SW2_82 		 0x82

#define USIMSESSIONACTIVESTATUS_NOACTIVE      0
#define USIMSESSIONACTIVESTATUS_ACTIVE        1
#define USIMSESSIONACTIVESTATUS_NEEDTOREADAID        2
#define USIMSESSIONACTIVESTATUS_SELECT_WITH_3GPP_AID       3
#define USIMSESSIONACTIVESTATUS_GET_EFDIR_HEADER           4
#define USIMSESSIONACTIVESTATUS_CAN_OPEN_AID               5
#define USIMSESSIONACTIVESTATUS_EXPECT_OPEN_AID               6


/*********************************
VCARD 数据结构相关的定义
**********************************/
#define VG_VERSION_MAINTAIN                          1   //VcardGenerator Version 

#define VG_HANDLEICC_UIMMODE_MAINTAIN               1<<6   //VcardGenerator 按UIM MODE 操作ICC  (CDMA)
#define VG_HANDLEICC_USIMMODE_MAINTAIN              1<<5   //VcardGenerator 按USIM MODE 操作ICC
#define VG_HANDLEICC_SIMMODE_MAINTAIN               1<<4   //VcardGenerator 按SIM MODE 操作ICC

#define VC_DATA_UIMMODE_MAINTAIN                    1<<2  //Vcard   包含UIM MODE 数据   (CDMA)
#define VC_DATA_USIMMODE_MAINTAIN                    1<<1  //Vcard   包含USIM MODE 数据  
#define VC_DATA_SIMMODE_MAINTAIN                     1<<0  //Vcard   包含SIM MODE 数据  (GSM)


/***********************
Vcard 固定 偏置 长度 
/***********************/
#define OFFSET_VCARD_LENGTH                          0
#define DEEPTH_VCARD_LENGTH                          2
#define OFFSET_VCARD_MAINTAIN_INFORMATION            OFFSET_VCARD_LENGTH+DEEPTH_VCARD_LENGTH
#define DEEPTH_VCARD_MAINTAIN_INFORMATION            2   
#define OFFSET_VCARD_MAINTAIN_INFORMATION_VERSION    OFFSET_VCARD_MAINTAIN_INFORMATION+1 
#define OFFSET_VCARD_BODY                            OFFSET_VCARD_MAINTAIN_INFORMATION+DEEPTH_VCARD_MAINTAIN_INFORMATION


#define OFFSET_VCARDCURRENTUSIMGROUPTAG_LENGTH      ulOffset_VcardCurrentUsimGroupTag+1
#define OFFSET_VCARDCURRENTSIMGROUPTAG_LENGTH       ulOffset_VcardCurrentSimGroupTag+1
/*
#define OFFSET_VCARD_CURRENT_FILE_IDENTIFIER         ulOffset_VcardCurrentFileLength+2
#define OFFSET_VCARD_CURRENT_FILE_ACCESS_STATUS      ulOffset_VcardCurrentFileLength+4
#define OFFSET_VCARD_CURRENT_FILE_HEADER_LENGTH      ulOffset_VcardCurrentFileLength+5
#define OFFSET_VCARD_CURRENT_FILE_HEADER_RESPONSE    ulOffset_VcardCurrentFileLength+6
#define OFFSET_VCARD_CURRENT_FILE_HEADER             ulOffset_VcardCurrentFileLength+8
*/
// #define OFFSET_VCARD_CURRENT_FILE_HEADER_LENGTH      ulOffset_VcardCurrentFileLength+5   //临时使用

#define OFFSET_VCARD_CURRENT_FILE_ACCESS_STATUS      ulOffset_VcardCurrentFileLength+2
#define OFFSET_VCARD_CURRENT_FILE_FID_LENGTH         ulOffset_VcardCurrentFileLength+3
#define OFFSET_VCARD_CURRENT_FILE_IDENTIFIER         ulOffset_VcardCurrentFileLength+4
#define OFFSET_VCARD_CURRENT_FILE_HEADER_RESPONSE    ulOffset_VcardCurrentFileHeaderLength+1
#define OFFSET_VCARD_CURRENT_FILE_HEADER             ulOffset_VcardCurrentFileHeaderLength+3

#define OFFSET_VCARD_CURRENT_FILE_BODY_RESPONSE      ulOffset_VcardCurrentFileBodyLength+2
#define OFFSET_VCARD_CURRENT_FILE_BODY               ulOffset_VcardCurrentFileBodyLength+4

#define OFFSET_VCARD_CURRENT_RECORD_LENGTH           ulOffset_VcardCurrentRecordSerialNumber+1
#define OFFSET_VCARD_CURRENT_RECORD                  ulOffset_VcardCurrentRecordSerialNumber+2



#define VCARD_FILE_ACCESS_ALLOW       0
#define VCARD_FILE_ACCESS_PROHIBIT    1

#define ERROR_IDENTIFIER_MISMATCH        1  
#define STATUS_NEXTFILENOTHROUGH         2
#define ERROR_NOPATHTONEXTFILE           3
#define ERROR_APDUTIMEOUT                4
#define ERROR_UNRECOGNIZEDAPDUOPERATION  5
#define ERROR_VCARD_OVERFLOW             6
#define ERROR_ERROR_CHV_PIN             7

#define ERROR_SIM_FILE_OPERATION_LIST_LENGTH_OVERFLOW 7
#define ERROR_SIM_FILE_OPERATION_LIST_MF_EXPLICIT         8        

class VcardGenerator
        {           
           private:
           	   vcard_generator_t vgs;           	   
           protected:	   
           	   int GetVcardFileProperty(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
           	   int GetVcardFileProperty_Uim(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
           	   int GetVcardFileProperty_Sim(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
           	   int GetVcardFilePropertyWithPath_Uim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property);
           	   int GetVcardFilePropertyWithPath_Sim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property);
           	   int GetVcardFileProperty_Usim(unsigned char length,unsigned char *identifier  ,\
	                                            vcard_file_property_t *vcard_file_property) ;
	             int GetVcardFileProperty_WithUsimRidType(unsigned char length,unsigned char *identifier  , unsigned char ridtype, \
	                                            vcard_file_property_t *vcard_file_property);
           	   int CheckNextFileThrough(unsigned char identifier[2],unsigned char next_identifier[2]);
           	   int CheckNextFileThrough_Sim(unsigned char identifier[2],unsigned char next_identifier[2]);
           	   int CheckNextFileThrough_Uim(unsigned char identifier[2],unsigned char next_identifier[2]);
           	   int GetFileParent(unsigned char identifier[2],unsigned char (*parent_identifier)[2]);
           	   int GetFileParent_Sim(unsigned char identifier[2],unsigned char (*parent_identifier)[2]);
           	   int GetFileParent_Uim(unsigned char identifier[2],unsigned char (*parent_identifier)[2]);
           	   int GetTagLocation_inFCP(unsigned char tag,unsigned char *Buf, int length);
           	   int GetTagLocation_inACP(unsigned char tag,unsigned char *Buf, int length);
           	   unsigned short FreshLengthAndReturnValue(unsigned char *SourceArray,unsigned short addlength,unsigned short *returnlength );
           	   int GenOperationFileList(unsigned char generatevcardmode);
           	   int GenOperationFileList_Sim();
           	   int GenOperationFileList_USim();
           	   int GenOperationFileList_Uim();
           	   int ProcessVcardGenerationT1();
           	   int ProcessVcardGenerationT0FromDisk();
           	   int VerifyResponse();
           	   int VerifyResponse_Sim();
           	   int VerifyResponse_USim();
           	   int VerifyResponse_Uim();
           	   int ProcessResponse();
           	   int ProcessResponse_Sim();
           	   int ProcessResponse_Uim();
           	   int SubProcessResponse_UimOrSim(unsigned char cardmode);
           	   int ProcessResponse_USim();
           	   int ProcessVcardGenerationT0FromSimcard();
           	   int ExistSameIdentifier(vcard_file_property_t *file_property_list,int file_property_list_length,unsigned char identifier[2]);
           	   int ValidVCardIdentifierList(vcard_file_property_t *file_property_list,int file_property_list_length,\
	                 alltype_vcard_identifier_t *vcardfile_identifier_list, int vcardfile_identifier_list_length);
	             int  IsUnsucessfulVerifyOrAuth_Fail_SimAndUim(unsigned short reopnselength, unsigned char* response );
	             int  IsUnsucessfulVerifyOrAuth_Fail_Usim(unsigned short reopnselength, unsigned char* response );
           	   unsigned long GetTickCount();
           	   int AddPin2Vcard();
           	   int GetRidTypeFromAid(unsigned char * aid_buf ,unsigned char aid_length);
               int char_arrayncmp(unsigned char * array1 ,unsigned char * array2 ,unsigned char length); 
            public:
              // int VcardGeneratorInit(); 
              //  int VcardGeneratorInit(unsigned char vcardsource,unsigned char transmissionprotocol);
                int VcardGeneratorInit(unsigned char vcardsource,unsigned char transmissionprotocol, unsigned char generatevcardmode);
                int ProcessVcardGeneration();
                void PrintFileOperationList();
                void PrintUsimFileOperationList();
                unsigned char GetVcardGeneratorStatus();
                unsigned char GetApduInteractiveStatus(); 
                void SetApduInteractiveStatus(unsigned char apduinteractivestatus );
                void SetVcardGeneratorStatus(unsigned char vcardgeneratorstatus);
                int GetSendDataToSim(unsigned char *sendbuf,unsigned short *sendlength);   
                int SetReceiveDataToVgs(unsigned char *revbuf,unsigned short revlength); 
                int GetVcardData(unsigned char *vcardbuf,unsigned long *vcardlength) ; 
                unsigned long GetVcardLengh( );         
				int SetVcardBuffToVGS(unsigned char *buff_vgsm, unsigned short len_vgsm);
         };


extern vcard_file_hierarchy_t vcard_file_hierarchy[6];
extern vcard_file_property_t vcard_file_property_list[];
extern vcard_file_property_t vcard_file_property_list_Usim[];   
extern vcard_file_property_t vcard_file_property_list_Uim[];
extern unsigned char vcardfile_identifier[][2];
extern unsigned char vcard_file_property_list_num;
//extern unsigned char vcard_file_property_list_Usim_num;  
extern unsigned short vcard_file_property_list_Usim_num;  
extern unsigned char vcard_file_property_list_Uim_num;
extern unsigned char command_select[5];
extern unsigned char command_get_reponse[5];
extern unsigned char command_read[5];
extern unsigned char command_read_record[5];

extern unsigned char usimselect_pathfromMfReturnFcp[];
extern unsigned char usimselect_pathByFidReturnFcp[];
extern unsigned char usimcommand_get_reponse[];
extern unsigned char usimcommand_read[];
extern unsigned char usimcommand_read_record[];  //P2=4 Absolute/current mode,
extern unsigned char usimselect_ByDFName_NoDataReturn[];
extern unsigned char usimselect_ByFIReturnFcpForSelectMF[];

extern unsigned char aid_rid_3gpp[5];
extern unsigned char aid_rid_etsi[5];
extern unsigned char aid_rid_3gpp2[5];	//2018.7.3

#define  TRANSLATOR_APDUCMD_DEEPTH            8
#define  TRANSLATOR_APDUARGUMENT_MAXLENGTH  255

#ifndef  NETAPIBUFMAXLENGTH
#define  NETAPIBUFMAXLENGTH                 0x200
#endif

typedef struct TRANSLATOR_DATA_S
{
	unsigned char ucTranslatorApduCommand[TRANSLATOR_APDUCMD_DEEPTH][5];				  
	unsigned char ucTranslatorApduArgument[TRANSLATOR_APDUCMD_DEEPTH][TRANSLATOR_APDUARGUMENT_MAXLENGTH] ;	
	unsigned char ucTranslatorApduArgumentLength[TRANSLATOR_APDUCMD_DEEPTH]	;
	unsigned short usCurrentTranslatorApduCounter;
	unsigned short usLastTranslatorApduCounter;
	unsigned short usCurrentFileOperationListCounter;
	unsigned char ucExpectDeliverToSim;
	vcard_file_operation_list_t FileOperationList[SIMFILEPATHDEEPTH];
	             	
}translator_data_t;

typedef struct  TRANSLATOR_APDU_LOGIC_CHANNEL_S
{	
	unsigned char ucCurrentLch_Identifier[CARD_IDENTIFIERDEEPTH];
	unsigned char ucCurrentLch_IdentifierLength;
	unsigned char ucExpectLch_Identifier[CARD_IDENTIFIERDEEPTH];
  unsigned char ucExpectLch_IdentifierLength;	
  unsigned char ucCurrentLch_RidType; //由 select AID 指令决定
  unsigned char ucCurrentLch_Type;   //由每次 select 指令决定
  id_with_path_t CurrentLchIdWithPath;
}translator_apdu_logic_channel_t;

typedef struct NETAPITOSIMCARD_TRANSLATOR_S
{
	unsigned char ucTranslatorInteractiveStatus;				  
	unsigned char ucTranslatorStatus ;	
	unsigned char ucTranslatorSimcardTransmissionProtocol;
	unsigned char ucCurrentCardIdentifier[CARD_IDENTIFIERDEEPTH];
	unsigned char ucCurrentCardIdentifierLength;
	unsigned char ucExpectIdentifier[CARD_IDENTIFIERDEEPTH];
	unsigned char ucExpectIdentifierLength;
	id_with_path_t CurrentIdWithPath;
	unsigned char  ucCurrentOffsetWithoutPathCommand;
	
	unsigned char ucDataBufFromNet[NETAPIBUFMAXLENGTH] ; 
	unsigned char ucDataPropertyFromNet;       
	unsigned short usDataBufLengthFromNet; 
	
	unsigned char ucDataBufToSimcard[APDU_BUF_MAX_LENGTH] ;  
	unsigned char ucDataPropertyToSimcard;     
	unsigned short usDataBufLengthToSimcard; 
	
	unsigned char ucDataBufToNet[NETAPIBUFMAXLENGTH] ; 
	unsigned char ucDataPropertyToNet;       
	unsigned short usDataBufLengthToNet; 
	
	unsigned char ucDataBufFromSimcard[APDU_BUF_MAX_LENGTH] ;  
	unsigned char ucDataPropertyFromSimcard;     
	unsigned short usDataBufLengthFromSimcard; 
	unsigned char ulApduCommandCounter;
	unsigned long  ucWaitGetInteractiveStartTime;
	
	unsigned char ucCurrentCardStatus;
	unsigned char ucRealCardType;
	translator_data_t TranslatorInnerData;	
	unsigned char ucCurrentReadMode;
	unsigned char ucCurrentUpdateMode;
	unsigned char ucCurrentSFI;	  
  translator_apdu_logic_channel_t Lcm[MAX_LOGICAL_CHANNELS_NUMBER];  
  unsigned char ucCurrentProcessLch ;     
  unsigned char ucCurrentSimCardRidType;	
  unsigned char ucCurrentRidType;
  unsigned char ucOptimizationMode;
  unsigned char ucTsStatus;
  unsigned char ResLength;
  unsigned char ucCurrentApduMode;
  unsigned char ucCurrentInnerDataProcessStatus;
  unsigned char ucSelectFileStatus;
}netapitosimcard_translator_t;



#define TRANSLATOR_STOP                                    0
#define TRANSLATOR_WORKING                                 1
#define TRANSLATOR_WAITING_EMU_DELIVER_RST_ICC             2


#define TRANSLATOR_INTERACTIVE_STATUS_IDLE                             0
#define TRANSLATOR_INTERACTIVE_STATUS_DATA_FROM_NET_IS_READY           1
#define TRANSLATOR_INTERACTIVE_STATUS_DATA_TO_NET_IS_READY             2
#define TRANSLATOR_INTERACTIVE_STATUS_DATA_TO_SIMCARD_IS_READY         3
#define TRANSLATOR_INTERACTIVE_STATUS_DATA_FROM_SIMCARD_IS_READY       4
#define TRANSLATOR_INTERACTIVE_STATUS_ICC_RST_IS_OK                    5
#define TRANSLATOR_INTERACTIVE_STATUS_ERROR                            6


#define EXPECT_DELIVER_TO_SIM_IDLE                             0
#define EXPECT_DELIVER_TO_SIM_APDU_COMMAND                     1
#define EXPECT_DELIVER_TO_SIM_APDU_AERUMENT                    2

#define GENERATE_VCARD_MODE_UNKOWN                             0
#define GENERATE_VCARD_MODE_SIM                                1<<0
#define GENERATE_VCARD_MODE_USIM                               1<<1
#define GENERATE_VCARD_MODE_UIM                                1<<2

class NetApiToSimTranslator
        {           
           private:
           	   netapitosimcard_translator_t nat;          	   
           protected:	   
           	   int TranslatorInitInnerData();
           	   int FuncGetVcardFileProperty(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
           	   int FuncGetVcardFileProperty_Uim(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
           	   int FuncGetVcardFileProperty_Usim(unsigned char length,unsigned char *identifier,vcard_file_property_t *vcard_file_property);
	             int FunGetVcardFilePropertyWithPath_Uim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property) ; 
	             int FunGetVcardFilePropertyWithPath_Sim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property) ;                                                             
           	   int FuncGetFileParent(unsigned char identifier[2],unsigned char (*parent_identifier)[2]);
           	   int FuncGetFileParent_Uim(unsigned char identifier[2],unsigned char (*parent_identifier)[2]);
           	   int  FuncCheckNextFileThrough(unsigned char identifier[2],unsigned char next_identifier[2]);
           	   int  FuncCheckNextFileThrough_Uim_File_Property(vcard_file_property_t file1,vcard_file_property_t file2);
           	   int  FuncCheckNextFileThrough_Sim_File_Property(vcard_file_property_t file1,vcard_file_property_t file2);
           	   int  FuncCheckNextFileThrough_Uim(unsigned char identifier[2],unsigned char next_identifier[2]);
           	   int DirectTransfer_NetToSim();
               int DirectTransfer_SimToNet(); 
               int TranslatorToSim(unsigned char mode);
               int TranslateNetApiToApdu();
               int TranslateNetApiToApdu_Uim();
               int TranslateNetApiToApdu_Usim();
               int NetApiToApduGenFileOperationList();
               int NetApiToApduGenFileOperationList_Uim();
               int NetApiToApduGenFileOperationList_Usim();
               int ProcessSimResponse();
               int ProcessSimResponse_Usim();
               int ProcessSimResponse_Usim_Lch0();
               int ProcessSimResponse_Usim_LchNot0();
               void UpdatePath(unsigned char identifier[2] );
               void UpdatePathLch(unsigned char identifier[2] ,unsigned char Lch);
               int  NetApiGetNetSideIdPathAndSaveOffsetWithoutPath(unsigned char (*pNetSideIdPath)[2],unsigned char *pPathlength);
               int  Tranfer_AllTypePath2FileProperty(alltype_vcard_identifier_t NetSide_Alltype_Identifier,vcard_file_property_t *pfile);
               int GenExpectedFidWithSfi_Usim();
               unsigned long GetTickCount();
               int char_arrayncmp(unsigned char * array1 ,unsigned char * array2 ,unsigned char length);
               void SetOptimizationMode(unsigned char optimizationmode ); 
            public:            
              int TranslatorInit( );             
              int TranslatorInitWithOptimizationMode(unsigned char optimizationmode ) ;        
              int SetTranslatorSimcardTransmissionProtocol(unsigned char transmissionprotocol);
              int SetTranslatorStatus(unsigned char ucTranslatorStatus);            
              int ProcessTranslator( ); 
              unsigned char GetTranslatorStatus();
              unsigned char GetTranslatorInteractiveStatus();
                                                                
              void SetcRealCardTypeToTranslator(unsigned char ucRealCardType);                                                      
              int SetTranslatorInteractiveStatus(unsigned char ucTranslatorInteractiveStatus); 
              unsigned char GetDataPropertyFromNet();
              int CopyDataToDeliverSimBuf(unsigned char* simbuf, unsigned short* lengthptr);
              int CopyDataFromSimToTranslator(unsigned char* simbuf, unsigned short length);  
              int CopyTranslatorDataToDeliverNetBuf(unsigned char* netbuf, unsigned short* lengthptr,unsigned char* DataPropertyToNet); 
              int CopyDataFromNetToTranslator(unsigned char* netbuf, unsigned short length,unsigned char DataPropertyFromNet);
              int SetCurrentCardStatusInTranslator(unsigned char ucCurrentCardStatus);
              void  FuncGetTsStatusAndLchAndResLength(unsigned char *Ptr_TsStatus,unsigned char * PtrCurrentLogicChannel,unsigned char * PtrExpectResLength);
              void  FuncSetTsStatus(unsigned char TsStatus);
              int  PseudoNetDataDeliver2Translater(unsigned char*PtrPseudoDataBuf,unsigned char PseudoDataLength,unsigned char PseudoDataProperity );
			  int SetVcardBuffToVGS(unsigned char *buff_vgsm, unsigned short len_vgsm);
			  int SetCurrentCardIdentifier(unsigned char *identifier);
            //  int GetCurrentCardStatusInTranslator( );                       
              
         };
         
// API 
//API for Identify file 
int GetVcardFileProperty(unsigned char identifier[2],vcard_file_property_t *vcard_file_property);
int CheckCurrentFileInVcard(unsigned char ucvcardbuf[] , unsigned long *currentoffset);
int CheckVcardValidate(unsigned char ucvcardbuf[]);
void InitVcardIndexList(vcard_sort_list_t *VcardIndexList  ,unsigned short ListVolume);

int GenVcardIndex(unsigned char ucvcardbuf[] , unsigned long maxbuflengh,vcard_sort_list_t *VcardIndexList,unsigned short ListVolume );
int GetIdentifierIndex(unsigned char identifier[2],vcard_sort_list_t *VcardIndexList);
int GetFileHeaderContext(int vcardindexlistserialnumber, vcard_sort_list_t *VcardIndexList  ,unsigned char *fileheaderbuf, unsigned char length);
int GetFileBodyLength(int vcardindexlistserialnumber,vcard_sort_list_t *VcardIndexList  );
int GetFileBodyContext(int vcardindexlistserialnumber,vcard_sort_list_t *VcardIndexList  , unsigned char *filebodybuf, unsigned char length);
int IsIdentifiableTransparentFile (unsigned char ucvcardbuf[] , unsigned long maxbuflengh,unsigned char identifier[2],\ 
                                   unsigned char FileHeaderLengthFromSim,unsigned char *PtrFileHeaderFromSim,\
                                   unsigned char FileBodyLengthFromSim,unsigned char *PtrFileBodyFromSim ); 
unsigned char CheckUiccInVcard(unsigned char ucvcardbuf[]);                                   
//API for generate adaptive speed Atr                                    
int ModifyAtr(unsigned char sim_atr_ori[BANK_FRAME_MAX_LENGTH],unsigned int len_ori  , unsigned char *ptr_sim_atr_mod,\
              unsigned int *ptr_len_mod,unsigned char EmuAdaptorSpeedLevel);                                        
#endif
