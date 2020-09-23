//*********************************************
//*
//*		QCN back up and restore 
//*
//*
//********************************************

#include "platform_def.h"
#include "qcn.h"
#include <string.h>
#include "serialif.h"
#include "download.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_ANDROID)
#include "os_linux.h"
#endif 

NVITEM s_nvItem;
FILE *qdn_file;
byte nv_pbuf[250] = {0};
int nv_pbuf_length;
boolean back_up_qcn_name = FALSE;  //when read sn or imei, just read nv but no write to QDN file
extern byte  dloadbuf[];
extern int receive_packet(void);
extern byte g_Receive_Buffer[];
extern int g_Receive_Bytes;
extern byte g_Transmit_Buffer[];	//发送的代码
extern int g_Transmit_Length;		
byte * dload_status = dloadbuf;
unsigned int DiagFrame(byte *pInBuf, unsigned int dwInBufLen, byte *pOutBuf, unsigned int dwOutBufSize)
{
	if (pInBuf == NULL || dwInBufLen == 0 || pOutBuf == NULL || dwOutBufSize == 0)
	{
		return 0;
	}
	int accm = 1;
	unsigned short  calc_fcs = HDLC_FCS_START;
	unsigned out_index = 0;
	byte c_byte = 0;  

	for (unsigned int i = 0; i < dwInBufLen; i++)
	{
		byte c = pInBuf[i];
		calc_fcs = hdlc_fcs_16(calc_fcs,c);
		//if (SHOULD_ESC_BYTE(c,accm))
		if ((c == 0x7E) || (c == 0x7D))
		{
			pOutBuf[out_index++] = HDLC_ESC_ASYNC;
			c ^= HDLC_ESC_COMPL;
		}
		pOutBuf[out_index++] = c;

	}  

	/*-------------------------------------------------------------------------
	Escape the 2 CRC bytes if necessary, process low-order CRC byte first
	-------------------------------------------------------------------------*/
	calc_fcs ^= 0xffff;
	c_byte = calc_fcs & 0x00ff;

	if( SHOULD_ESC_BYTE(c_byte, accm) )
	{
		pOutBuf[out_index++] = HDLC_ESC_ASYNC;
		pOutBuf[out_index++]= (uint8)(c_byte ^ HDLC_ESC_COMPL);
	}
	else /* no escaping needed */
	{
		pOutBuf[out_index++] = (uint8)c_byte;
	}

	/*-------------------------------------------------------------------------
	Process high-order CRC byte
	-------------------------------------------------------------------------*/
	c_byte = (calc_fcs >> 8); 
	if( SHOULD_ESC_BYTE(c_byte, accm) )
	{
		pOutBuf[out_index++] = HDLC_ESC_ASYNC;
		pOutBuf[out_index++] = (uint8)(c_byte ^ HDLC_ESC_COMPL);
	}
	else /* no escaping needed */
	{
		pOutBuf[out_index++] = (uint8)c_byte;
	}

	pOutBuf[out_index++] = HDLC_FLAG;
	return out_index;
}
unsigned int DiagUnframe(byte *pInBuf, unsigned int dwInBufLen, byte *pOutBuf, unsigned int dwOutBufSize)
{
	byte c =pInBuf[0];
	if (pInBuf == NULL || dwInBufLen == 0 || pOutBuf == NULL || dwOutBufSize == 0)
	{
		return 0;
	}
	unsigned out_index = 0;
	for (unsigned int i = 0; i < dwInBufLen; i++)
	{
		c = pInBuf[i];
		/*if (c == 0x7D)
		{
		c = pInBuf[++i] ^ 0x20;
		}*/
		pOutBuf[out_index++] = c;
	}
	return out_index;
};



int save_nv(unsigned char * buf, int nv_pbuf_length)
{
	int result =(int)fwrite(buf,sizeof(unsigned char),nv_pbuf_length,qdn_file);
	return result;	
}

int restore_nv(unsigned char * buf, int nv_pbuf_length)
{
	int result =(int)fread(buf,sizeof(unsigned char),nv_pbuf_length,qdn_file);
	//	fseek(qdn_file,0,SEEK_CUR);
	return result;	
}

void get_meid_number(unsigned char * meid,char * meid_ptr)
{
	byte info[16];
	//byte info1[2];
	int info1;
	int i = 0;
	itoa(0,(meid_ptr+0),16) ;
	itoa(0,(meid_ptr+1),16) ;
	for(i;i<g_Receive_Bytes;i++)
	{
		if(*(meid+i)=='\0')
		{
			break;
		}
		info1=(int)(*(meid+i));
		itoa(info1,(char *)&info[i],16);
		strcat(&meid_ptr[2],(const char *)&info[i]);

	}
#if 0 
	i=0;
	dsatutil_itoa(meid_hi, info, 16);
	int  n = strlen((char *)(info));
	if (n<8)
	{
		for (i=0;i<(8-n);i++)
		{
			buf_hi[i] = '0';
		}
	}
	memcpy(buf_hi+i, info, n);
	memcpy(meid_ptr,buf_hi+i,n);
	i=0;	
	dsatutil_itoa(meid_lo, info, 16);
	n = strlen((char *)(info));
	if (n<8)
	{
		for (i=0;i<(8-n);i++)
		{
			buf_lo[i] = '0';
		}
	}
	memcpy(buf_lo+i, info, n);
	/* Convert the high 32 bit unsigned number to hex */
	memset(dloadbuf,0,sizeof(dloadbuf));
	sprintf(dloadbuf,"%s%s",buf_hi+i,buf_lo+i);
	memcpy(meid_ptr,dloadbuf,i+i);
#endif
}

int dsat_get_imei
(
 unsigned char * ue_imei,                  /* Pointer to return buffer */
 char* rb_ptr
 )
{
	unsigned char imei_bcd_len = 0, n = 0, digit;
	char imei_ascii[(NV_UE_IMEI_SIZE-1)*2];


	/* Convert it to ASCII */
	imei_bcd_len = ue_imei[0];

	if( imei_bcd_len <= (NV_UE_IMEI_SIZE-1) )
	{
		/* This is a valid IMEI */
		memset(imei_ascii, 0, (NV_UE_IMEI_SIZE-1)*2);

		for( n = 1; n <= imei_bcd_len; n++ )
		{
			digit = ue_imei[n] & 0x0F;
			if( ( digit <= 9 ) || ( n <= 1 ) )
			{
				imei_ascii[ (n - 1) * 2 ] = digit + '0';
			}
			else
			{
				imei_ascii[ (n - 1) * 2 ] = '\0';
				break;
			}

			digit = ue_imei[n] >> 4;
			if( ( digit <= 9 ) || ( n <= 1 ) )
			{
				imei_ascii[ ((n - 1) * 2) + 1 ] = digit + '0';
			}
			else
			{
				imei_ascii[ ((n - 1) * 2) + 1 ] = '\0';
				break;
			}
		}

		/* Lose the first byte because it is just the ID */
		memcpy( rb_ptr, imei_ascii + 1, (NV_UE_IMEI_SIZE-1)*2-1 );
		rb_ptr[15]='\0';
		return 1;
	}
	else
	{
		/* This is an invalid IMEI */
		return 0;
	} 
}

#ifdef FEATURE_NEW_QCN_BACKUP
int NV_READ_F(unsigned short  _iNV_Item,unsigned char * _iNV_Data, int array_index,int array_size)
{
	unsigned short nv_status = 0;
	g_Transmit_Buffer[0] = 38;
	g_Transmit_Buffer[1] = _iNV_Item&0xFF;
	g_Transmit_Buffer[2] = (_iNV_Item>>8)&0xFF;
	if(array_size==0)
		memcpy((&g_Transmit_Buffer[3]),_iNV_Data,128);
	else
	{
		g_Transmit_Buffer[3]=array_index;
		memcpy((&g_Transmit_Buffer[4]),_iNV_Data,127);
	}
	g_Transmit_Buffer[131]=0;
	g_Transmit_Buffer[132]=0;
	g_Transmit_Length= 133;  
	memset(nv_pbuf,0,250);
	nv_pbuf_length =DiagFrame(g_Transmit_Buffer,g_Transmit_Length,nv_pbuf,128);
	WriteABuffer(hCom,(unsigned char*)nv_pbuf,nv_pbuf_length);
	do{
		if(receive_packet() == 1){
			DiagUnframe(g_Receive_Buffer, g_Receive_Bytes, nv_pbuf, 123);
			nv_pbuf_length=131;
			if(back_up_qcn_name == TRUE)
			{
				back_up_qcn_name = FALSE;
				return NV_DONE_S;
			}
			switch(nv_pbuf[0])
			{ 
			case 38: 
				nv_status = (unsigned short)(nv_pbuf[132])<<8|nv_pbuf[131];
				if(nv_status==NV_DONE_S)
					save_nv(nv_pbuf,nv_pbuf_length);
				return NV_DONE_S;
			case 20:   //this nv is not allowed to be read
				//printf("NV %d do not exist\r\n",_iNV_Item);
				return NV_BADPARM_S;
			case 66:  //this nv is a sp item and it's locked 
				//printf("NV %d do not exist\r\n",_iNV_Item);
				return NV_BADPARM_S;
			default:
				QdlContext->text_cb("backup QCN:NV %d backup failed \r\n",_iNV_Item);
				return NV_FAIL_S;
			}
		}
	}while(1);		
}

int  SaveQdnThreadFunc(qdl_context *pQdlContext)
{
	if(qdn_file!=NULL)
		qdn_file =NULL;

	qdn_file = fopen("backup.qdn","wb+");
	if(qdn_file == NULL)
	{  
		pQdlContext->text_cb("can't open backup.qdn");
		pQdlContext->logfile_cb("can't open backup.qdn");
		pQdlContext->msg_cb(1,"can't create the file \"backup.qdn\"","Error");
		qdl_sleep(2000);
		return 0;
	}
	pQdlContext->prog_cb(0,100, 0);
	// Get the item number
	unsigned short _iStatus = NV_DONE_S;
	int array_size = 0;
	unsigned short _iNV_Item = 0;
	const unsigned short c_iSizeRequest = 128;
	unsigned char _iNV_Data[ c_iSizeRequest ];
	int array_index;
	int nv_item_index = sizeof(nvim_item_info_table)/sizeof(nvim_item_info_type);
	for( _iNV_Item = 0; _iNV_Item < nv_item_index; _iNV_Item++ ){
		memset( _iNV_Data, 0, c_iSizeRequest );
		//判断某个NV是否有效 
		if(nvim_item_info_table[_iNV_Item].is_present){
			array_size = nvim_item_info_table[_iNV_Item].array_size;
			if (array_size == 0)  /* If not an array, 1 means an array of 1 */
			{
				_iStatus = NV_READ_F( _iNV_Item,_iNV_Data,array_index,array_size);
				if(NV_BADPARM_S == _iStatus)
					continue;
			}
			else /* More than one item */ {
				array_index = 0;
				while (array_index <= array_size) {
					_iStatus = NV_READ_F(_iNV_Item,_iNV_Data,array_index,array_size);
					if(NV_BADPARM_S == _iStatus)
						break;
					array_index ++;
				}
			}
		}
		else//无效则执行下一个NV项
		{
			continue;
		}
		if(NV_DONE_S == _iStatus)
		{
//			pQdlContext->prog_cb((_iNV_Item+1),nv_item_index, 0);
			pQdlContext->logfile_cb("save NV finish");
		}
		if(_iNV_Item == 550)
		{
			s_nvItem._iNV_Item = 550;
			memcpy(s_nvItem._iNV_Data,&nv_pbuf[3],128);
		}

	}
	pQdlContext->prog_cb((_iNV_Item+1),nv_item_index, 0);
	pQdlContext->text_cb("\r\nbackup finished!");
	//not need to backup the EFS, because the PC manager will write a PDP when dialing-up.
	//f.Close();
	fclose(qdn_file);
	if(qdn_file!=NULL)
		qdn_file =NULL;
	qdl_sleep(1000);
	return 1;
}


int NV_WRITE_F(unsigned short  _iNV_Item)
{

	g_Transmit_Buffer[0]=39;
	memset(nv_pbuf,0,250);
	g_Transmit_Buffer[131]=0;
	g_Transmit_Buffer[132]=0;
	g_Transmit_Length= 133;
	nv_pbuf_length =DiagFrame(g_Transmit_Buffer,g_Transmit_Length,nv_pbuf,128);
	WriteABuffer(hCom,(unsigned char*)nv_pbuf,nv_pbuf_length);
	do{
		if(receive_packet() == 1){
			nv_pbuf_length = DiagUnframe(g_Receive_Buffer, g_Receive_Bytes, nv_pbuf, 123);
			switch(nv_pbuf[0])
			{
			case 39:
				return NV_DONE_S;
			case 20:   //this nv is not allowed to be read
				//printf("NV %d do not exist\r\n",_iNV_Item);
				return NV_BADPARM_S;
			case 66:  //this nv is a sp item and it's locked 
				//printf("NV %d do not exist\r\n",_iNV_Item);
				return NV_BADPARM_S;
			default:
				QdlContext->text_cb("\r\nrestore QCN:NV %d restore failed \r\n",_iNV_Item);
				return NV_FAIL_S;
			}
		}
	}while(1);
}


int RestoreQdnThreadFunc(qdl_context *pQdlContext)
{
	char rb_ptr[40];
	if(qdn_file!=NULL)
		qdn_file =NULL;

	qdn_file = fopen( "backup.qcn","rb");
	if(qdn_file == NULL)
	{  
		pQdlContext->text_cb("can't open backup.qcn");
		pQdlContext->logfile_cb("can't open backup.qcn");
		pQdlContext->msg_cb(1,"can't open the file \"backup.qcn\"","Error");
		qdl_sleep(1000);
		return 0;
	}
	fseek(qdn_file,0 ,SEEK_END);
	int file_size = ftell(qdn_file);
	fseek(qdn_file,0 ,SEEK_SET);
	// Get the item number
	unsigned short _iStatus = NV_DONE_S;
	unsigned short _iNV_Item = 0;
	int result = 0;
	int file_read = 0;
	int nv_item_index = sizeof(nvim_item_info_table)/sizeof(nvim_item_info_type);

	while(1){
		memset(g_Transmit_Buffer,0,300);
		result = restore_nv(g_Transmit_Buffer,131);
		file_read += result;
		if(result == 0)
		{
			//restore success
			break;
		}
		_iNV_Item = ((unsigned short)(g_Transmit_Buffer[2])<<8)|g_Transmit_Buffer[1];

		_iStatus = NV_WRITE_F( _iNV_Item);
		if(NV_DONE_S == _iStatus)
		{
			//printf("Restore NV:%d%% finish\r",((_iNV_Item+1)*100/nv_item_index));	
			pQdlContext->prog_cb(file_read,file_size, 0);
			pQdlContext->logfile_cb("write NV finish");
		}
		//qdl_sleep(200);
	}
	//not need to backup the EFS, because the PC manager will write a PDP when dialing-up.
	//f.Close();

	fclose(qdn_file);
	if(qdn_file!=NULL)
		qdn_file =NULL;

	char new_pdn_file[260] = {0};
#ifndef FEATURE_QDN_SN_NAME
	if(pQdlContext->pDloadCfg.TargetPlatform ==TARGET_PLATFORM_6085)
	{
		_iNV_Item = 1943; //EVDO use MEID number
		back_up_qcn_name = TRUE;
		_iStatus = NV_READ_F( _iNV_Item,s_nvItem._iNV_Data,0,0);
		get_meid_number(&g_Receive_Buffer[3],rb_ptr);

		//memcpy(rb_ptr,&g_Receive_Buffer[3],128);
		//dsat_get_imei(s_nvItem._iNV_Data,rb_ptr);
		pQdlContext->text_cb("\r\nrestore finished!MEID : %s\r\n",rb_ptr);
		sprintf(new_pdn_file, "%s.qdn", rb_ptr);
		pQdlContext->logfile_cb(new_pdn_file);
	}
	else
	{	_iNV_Item = 550;
	back_up_qcn_name = TRUE;
	_iStatus = NV_READ_F( _iNV_Item,s_nvItem._iNV_Data,0,0);
	memcpy(s_nvItem._iNV_Data,&g_Receive_Buffer[3],128);
	dsat_get_imei(s_nvItem._iNV_Data,rb_ptr);
	pQdlContext->text_cb("\r\nrestore finished!IMEI : %s\r\n",rb_ptr);
	sprintf(new_pdn_file,  "%s.qdn", rb_ptr);
	pQdlContext->logfile_cb(new_pdn_file);
	}
	if(rename("backup.qdn",new_pdn_file)!=0)
	{
		//pQdlContext->text_cb("can't rename the qdn file!");
		qdl_sleep(2000);
	}
	qdl_sleep(1000);
#else
	char sn[16] = { 0 };
	char sn_temp[10] = { 0 };

	/*-------------------------------------------------------
	General commands with no arguments should return the 
	information requested in the form of a string
	---------------------------------------------------------*/
	back_up_qcn_name = TRUE;
	_iStatus = NV_READ_F( 567,s_nvItem._iNV_Data,0,0);
	memcpy(s_nvItem._iNV_Data,&g_Receive_Buffer[3],128);

	sprintf( sn, "%c%c%c%c%c%c%c%c",
		s_nvItem._iNV_Data[0],                     s_nvItem._iNV_Data[1],
		s_nvItem._iNV_Data[2],                     s_nvItem._iNV_Data[3],
		s_nvItem._iNV_Data[4],                     s_nvItem._iNV_Data[5],
		s_nvItem._iNV_Data[6],                     s_nvItem._iNV_Data[7] );
	printf("sn number is %s\r\n",sn);	
	_iStatus = NV_READ_F( 569,s_nvItem._iNV_Data,0,0);
	memcpy(s_nvItem._iNV_Data,&g_Receive_Buffer[3],128);

	sprintf( sn_temp, "%c%c%c%c%c%c%c",            s_nvItem._iNV_Data[0],
		s_nvItem._iNV_Data[1],                         s_nvItem._iNV_Data[2],
		s_nvItem._iNV_Data[3],                         s_nvItem._iNV_Data[4],
		s_nvItem._iNV_Data[5],                         s_nvItem._iNV_Data[6] );
	printf("sn_temp number is %s",sn_temp);	
	strcat( sn, sn_temp );

	sn[15] = '\0';
	sprintf(new_pdn_file, "%sQCN Backup\\%s.qdn", g_thePreConfig.ExePath, sn);
	save_log(new_pdn_file);
	rename(lpQdnParam->sQdnFile,new_pdn_file);
	qdl_sleep(1000);
	back_up_qcn_name = FALSE;
	_chdir(g_thePreConfig.ExePath); 
#endif
	return 1;
}
#endif


