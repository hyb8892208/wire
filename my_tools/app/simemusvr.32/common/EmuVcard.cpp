// TestSimBox.cpp : Defines the entry point for the console application.
//
#include "Misc.h"
#include "card.h"
#include "EmuVcard.h"  

#include <iostream>
#include "stdlib.h" 


#include <string.h>
#include <errno.h>

#include <time.h>
#include <termios.h> 


/*******************************
EmuVcard Concerned
*******************************/
rid_inform_t ridset[] ={
	{{0xa0,0,0,0,0x87},RIDTYPE_3GPP },
	{{0xa0,0,0,03,0x43},RIDTYPE_3GPP2 },
	{{0xa0,0,0,0,0x09},RIDTYPE_ETSI }
};

int Emu_Engine::GetTagLocation_inFCP(unsigned char tag,unsigned char *Buf, int length){
	   int tmplocation;
	   if (!( (*Buf== TAG_FCP_TEMPLATE)&& *(Buf+1)==(unsigned char) length))
	   	  return -1;
	   tmplocation =2;
	//   while (tmplocation<length) {
	     while (tmplocation<length+2) {
	   	if ( *(Buf+tmplocation)!=tag)
	   		 tmplocation+= *(Buf+tmplocation+1)+2;
	   	else
	   		break;
	   	}
     if (tmplocation>=length+2)  
     	 return -1;
	 return tmplocation;   
	}

unsigned short Emu_Engine::FreshLengthAndReturnValue(unsigned char *SourceArray,unsigned short addlength,unsigned short *returnlength ){
	unsigned long tempvalue;
	tempvalue= (((unsigned short) (*SourceArray))<<8)+ (*(SourceArray+1))+ addlength;
	if (tempvalue>0xffff-DEEPTH_VCARD_LENGTH){
		   std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;	
	     eae.ucStatusCode=STATUS_EAE_ERROR_VCARD_OVERFLOW;
	     return -1;        
		}
	 else	{
	 	      *(SourceArray+1)=(unsigned char)tempvalue;
	 	      * SourceArray=(unsigned char)(tempvalue>>8);
	 	       *returnlength=(unsigned short)tempvalue;
	 	
	 	}
		return 1;
}

//比较数组前n个字节是否相同， 相同 返回0  //AID 比较工具， AID中含数字0， 不能使用strncmp比较
int Emu_Engine::char_arrayncmp(unsigned char * array1 ,unsigned char * array2 ,unsigned char length){
unsigned char tmplength;	
	for (tmplength=0;tmplength<length;tmplength++)
	   if (*(array1+tmplength)!=* (array2+tmplength))
	   	 return -1;
	if (tmplength==length)
		return 0;
}

/**************************************************************************** 
* 函数名称 : GetVcardFileProperty
* 功能描述 : 检查文件标识是否属于GSM11.11定义的文件
* 参    数 : identifier ：文件标识，
* 参    数 :  vcard_file_property：文件属性表指针
* 参    数 : 
* 返 回 值 : 属于 返回 1  不属于 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int Emu_Engine::GetVcardFileProperty(unsigned char identifier[2],vcard_file_property_t *vcard_file_property) {
		int iListCounter;		
	  
		for (iListCounter=0;iListCounter<vcard_file_property_list_num;iListCounter++){
		  	if( (vcard_file_property_list[iListCounter].identifier[0]==identifier[0])&&(vcard_file_property_list[iListCounter].identifier[1]==identifier[1])){	  	   
		  	     (*vcard_file_property)=vcard_file_property_list[iListCounter];		  	  
		  	     break;
		  	}
		}
	  if (iListCounter==vcard_file_property_list_num){
			 return -1;
		}
	  return 1;
	} 

	
int Emu_Engine::GetVcardFileProperty_Uim(unsigned char identifier[2],vcard_file_property_t *vcard_file_property) {
		int iListCounter;		
	  
		for (iListCounter=0;iListCounter<vcard_file_property_list_Uim_num;iListCounter++){
		  	if( (vcard_file_property_list_Uim[iListCounter].identifier[0]==identifier[0])&&(vcard_file_property_list_Uim[iListCounter].identifier[1]==identifier[1])){	  	   
		  	     (*vcard_file_property)=vcard_file_property_list_Uim[iListCounter];		  	  
		  	     break;
		  	}
		}
	if (iListCounter==vcard_file_property_list_Uim_num){			
			 return -1;
			}
	  return 1;
	}
	
// 带路径查询避免同名文件冲突
int Emu_Engine::GetVcardFilePropertyWithPath_Uim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property) {
		int iListCounter;		
	  
	  if (vcardfile_identifier.length==1)
	  	 return (GetVcardFileProperty_Uim(vcardfile_identifier.identifier1,vcard_file_property));
	  	 
	  else
	  if (vcardfile_identifier.length==2){
	  	  	for (iListCounter=0;iListCounter<vcard_file_property_list_Uim_num;iListCounter++){
		  	      if( (vcard_file_property_list_Uim[iListCounter].identifier[0]==vcardfile_identifier.identifier2[0])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].identifier[1]==vcardfile_identifier.identifier2[1])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].parent_identifier[0]==vcardfile_identifier.identifier1[0])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].parent_identifier[1]==vcardfile_identifier.identifier1[1])\
		  	      	){	  	   
		  	             (*vcard_file_property)=vcard_file_property_list_Uim[iListCounter];		  	  
		  	               break;
		  	        }
		      }
		      if (iListCounter==vcard_file_property_list_Uim_num){
			      
			      return -1;
			    }  		
	  }
		else
	  if (vcardfile_identifier.length==3){   
	  	  	for (iListCounter=0;iListCounter<vcard_file_property_list_Uim_num;iListCounter++){
		  	      if( (vcard_file_property_list_Uim[iListCounter].identifier[0]==vcardfile_identifier.identifier3[0])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].identifier[1]==vcardfile_identifier.identifier3[1])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].parent_identifier[0]==vcardfile_identifier.identifier2[0])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].parent_identifier[1]==vcardfile_identifier.identifier2[1])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].grandfather_identifier[0]==vcardfile_identifier.identifier1[0])\
		  	      	&&(vcard_file_property_list_Uim[iListCounter].grandfather_identifier[1]==vcardfile_identifier.identifier1[1])\
		  	      	){	  	   
		  	             (*vcard_file_property)=vcard_file_property_list_Uim[iListCounter];		  	  
		  	               break;
		  	        }
		      }
		      if (iListCounter==vcard_file_property_list_Uim_num){
			       
			      return -1;
			    }  		
	  }
	  else
	  	return -1;
	
	  return 1;
	}
	
// 带路径查询避免同名文件冲突
int Emu_Engine::GetVcardFilePropertyWithPath_Sim(alltype_vcard_identifier_t vcardfile_identifier,vcard_file_property_t *vcard_file_property) {
		int iListCounter;		
	  
	  if (vcardfile_identifier.length==1)
	  	 return (GetVcardFileProperty(vcardfile_identifier.identifier1,vcard_file_property));
	  	 
	  else
	  if (vcardfile_identifier.length==2){
	  	  	for (iListCounter=0;iListCounter<vcard_file_property_list_num;iListCounter++){
		  	      if( (vcard_file_property_list[iListCounter].identifier[0]==vcardfile_identifier.identifier2[0])\
		  	      	&&(vcard_file_property_list[iListCounter].identifier[1]==vcardfile_identifier.identifier2[1])\
		  	      	&&(vcard_file_property_list[iListCounter].parent_identifier[0]==vcardfile_identifier.identifier1[0])\
		  	      	&&(vcard_file_property_list[iListCounter].parent_identifier[1]==vcardfile_identifier.identifier1[1])\
		  	      	){	  	   
		  	             (*vcard_file_property)=vcard_file_property_list[iListCounter];		  	  
		  	               break;
		  	        }
		      }
		      if (iListCounter==vcard_file_property_list_num){
			      
			      return -1;
			    }  		
	  }
		else
	  if (vcardfile_identifier.length==3){   
	  	  	for (iListCounter=0;iListCounter<vcard_file_property_list_num;iListCounter++){
		  	      if( (vcard_file_property_list[iListCounter].identifier[0]==vcardfile_identifier.identifier3[0])\
		  	      	&&(vcard_file_property_list[iListCounter].identifier[1]==vcardfile_identifier.identifier3[1])\
		  	      	&&(vcard_file_property_list[iListCounter].parent_identifier[0]==vcardfile_identifier.identifier2[0])\
		  	      	&&(vcard_file_property_list[iListCounter].parent_identifier[1]==vcardfile_identifier.identifier2[1])\
		  	      	&&(vcard_file_property_list[iListCounter].grandfather_identifier[0]==vcardfile_identifier.identifier1[0])\
		  	      	&&(vcard_file_property_list[iListCounter].grandfather_identifier[1]==vcardfile_identifier.identifier1[1])\
		  	      	){	  	   
		  	             (*vcard_file_property)=vcard_file_property_list[iListCounter];		  	  
		  	               break;
		  	        }
		      }
		      if (iListCounter==vcard_file_property_list_num){
			       
			      return -1;
			    }  		
	  }
	  else
	  	return -1;
	
	  return 1;
	}


 int Emu_Engine::GetVcardFileProperty_WithUsimRidType(unsigned char length,unsigned char *identifier  , unsigned char ridtype, \
	                                            vcard_file_property_t *vcard_file_property)
{
		int iListCounter;		
	  if (length<2)
	  	return -1;
		for (iListCounter=0;iListCounter<vcard_file_property_list_Usim_num;iListCounter++){
			  if (length==2)
			  	 if ((vcard_file_property_list_Usim[iListCounter].identifier[0]== *identifier)\
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[1]== *(identifier+1) \
			  	   && (vcard_file_property_list_Usim[iListCounter].application_id_type== ridtype) )
			  	   ) {
			  	    (*vcard_file_property)=vcard_file_property_list_Usim[iListCounter];		 
			  	   break;
			  	 }
			  	
		  	if (length==4)
			  	 if ((vcard_file_property_list_Usim[iListCounter].parent_identifier[0]== *identifier)\
			  	   && (vcard_file_property_list_Usim[iListCounter].parent_identifier[1]== *(identifier+1)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[0]== *(identifier+2)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[1]== *(identifier+3) \
			  	   && (vcard_file_property_list_Usim[iListCounter].application_id_type== ridtype) )
			  	   ){
			  	    (*vcard_file_property)=vcard_file_property_list_Usim[iListCounter];	
			  	   break;
			  	   
			      }
			  if (length==6)
			  	 if ((vcard_file_property_list_Usim[iListCounter].parent_identifier[0]== *(identifier+2))\
			  	   && (vcard_file_property_list_Usim[iListCounter].parent_identifier[1]== *(identifier+3)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[0]== *(identifier+4)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[1]== *(identifier+5) \
			  	   && (vcard_file_property_list_Usim[iListCounter].application_id_type== ridtype) )
			  	   ){
			  	    (*vcard_file_property)=vcard_file_property_list_Usim[iListCounter];	
			  	   break;
			  	   
			      }
		}
	if (iListCounter==vcard_file_property_list_Usim_num){
			
			 return -1;
			}
	  return 1;
	}	
	
/**************************************************************************** 
* 函数名称 : GetVcardFileProperty_Usim
* 类       ：VcardGenerator
* 功能描述 : 检查文件标识是否属于TS31.102定义的文件
* 参    数 : identifier ：文件标识，
* 参    数 :  vcard_file_property：文件属性表指针
* 参    数 : 
* 返 回 值 : 属于 返回 1  不属于 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int Emu_Engine::GetVcardFileProperty_Usim(unsigned char length,unsigned char *identifier  ,\
	                                            vcard_file_property_t *vcard_file_property) {
		int iListCounter;		
	  if (length<2)
	  	return -1;
		for (iListCounter=0;iListCounter<vcard_file_property_list_Usim_num;iListCounter++){
			  if (length==2)
			  	 if ((vcard_file_property_list_Usim[iListCounter].identifier[0]== *identifier)\
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[1]== *(identifier+1))
			  	   ) {
			  	    (*vcard_file_property)=vcard_file_property_list_Usim[iListCounter];		 
			  	   break;
			  	 }
			  	
		  	if (length==4)
			  	 if ((vcard_file_property_list_Usim[iListCounter].parent_identifier[0]== *identifier)\
			  	   && (vcard_file_property_list_Usim[iListCounter].parent_identifier[1]== *(identifier+1)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[0]== *(identifier+2)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[1]== *(identifier+3))
			  	   ){
			  	    (*vcard_file_property)=vcard_file_property_list_Usim[iListCounter];	
			  	   break;
			  	   
			      }
			  if (length==6)
			  	 if ((vcard_file_property_list_Usim[iListCounter].parent_identifier[0]== *(identifier+2))\
			  	   && (vcard_file_property_list_Usim[iListCounter].parent_identifier[1]== *(identifier+3)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[0]== *(identifier+4)) \
			  	   && (vcard_file_property_list_Usim[iListCounter].identifier[1]== *(identifier+5))
			  	   ){
			  	    (*vcard_file_property)=vcard_file_property_list_Usim[iListCounter];	
			  	   break;
			  	   
			      }
		}
	if (iListCounter==vcard_file_property_list_Usim_num){
			
			 return -1;
			}
	  return 1;
	}	
int Emu_Engine::IsValidIdentifier(unsigned char length,unsigned char *identifier  ) {
		int iListCounter;		
		
		if (!((length==2)||(length==4)||(length==6)))
	  	return 0;
		
		if (length==2)
			 if  (!(

     	     				 		   (*identifier==FILE_1ST_ID_MF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (*identifier==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (*identifier==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	     ))
     	    return 0;
    
    if (length==4)
			 if  (
			 	    (!(

     	     				 		   (*identifier==FILE_1ST_ID_MF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (*identifier==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (*identifier==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	     )) || \
     	       (!(

     	     				 		   (*(identifier+2)==FILE_1ST_ID_MF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	        ))
     	     )
     	    return 0;
		
		 if (length==6)
			 if  (
			 	    (!(

     	     				 		   (*identifier==FILE_1ST_ID_MF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (*identifier==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (*identifier==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (*identifier==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	      )) || \
     	      (!(

     	     				 		   (*(identifier+2)==FILE_1ST_ID_MF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (*(identifier+2)==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	      ))||\
     	      (!(

     	     				 		   (*(identifier+4)==FILE_1ST_ID_MF)||\
     	     				 		   (*(identifier+4)==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (*(identifier+4)==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (*(identifier+4)==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (*(identifier+4)==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (*(identifier+4)==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	      ))   
     	     )
     	    return 0;
	  
	  return 1;
	}	

/**************************************************************************** 
* 函数名称 : CheckCurrentFileInVcard
* 功能描述 : 检查Vcard内1个文件段的合法性
* 参    数 : ucvcardbuf ：Vcard Buf 
* 参    数 :  currentoffset 文件偏置指针
* 参    数 : 
* 返 回 值 : 合法 返回 1  合法 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::CheckCurrentFileInVcard(unsigned char ucvcardbuf[] , unsigned long *currentoffset){
	 unsigned long vcardbuflength,offsetcurrentfile_length,offsetcurrentfile_identifier,offsetcurrentfile_access_status,
	offsetcurrentfile_header_length,offsetcurrentfile_header_response,offsetcurrentfile_header,offsetcurrentfile_body_length,
	offsetcurrentfile_body_response,offsetcurrentfile_body,offsetcurrentrecord_reponse,offsetcurrentrecord_length;
	vcard_file_property_t file1;
	unsigned char identifier[2];
	unsigned short currentfile_length,currentfile_header_length,currentfile_body_length,currentrecord_length;
	
	offsetcurrentfile_length=*currentoffset;
	currentfile_length=   ((unsigned short) ucvcardbuf[offsetcurrentfile_length]<<8)+(unsigned short) ucvcardbuf[offsetcurrentfile_length+1];
	if (currentfile_length<5)
		   return -1;	
	offsetcurrentfile_identifier=	offsetcurrentfile_length+2;	
	identifier[0]=ucvcardbuf[offsetcurrentfile_identifier];
	identifier[1]=ucvcardbuf[offsetcurrentfile_identifier+1];	   	       	   	    
  if (GetVcardFileProperty(identifier, &file1)<0) 
	   	 return -1;	
  offsetcurrentfile_access_status=	offsetcurrentfile_length+4;	
	if (!( (ucvcardbuf[offsetcurrentfile_access_status]==VCARD_FILE_ACCESS_ALLOW)||(ucvcardbuf[offsetcurrentfile_access_status]==VCARD_FILE_ACCESS_PROHIBIT)))     
	   	 return -1;	   
	if (currentfile_length==5){
		   *currentoffset+=5;
		   return 1;
		 }
	offsetcurrentfile_header_length=offsetcurrentfile_length+5;
	currentfile_header_length=ucvcardbuf[offsetcurrentfile_header_length];	   
	if ((file1.property&FILE_TYPE_MF)|(file1.property&FILE_TYPE_DF)){
		   if (currentfile_header_length<22)
		        return -1;		
	}	
	else{
		    if (file1.property&FILE_TYPE_EF)
		        if (currentfile_header_length!=15)
		            return -1;		
		}
		
	offsetcurrentfile_header_response	=offsetcurrentfile_length+6;   		   
	if (!(ucvcardbuf[offsetcurrentfile_header_response]==SW1_90)&&(ucvcardbuf[offsetcurrentfile_header_response+1]==SW2_00))
	     	return -1;	
	if ((file1.property&FILE_TYPE_MF)|(file1.property&FILE_TYPE_DF)){  
		      	if (currentfile_length!=currentfile_header_length+8)
		      		   return -1;	
		      	else{
		      		  *currentoffset+=currentfile_length;
		      		  return 1;
		      		}	   
		      		  
  }
  if ((file1.property&FILE_TYPE_EF)&&(file1.property&EF_STRUCTURE_TRANSPARENT)){
  	     offsetcurrentfile_body_length=offsetcurrentfile_length+currentfile_header_length+8;
  	     currentfile_body_length=((unsigned short) ucvcardbuf[offsetcurrentfile_body_length]<<8)+(unsigned short) ucvcardbuf[offsetcurrentfile_body_length+1];
  	     offsetcurrentfile_body_response=offsetcurrentfile_body_length+2;
  	     
  	     if (!(((ucvcardbuf[offsetcurrentfile_body_response]==SW1_90)&&(ucvcardbuf[offsetcurrentfile_body_response+1]==SW2_00))||
  	     	  ((ucvcardbuf[offsetcurrentfile_body_response]==SW1_91)&&(ucvcardbuf[offsetcurrentfile_body_response+1]!=SW2_00))))
  	     	   return -1;
  	     	   
  	     if (currentfile_length!=currentfile_header_length+8+currentfile_body_length+4)
		      		   return -1;	
		      	else{
		      		  *currentoffset+=currentfile_length;
		      		  return 1;
		      		}	  	        	    	     	
  }
  if ((file1.property&FILE_TYPE_EF)&&(file1.property&EF_STRUCTURE_LINEARFIXED||file1.property&EF_STRUCTURE_CYCLIC)){
  	   offsetcurrentrecord_reponse=offsetcurrentfile_header_response+currentfile_header_length+2;
  	   if (!(((ucvcardbuf[offsetcurrentrecord_reponse]==SW1_90)&&(ucvcardbuf[offsetcurrentrecord_reponse+1]==SW2_00))||
  	     	  ((ucvcardbuf[offsetcurrentrecord_reponse]==SW1_91)&&(ucvcardbuf[offsetcurrentrecord_reponse+1]!=SW2_00))))
  	     	   return -1;
  	     	   
  	   if (currentfile_length<currentfile_header_length+10)
		      		   return -1;	
		   if (currentfile_length==currentfile_header_length+10){
		      		  *currentoffset+=currentfile_length;
		      		  return 1;
		   }
		   if (currentfile_length>currentfile_header_length+10){
		   	        offsetcurrentrecord_length=offsetcurrentrecord_reponse+3;
		   	        currentrecord_length=ucvcardbuf[offsetcurrentrecord_length];
		   	        if ((currentfile_length-currentfile_header_length-10)%(currentrecord_length+2))
		   	        	   return -1;	
		   	        else	{   
		      		           *currentoffset+=currentfile_length;
		      		           return 1;
		      		  }       
		   }		 	    	     	   
  	}
  return 1;  	   
}	
/**************************************************************************** 
* 函数名称 : CheckVcardValidate
* 功能描述 : 检查Vcard的合法性
* 参    数 : ucvcardbuf ：Vcard Buf 
* 参    数 : 
* 参    数 : 
* 返 回 值 : 合法 返回 1  不合法 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	

int Emu_Engine::CheckVcardValidate(unsigned char ucvcardbuf[]){
	unsigned long currentoffset,vcardbuflength;
  unsigned short tmpoffset;
   
 //TODO
 return 1;				
}

int Emu_Engine::SeekLocalOk_Usim(unsigned char RecordStartNo,unsigned char* SeekResponse){
int tmprecordbodylength,tmpRecordVolume,tmpi;
unsigned char tmpbuf1[255];
	   
	   if ((eae.ucCurrentSimCardIdentifier[4]==0x4f)&&(eae.ucCurrentSimCardIdentifier[5]==0x31))
	   	  int p=1;
	   if (!eae.ucCurrentProcessLogicalChannel){
	         if (eae.iCurrentVcardIndexListSerialnumber>0xf00)
	         	  return -1;
	         else{
	         	      tmprecordbodylength=GetRecordBodyLength(eae.iCurrentVcardIndexListSerialnumber);
	         	      tmpRecordVolume=GetRecordVolume(eae.iCurrentVcardIndexListSerialnumber);
	         	      if (tmprecordbodylength<eae.ucCurrentArgumentLength)
	         	      	  return -1;
	         	       eae.ucMeetSeekVolume=0;
	         	         
	         	      for (tmpi=RecordStartNo;tmpi<=tmpRecordVolume;tmpi++){
	         	        if (GetFileRecordWithCurrentFileProperty(eae.iCurrentVcardIndexListSerialnumber, tmpi,  tmpbuf1, tmprecordbodylength)<0) 
	         	        	 return -1;
	         	        if (!char_arrayncmp (   eae.ucCurrentArgument,   tmpbuf1, eae.ucCurrentArgumentLength )){
	         	        	  eae.ucMeetSeekRecordNo[eae.ucMeetSeekVolume]=tmpi;
	         	        	  eae.ucMeetSeekVolume++;	         	        	  
	         	        }          	        		                                                                    
	         	      }
	         	}         	           	
	   }
	    else{
  	            if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber>0xf00)  
   	                  return -1; 	  
                else	{  
                	      tmprecordbodylength=GetRecordBodyLength(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber); 
 	                      tmpRecordVolume=GetRecordVolume(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber);
 	   	                  if (tmprecordbodylength<eae.ucCurrentArgumentLength)
	         	      	      return -1;
	         	            eae.ucMeetSeekVolume=0;
	         	            
	         	            for (tmpi=RecordStartNo;tmpi<=tmpRecordVolume;tmpi++){
	         	              if (GetFileRecordWithCurrentFileProperty(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber, tmpi,  tmpbuf1, tmprecordbodylength)<0) 
	         	        	        return -1;
	         	              if (!char_arrayncmp (   eae.ucCurrentArgument,   tmpbuf1, eae.ucCurrentArgumentLength )){
	         	        	          eae.ucMeetSeekRecordNo[eae.ucMeetSeekVolume]=tmpi;
	         	        	          eae.ucMeetSeekVolume++;	         	        	  
	         	              }          	        		                                                                    
	         	            }
	         	    }       	     
     } 
	   if (!eae.ucMeetSeekVolume){
	   	  *SeekResponse= 0x62;
	   	  *(SeekResponse+1)= 0x82;
	   }
	   else	{
	   	  *SeekResponse= 0x61;
	   	  *(SeekResponse+1)= eae.ucMeetSeekVolume;
	   } 
	   	  
 return 1;				
}


/**************************************************************************** 
* 函数名称 : InitVcardIndexList
* 功能描述 : 初始化Vcard 索引表
* 参    数 : VcardIndexList ：索引表指针 
* 参    数 : ListVolume 索引表记录数
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
void Emu_Engine::InitVcardIndexList( vcard_sort_list_t *VcardIndexList ,unsigned short ListVolume){
	 memset(VcardIndexList, 0, sizeof(vcard_sort_list_t)*ListVolume);
} 

int Emu_Engine::IsUnsuitableInputAtEnablePin(unsigned char inputpinlen ){
	  if (eae.ucDefaultChv1Status==CHV1_STATUS_ENABLE)
	  	if (inputpinlen)
	  		return 1;
	  return 0;  		
}
// 外部接口 ;从Emu中获取目前关联卡的PIN值
// 返回： 0：目前不能从EMU得到PIN的信息； 1： 目前可获得Pin信息（pin 只有在从Vcard 中得到后才能暴露给外部）
// ptrPinLength 指向的PinLength=0，表示无PIN，=8 表示有Pin
//ptrPinBuf 指向具体的Pin值的Buf

unsigned char Emu_Engine::GetPinValueFromEmu(unsigned char *ptrPinLength,unsigned char* ptrPinBuf){
	if (eae.ucProcessIndexStatus!=PROCESSINDEXSTATUS_PROCESSED)
		return 0;
	*ptrPinLength=eae.ucPinLength;
	if (!eae.ucPinLength)
		return 1;
	 memcpy(ptrPinBuf,eae.ucPinBuf,8);
	 return 1;
}
	
int Emu_Engine::IsRightPin(unsigned char inputpinlen,unsigned char * inputpin ){
unsigned char tmpcnt;
	if (inputpinlen!=eae.ucPinLength)
		return 0;
	for (tmpcnt=0;tmpcnt<inputpinlen;tmpcnt++){
		 if (*(inputpin+tmpcnt)!=eae.ucPinBuf[tmpcnt])
		 	 return 0;
	}
	return 1;		
}
// 取出Vcard 中的Pin值， 如果length=0,表明无Pin
int Emu_Engine::GetPin(unsigned char ucvcardbuf[]){
  unsigned long vcardbuflength,offsetcurrent=OFFSET_VCARD_BODY;
  unsigned short tmpoffset,tmppinlen;	
   // 取出Vcard 长度
      if (FreshLengthAndReturnValue(ucvcardbuf,0, &tmpoffset)==-1)
        	return -1;
      vcardbuflength=tmpoffset+2;  
      
     while (offsetcurrent<vcardbuflength){
        	 if (TAG_PIN==ucvcardbuf[offsetcurrent])
       	        break;
        	 else {
        	 	       // 取出tlv body 长度
                 if (FreshLengthAndReturnValue(&ucvcardbuf[offsetcurrent+1],0, &tmpoffset)==-1) //取出2字节的TLV_length
                   	return -1;
                 offsetcurrent+=tmpoffset+3; // 3为 Tag +Tlv_length所占字节	     	 	
         	 }	     	 	      		     	  		     	  	
	   }
	   
	   if (offsetcurrent<vcardbuflength){  //找到TAG_PIN
	     	          
	     	         // offsetcurrentfile_length=offsetcurrent+3;
	     	 	       // 取出tlv body 长度
                 if (FreshLengthAndReturnValue(&ucvcardbuf[offsetcurrent+1],0, &tmppinlen)==-1) //取出2字节的TLV_length
                   	return -1;
                 eae.ucPinLength=tmppinlen&0xff;
                 if (eae.ucPinLength==8) { //存在PIN in Vcard
                   if(vcardbuflength>=(offsetcurrent+3+eae.ucPinLength)){
                   	  memcpy(eae.ucPinBuf,&ucvcardbuf[offsetcurrent+3],8);
                   	   std::cout<<std::hex <<" Pin Value:";
    	    	 	         for (int pin_locate=0;pin_locate<eae.ucPinLength;pin_locate++)
    	    	 	            std::cout<<std::hex <<(unsigned short) eae.ucPinBuf[pin_locate]<<" ";
    	    	 	         std::cout<<std::endl;
                   	}
                   else
                 	  eae.ucPinLength=0;	
                 }
                 else
                 	  eae.ucPinLength=0;
                 	                
	   }
     return 1;   
}

/**************************************************************************** 
* 函数名称 : GenVcardIndex
* 功能描述 : 产生Vcard 索引表 
* 参    数 : ucvcardbuf ：Vcard指针 
* 参    数 : maxbuflengh： Vcard Buf 最大长度
* 参    数 : VcardIndexList： ：索引表指针
* 参    数 : ListVolume 索引表记录数
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	

int Emu_Engine::GenVcardIndex(unsigned char ucvcardbuf[] , unsigned long maxbuflengh,vcard_sort_list_t *VcardIndexList ,\
	unsigned char tag_filegroup ,unsigned short ListVolume ){
	unsigned short tmpoffset;	
	unsigned long offsetcurrentfile_length,offsetcurrentgroupend,offsetcurrent=OFFSET_VCARD_BODY;
	unsigned char currentlistnum=0,currentrecordnum=0;
	vcard_file_property_t file1;
	alltype_vcard_identifier_t tmppathidentifier;
	int tmpt;
	
//	unsigned char identifier[2];
  unsigned char identifier[20];
	unsigned long vcardbuflength,offsetcurrentfile_identifier,offsetcurrentfile_access_status,
	offsetcurrentfile_header_length,offsetcurrentfile_header_response,offsetcurrentfile_header,offsetcurrentfile_body_length,
	offsetcurrentfile_body_response,offsetcurrentfile_body,offsetcurrentrecord_reponse,offsetcurrentrecord_length,offsetcurrentrecord;
	unsigned short currentfile_length,currentfile_header_length,currentfile_body_length,currentrecord_length;
		
      // 取出Vcard 长度
      if (FreshLengthAndReturnValue(ucvcardbuf,0, &tmpoffset)==-1)
        	return -1;
      vcardbuflength=tmpoffset+2; 
         	
	     //查找Tag是否合法
	     switch (tag_filegroup){
	       case TAG_VCARDCURRENTUSIMGROUP:
	     	 case TAG_VCARDCURRENTSIMGROUP:  
	     	 case TAG_VCARDCURRENTUIMGROUP:
	     	 	    break;
	     	 default:
	     	 	    return -1;	     	
	     	}
	     	
	     while (offsetcurrent<vcardbuflength){
        	 if (tag_filegroup==ucvcardbuf[offsetcurrent])
       	        break;
        	 else {
        	 	       // 取出tlv body 长度
                 if (FreshLengthAndReturnValue(&ucvcardbuf[offsetcurrent+1],0, &tmpoffset)==-1) //取出2字节的TLV_length
                   	return -1;
                 offsetcurrent+=tmpoffset+3; // 3为 Tag +Tlv_length所占字节	     	 	
         	 }	     	 	      		     	  		     	  	
	     	}
	    
	     if (offsetcurrent<vcardbuflength){
	     	          
	     	          offsetcurrentfile_length=offsetcurrent+3;
	     	 	       // 取出tlv body 长度
                 if (FreshLengthAndReturnValue(&ucvcardbuf[offsetcurrent+1],0, &tmpoffset)==-1) //取出2字节的TLV_length
                   	return -1;
                 offsetcurrent+=tmpoffset+3; // 3为 Tag +Tlv_length所占字节	 
                 
	               offsetcurrentgroupend=offsetcurrentfile_length+tmpoffset; //group 的尾部偏置
	      }
	   	 
	   	 if (vcardbuflength<maxbuflengh){		
  	      if (CheckVcardValidate(ucvcardbuf)==1){ 	   	   
  	         InitVcardIndexList(VcardIndexList,ListVolume);
             while (offsetcurrentfile_length<offsetcurrentgroupend){ 
  	         // 取出tlv body 长度
                 if (FreshLengthAndReturnValue(&ucvcardbuf[offsetcurrentfile_length],0, &tmpoffset)==-1) //取出2字节的当前文件长度
                   	  return -1;
                 currentfile_length= tmpoffset;    	 
  	       	     
  	       	 
       	       	 VcardIndexList[currentlistnum].ptr_identifier=&ucvcardbuf[offsetcurrentfile_length+4];
       	       	 VcardIndexList[currentlistnum].ptr_access_flag=&ucvcardbuf[offsetcurrentfile_length+2]; //See card 头文件
       	       	 VcardIndexList[currentlistnum].ptr_identifier_length=&ucvcardbuf[offsetcurrentfile_length+3];
       	       	 if (currentfile_length==(4+*(VcardIndexList[currentlistnum].ptr_identifier_length))){    //如果没有文件头和文件体
       	       //	 if (currentfile_length==5){
       	       	 	   offsetcurrentfile_length+=currentfile_length;
       	       	 	   currentlistnum++;
       	       	 }
       	       	 else{
       	       	 	     VcardIndexList[currentlistnum].ptr_fileheaderlength=&ucvcardbuf[offsetcurrentfile_length+\
       	       	 	                                                          4+*VcardIndexList[currentlistnum].ptr_identifier_length];
       	       	 	     VcardIndexList[currentlistnum].ptr_fileheaderresponse=&ucvcardbuf[offsetcurrentfile_length+\
       	       	 	                                                          5+*VcardIndexList[currentlistnum].ptr_identifier_length];
                        VcardIndexList[currentlistnum].ptr_fileheader=&ucvcardbuf[offsetcurrentfile_length+\
       	       	 	                                                          7+*VcardIndexList[currentlistnum].ptr_identifier_length];		       	 	                                                          
       	       	 	
       	       	 	  if ((tag_filegroup== TAG_VCARDCURRENTSIMGROUP)||(tag_filegroup== TAG_VCARDCURRENTUIMGROUP)){ 
       	       	 	    
                        if (tag_filegroup== TAG_VCARDCURRENTSIMGROUP)  {	
                                if (*VcardIndexList[currentlistnum].ptr_identifier_length==2){
                                	    tmppathidentifier.length=1;
                                	    tmppathidentifier.identifier1[0]=*VcardIndexList[currentlistnum].ptr_identifier;
                                	    tmppathidentifier.identifier1[1]=*(VcardIndexList[currentlistnum].ptr_identifier+1); 
                                }
                                else
                                if (*VcardIndexList[currentlistnum].ptr_identifier_length==4){
                                	    tmppathidentifier.length=2;
                                	    tmppathidentifier.identifier1[0]=*VcardIndexList[currentlistnum].ptr_identifier;
                                	    tmppathidentifier.identifier1[1]=*(VcardIndexList[currentlistnum].ptr_identifier+1); 
                                	    tmppathidentifier.identifier2[0]=*(VcardIndexList[currentlistnum].ptr_identifier+2);
                                	    tmppathidentifier.identifier2[1]=*(VcardIndexList[currentlistnum].ptr_identifier+3); 
                                }	
                                else
                                if (*VcardIndexList[currentlistnum].ptr_identifier_length==6){
                                	    tmppathidentifier.length=3;
                                	    tmppathidentifier.identifier1[0]=*VcardIndexList[currentlistnum].ptr_identifier;
                                	    tmppathidentifier.identifier1[1]=*(VcardIndexList[currentlistnum].ptr_identifier+1); 
                                	    tmppathidentifier.identifier2[0]=*(VcardIndexList[currentlistnum].ptr_identifier+2);
                                	    tmppathidentifier.identifier2[1]=*(VcardIndexList[currentlistnum].ptr_identifier+3); 
                                	    tmppathidentifier.identifier3[0]=*(VcardIndexList[currentlistnum].ptr_identifier+4);
                                	    tmppathidentifier.identifier3[1]=*(VcardIndexList[currentlistnum].ptr_identifier+5); 
                                }
                                	else
                                		 return -1;	
                                	
                                 if (*(VcardIndexList[currentlistnum].ptr_fileheader+7-1)==0x4) //EF Type
                                	 		 
                                    switch   (*(VcardIndexList[currentlistnum].ptr_fileheader+14-1)){
    						            	        case 0:
    						            	          VcardIndexList[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_TRANSPARENT;
                                        break;
    						            	        case 1:
    						            	          VcardIndexList[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_LINEARFIXED;
    						            	           break;
    						            	        case 3:
    						            	          VcardIndexList[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_CYCLIC;
    						            	          break;
    						            	        default:
    						            	 	        VcardIndexList[currentlistnum].ucCurrentEFType=0;
    						            	           break;
  						            	        } 		 
                                
                               if (GetVcardFilePropertyWithPath_Sim(tmppathidentifier, &file1)<0)
                                     return -1;
          	            
          	             }
          	             else{
          	          	      if (tag_filegroup== TAG_VCARDCURRENTUIMGROUP){        	   	    
                                if (*VcardIndexList[currentlistnum].ptr_identifier_length==2){
                                	    tmppathidentifier.length=1;
                                	    tmppathidentifier.identifier1[0]=*VcardIndexList[currentlistnum].ptr_identifier;
                                	    tmppathidentifier.identifier1[1]=*(VcardIndexList[currentlistnum].ptr_identifier+1); 
                                	}
                                else
                                if (*VcardIndexList[currentlistnum].ptr_identifier_length==4){
                                	    tmppathidentifier.length=2;
                                	    tmppathidentifier.identifier1[0]=*VcardIndexList[currentlistnum].ptr_identifier;
                                	    tmppathidentifier.identifier1[1]=*(VcardIndexList[currentlistnum].ptr_identifier+1); 
                                	    tmppathidentifier.identifier2[0]=*(VcardIndexList[currentlistnum].ptr_identifier+2);
                                	    tmppathidentifier.identifier2[1]=*(VcardIndexList[currentlistnum].ptr_identifier+3); 
                                	}	
                                else
                                if (*VcardIndexList[currentlistnum].ptr_identifier_length==6){
                                	    tmppathidentifier.length=3;
                                	    tmppathidentifier.identifier1[0]=*VcardIndexList[currentlistnum].ptr_identifier;
                                	    tmppathidentifier.identifier1[1]=*(VcardIndexList[currentlistnum].ptr_identifier+1); 
                                	    tmppathidentifier.identifier2[0]=*(VcardIndexList[currentlistnum].ptr_identifier+2);
                                	    tmppathidentifier.identifier2[1]=*(VcardIndexList[currentlistnum].ptr_identifier+3); 
                                	    tmppathidentifier.identifier3[0]=*(VcardIndexList[currentlistnum].ptr_identifier+4);
                                	    tmppathidentifier.identifier3[1]=*(VcardIndexList[currentlistnum].ptr_identifier+5); 
                                	}
                                	else
                                		 return -1;	
                                		 
                               if (*(VcardIndexList[currentlistnum].ptr_fileheader+7-1)==0x4) //EF Type
                                	 		 
                                    switch   (*(VcardIndexList[currentlistnum].ptr_fileheader+14-1)){
    						            	        case 0:
    						            	          VcardIndexList[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_TRANSPARENT;
                                        break;
    						            	        case 1:
    						            	          VcardIndexList[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_LINEARFIXED;
    						            	           break;
    						            	        case 3:
    						            	          VcardIndexList[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_CYCLIC;
    						            	          break;
    						            	        default:
    						            	 	        VcardIndexList[currentlistnum].ucCurrentEFType=0;
    						            	           break;
  						            	        } 		 
  						            	        
                               if (GetVcardFilePropertyWithPath_Uim(tmppathidentifier, &file1)<0) 
                                   return -1;
          	                  }          	          	     
          	          	}
          	                  
       	       	 	  //  if ((file1.property&FILE_TYPE_MF)|(file1.property&FILE_TYPE_DF)){  	
       	       	 	    if ((file1.property&FILE_TYPE_MF)|(file1.property&FILE_TYPE_DF)|(file1.property&EF_READ_HEADER_ONLY)){ 	      	
       	      		         offsetcurrentfile_length+=currentfile_length;
       	      		         currentlistnum++;		      		 
       	      		    }
       	      		    else{
       	      		    		  file1.property|= VcardIndexList[currentlistnum].ucCurrentEFType;
       	      		    		  offsetcurrentfile_header_length=offsetcurrentfile_length+4+*VcardIndexList[currentlistnum].ptr_identifier_length;
                             currentfile_header_length=ucvcardbuf[offsetcurrentfile_header_length];	 
       	      		    	    if ((file1.property&FILE_TYPE_EF)&&(file1.property&EF_STRUCTURE_TRANSPARENT)){
       	      		    	    	    VcardIndexList[currentlistnum].ptr_filebodylength=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
       	      		    	    	                                7+*VcardIndexList[currentlistnum].ptr_identifier_length];
       	      		    	    	    VcardIndexList[currentlistnum].ptr_filebodyresponse=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
       	      		    	    	                                    7+2+*VcardIndexList[currentlistnum].ptr_identifier_length];
       	      		    	          VcardIndexList[currentlistnum].ptr_filebody=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
       	                                                          7+2+2+*VcardIndexList[currentlistnum].ptr_identifier_length];
       	      		    	         offsetcurrentfile_length+=currentfile_length;
       	      		                currentlistnum++;
       	      	        	  }
       	      	        	  else{
       	      	        	  	     if ((file1.property&FILE_TYPE_EF)&&(file1.property&EF_STRUCTURE_LINEARFIXED||file1.property&EF_STRUCTURE_CYCLIC)){
                                         VcardIndexList[currentlistnum].ptr_recordresponse=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
                                                             7+*VcardIndexList[currentlistnum].ptr_identifier_length];
                                        offsetcurrentrecord=offsetcurrentfile_length+currentfile_header_length+\
       	      		    	    	                            9+*VcardIndexList[currentlistnum].ptr_identifier_length;
                                        while (offsetcurrentrecord<offsetcurrentfile_length+currentfile_length){
       	      		    	    	         	     
       	      		    	    	         	     VcardIndexList[currentlistnum].ptr_recordbody[currentrecordnum].ptr_recordserialnumber=&ucvcardbuf[offsetcurrentrecord];
       	      		    	    	               VcardIndexList[currentlistnum].ptr_recordbody[currentrecordnum].ptr_recordlength=&ucvcardbuf[offsetcurrentrecord+1];
       	      		    	    	               VcardIndexList[currentlistnum].ptr_recordbody[currentrecordnum].ptr_recordcontext=&ucvcardbuf[offsetcurrentrecord+2];
       	      		    	    	               
       	      		    	    	               
       	      		    	    	               currentrecord_length=ucvcardbuf[offsetcurrentrecord+1];
       	      		    	    	         	     offsetcurrentrecord=offsetcurrentrecord+ currentrecord_length+2; 
       	      		    	    	         	     currentrecordnum++;    		    	    	         	
       	      		    	    	         	}	    	    	          
       	      		    	    	         if (offsetcurrentrecord==offsetcurrentfile_length+currentfile_length){
       	      		    	    	         	     offsetcurrentfile_length+=currentfile_length;
       	      		                           currentlistnum++;
       	      		                           currentrecordnum=0;
       	      		    	    	         	}
       	      		    	    	         else	
       	      		    	    	         	    return -1;			     		    	    	            	     	   
       	                           }
       	                           else
       	                           	        return -1;
       	      	        	  }		      		    	
       	      		    }	
       	      		  }
       	      		  else {
       	      		  	   if (tag_filegroup== TAG_VCARDCURRENTUSIMGROUP){ 
       	       	 	             for (int i=0;i< *VcardIndexList[currentlistnum].ptr_identifier_length;i++){
                                      identifier[i]=*(VcardIndexList[currentlistnum].ptr_identifier+i);   	      
                                } 	
                                tmpt= GetTagLocation_inFCP(TAG_FILE_DESCRIPTOR_WITH_TEMPLATE,VcardIndexList_Usim[currentlistnum].ptr_fileheader,\
	 	 	 	 	     	    	    	                 (int )((*VcardIndexList_Usim[currentlistnum].ptr_fileheaderlength)-2));  //减去 tag 和 tlvlength
  						                   if (tmpt==-1)
  						                       	 return -1;
  						                       	 
  						                       	 
                                switch   ((*(VcardIndexList_Usim[currentlistnum].ptr_fileheader+tmpt+2))&0x7){
  						            	        case 1:
  						            	          VcardIndexList_Usim[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_TRANSPARENT;
                                      break;
  						            	        case 2:
  						            	          VcardIndexList_Usim[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_LINEARFIXED;
  						            	           break;
  						            	        case 6:
  						            	          VcardIndexList_Usim[currentlistnum].ucCurrentEFType=FILE_TYPE_EF|EF_STRUCTURE_CYCLIC;
  						            	          break;
  						            	        default:
  						            	 	        VcardIndexList_Usim[currentlistnum].ucCurrentEFType=0;
  						            	           break;
  						            	    }
  						            	    if (VcardIndexList_Usim[currentlistnum].ucCurrentEFType&FILE_TYPE_EF){
  						            	    	if (*(VcardIndexList_Usim[currentlistnum].ptr_identifier+3)==0xd9)
  						            	    		  int err2=0;
  						            	       tmpt= GetTagLocation_inFCP(TAG_SFI_WITH_FCP_TEMPLATE,VcardIndexList_Usim[currentlistnum].ptr_fileheader,\
	 	 	 	 	     	    	    	                 (int )((*VcardIndexList_Usim[currentlistnum].ptr_fileheaderlength)-2));  //减去 tag 和 tlvlength
  						                   if (tmpt==-1)  //如果不存在Tag， SFI为Identifier 前5位
  						                   	      VcardIndexList_Usim[currentlistnum].ucSfi=(*(VcardIndexList_Usim[currentlistnum].ptr_identifier+((*VcardIndexList_Usim[currentlistnum].ptr_identifier_length)-1)))&0x1f; 
  						                   else	{
  						                   	      if ((*(VcardIndexList_Usim[currentlistnum].ptr_fileheader+tmpt+1))==1)  //Tag 存在，且TLVLength=1 
  						                   	         if (!((*(VcardIndexList_Usim[currentlistnum].ptr_fileheader+tmpt+2))&7)) //且TLV值后3位为0
  						                   	         	  VcardIndexList_Usim[currentlistnum].ucSfi=(*(VcardIndexList_Usim[currentlistnum].ptr_fileheader+tmpt+2))>>3; //SFI 为TLV值前5位
  						                   	        
  						                   	}       						                   	  						                  
                                }
                            //     if (currentlistnum==0x32)
                       	    //    	  	     	   currentlistnum=0x32;
       						           //  if (GetVcardFileProperty_Usim(*VcardIndexList[currentlistnum].ptr_identifier_length, identifier,&file1)<0) 	
       						              if ( GetVcardFileProperty_WithUsimRidType(*VcardIndexList[currentlistnum].ptr_identifier_length, identifier, (*(VcardIndexList[currentlistnum].ptr_access_flag))>>5,&file1)<0) 
       						                	 return -1;	               	    
                                   
                        	 	   // if ((file1.property&FILE_TYPE_MF)|(file1.property&FILE_TYPE_DF)){  	
                        	 	   if ((file1.property&FILE_TYPE_MF)|(file1.property&FILE_TYPE_DF)|(file1.property&EF_READ_HEADER_ONLY)){   //20180928	      	
                       		         offsetcurrentfile_length+=currentfile_length;
                       		         currentlistnum++;		      		 
                       		    }
                       		    else{
                       		    	    file1.property|= VcardIndexList_Usim[currentlistnum].ucCurrentEFType;
                       		    	    
                       		    	    offsetcurrentfile_header_length=offsetcurrentfile_length+4+*VcardIndexList[currentlistnum].ptr_identifier_length;
                                     currentfile_header_length=ucvcardbuf[offsetcurrentfile_header_length];	 
                       		    	    if ((file1.property&FILE_TYPE_EF)&&(file1.property&EF_STRUCTURE_TRANSPARENT)){
                       		    	    	 VcardIndexList[currentlistnum].ptr_filebodylength=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
                       		    	    	                                7+*VcardIndexList[currentlistnum].ptr_identifier_length];
                       		    	    	    VcardIndexList[currentlistnum].ptr_filebodyresponse=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
                       		    	    	                                    7+2+*VcardIndexList[currentlistnum].ptr_identifier_length];
                                           VcardIndexList[currentlistnum].ptr_filebody=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
                                                                           7+2+2+*VcardIndexList[currentlistnum].ptr_identifier_length];
                  	                        offsetcurrentfile_length+=currentfile_length;
                       		                currentlistnum++;
                       	        	  }
                       	        	  else{
                       	        	  	     if ((file1.property&FILE_TYPE_EF)&&(file1.property&EF_STRUCTURE_LINEARFIXED||file1.property&EF_STRUCTURE_CYCLIC)){
                                              
                                                VcardIndexList[currentlistnum].ptr_recordresponse=&ucvcardbuf[offsetcurrentfile_length+currentfile_header_length+\
                                                                     7+*VcardIndexList[currentlistnum].ptr_identifier_length];
                       		    	    	         offsetcurrentrecord=offsetcurrentfile_length+currentfile_header_length+\
                       		    	    	                            9+*VcardIndexList[currentlistnum].ptr_identifier_length;
                       		    	    	         while (offsetcurrentrecord<offsetcurrentfile_length+currentfile_length){
                       		    	    	         	     VcardIndexList[currentlistnum].ptr_recordbody[currentrecordnum].ptr_recordserialnumber=&ucvcardbuf[offsetcurrentrecord];
                       		    	    	               VcardIndexList[currentlistnum].ptr_recordbody[currentrecordnum].ptr_recordlength=&ucvcardbuf[offsetcurrentrecord+1];
                       		    	    	               VcardIndexList[currentlistnum].ptr_recordbody[currentrecordnum].ptr_recordcontext=&ucvcardbuf[offsetcurrentrecord+2];
                       		    	    	               currentrecord_length=ucvcardbuf[offsetcurrentrecord+1];
                       		    	    	         	     offsetcurrentrecord=offsetcurrentrecord+ currentrecord_length+2; 
                       		    	    	         	     currentrecordnum++;    		    	    	         	
                       		    	    	         	}	    	    	          
                       		    	    	         if (offsetcurrentrecord==offsetcurrentfile_length+currentfile_length){
                       		    	    	         	     offsetcurrentfile_length+=currentfile_length;
                       		                           currentlistnum++;
                       		                           currentrecordnum=0;
                       		    	    	         	}
                       		    	    	         else	
                       		    	    	         	    return -1;			     		    	    	            	     	   
                                            }
                                            else
                                            	        return -1;
                       	        	  }		      		    	
                       		    }	
       	      		       }
      
       	      		  } 	      		    		 
  	       	     } 		       	 	
      	     }	    
  	      }
  	   else	 
  	       return -1;
	     }	
	     else
			   return -1;
   		if ((tag_filegroup== TAG_VCARDCURRENTSIMGROUP)||(tag_filegroup== TAG_VCARDCURRENTUIMGROUP)	) {
   			// std::endl;
   			 std::cout<<GREEN<<"Uim/Sim IndexList "<<std::endl ;   
   	      PrintVcard(VcardIndexList);  
   	  }
   	  if (tag_filegroup== TAG_VCARDCURRENTUSIMGROUP){	 
   	  	// std::endl;
   	     std::cout<<GREEN<<"Usim IndexList "<<std::endl ;  
   	      PrintVcardUsim(VcardIndexList);     
   	  }
	
  return 1;
}




// 检查buf 中是否含Sim Vcard 和 Usim Vcard
unsigned char Emu_Engine::CheckUiccInVcard(unsigned char ucvcardbuf[]){
	  return ((ucvcardbuf[OFFSET_VCARD_MAINTAIN_INFORMATION])&(VC_DATA_USIMMODE_MAINTAIN|VC_DATA_SIMMODE_MAINTAIN|VC_DATA_UIMMODE_MAINTAIN));
}

// 检查buf 中是否 support Usim ,Uim ,Sim APDU
unsigned char Emu_Engine::CheckHandleModeInVcard(unsigned char ucvcardbuf[]){
	  return ((ucvcardbuf[OFFSET_VCARD_MAINTAIN_INFORMATION])&(VG_HANDLEICC_UIMMODE_MAINTAIN|VG_HANDLEICC_SIMMODE_MAINTAIN|VG_HANDLEICC_USIMMODE_MAINTAIN));
}


int Emu_Engine::GenVcardIndexAll(unsigned char ucvcardbuf[]){
		 if (CheckUiccInVcard(ucvcardbuf)&VC_DATA_UIMMODE_MAINTAIN) {
		 	        if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardIndexList , TAG_VCARDCURRENTUIMGROUP, MAX_VCARDINDEX )==-1)
			 	  	     return -1;	
			 	  	//  eae.ucCurrentEaeCapability|=  	VC_DATA_UIMMODE_MAINTAIN;
			 	  	  //20200330 处理Invaild card   
			 	  	  if (CheckHandleModeInVcard(ucvcardbuf)&VG_HANDLEICC_UIMMODE_MAINTAIN)
			 	  	       eae.ucCurrentEaeCapability|=  	VG_HANDLEICC_UIMMODE_MAINTAIN;
			 	  	  else
			 	  	  	 std::cout<<RED<<"Do not support UIM mode "<<std::endl ;     
		 	}
		  else{	
		  	    if (CheckUiccInVcard(ucvcardbuf)&VC_DATA_SIMMODE_MAINTAIN){
			 	       if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardIndexList  , TAG_VCARDCURRENTSIMGROUP, MAX_VCARDINDEX )==-1)			 	  	
			 	  	     return -1;
			 	      // eae.ucCurrentEaeCapability|=  	VC_DATA_SIMMODE_MAINTAIN; 	  
			 	       //20200330 处理Invaild card   
			 	  	  if (CheckHandleModeInVcard(ucvcardbuf)&VG_HANDLEICC_SIMMODE_MAINTAIN)
			 	  	       eae.ucCurrentEaeCapability|=  	VG_HANDLEICC_SIMMODE_MAINTAIN;	
			 	  	  else
			 	  	  	 std::cout<<RED<<"Do not support SIM mode "<<std::endl ;    
			 	    }	 
			 }	    
			 if (CheckUiccInVcard(ucvcardbuf)&VC_DATA_USIMMODE_MAINTAIN){
			 	  if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardIndexList_Usim  , TAG_VCARDCURRENTUSIMGROUP, MAX_VCARDINDEX )==-1)
			 	  	  return -1;	  	
			   // eae.ucCurrentEaeCapability|=  	VC_DATA_USIMMODE_MAINTAIN; 
			   //20200330 处理Invaild card   
			 	  	  if (CheckHandleModeInVcard(ucvcardbuf)&VG_HANDLEICC_USIMMODE_MAINTAIN)
			 	  	       eae.ucCurrentEaeCapability|=  	VG_HANDLEICC_USIMMODE_MAINTAIN;
			 	  	  else
			 	  	  	 std::cout<<RED<<"Do not support USIM mode "<<std::endl ;    
			 }
			
			 //2018.1.5
			 GetPin(ucvcardbuf) ; 
			 eae.ucProcessIndexStatus=PROCESSINDEXSTATUS_PROCESSED;
			 	
			 if (!eae.ucPinLength)  
			 	 eae.ucDefaultChv1Status=CHV1_STATUS_ENABLE;
			 else
			 if (eae.ucPinLength==8) 
			 	 eae.ucDefaultChv1Status=CHV1_STATUS_DISABLE;
			 else	{
			 	      eae.ucDefaultChv1Status=CHV1_STATUS_ENABLE_UNKOWN;
			 	      std::cout<<RED<<"CHV1_STATUS_ENABLE_UNKOWN "<<std::endl ; 
			 	      return -1;	  
			 	} 
			 	 	
   return 1;
	}
  				
int Emu_Engine::GenVcardIndexAll(unsigned char ucvcardbuf[],vcard_sort_list_t *VcardList)
{
    if (CheckUiccInVcard(ucvcardbuf)&VC_DATA_UIMMODE_MAINTAIN) 
    {
        if (GenVcardIndex( ucvcardbuf , VCARD_MAX_LENGTH, VcardList , TAG_VCARDCURRENTUIMGROUP, MAX_VCARDINDEX )==-1)
            return -1; 
    }
    else{ 
       if (CheckUiccInVcard(ucvcardbuf)&VC_DATA_USIMMODE_MAINTAIN){
            if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardList  , TAG_VCARDCURRENTUSIMGROUP, MAX_VCARDINDEX )==-1)
                return -1;        
          eae.ucCurrentEaeCapability|=    VC_DATA_USIMMODE_MAINTAIN; 
       }
       if (CheckUiccInVcard(ucvcardbuf)&VC_DATA_SIMMODE_MAINTAIN){
            if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardList   , TAG_VCARDCURRENTSIMGROUP, MAX_VCARDINDEX )==-1)                 
                return -1;
            eae.ucCurrentEaeCapability|=      VC_DATA_SIMMODE_MAINTAIN;       
            }   
       }
    return 1;
}		           
  	     
/**************************************************************************** 
* 函数名称 : PrintVcard
* 功能描述 : 打印Vcard 内容 
* 参    数 : VcardIndexList： ：索引表指针 
* 参    数 : 
* 参    数 : 
* 参    数 : 
* 返 回 值 : 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::PrintVcard(vcard_sort_list_t VcardIndexList[]){
	int filenum,fileheadernum,filebodyrnum,recorditemnum,recordcontextnum;
    	    for ( int filenum=0;filenum<MAX_VCARDINDEX;filenum++){
    	    	 if (VcardIndexList[filenum].ptr_identifier!=0){
    	    	 	   std::cout<<RESET<<std::endl<<std::flush;
    	    	 	   std::cout<<std::hex <<"VcardIndexList["<<filenum<<"]:Identifier ";
    	    	 	   for (unsigned char idlength=0;idlength<*VcardIndexList[filenum].ptr_identifier_length;idlength++)	
    	    	 	   	  std::cout<<std::hex<<(unsigned short) *(VcardIndexList[filenum].ptr_identifier+idlength);
    	    	 
    	    	 }
    	     	 if (VcardIndexList[filenum].ptr_access_flag!=0)
    	    	 	   std::cout<<std::hex <<" AccessFlag:"<<(unsigned short) *VcardIndexList[filenum].ptr_access_flag;
    	    	 if (VcardIndexList[filenum].ptr_fileheaderlength!=0)
    	    	 	   std::cout<<std::hex <<" Headerlength:"<<(unsigned short) *VcardIndexList[filenum].ptr_fileheaderlength;
    	    	 if (VcardIndexList[filenum].ptr_fileheaderresponse!=0)
    	    	 	   std::cout<<std::hex <<" HeaderRes:"<<(unsigned short) *VcardIndexList[filenum].ptr_fileheaderresponse<<" "<<(unsigned short) *(VcardIndexList[filenum].ptr_fileheaderresponse+1);
    	    	 if (VcardIndexList[filenum].ptr_fileheader!=0){
    	    	 	    std::cout<<std::hex <<" Header:";
    	    	 	    for (int fileheadernum=0;fileheadernum<*VcardIndexList[filenum].ptr_fileheaderlength;fileheadernum++)
    	    	 	         std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_fileheader+fileheadernum)<<" ";
    	    	 	}
    	    	 if (VcardIndexList[filenum].ptr_filebodylength!=0){
    	    	 	   unsigned short tmpbodylength= ((*VcardIndexList[filenum].ptr_filebodylength)<<8)+*(VcardIndexList[filenum].ptr_filebodylength+1);
    	    	 	    std::cout<<std::hex <<" BodyLength:"<<(unsigned short) tmpbodylength;
    	       }
    	    	 if (VcardIndexList[filenum].ptr_filebodyresponse!=0)
    	    	 	   std::cout<<std::hex <<" BodyRes:"<<(unsigned short) *VcardIndexList[filenum].ptr_filebodyresponse<<" "<<(unsigned short) *(VcardIndexList[filenum].ptr_filebodyresponse+1);
    	    	 	   	
    	    	 if (VcardIndexList[filenum].ptr_filebody!=0){
    	    	 	    std::cout<<RESET<<std::endl<<std::flush;
    	    	 	    std::cout<<std::hex <<"Body:";
    	    	 	    for (int filebodyrnum=0;filebodyrnum<((unsigned short) (*VcardIndexList[filenum].ptr_filebodylength<<8)+*(VcardIndexList[filenum].ptr_filebodylength+1));filebodyrnum++)
    	    	 	         std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_filebody+filebodyrnum)<<" ";
    	    	 	}
    	    	 	if (VcardIndexList[filenum].ptr_recordresponse!=0)
    	    	 	   std::cout<<std::hex <<" RecordResp:"<<(unsigned short) *VcardIndexList[filenum].ptr_recordresponse<<" "<<(unsigned short) *(VcardIndexList[filenum].ptr_recordresponse+1);
     	    	  recorditemnum=0;
    	   	    while (VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
    	   	    	  std::cout<<RESET<<std::endl<<std::flush;
    	    	  	  std::cout<<std::hex <<"RecordItem:"<<recorditemnum<<" : RecordNumber:"
    	    	  	  <<(unsigned short) *VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordserialnumber
    	    	  	  <<" RecordLength:"<<(unsigned short) *VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordlength<<std::endl<<std::flush;    	    	  	  
    	    	  	  std::cout<<std::hex <<"Record Body: ";
    	    	  	  for (recordcontextnum=0;recordcontextnum<(unsigned short) *VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordlength;recordcontextnum++)
    	    	 	         std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordcontext+recordcontextnum)<<" ";    	    	 	  
    	    	 	    recorditemnum++;	
    	    	  }   	   	 	      	    	 	   	   	    	 	    	    	
    	    } 
    	    std::cout<<std::endl;  	
}

int Emu_Engine::PrintVcardUsim(vcard_sort_list_t VcardIndexList[]){
	return 0;
	int filenum,fileheadernum,filebodyrnum,recorditemnum,recordcontextnum;
    	    for ( int filenum=0;filenum<MAX_VCARDINDEX;filenum++){
    	    	 if (VcardIndexList[filenum].ptr_identifier!=0){
    	    	 	   std::cout<<RESET<<std::endl<<std::flush;
    	    	 	   std::cout<<std::hex <<"VcardIndexList["<<filenum<<"]:Identifier ";
    	    	 	   for (int i=0;i<*VcardIndexList[filenum].ptr_identifier_length;i++)
    	    	       std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_identifier+i);
    	    	 }
    	    	  if (VcardIndexList[filenum].ucSfi!=0){
    	    	 	    std::cout<<std::hex <<" SFI:"<<std::hex <<(unsigned short)VcardIndexList[filenum].ucSfi<<" ";
    	    	 	}
    	     	 /*if (VcardIndexList[filenum].ptr_access_flag!=0)
    	    	 	   std::cout<<std::hex <<" AccessFlag:"<<(unsigned short) *VcardIndexList[filenum].ptr_access_flag;
    	    	 	   */
    	    	 	   //access_flag  bit 0 表示 访问状态   bit5-7 表示rid类型 bit3-4 LC文件缺省值记录 01：ff，10：0
    	    	 	   
    	    	 if (VcardIndexList[filenum].ptr_access_flag!=0){
    	    	 	   std::cout<<std::hex <<" AccessFlag:"<<(unsigned short) ((*VcardIndexList[filenum].ptr_access_flag )&0x1);
    	    	 	   std::cout<<std::hex <<" RidType:"<<(unsigned short) ((*VcardIndexList[filenum].ptr_access_flag )>>5);	
    	    	 	  }
    	    	 	  	
    	    	 if (VcardIndexList[filenum].ptr_fileheaderlength!=0)
    	    	 	   std::cout<<std::hex <<" Headerlength:"<<(unsigned short) *VcardIndexList[filenum].ptr_fileheaderlength;
    	    	 if (VcardIndexList[filenum].ptr_fileheaderresponse!=0)
    	    	 	   std::cout<<std::hex <<" HeaderRes:"<<(unsigned short) *VcardIndexList[filenum].ptr_fileheaderresponse<<" "<<(unsigned short) *(VcardIndexList[filenum].ptr_fileheaderresponse+1);
    	    	 if (VcardIndexList[filenum].ptr_fileheader!=0){
    	    	 	    std::cout<<std::hex <<" Header:";
    	    	 	    for (int fileheadernum=0;fileheadernum<*VcardIndexList[filenum].ptr_fileheaderlength;fileheadernum++)
    	    	 	         std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_fileheader+fileheadernum)<<" ";
    	    	 	}
    	    	 if (VcardIndexList[filenum].ptr_filebodylength!=0){
    	    	 	   unsigned short tmpbodylength= ((*VcardIndexList[filenum].ptr_filebodylength)<<8)+*(VcardIndexList[filenum].ptr_filebodylength+1);
    	    	 	    std::cout<<std::hex <<" BodyLength:"<<(unsigned short) tmpbodylength;
    	    	 	  }
    	    	 if (VcardIndexList[filenum].ptr_filebodyresponse!=0)
    	    	 	   std::cout<<std::hex <<" BodyRes:"<<(unsigned short) *VcardIndexList[filenum].ptr_filebodyresponse<<" "<<(unsigned short) *(VcardIndexList[filenum].ptr_filebodyresponse+1);
    	    	 	   	
    	    	 if (VcardIndexList[filenum].ptr_filebody!=0){
    	    	 	    std::cout<<RESET<<std::endl<<std::flush;
    	    	 	    std::cout<<std::hex <<"Body:";
    	    	 	    for (int filebodyrnum=0;filebodyrnum<((unsigned short) (*VcardIndexList[filenum].ptr_filebodylength<<8)+*(VcardIndexList[filenum].ptr_filebodylength+1));filebodyrnum++)
    	    	 	         std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_filebody+filebodyrnum)<<" ";
    	    	 	}
    	    	 	if (VcardIndexList[filenum].ptr_recordresponse!=0)
    	    	 	   std::cout<<std::hex <<" RecordResp:"<<(unsigned short) *VcardIndexList[filenum].ptr_recordresponse<<" "<<(unsigned short) *(VcardIndexList[filenum].ptr_recordresponse+1);
     	    	  recorditemnum=0;
    	   	    while (VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
    	   	    	  std::cout<<RESET<<std::endl<<std::flush;
    	    	  	  std::cout<<std::hex <<"RecordItem:"<<recorditemnum<<" : RecordNumber:"
    	    	  	  <<(unsigned short) *VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordserialnumber
    	    	  	  <<" RecordLength:"<<(unsigned short) *VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordlength<<std::endl<<std::flush;    	    	  	  
    	    	  	  std::cout<<std::hex <<"Record Body: ";
    	    	  	  for (recordcontextnum=0;recordcontextnum<(unsigned short) *VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordlength;recordcontextnum++)
    	    	 	         std::cout<<std::hex <<(unsigned short) *(VcardIndexList[filenum].ptr_recordbody[recorditemnum].ptr_recordcontext+recordcontextnum)<<" ";    	    	 	  
    	    	 	    recorditemnum++;	
    	    	  }   	   	 	      	    	 	   	   	    	 	    	    	
    	    }
    	     std::cout<<std::endl;  	   	
}

//Vcard 索引表中是否存在该identifier
int Emu_Engine::GetIdentifierIndex(unsigned char identifier[2]){  
int filenum;
    	    for ( int filenum=0;filenum<MAX_VCARDINDEX;filenum++)
    	    	  if (VcardIndexList[filenum].ptr_identifier!=0){
    	    	  	 if ((*VcardIndexList[filenum].ptr_identifier==identifier[0])&&(*(VcardIndexList[filenum].ptr_identifier+1)==identifier[1])){
    	    	  	 	   if (*VcardIndexList[filenum].ptr_access_flag==VCARD_FILE_ACCESS_PROHIBIT)
    	    	   	         return -1;
    	    	   	     else    
    	    	  	 	       return filenum;    	    	  	 	 
    	    	  	 }	   
    	    	  }
    	     return -1;	    	        	       	    	      	    	   	   	        	    	    	    
}

int Emu_Engine::GetIdentifierIndexWithPath(unsigned char identifier[2]){  
int filenum,idoffset,pathidoffset;
unsigned char tmppathid[20],tmpvcardindexid[20];
char *str;
	   // 确保路径中无 0 
          tmppathid[0]=eae.CurrentIdWithPath.firstlevelid[0];
          tmppathid[1]=eae.CurrentIdWithPath.firstlevelid[1];
          tmppathid[2]=eae.CurrentIdWithPath.secondlevelid[0];
          tmppathid[3]=eae.CurrentIdWithPath.secondlevelid[1];
          tmppathid[4]=eae.CurrentIdWithPath.thirdlevelid[0];
          tmppathid[5]=eae.CurrentIdWithPath.thirdlevelid[1];
          tmppathid[6]=0;  //字符串结尾
          
    	    for ( int filenum=0;filenum<MAX_VCARDINDEX;filenum++)
    	    	  if (VcardIndexList[filenum].ptr_identifier!=0){    	    	  	     	    	  	  	  
    	    	  	  	// for (idlength=*VcardIndexList[filenum].ptr_identifier_length;idlength>0;idlength--)
    	    	  	  	
    	    	  	  	
    	    	  	   if ((*(VcardIndexList[filenum].ptr_identifier+(*VcardIndexList[filenum].ptr_identifier_length-2))==identifier[0])&&\
    	    	  	   	   (*(VcardIndexList[filenum].ptr_identifier+(*VcardIndexList[filenum].ptr_identifier_length-1))==identifier[1])){
    	    	  	   	   
    	    	  	   	   	  if  (!(
    	    	  	   	   	  	   (*VcardIndexList[filenum].ptr_identifier_length==4)||(*VcardIndexList[filenum].ptr_identifier_length==6)\
    	    	  	   	   	  	   ||(*VcardIndexList[filenum].ptr_identifier_length==8)
    	    	  	   	   	  	   ))
    	    	  	   	   	  	   {
    	    	  	   	   	  	   	  std::cout<<RED<<"ERROR Identifier Length"<<RESET<<std::endl<<std::flush;
	     	    	  	                   return -1;
    	    	  	   	   	  	   	}
    	    	  	   	   	  else {
    	    	  	   	   	  	   
    	    	  	   	   	           for  (idoffset=0;idoffset<*VcardIndexList[filenum].ptr_identifier_length-2;idoffset++){
    	    	  	   	   	               tmpvcardindexid[idoffset]=*(VcardIndexList[filenum].ptr_identifier+idoffset);
    	    	  	   	   	           }
    	    	  	   	   	           tmpvcardindexid[idoffset]=0; // 字符串后缀
    	    	  	   	   	           //同名Identifier 在Vcard 中一定是带路径的
    	    	  	   	   	        //   if (!strstr((char *)tmpvcardindexid,(char *)tmppathid))  	    	  	   	   	         
    	    	  	   	   	           if (!strstr((char *)tmppathid,(char *)tmpvcardindexid))
    	    	  	   	   	           	  continue;
    	    	  	   	   	  }        	    	  	   	    	  	    	  	   	    	
    	    	  	 	       if (*VcardIndexList[filenum].ptr_access_flag==VCARD_FILE_ACCESS_PROHIBIT)
    	    	   	           return -1;
    	    	   	         else    
    	    	  	 	         return filenum;    	    	  	 	 
    	    	  	     }
    	    	  	     
    	    	  }
    	     return -1;	    	        	       	    	      	    	   	   	        	    	    	    
}

//Vcard 索引表中是否存在该Usimidentifier
int Emu_Engine::GetIdentifierIndex_Usim(unsigned char length,unsigned char identifier[IDENTIFIERDEEPTH]){  
int filenum;
unsigned char i;
    	  for ( int filenum=0;filenum<MAX_VCARDINDEX;filenum++)
    	    	if (VcardIndexList_Usim[filenum].ptr_identifier!=0){
    	    	  	if (*(VcardIndexList_Usim[filenum].ptr_identifier_length)==length){
        	    	  	 i=length;
        	    	  	 while (i){
        	    	  	 	   if ((*(VcardIndexList_Usim[filenum].ptr_identifier+i-1))!=identifier[i-1])
        	    	  	 	        break;
        	    	  	 	   else
        	    	  	 	   	  i--;     
        	    	  	 }
        	    	  	 if (!i) {
        	    	  	 	 if (*VcardIndexList_Usim[filenum].ptr_access_flag==VCARD_FILE_ACCESS_PROHIBIT)
        	    	   	         return -1;
        	    	   	     else    
        	    	  	 	       return filenum; 
        	    	  	 	
        	    	  	 }       	    	  
        	    	}		   
    	    	}
    	  return -1;	    	        	       	    	      	    	   	   	        	    	    	    
}

int Emu_Engine::GetIdentifierIndex_UsimWithRidType(unsigned char length,unsigned char identifier[IDENTIFIERDEEPTH], unsigned ridtype){  
int filenum;
unsigned char i;
        // 如果 id 为 非AID， 不存在重名
        for ( filenum=0;filenum<MAX_VCARDINDEX;filenum++)
    	    	if ((VcardIndexList_Usim[filenum].ptr_identifier!=0)&&\
    	    		( ((*VcardIndexList_Usim[filenum].ptr_access_flag)>>5)==RIDTYPE_NONEEDAID)
    	    		)
    	    		{
    	    	  	if (*(VcardIndexList_Usim[filenum].ptr_identifier_length)==length){
        	    	  	 i=length;
        	    	  	 while (i){
        	    	  	 	   if   ((*(VcardIndexList_Usim[filenum].ptr_identifier+i-1))!=identifier[i-1])    
        	    	  	 	        break;
        	    	  	 	   else
        	    	  	 	   	  i--;     
        	    	  	 }
        	    	  	 if (!i) {
        	    	  	 	 if (*VcardIndexList_Usim[filenum].ptr_access_flag==VCARD_FILE_ACCESS_PROHIBIT)
        	    	   	         return -1;
        	    	   	     else   
        	    	  	 	       return filenum; 
        	    	  	 	        
        	    	  	 	
        	    	  	 }       	    	  
        	    	}		   
    	    	}
        // 如果 id 为 AID， 可能存在重名
    	  for (  filenum=0;filenum<MAX_VCARDINDEX;filenum++)
    	    	if ((VcardIndexList_Usim[filenum].ptr_identifier!=0)&&\
    	    		( ((*VcardIndexList_Usim[filenum].ptr_access_flag)>>5)==ridtype)
    	    		)
    	    		{
    	    	  	if (*(VcardIndexList_Usim[filenum].ptr_identifier_length)==length){
        	    	  	 i=length;
        	    	  	 while (i){
        	    	  	 	   if   ((*(VcardIndexList_Usim[filenum].ptr_identifier+i-1))!=identifier[i-1])    
        	    	  	 	        break;
        	    	  	 	   else
        	    	  	 	   	  i--;     
        	    	  	 }
        	    	  	 if (!i) {
        	    	  	 	 if (((*VcardIndexList_Usim[filenum].ptr_access_flag)&0x1)==VCARD_FILE_ACCESS_PROHIBIT)
        	    	   	         return -1;
        	    	   	     else    
        	    	  	 	       return filenum; 
        	    	  	 	
        	    	  	 }       	    	  
        	    	}		   
    	    	}
    	  return -1;	    	        	       	    	      	    	   	   	        	    	    	    
}


/**************************************************************************** 
* 函数名称 : GetIdentifierResponse
* 功能描述 : 获取文件的响应 ，置于identifierresponse
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : identifierresponse 响应指针
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetIdentifierResponse(int vcardindexlistserialnumber, unsigned char (*identifierresponse)[2]){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){
	     	       if (VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength!=0){
	     	    	   (*identifierresponse)[0]=SW1_9F;
	     	    	   (*identifierresponse)[1]= *VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength;
	     	    	   
	     	      	}
	     	      else{  //File not found
	     	    	   (*identifierresponse)[0]=SW1_94;
	     	    	   (*identifierresponse)[1]=0x04;     	    	
	     	    	}	     	    	
	     	      return 1;	
	     	  }
	     	  if (
	   	                                        	   //TODO 补充activated
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				)
  	   			 {
	     	       if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength!=0){
	     	    	   (*identifierresponse)[0]=SW1_61;
	     	    	   (*identifierresponse)[1]= *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength;
	     	    	   
	     	      	}
	     	      else{  //File not found
	     	    	   (*identifierresponse)[0]=SW1_6A;
	     	    	   (*identifierresponse)[1]=SW2_82;     	    	
	     	    	}	     	    	
	     	      return 1;	
	     	  }
	     	       	
	     	} 	   	        	    	    	    
}

/**************************************************************************** 
* 函数名称 : GetFileHeaderResponse
* 功能描述 : 获取文件的文件头响应 置于fileheaderresponse
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : fileheaderresponse 文件头响应指针
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetFileHeaderResponse(int vcardindexlistserialnumber, unsigned char (*fileheaderresponse)[2]){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){
	     	       if (VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderresponse!=0){
	     	    	     (*fileheaderresponse)[0]=*VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderresponse;
	     	    	     (*fileheaderresponse)[1]=*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderresponse+1);
	     	    	       return 1;	
	     	    	 }
	     	    else
	     	    	return -1;
	     	    }
	     	    
	     	    if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	       if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderresponse!=0){
	     	    	     (*fileheaderresponse)[0]=*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderresponse;
	     	    	     (*fileheaderresponse)[1]=*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderresponse+1);
	     	    	       return 1;	
	     	    	 }
	     	    else
	     	    	return -1;
	     	    }
	     	    
	     	    	   		     	         	
	     }
	     	      	   	        	    	    	    
}

/**************************************************************************** 
* 函数名称 : GetFileHeaderContext
* 功能描述 : 获取文件的文件头内容，长度为length， 置于fileheaderbuf
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : fileheaderbuf 文件头Buf指针
* 参    数 : length 内容长度
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetFileHeaderContext(int vcardindexlistserialnumber, unsigned char *fileheaderbuf, unsigned char length){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){
	     	       if (VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength!=0){
	     	    	     if (*VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength>=length)
	     	    	  	      memcpy(fileheaderbuf,VcardIndexList[vcardindexlistserialnumber].ptr_fileheader ,length);	     	    	   
  	    	         else{
	     	    	  	    std::cout<<RED<<"GetFileHeaderContext Fail Length OverFlow"<<RESET<<std::endl<<std::flush;
	     	    	  	    return -1;
	     	    	  }	        	
	     	    	}
	     	    else
	     	    	return -1; 
	     	    }
	     	    
	     	   if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	       if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength!=0){
	     	    	     if (*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength>=length)
	     	    	  	      memcpy(fileheaderbuf,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader ,length);	     	    	   
  	    	         else{
	     	    	  	    std::cout<<RED<<"GetFileHeaderContext Fail Length OverFlow"<<RESET<<std::endl<<std::flush;
	     	    	  	    return -1;
	     	    	  }	        	
	     	    	}
	     	    else
	     	    	return -1; 
	     	    }  	   		     	         	
	     } 	
	     return 1;    	        	    	    	    
}

int  Emu_Engine::GetFileHeaderLength(int vcardindexlistserialnumber, unsigned char *length){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){
	     	       if (VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength!=0)
	     	    	     *length=*VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength;
	     	    	  	 
	     	      else
	     	    	  return -1; 
	     	    }
	     	    
	     	   if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	       if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength!=0)     	    	     
	     	    	     *length=*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength;	     	    	  	 
	     	       else
	     	    	   return -1; 	     	     
	     	    }  	   		     	         	
	     } 	
	     return 1;    	        	    	    	    
}

/**************************************************************************** 
* 函数名称 : GetFileBodyResponse
* 功能描述 : 获取文件的文件体响应 置于filebodyresponse
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : filebodyresponse 文件体响应Buf指针
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetFileBodyResponse(int vcardindexlistserialnumber, unsigned char (*filebodyresponse)[2]){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){
	     	       if (VcardIndexList[vcardindexlistserialnumber].ptr_filebodyresponse!=0){
	     	    	     (*filebodyresponse)[0]=*VcardIndexList[vcardindexlistserialnumber].ptr_filebodyresponse;
	     	    	     (*filebodyresponse)[1]=*(VcardIndexList[vcardindexlistserialnumber].ptr_filebodyresponse+1);
	     	    	        return 1;	
	     	    	 }
	     	       else
	     	    	   return -1; 
	     	    } 
	     	    
	     	    if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	       if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodyresponse!=0){
	     	    	     (*filebodyresponse)[0]=*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodyresponse;
	     	    	     (*filebodyresponse)[1]=*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodyresponse+1);
	     	    	        return 1;	
	     	    	 }
	     	       else
	     	    	   return -1; 
	     	    }  		 		     	         	
	     } 	   	        	    	    	    
}

/**************************************************************************** 
* 函数名称 : GetFileBodyLength
* 功能描述 : 获取文件的文件体长度 
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : 
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 长度  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetFileBodyLength(int vcardindexlistserialnumber){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	     if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){
	     	        if (VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength!=0){
	     	    	       return  ((unsigned short) (*VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength)<<8)+
	     	    	              (unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength+1));	
	     	    	  }  
	     	    else
	     	    	return -1;   	
	     	    }	
	     	    
	     	     if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	        if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength!=0){
	     	    	       return  ((unsigned short) (*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength)<<8)+
	     	    	              (unsigned short) (*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength+1));	
	     	    	  }  
	     	    else
	     	    	return -1;   	
	     	    }		 	     	         	
	     } 	   	        	    	    	    
}

/**************************************************************************** 
* 函数名称 : GetFileBodyContext
* 功能描述 : 获取文件的文件体内容，长度为length， 置于filebodybuf
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : filebodybuf 文件体Buf指针
* 参    数 : length 内容长度
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
//int Emu_Engine::GetFileBodyContext(int vcardindexlistserialnumber, unsigned char *filebodybuf, unsigned char length){  
int Emu_Engine::GetFileBodyContext(int vcardindexlistserialnumber, unsigned char *filebodybuf, unsigned short length){  	    
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
	     	      if (VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength!=0){
	     	    	    if (((unsigned short)*(VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength)<<8)+
	     	    	    	  (unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength+1))>=length)
	     	    	  	        memcpy(filebodybuf,VcardIndexList[vcardindexlistserialnumber].ptr_filebody ,length);	     	    	   
  	    	        else{
	     	    	  	  std::cout<<RED<<"GetFileBodyContext Fail Length OverFlow"<<RESET<<std::endl<<std::flush;
	     	    	  	  return -1;
	     	    	    }	        	
	     	    	}
	     	      else
	     	       	return -1;  
	     	    }	
	     	    
	     	     if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	      if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength!=0){
	     	    	    if (((unsigned short)*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength)<<8)+
	     	    	    	  (unsigned short) (*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength+1))>=length)
	     	    	  	        memcpy(filebodybuf,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebody ,length);	     	    	   
  	    	        else{
	     	    	  	  std::cout<<RED<<"GetFileBodyContext Fail Length OverFlow"<<RESET<<std::endl<<std::flush;
	     	    	  	  return -1;
	     	    	    }	        	
	     	    	}
	     	      else
	     	       	return -1;  
	     	    }	 		 		     	         	
	     } 	
	     return 1;    	        	    	    	    
}

int Emu_Engine::SetFileBodyContext(int vcardindexlistserialnumber, unsigned short offset,unsigned char *filebodybuf, unsigned short length){  	    
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
	     	      if (VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength!=0){
	     	    	    if (((unsigned short)*(VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength)<<8)+
	     	    	    	  (unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_filebodylength+1))>=(offset+length))
	     	    	  	      //  memcpy(filebodybuf,VcardIndexList[vcardindexlistserialnumber].ptr_filebody ,length);	
	     	    	  	      memcpy(VcardIndexList[vcardindexlistserialnumber].ptr_filebody+offset ,filebodybuf,length);     	    	   
  	    	        else{
	     	    	  	  std::cout<<RED<<"GetFileBodyContext Fail Length OverFlow"<<RESET<<std::endl<<std::flush;
	     	    	  	  return -1;
	     	    	    }	        	
	     	    	}
	     	      else
	     	       	return -1;  
	     	    }	
	     	    
	     	     if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	     	      if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength!=0){
	     	    	    if (((unsigned short)*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength)<<8)+
	     	    	    	  (unsigned short) (*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebodylength+1))>=(offset+length))
	     	    	  	     //   memcpy(filebodybuf,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebody ,length);	  
	     	    	  	     memcpy(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_filebody+offset,filebodybuf ,length);	    	    	   
  	    	        else{
	     	    	  	  std::cout<<RED<<"GetFileBodyContext Fail Length OverFlow"<<RESET<<std::endl<<std::flush;
	     	    	  	  return -1;
	     	    	    }	        	
	     	    	}
	     	      else
	     	       	return -1;  
	     	    }	 		 		     	         	
	     } 	
	     return 1;    	        	    	    	    
}


/**************************************************************************** 
* 函数名称 : GetFileRecordResponse
* 功能描述 : 获取文件的文件记录响应 filerecordresponse
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 : filerecordresponse 文件记录响应Buf指针
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetFileRecordResponse(int vcardindexlistserialnumber, unsigned char (*filerecordresponse)[2]){ 
	     
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
			     	    if (VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse!=0){
			     	    	   (*filerecordresponse)[0]=*VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse;
			     	    	   (*filerecordresponse)[1]=*(VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse+1);
			     	    	    return 1;	
			     	    }
			     	    else
			     	    	return -1;   
	     	    }	
	     	   if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
			     	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse!=0){
			     	    	   (*filerecordresponse)[0]=*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse;
			     	    	   (*filerecordresponse)[1]=*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse+1);
			     	    	    return 1;	
			     	    }
			     	    else
			     	    	return -1;   
	     	    }					     	         	
	     } 	   	        	    	    	    
} 


/**************************************************************************** 
* 函数名称 : GetRecordVolume
* 功能描述 : 获取Vcard中某文件的文件记录数 
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 :  
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 记录数量  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetRecordVolume(int vcardindexlistserialnumber){ 
unsigned short	 volume;  
int tmpt;  
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	     if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
	 	 	 	 	     	    if (VcardIndexList[vcardindexlistserialnumber].ptr_fileheader!=0){
	 	 	 	 	     	    	    if (VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	 	 	     	    	    	    if  (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength)>=15){
	 	 	 	 	     	    	    	           if (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+15-1)){	     	    	    	    	      
	 	 	 	 	     	    	    	    	           volume= (((unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+2))<<8)+
	 	 	 	 	     	    	                               (unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+3)))
	 	 	 	 	     	    	                                %
	 	 	 	 	     	    	                               (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+15-1));
	 	 	 	 	     	    	                       if (! volume)
	 	 	 	 	     	    	             	                  return  (((unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+2))<<8)+
	 	 	 	 	     	    	                                        (unsigned short) (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+3)))
	 	 	 	 	     	    	                                        /
	 	 	 	 	     	    	                                       (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+15-1)); 
	 	 	 	 	     	    	                       else          
	 	 	 	 	     	    	             	               return -1; 
	 	 	 	 	     	    	             	    }
	 	 	 	 	     	    	             	    else          
	 	 	 	 	     	    	             	               return -1;                 
	 	 	 	 	     	    	    	    	  
	 	 	 	 	     	    	    	    }
	 	 	 	 	     	    	    	    else
	 	 	 	 	     	    	               return -1;
	 	 	 	 	     	    	               	
	 	 	 	 	     	    	    }
	 	 	 	 	     	    	    else
	 	 	 	 	     	    	          return -1;
	 	 	 	 	     	    	    
	 	 	 	 	     	    	    		
	 	 	 	 	     	    }
	 	 	 	 	     	    else
	 	 	 	 	     	    	return -1;
	 	 	 	 	   }
	 	 	 	 	   
	 	 	 	 	   if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){ 
	 	 	 	 	     	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader!=0){
	 	 	 	 	     	    	
	 	 	 	 	     	    	
	 	 	 	 	     	    	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	 	 	     	    	    	
	 	 	 	 	     	    	    	       tmpt= GetTagLocation_inFCP(TAG_FILE_DESCRIPTOR_WITH_TEMPLATE,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader,\
	 	 	 	 	     	    	    	                 (int )((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength)-2));  //减去 tag 和 tlvlength
  						                     if (tmpt==-1)
  						                       	 return -1;
  						                 	   if (*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader+tmpt+1)!=5) //FILE_DESCRIPTOR_lengh=5 while no tranparent EF
  						                 	 	     return -1;
  						                 	 	  volume=*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader+tmpt+6);
  						                 	 	  if (volume)
  						                 	 	    return(volume);
	 	 	 	 	     	    	    	        else
	 	 	 	 	     	    	    	        	 return -1;	 	 	 	 	     	    	    	
	 	 	 	 	     	    	    }
	 	 	 	 	     	    	    else
	 	 	 	 	     	    	          return -1;
	 	 	 	 	     	    	    
	 	 	 	 	     	    	    		
	 	 	 	 	     	    }
	 	 	 	 	     	    else
	 	 	 	 	     	    	return -1;
	 	 	 	 	   } 	   		     	       	   		     	         	
	     } 	   	        	    	    	    
}

/**************************************************************************** 
* 函数名称 : GetRecordBodyLength
* 功能描述 : 获取Vcard中某文件的文件记录体大小 
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 :  
* 参    数 : 
* 参    数 : 
* 返 回 值 : 成功 返回 记录体大小  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetRecordBodyLength(int vcardindexlistserialnumber){  
int tmpt,volume; 
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	    if((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
				     	    if (VcardIndexList[vcardindexlistserialnumber].ptr_fileheader!=0){
				     	    	    if (VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse!=0){
				     	    	    	    if  (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheaderlength)>=15){
				     	    	    	           if (*(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+15-1))	     	    	    	    	           	    	    	    	           
				     	    	             	         return   *(VcardIndexList[vcardindexlistserialnumber].ptr_fileheader+15-1); 	     	    	                         	    	             	    
				     	    	             	   else          
				     	    	             	         return -1;                      	    	    	    	  
				     	    	    	    }
				     	    	    	    else
				     	    	               return -1;
				     	    	               	
				     	    	    }
				     	    	    else
				     	    	          return -1;	     	    	    	     	    	    		
				     	    }
				     	    else
				     	    	return -1;	     	    	   		     	         	
				     }
				     
				     if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
				     	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader!=0){
				     	    	 tmpt= GetTagLocation_inFCP(TAG_FILE_DESCRIPTOR_WITH_TEMPLATE,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader,\
	 	 	 	 	     	    	    	                 (int )((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheaderlength)-2));  //减去 tag 和 tlvlength
  						                     if (tmpt==-1)
  						                       	 return -1;
  						                 	   if (*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader+tmpt+1)!=5) //FILE_DESCRIPTOR_lengh=5 while no tranparent EF
  						                 	 	     return -1;
  						                 	 	  volume=(((unsigned short)(*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader+tmpt+4)))<<8)\
  						                 	 	         + (*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_fileheader+tmpt+5));
  						                 	 	  if (volume)
  						                 	 	    return(volume);
	 	 	 	 	     	    	    	        else
	 	 	 	 	     	    	    	        	 return -1;
				     	    					     	    	 	    	    	     	    	    		
				     	    }
				     	    else
				     	    	return -1;	     	    	   		     	         	
				     }
	   } 	   	        	    	    	    
}	     	    	            

/**************************************************************************** 
* 函数名称 : GetFileRecord
* 功能描述 : 获取Vcard中某文件的1条文件记录内容， 置于filerecordbuf
* 参    数 : vcardindexlistserialnumber ：索引表索引号 
* 参    数 :  recordserialnumber：记录号
* 参    数 :  filerecordbuf ：记录Buf
* 参    数 :  length 长度
* 返 回 值 : 成功 返回 1  不成功 返回 -1
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetFileRecord(int vcardindexlistserialnumber,unsigned char recordserialnumber, unsigned char *filerecordbuf, unsigned char length){ 
int recorditemnum, rerordvolume,recordlength	; 
unsigned char tmpilengh;
vcard_file_property_t file1;
unsigned char identifier[IDENTIFIERDEEPTH];   
       eae.ucReadRecordStatus=READRECORDSTATUS_NORMAL;
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	     if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
	 	 	     	    if (VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	     	    	     for (recorditemnum=0;recorditemnum<MAX_RECORD_VOLUME_PER_LCFILE;recorditemnum++){
	 	 	     	    	         if (VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
	 	 	     	    	         	     if (( *VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber==recordserialnumber)&&
	 	 	     	    	         	     	   ( *VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordlength==length)){
	 	 	     	    	         	     	   	   memcpy(filerecordbuf,VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordcontext ,length);
	 	 	     	    	         	             return 1;
	 	 	     	    	         	     }     	    	         	       	    	         	     	   
	 	 	     	    	         }
	 	 	     	    	         else
	 	 	     	    	         	   break;
	 	 	     	    	     }  
	 	 	     	    	     
	 	 	     	    	     if ((rerordvolume=GetRecordVolume(vcardindexlistserialnumber))<0)
	 	 	     	    	         	     return -1;
	 	 	     	    	     else{
	 	 	     	    	         	     if (rerordvolume<recordserialnumber){
	 	 	     	    	         	     	  eae.ucReadRecordStatus=READRECORDSTATUS_NUMBER_OVERFLOW;
	 	 	     	    	         	     	  return -1;
	 	 	     	    	         	     }
	 	 	     	    	     }
	 	 	     	    	     if ((recordlength=GetRecordBodyLength(vcardindexlistserialnumber))<0)
	 	 	     	    	         	      return -1;
	 	 	     	    	      else{
	 	 	     	    	         	     if (recordlength!=length)
	 	 	     	    	         	     	  return -1;
	 	 	     	    	      }	 
  	     	    	         	       
	 	 	     	    	     identifier[0]=*VcardIndexList[vcardindexlistserialnumber].ptr_identifier;
	 	 	     	    	     identifier[1]=*(VcardIndexList[vcardindexlistserialnumber].ptr_identifier+1);
	 	 	     	    	     if (GetVcardFileProperty(identifier, &file1)<0) 
	 	 	   	                  return -1;
	 	 	   	             if (((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_FF_LINEARFIXED_CYCLIC))||\
	 	 	   	             	((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_FF))
	 	 	   	             	
	 	 	   	             	){
	 	 	   	             	      if (file1.property&STARTBYTE_1_LINEARFIXED_CYCLIC){
	 	 	   	             	      	memset(filerecordbuf, 0, 1);
	 	 	   	             	      	memset(filerecordbuf+1, 0xff, length-1);
	 	 	   	             	      	}
	 	 	   	             	      
	 	 	   	             	      	else{
	 	 	   	             	      	 if (file1.property&STARTBYTE_2_LINEARFIXED_CYCLIC){
	 	 	   	             	           	memset(filerecordbuf, 0, 2);
	 	 	   	             	          	memset(filerecordbuf+2, 0xff, length-2);
	 	 	   	             	      	  }
	 	 	   	             	      	  else		
	 	 	   	             	             memset(filerecordbuf, 0xff, length);
	 	 	   	             	        }     
	 	 	   	             	      return 1;
	 	 	   	             	}
	 	 	   	             if (((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_0_LINEARFIXED_CYCLIC))||\
	 	 	   	             	((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_0))
	 	 	   	             	){
	 	 	   	             	      memset(filerecordbuf, 0, length);
	 	 	   	             	      return 1;
	 	 	   	             }	 
	 	 	   	             else
	 	 	     	    	       return -1; 	   	
	 	 	     	    	}
	 	 	     	    else
	 	 	     	    	return -1; 
	 	 	     	 }
	 	 	     	 
	 	 	     	 if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	 	 	     	    
	 	 	     	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse!=0){	 	 	     	    	
	 	 	     	    	     for (recorditemnum=0;recorditemnum<MAX_RECORD_VOLUME_PER_LCFILE;recorditemnum++){
	 	 	     	    	         if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
	 	 	     	    	         	     if (( *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber==recordserialnumber)&&
	 	 	     	    	         	     	   ( *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordlength==length)){
	 	 	     	    	         	     	   	   memcpy(filerecordbuf,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordcontext ,length);
	 	 	     	    	         	          //   eae.ucCurrentRecordProrityInVcard=RECORDPRORITYINVCARD_AVAILABLE;
	 	 	     	    	         	             return 1;
	 	 	     	    	         	     }     	    	         	       	    	         	     	   
	 	 	     	    	         }
	 	 	     	    	         else
	 	 	     	    	         	   break;
	 	 	     	    	     }  
	 	 	     	    	    //  eae.ucCurrentRecordProrityInVcard=RECORDPRORITYINVCARD_UNAVAILABLE;
	 	 	     	    	     if ((rerordvolume=GetRecordVolume(vcardindexlistserialnumber))<0)
	 	 	     	    	         	     return -1;
	 	 	     	    	     else{
	 	 	     	    	         	     if (rerordvolume<recordserialnumber){
	 	 	     	    	         	     	   eae.ucReadRecordStatus=READRECORDSTATUS_NUMBER_OVERFLOW;
	 	 	     	    	         	     	  return -1;
	 	 	     	    	         	     }
	 	 	     	    	     }
	 	 	     	    	     if ((recordlength=GetRecordBodyLength(vcardindexlistserialnumber))<0)
	 	 	     	    	         	      return -1;
	 	 	     	    	      else{
	 	 	     	    	         	     if (recordlength!=length)
	 	 	     	    	         	     	  return -1;
	 	 	     	    	      }	   	     	    	         	       
	 	 	     	    	      for ( tmpilengh=0;tmpilengh<(*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier_length);tmpilengh++)
	 	 	     	    	         identifier[tmpilengh]=*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier+tmpilengh);
	 	 	     	    	  //  if (GetVcardFileProperty_Usim((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier_length),identifier, &file1)<0)
	 	 	     	    	      if (GetVcardFileProperty_WithUsimRidType((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier_length),identifier,\
	 	 	     	    	      	       (*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_access_flag)>>5,          &file1)<0)
	                   
	                           return -1;                
	 	 	     	            if (((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_FF_LINEARFIXED_CYCLIC))||\
	 	 	     	            	((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_FF))
	 	 	     	            	
	 	 	     	            	){
	 	 	   	             	      if (file1.property&STARTBYTE_1_LINEARFIXED_CYCLIC){
	 	 	   	             	      	memset(filerecordbuf, 0, 1);
	 	 	   	             	      	memset(filerecordbuf+1, 0xff, length-1);
	 	 	   	             	      	}
	 	 	   	             	      else{
	 	 	   	             	      	 if (file1.property&STARTBYTE_2_LINEARFIXED_CYCLIC){
	 	 	   	             	           	memset(filerecordbuf, 0, 2);
	 	 	   	             	          	memset(filerecordbuf+2, 0xff, length-2);
	 	 	   	             	      	  }	
	 	 	   	             	         else	
	 	 	   	             	            memset(filerecordbuf, 0xff, length);
	 	 	   	             	      }   
	 	 	   	             	      return 1;
	 	 	   	             	}
	 	 	   	             if (((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_0_LINEARFIXED_CYCLIC))||\
	 	 	   	             	 ((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_0))
	 	 	   	             	){
	 	 	   	             	      memset(filerecordbuf, 0, length);
	 	 	   	             	      return 1;
	 	 	   	             }	 
	 	 	   	             else
	 	 	     	    	       return -1; 	   	
	 	 	     	    	}
	 	 	     	    else
	 	 	     	    	return -1; 
	 	 	     	 } 	 	  		     	         	
	     } 	
	     return 1;    	        	    	    	    
}

int Emu_Engine::SetFileRecord(int vcardindexlistserialnumber,unsigned char recordserialnumber, unsigned char *filerecordbuf, unsigned char length){ 
int recorditemnum, rerordvolume,recordlength	; 
unsigned char tmpilengh;
vcard_file_property_t file1;
unsigned char identifier[IDENTIFIERDEEPTH];    
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	     if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
	 	 	     	    if (VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	     	    	     for (recorditemnum=0;recorditemnum<MAX_RECORD_VOLUME_PER_LCFILE;recorditemnum++){
	 	 	     	    	         if (VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
	 	 	     	    	         	     if (( *VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber==recordserialnumber)&&
	 	 	     	    	         	     	   ( *VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordlength==length)){
	 	 	     	    	         	     	   	   memcpy(VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordcontext ,filerecordbuf,length);
	 	 	     	    	         	             return 1;
	 	 	     	    	         	     }     	    	         	       	    	         	     	   
	 	 	     	    	         }
	 	 	     	    	         else
	 	 	     	    	         	   break;
	 	 	     	    	     }  
	 	 	     	    	     return -1;
	 	 	     	    	     
	 	 	   	            
	 	 	   	      }	 
	 	 	   	      else
	 	 	     	    	      return -1; 	   	
	 	 	     	  
	 	 	       }
	 	 	     	 
	 	 	     	 if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	 	 	     	    
	 	 	     	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	     	    	
	 	 	     	    	     for (recorditemnum=0;recorditemnum<MAX_RECORD_VOLUME_PER_LCFILE;recorditemnum++){
	 	 	     	    	         if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
	 	 	     	    	         	     if (( *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber==recordserialnumber)&&
	 	 	     	    	         	     	   ( *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordlength==length)){
	 	 	     	    	         	     	   	   memcpy(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordcontext ,filerecordbuf,length);	 	 	     	    	         	         
	 	 	     	    	         	             return 1;
	 	 	     	    	         	     }     	    	         	       	    	         	     	   
	 	 	     	    	         }
	 	 	     	    	         else
	 	 	     	    	         	   break;
	 	 	     	    	     }  
	 	 	     	    	      return -1;   	    	         	       
	 	 	     	    	        
	 	 	   	               	
	 	 	     	    }
	 	 	     	    else
	 	 	     	    	return -1; 
	 	 	     	 } 	 	  		     	         	
	     } 	
	     return 1;    	        	    	    	    
}


int Emu_Engine::GetFileRecordWithCurrentFileProperty(int vcardindexlistserialnumber,unsigned char recordserialnumber, unsigned char *filerecordbuf, unsigned char length){ 
int recorditemnum, rerordvolume,recordlength	; 
unsigned char tmpilengh;
vcard_file_property_t file1;
unsigned char identifier[IDENTIFIERDEEPTH];   
       eae.ucReadRecordStatus=READRECORDSTATUS_NORMAL; 
	     if (!(vcardindexlistserialnumber>=0)&&(vcardindexlistserialnumber<MAX_VCARDINDEX))
	           return -1;
	     else{ 
	     	     if ((eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)||(eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)){ 
	 	 	     	    if (VcardIndexList[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	     	    	     for (recorditemnum=0;recorditemnum<MAX_RECORD_VOLUME_PER_LCFILE;recorditemnum++){
	 	 	     	    	         if (VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
	 	 	     	    	         	     if (( *VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber==recordserialnumber)&&
	 	 	     	    	         	     	   ( *VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordlength==length)){
	 	 	     	    	         	     	   	   memcpy(filerecordbuf,VcardIndexList[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordcontext ,length);
	 	 	     	    	         	             return 1;
	 	 	     	    	         	     }     	    	         	       	    	         	     	   
	 	 	     	    	         }
	 	 	     	    	         else
	 	 	     	    	         	   break;
	 	 	     	    	     }  
	 	 	     	    	     
	 	 	     	    	     if ((rerordvolume=GetRecordVolume(vcardindexlistserialnumber))<0)
	 	 	     	    	         	     return -1;
	 	 	     	    	     else{
	 	 	     	    	         	     if (rerordvolume<recordserialnumber){
	 	 	     	    	         	     	  eae.ucReadRecordStatus=READRECORDSTATUS_NUMBER_OVERFLOW;
	 	 	     	    	         	     	  return -1;
	 	 	     	    	         	     	}
	 	 	     	    	     }
	 	 	     	    	     if ((recordlength=GetRecordBodyLength(vcardindexlistserialnumber))<0)
	 	 	     	    	         	      return -1;
	 	 	     	    	      else{
	 	 	     	    	         	     if (recordlength!=length)
	 	 	     	    	         	     	  return -1;
	 	 	     	    	      }	 
  	     	             if (((eae.ucCurrentSimCardIdentifierProperty&SPARE_MODE_LINEARFIXED_CYCLIC)&&\
	 	 	   	             	(eae.ucCurrentSimCardIdentifierProperty&DEFAULT_FF_LINEARFIXED_CYCLIC))||\
	 	 	   	             	((eae.ucCurrentSimCardIdentifierProperty&SPARE_MODE_LINEARFIXED_CYCLIC)&&(eae.ucCurrentSimCardIdentifierProperty&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_FF))
	 	 	   	             	
	 	 	   	             	){
	 	 	   	             		if (eae.ucCurrentSimCardIdentifierProperty&STARTBYTE_1_LINEARFIXED_CYCLIC){
	 	 	   	             	   //   if (file1.property&STARTBYTE_1_LINEARFIXED_CYCLIC){
	 	 	   	             	      	memset(filerecordbuf, 0, 1);
	 	 	   	             	      	memset(filerecordbuf+1, 0xff, length-1);
	 	 	   	             	  }
	 	 	   	             	  else{
	 	 	   	             	      	 if (eae.ucCurrentSimCardIdentifierProperty&STARTBYTE_2_LINEARFIXED_CYCLIC){
	 	 	   	             	           	memset(filerecordbuf, 0, 2);
	 	 	   	             	          	memset(filerecordbuf+2, 0xff, length-2);
	 	 	   	             	      	  }	
	 	 	   	             	        
	 	 	   	             	         else	
	 	 	   	             	         memset(filerecordbuf, 0xff, length);
	 	 	   	             	        
	 	 	   	             	     }
	 	 	   	             	   return 1;   
	 	 	   	             	}
	 	 	   	              if (((eae.ucCurrentSimCardIdentifierProperty&SPARE_MODE_LINEARFIXED_CYCLIC)&&\
	 	 	   	            	(eae.ucCurrentSimCardIdentifierProperty&DEFAULT_0_LINEARFIXED_CYCLIC))||\
	 	 	   	            	((eae.ucCurrentSimCardIdentifierProperty&SPARE_MODE_LINEARFIXED_CYCLIC)&&(eae.ucCurrentSimCardIdentifierProperty&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_0))
	 	 	   	             	
	 	 	   	            	
	 	 	   	            	){
	 	 	   	             	      memset(filerecordbuf, 0, length);
	 	 	   	             	      return 1;
	 	 	   	              }	 
	 	 	   	              else
	 	 	     	    	       return -1; 	   	
	 	 	     	    	}
	 	 	     	    else
	 	 	     	    	return -1; 
	 	 	     	 }
	 	 	     	 
	 	 	     	 if (	   	             
	   	          (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
  	   	      (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
  	   				){
	 	 	     	    
	 	 	     	    if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordresponse!=0){
	 	 	     	    	     if (eae.ucFromEmuDataBuf[APDU_CMD_OFFSET_P3]!=0x26)
	 	 	     	    	     	   recorditemnum=0;
	 	 	     	    	     for (recorditemnum=0;recorditemnum<MAX_RECORD_VOLUME_PER_LCFILE;recorditemnum++){
	 	 	     	    	         if (VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber!=0){
	 	 	     	    	         	     if (( *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordserialnumber==recordserialnumber)&&
	 	 	     	    	         	     	   ( *VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordlength==length)){
	 	 	     	    	         	     	   	   memcpy(filerecordbuf,VcardIndexList_Usim[vcardindexlistserialnumber].ptr_recordbody[recorditemnum].ptr_recordcontext ,length);
	 	 	     	    	         	             return 1;
	 	 	     	    	         	     }     	    	         	       	    	         	     	   
	 	 	     	    	         }
	 	 	     	    	         else
	 	 	     	    	         	   break;
	 	 	     	    	     }  
	 	 	     	    	     
	 	 	     	    	     if ((rerordvolume=GetRecordVolume(vcardindexlistserialnumber))<0)
	 	 	     	    	         	     return -1;
	 	 	     	    	     else{
	 	 	     	    	         	     if (rerordvolume<recordserialnumber)
	 	 	     	    	         	     	  return -1;
	 	 	     	    	     }
	 	 	     	    	     if ((recordlength=GetRecordBodyLength(vcardindexlistserialnumber))<0)
	 	 	     	    	         	      return -1;
	 	 	     	    	      else{
	 	 	     	    	         	     if (recordlength!=length)
	 	 	     	    	         	     	  return -1;
	 	 	     	    	      }	   	     	    	         	       
	 	 	     	    	      for ( tmpilengh=0;tmpilengh<(*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier_length);tmpilengh++)
	 	 	     	    	         identifier[tmpilengh]=*(VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier+tmpilengh);
	 	 	     	    	  //  if (GetVcardFileProperty_Usim((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier_length),identifier, &file1)<0)
	 	 	     	    	       if (GetVcardFileProperty_WithUsimRidType((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_identifier_length),identifier,\
	 	 	     	    	      	       (*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_access_flag)>>5,          &file1)<0)
	                           return -1;                
	 	 	     	          if (((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_FF_LINEARFIXED_CYCLIC))||\
	 	 	     	          		((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_FF))
	 	 	     	            	
	 	 	     	          	){
	 	 	   	             	      if (file1.property&STARTBYTE_1_LINEARFIXED_CYCLIC){
	 	 	   	             	      	memset(filerecordbuf, 0, 1);
	 	 	   	             	      	memset(filerecordbuf+1, 0xff, length-1);
	 	 	   	             	      	}
	 	 	   	             	      
	 	 	   	             	      else{
	 	 	   	             	      	 if (file1.property&STARTBYTE_2_LINEARFIXED_CYCLIC){
	 	 	   	             	           	memset(filerecordbuf, 0, 2);
	 	 	   	             	          	memset(filerecordbuf+2, 0xff, length-2);
	 	 	   	             	      	  }	
	 	 	   	             	      	  else  
	 	 	   	             	            memset(filerecordbuf, 0xff, length);
	 	 	   	             	      }
	 	 	   	             	      return 1;
	 	 	   	            }
	 	 	   	            if (((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_0_LINEARFIXED_CYCLIC))||\
	 	 	   	            	((file1.property&SPARE_MODE_LINEARFIXED_CYCLIC)&&(file1.property&DEFAULT_UNKOWN_LINEARFIXED_CYCLIC)&&\
	 	 	     	            	(((*VcardIndexList_Usim[vcardindexlistserialnumber].ptr_access_flag)&0x1e)==VCARD_FLAG_LCEFREORDDEAULT_0))
	 	 	     	            	)
	 	 	   	            	{
	 	 	   	             	      memset(filerecordbuf, 0, length);
	 	 	   	             	      return 1;
	 	 	   	            }	 
	 	 	   	             else
	 	 	     	    	       return -1; 	   	
	 	 	     	    	}
	 	 	     	    else
	 	 	     	    	return -1; 
	 	 	     	 } 	 	  		     	         	
	     } 	
	     return 1;    	        	    	    	    
}

void Emu_Engine::TestVcardIndex(){
 int vcard_file_property_listnum,vcardindexlistserialnumber,length,recordvolume,recordlength;	
    unsigned char response[2],identifierresponse[2],fileheaderresponse[2],filebodyresponse[2],filerecordresponse[2];  
    unsigned char buf[255],filerecordbuf[255];
    
         std::cout<<std::endl<<std::flush; 
         for (vcard_file_property_listnum=0; vcard_file_property_listnum<vcard_file_property_list_num ;vcard_file_property_listnum++){
         	    if( (vcardindexlistserialnumber= GetIdentifierIndex(vcard_file_property_list[vcard_file_property_listnum].identifier))<0){
                     std::cout<<RED<<std::hex<<(unsigned short) vcard_file_property_list[vcard_file_property_listnum].identifier[0]<<
                     	(unsigned short)vcard_file_property_list[vcard_file_property_listnum].identifier[1]<<
                     	" not exist in Vcard"<<RESET<<std::endl<<std::flush;
         	    }
         	    else{        	    		
                     std::cout<<GREEN<<std::hex<<"VcardIndexList["<<vcardindexlistserialnumber<<"]   Identifier "<<(unsigned short) *VcardIndexList[vcardindexlistserialnumber].ptr_identifier<<
                     	(unsigned short)*(VcardIndexList[vcardindexlistserialnumber].ptr_identifier+1)<<RESET<<std::endl<<std::flush;                    		
                     
                     if (GetIdentifierResponse(vcardindexlistserialnumber,   &identifierresponse)<0)
                     	     std::cout<<RED<<std::hex<<" not exist identifierresponse in Vcard"<<RESET<<std::endl<<std::flush;                    	                     	
                     else
                         std::cout<<GREEN<<std::hex<<"identifierresponse "<<(unsigned short) identifierresponse[0]<<" "<<
                     	  (unsigned short)(unsigned short) identifierresponse[1]<<RESET<<std::endl<<std::flush;    	
                   
                    if (GetFileHeaderResponse(vcardindexlistserialnumber,   &fileheaderresponse)<0)
                     	     std::cout<<RED<<std::hex<<" not exist FileHeaderResponse in Vcard"<<RESET<<std::endl<<std::flush;                    	                     	
                     else
                         std::cout<<GREEN<<std::hex<<"FileHeaderResponse "<<(unsigned short) fileheaderresponse[0]<<" "<<
                     	  (unsigned short)(unsigned short) fileheaderresponse[1]<<RESET<<std::endl<<std::flush; 
                    
                    length=(unsigned short) identifierresponse[1]; 	
                    
                    if (GetFileHeaderContext(vcardindexlistserialnumber, buf,length )<0)
                    	 	  	 std::cout<<RED<<std::hex<<" GetFileHeaderContext Err"<<RESET<<std::endl<<std::flush; 
                    else	 	 
                     	  	for (int tmpp=0;tmpp<length;tmpp++)
                     	  	     std::cout<<GREEN<<std::hex<<" "<<(unsigned short)buf[tmpp];
                    std::cout<<std::endl<<std::flush;  
                    	
                    
                    if (GetFileBodyResponse(vcardindexlistserialnumber,   &filebodyresponse)<0)
                     	     std::cout<<RED<<std::hex<<" not exist filebodyresponse in Vcard"<<RESET<<std::endl<<std::flush;                    	                     	
                     else
                         std::cout<<GREEN<<std::hex<<"filebodyresponse "<<(unsigned short) filebodyresponse[0]<<" "<<
                     	  (unsigned short)(unsigned short) filebodyresponse[1]<<RESET<<std::endl<<std::flush; 
                     	  	
                    length= GetFileBodyLength(vcardindexlistserialnumber)	;
                    if (length<0)
                    	   std::cout<<RED<<std::hex<<" Filebody length Err"<<RESET<<std::endl<<std::flush;
                    else{
                    	    if (GetFileBodyContext(vcardindexlistserialnumber, buf,length )<0)
                    	 	 	   std::cout<<RED<<std::hex<<" GetFileBodyContext Err"<<RESET<<std::endl<<std::flush; 
                           else	{ 	 
                     	  	     for (int tmpp=0;tmpp<length;tmpp++)
                     	  	          std::cout<<GREEN<<std::hex<<" "<<(unsigned short)buf[tmpp];
                               std::cout<<std::endl<<std::flush;                     	
                           }   
                    }	
                   
                    if (GetFileRecordResponse(vcardindexlistserialnumber,   &filerecordresponse)<0)
                     	     std::cout<<RED<<std::hex<<" not exist filerecordresponse in Vcard"<<RESET<<std::endl<<std::flush;                    	                     	
                    else
                         std::cout<<YELLOW<<std::hex<<"filerecordresponse "<<(unsigned short) filerecordresponse[0]<<" "<<
                     	  (unsigned short)(unsigned short) filerecordresponse[1]<<RESET<<std::endl<<std::flush; 
                    
                    recordvolume= GetRecordVolume(vcardindexlistserialnumber);	                                   
                    if (recordvolume<0)
                     	     std::cout<<RED<<std::hex<<"  GetRecordVolume Err"<<RESET<<std::endl<<std::flush;                    	                     	
                    else
                         std::cout<<std::hex<<"GetRecordVolume: "<<(unsigned short) recordvolume<<RESET<<std::endl<<std::flush;
                     	   	
                    recordlength= GetRecordBodyLength( vcardindexlistserialnumber);                  
                    if (recordlength<0)
                     	     std::cout<<RED<<std::hex<<"  GetRecordLength Err"<<RESET<<std::endl<<std::flush;                    	                     	
                    else
                         std::cout<<std::hex<<"GetRecordLength: "<<(unsigned short) recordlength<<RESET<<std::endl<<std::flush;	
                    for (int tmprecordserialnumber=1; tmprecordserialnumber<=recordvolume; tmprecordserialnumber++) {  	
                             if (GetFileRecord(vcardindexlistserialnumber,  tmprecordserialnumber,  filerecordbuf, recordlength)<0){
                          	      std::cout<<RED<<std::hex<<" GetFileRecord Err"<<RESET<<std::endl<<std::flush; 
                          	 }
                          	 else	{ 	 
                     	  	         for (int tmpp=0;tmpp<recordlength;tmpp++)
                     	  	              std::cout<<GREEN<<std::hex<<" "<<(unsigned short)filerecordbuf[tmpp];
                                   std::cout<<std::endl<<std::flush; 
                             }      	   
                    }     	                           	                                              	                              	      
         	    }
         }
}   

// EmuAPDU Engine
/**************************************************************************** 
* 函数名称 : EmuApduEngineInit
* 功能描述 : 初始化 emu_apdu_engine_t
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :  返回 1  
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
//int Emu_Engine::EmuApduEngineInit(unsigned char optimizationmode){   
int Emu_Engine::EmuApduEngineInit(){    	
     memset(&eae, 0, sizeof(emu_apdu_engine_t));      
     return 1;
}   
int Emu_Engine::EmuApduEngineInitWithOptimizationMode(unsigned char optimizationmode ){    	
     memset(&eae, 0, sizeof(emu_apdu_engine_t));  
     SetOptimizationMode(optimizationmode);   
     return 1;
}  

//获取eae中 远端对应card 插拔状态
int Emu_Engine::GetRemoteSimCardPlugStatus(){
	  return eae.ucRemoteSimCardPlug ;
	}
int Emu_Engine::SetRemoteSimCardPlugStatus(unsigned char ucRemoteSimCardPlug){
	  eae.ucRemoteSimCardPlug=ucRemoteSimCardPlug ;
	  return 1;
	}

/**************************************************************************** 
* 函数名称 : GetEmuEngineStatus
* 功能描述 : 获取eae 状态， 外部操作者根据状态进行启Eae之前的预处理
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :  返回 eae 内部状态信息  
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::GetEmuEngineStatus(){
	  return eae.ucEmuEngineStatus ;
	}
	
/**************************************************************************** 
* 函数名称 : SetEmuEngineStatus
* 功能描述 : 设置eae 状态， 外部操作者操作后，通知eae状态变化
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :    
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/		
int Emu_Engine::SetEmuEngineStatus(unsigned char ucEmuEngineStatus){
	  eae.ucEmuEngineStatus=ucEmuEngineStatus ;
	  return 1;
	}		

/**************************************************************************** 
* 函数名称 : GetEmuApduNetApiStatus
* 功能描述 : 获取eae中的数据状态， 操作者根据状态信息决定对数据的操作
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :    
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/		
int Emu_Engine::GetEmuApduNetApiStatus(){
	  return eae.ucEmuApduNetApiStatus ;
	}

/**************************************************************************** 
* 函数名称 : SetEmuApduNetApiStatus
* 功能描述 : 设置数据状态， 操作者在数据准备好后设置数据状态后，再调用ProcessEmuDataCmd 处理数据
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :    
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/			
int Emu_Engine::SetEmuApduNetApiStatus(unsigned char ucEmuApduNetApiStatus){
	  eae.ucEmuApduNetApiStatus=ucEmuApduNetApiStatus ;
	  return 1;
	}				

void Emu_Engine::SetOptimizationMode(unsigned char optimizationmode ){
	  eae.ucOptimizationMode=optimizationmode;
	  std::cout<<GREEN<<" Optimizationmode: "<<(unsigned short)eae.ucOptimizationMode<<RESET<<std::endl;
}
/**************************************************************************** 
* 函数名称 : PreProcessEngine
* 功能描述 : 预处理引擎，改变ucEmuEngineStatus， 指导外部操作者的操作
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 : 成功 返回 1  不成功 返回 -1   
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/			
int Emu_Engine::PreProcessEngine(unsigned char ucvcardbuf[] ){
	if (eae.ucRemoteSimCardPlug==REMOTE_SIMCARD_IS_PULL_OUT){
         if (eae.ucEmuEngineStatus!=EMU_ENGINE_STOP){
         	   while (!(eae.ucEmuApduNetApiStatus==EMU_APDU_NETAPI_STATUS_IDLE||eae.ucEmuApduNetApiStatus==EMU_APDU_NETAPI_STATUS_ERROR));        	     
         	   //SetEmuEngineStatus( EMU_ENGINE_STOP); 
         	   EmuApduEngineInit(); 
         	 
         	   std::cout<<GREEN<<"EmuApduEngineInit. "<<RESET<<std::endl<<std::flush;	        	          	 
         }
         return 1;   		
	}else{ 
		    if (eae.ucRemoteSimCardPlug==REMOTE_SIMCARD_IS_PLUG_IN){
	          switch(eae.ucEmuEngineStatus){
  					
  		       case EMU_ENGINE_STOP: 
  		      	             eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_ATR;
  						             std::cout<<GREEN<<"Waiting atr ready"<<RESET<<std::endl<<std::flush;	
  						             return 1;			   		              		      
  		       case EMU_ENGINE_WAITING_ATR: 
  						             if (eae.ucEmuSimcardAtrReady==EMU_SIMCARD_ATR_IS_READY){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_DELIVER_ATR_TO_EMU;
  						                  std::cout<<GREEN<<"Waiting_deliver_atr_to_emu"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Emu_simcard_atr_is_not ready"<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             } 						             	 			             	     		                     		                      		                            
  		       case EMU_ENGINE_WAITING_DELIVER_ATR_TO_EMU: 
  						             if (eae.ucDeliverAtrToEmu==DELIVER_ATR_TO_EMU_IS_READY){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_TRANSMISSION_PROTOCOL;
  						                  std::cout<<GREEN<<"Waiting_transmission_protocol"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Deliver_atr_to_emu_is_not READY "<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             }
						 case EMU_ENGINE_WAITING_TRANSMISSION_PROTOCOL: 
  						             if (eae.ucTransmissionProtocol==TRANSMISSION_PROTOCOL_T0){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_VCARD;
  						                  std::cout<<GREEN<<"EMU_ENGINE_waiting_vcard"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             if (eae.ucTransmissionProtocol==TRANSMISSION_PROTOCOL_T1){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_EMU_DELIVER_RST_ICC;
  						                  std::cout<<GREEN<<"Waiting_emu_deliver_rst_icc"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Transmission_protocol_is_not ready "<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             }
						 case EMU_ENGINE_WAITING_VCARD: 
  						             if (eae.ucLocoalVCardReady==LOCOAL_VCARD_IS_READY){
  						             	    if (GenVcardIndexAll(ucvcardbuf)==-1){    
  						             	 //  if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardIndexList  ,  MAX_VCARDINDEX )==-1){    	                                 
    	                                std::cout<<RED<<"GenVcardIndex Fail"<<RESET<<std::endl<<std::flush; 
    	                                return -1;	
    	                         }
    	                         else{
    	                                eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_EMU_DELIVER_RST_ICC;
  						                        std::cout<<GREEN<<"Waiting_emu_deliver_rst_icc"<<RESET<<std::endl<<std::flush;	
  						                        return 1;
    	                         } 						             	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Locoal_vcard_is_not READY "<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             }
  		    
             default:
            	           break;  
          } 
	      }
	      else{
	            std::cout<<RED<<"GetRemoteSimCardPlugStatus Error"<<RESET<<std::endl<<std::flush;
	            return -1;   	
	      }	      	
	}   
}  

int Emu_Engine::PreProcessEngineWithOptimizationMode(unsigned char ucvcardbuf[],unsigned char optimizationmode ){
	if (eae.ucRemoteSimCardPlug==REMOTE_SIMCARD_IS_PULL_OUT){
         if (eae.ucEmuEngineStatus!=EMU_ENGINE_STOP){
         	   while (!(eae.ucEmuApduNetApiStatus==EMU_APDU_NETAPI_STATUS_IDLE||eae.ucEmuApduNetApiStatus==EMU_APDU_NETAPI_STATUS_ERROR));        	     
         	   //SetEmuEngineStatus( EMU_ENGINE_STOP); 
         	   EmuApduEngineInitWithOptimizationMode(optimizationmode); 
         	 
         	   std::cout<<GREEN<<"EmuApduEngineInit. "<<RESET<<std::endl<<std::flush;	        	          	 
         }
         return 1;   		
	}else{ 
		    if (eae.ucRemoteSimCardPlug==REMOTE_SIMCARD_IS_PLUG_IN){
	          switch(eae.ucEmuEngineStatus){
  					
  		       case EMU_ENGINE_STOP: 
  		      	             eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_ATR;
  						             std::cout<<GREEN<<"Waiting atr ready"<<RESET<<std::endl<<std::flush;	
  						             return 1;			   		              		      
  		       case EMU_ENGINE_WAITING_ATR: 
  						             if (eae.ucEmuSimcardAtrReady==EMU_SIMCARD_ATR_IS_READY){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_DELIVER_ATR_TO_EMU;
  						                  std::cout<<GREEN<<"Waiting_deliver_atr_to_emu"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Emu_simcard_atr_is_not ready"<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             } 						             	 			             	     		                     		                      		                            
  		       case EMU_ENGINE_WAITING_DELIVER_ATR_TO_EMU: 
  						             if (eae.ucDeliverAtrToEmu==DELIVER_ATR_TO_EMU_IS_READY){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_TRANSMISSION_PROTOCOL;
  						                  std::cout<<GREEN<<"Waiting_transmission_protocol"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Deliver_atr_to_emu_is_not READY "<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             }
						 case EMU_ENGINE_WAITING_TRANSMISSION_PROTOCOL: 
  						             if (eae.ucTransmissionProtocol==TRANSMISSION_PROTOCOL_T0){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_VCARD;
  						                  std::cout<<GREEN<<"EMU_ENGINE_waiting_vcard"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             if (eae.ucTransmissionProtocol==TRANSMISSION_PROTOCOL_T1){
  						             	    eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_EMU_DELIVER_RST_ICC;
  						                  std::cout<<GREEN<<"Waiting_emu_deliver_rst_icc"<<RESET<<std::endl<<std::flush;	
  						                  return 1;	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Transmission_protocol_is_not ready "<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             }
						 case EMU_ENGINE_WAITING_VCARD: 
  						             if (eae.ucLocoalVCardReady==LOCOAL_VCARD_IS_READY){
  						             	    if (GenVcardIndexAll(ucvcardbuf)==-1){    
  						             	 //  if (GenVcardIndex( ucvcardbuf ,   VCARD_MAX_LENGTH, VcardIndexList  ,  MAX_VCARDINDEX )==-1){    	                                 
    	                                std::cout<<RED<<"GenVcardIndex Fail"<<RESET<<std::endl<<std::flush; 
    	                                return -1;	
    	                         }
    	                         else{
    	                                eae.ucEmuEngineStatus=EMU_ENGINE_WAITING_EMU_DELIVER_RST_ICC;
  						                        std::cout<<GREEN<<"Waiting_emu_deliver_rst_icc"<<RESET<<std::endl<<std::flush;	
  						                        return 1;
    	                         } 						             	
  						             }
  						             else{
  						             	    std::cout<<RED<<"Locoal_vcard_is_not READY "<<RESET<<std::endl<<std::flush;	
  						                  return -1;	
  						             }
  		    
             default:
            	           break;  
          } 
	      }
	      else{
	            std::cout<<RED<<"GetRemoteSimCardPlugStatus Error"<<RESET<<std::endl<<std::flush;
	            return -1;   	
	      }	      	
	}   
}

/**************************************************************************** 
* 函数名称 : VerifyCommandFromEmu
* 功能描述 : 检查来自Emu的命令是否能识别
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 : 成功 返回 1  命令类型不为 0xA0  返回 -1   命令值无法识别  返回 -2 ，命令长度不为5   返回 -3 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/		
int Emu_Engine::VerifyCommandFromEmu() {
	//TODO
	 //20181029
	// if (!((eae.ucCurrentEaeCapability&VC_DATA_SIMMODE_MAINTAIN)||(eae.ucCurrentEaeCapability&VC_DATA_UIMMODE_MAINTAIN)))
	//20200330 处理Invaild card
	if (!((eae.ucCurrentEaeCapability&VG_HANDLEICC_SIMMODE_MAINTAIN)||(eae.ucCurrentEaeCapability&VG_HANDLEICC_UIMMODE_MAINTAIN))) 
	 	  if (eae.ucFromEmuDataBuf[OFFSET_CLA]==0XA0)
	 	             return -1;
	 	             
	// if (!(eae.ucCurrentEaeCapability&VC_DATA_USIMMODE_MAINTAIN))
	//20200330 处理Invaild card
	if (!(eae.ucCurrentEaeCapability&VG_HANDLEICC_USIMMODE_MAINTAIN))
	 	  if (eae.ucFromEmuDataBuf[OFFSET_CLA]==0X0)
	 	             return -1;
	 
	 if ((eae.ucFromEmuDataBuf[OFFSET_CLA]==0X0)&&(eae.ucFromEmuDataBuf[OFFSET_INS]==0X0))  //硬件故障时 可能有 0 0 0 0 0 
	 	    return -1;
	  eae.ucCurrentProcessLogicalChannel=0;	    
	 if (eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_USIM_UIM_ENBALE){
	 	  if (!((eae.ucFromEmuDataBuf[OFFSET_CLA]==0XA0)||(eae.ucFromEmuDataBuf[OFFSET_CLA]==0X0)||(eae.ucFromEmuDataBuf[OFFSET_CLA]==0X80)))
	 	      return -1;
	 }
	 else{
	 	      if (eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY){
	 	         if (eae.ucFromEmuDataBuf[OFFSET_CLA]!=0XA0)
	 	             return -1;
	        }
	        else{ 
	        	if (eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY){
	        		   if (!((eae.ucFromEmuDataBuf[OFFSET_CLA]==0XA0)||(eae.ucFromEmuDataBuf[OFFSET_CLA]==0X80)))
	 	             return -1;
	        		}
	          else{
	          	
	              if (
	        	    	  (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\
	        	    	  (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
	        	    	 )
	        	   
	        	    {
	 	                   //logical channel iso7816-4  5.1.1
	 	                if 	 (((eae.ucFromEmuDataBuf[OFFSET_CLA]&0xF)>=1)&&((eae.ucFromEmuDataBuf[OFFSET_CLA]&0xF)<=3)){
	 	                     //  eae.ucCurrentProcessLogicalChannel=1;   
	 	                       eae.ucCurrentProcessLogicalChannel=eae.ucFromEmuDataBuf[OFFSET_CLA]&0xF; 
	 	                	}
	 	                	else{
	 	                      
	 	                    if (!((eae.ucFromEmuDataBuf[OFFSET_CLA]==0X0)||(eae.ucFromEmuDataBuf[OFFSET_CLA]==0X80)))
	 	                       return -1;
	 	                  }
	 	                 
	              }
	 	        }
	 	    }
	 	}     
	 
	 switch (eae.ucFromEmuDataBuf[OFFSET_INS]){	  
	 	    case APDU_COMMAND_SELECT:           
  	    case APDU_COMMAND_STATUS:           
  	    case APDU_COMMAND_READ_BINARY:      
  	    case APDU_COMMAND_UPDATE_BINARY:    
  	    case APDU_COMMAND_READ_RECORD:      
  	    case APDU_COMMAND_UPDATE_RECORD:    
  	    case APDU_COMMAND_SEEK:             
  	    case APDU_COMMAND_INCREASE:         
  	    case APDU_COMMAND_VERIFY_CHV:       
  	    case APDU_COMMAND_CHANGE_CHV:       
  	    case APDU_COMMAND_DISABLE_CHV:      
  	    case APDU_COMMAND_ENABLE_CHV:       
  	    case APDU_COMMAND_UNBLOCK_CHV:      
  	    case APDU_COMMAND_INVALIDATE:      
	 	    case APDU_COMMAND_REHABILITATE:     
  	    case APDU_COMMAND_RUN_GSM_ALGORITHM:
  	    case APDU_COMMAND_SLEEP:            
  	    case APDU_COMMAND_GET_RESPONSE:     
  	    case APDU_COMMAND_TERMINAL_PROFILE: 
  	    case APDU_COMMAND_ENVELOPE:         
  	    case APDU_COMMAND_FETCH:            
  	    case APDU_COMMAND_TERMINAL_RESPONSE:
  	    case APDU_COMMAND_GET_CHALLENGE: 
  	    case APDU_COMMAND_TERMINAL_CAPABILITY:         
  	    case APDU_COMMAND_MANAGE_SECURE_CHANNEL:            
  	    case APDU_COMMAND_TRANSACT_DATA: 	
  	    	
  	    	
  	    case APDU_COMMAND_UIM_STORE_ESN_ME: 
  	    case APDU_COMMAND_UIM_BASE_STATION_CHALLENGE:  	
  	    case APDU_COMMAND_UIM_CONFIRM_SSD: 
  	    case APDU_COMMAND_UIM_GENERATE_KEY_VPM:
  	    case APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION:
  	    case APDU_COMMAND_MANAGE_CHANNEL:

  	    case APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY:   
        case APDU_COMMAND_UIM_KEY_GENERATION_REQUEST:
        case APDU_COMMAND_UIM_CONFIGURATION_REQUEST:
        case APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST:   
        case APDU_COMMAND_UIM_OTAPA_REQUEST:          
        case APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST: 
        case APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST:      
        case APDU_COMMAND_UIM_VALID:                  
        case APDU_COMMAND_UIM_COMMIT:                 

  	    	   break;  	   
  	    default: 
	 	         return -2;	  
	 	    }       
	 if (eae.usFromEmuDataBufLength!=5)
	 	   return -3;
	 return 1;	
	} 
	
//检查来自Emu的命令参数	
int Emu_Engine::VerifyArgumentFromEmu() {
	//TODO
	 return 1;	
	} 	 
/**************************************************************************** 
* 函数名称 : VerifyDataFromEmu
* 功能描述 : 检查来自Emu的命令是否能识别，参数是否正确
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 : 成功 返回 1 不正确  返回 -1 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/		
int Emu_Engine::VerifyDataFromEmu(){
	  if (eae.ucCurrentExpect==EXPECT_IS_COMMAND){
	  	    if (VerifyCommandFromEmu()<0)
	  	    	 return -1;  
	  	}
	  else{
	  	   if (eae.ucCurrentExpect==EXPECT_IS_ARGUMENT_DATA){
	  	   	     if (VerifyArgumentFromEmu()<0)
	  	    	         return -1;  
	      	}
	      else{
	            std::cout<<RED<<"DataFromEmu can not identification"<<RESET<<std::endl<<std::flush;
	            return -1;   	
	      }	     	      		
	  	}	  	
	  return 1;	
}

/**************************************************************************** 
* 函数名称 : DirectTransfer_EmuToNet
* 功能描述 : 将来自Emu 的数据直接转发给网络侧Buf
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 : 成功 返回 1 不正确  返回 -1 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::DirectTransfer_EmuToNet(){
	   if (!((eae.ucFromEmuDataProperty==DATA_PROPERTY_IS_DATA)||(eae.ucFromEmuDataProperty==DATA_PROPERTY_IS_REQ_RST_ICC))){
	   	      std::cout<<RED<<"Property is Err"<<RESET<<std::endl<<std::flush;
	   	      return -1; 	
	   	}
     eae.usToNetDataBufLength=eae.usFromEmuDataBufLength;
	   memcpy(eae.ucToNetDataBuf,eae.ucFromEmuDataBuf,eae.usFromEmuDataBufLength);  	   	
	   eae.ucToNetDataProperty=eae.ucFromEmuDataProperty;	   
	   eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  
	   return 1;  
}


int Emu_Engine::CopyApduCommandToEngine(){
      memcpy(eae.ucLastCommand,eae.ucCurrentCommand,eae.ucCurrentCommandLength);	
	    eae.ucLastCommandLength=eae.ucCurrentCommandLength;
	    memcpy(eae.ucLastArgument,eae.ucCurrentArgument,eae.ucCurrentArgumentLength);	
	    eae.ucLastArgumentLength=eae.ucCurrentArgumentLength;
	    
	    memcpy(eae.ucCurrentCommand,eae.ucFromEmuDataBuf,eae.usFromEmuDataBufLength);	
	    eae.ucCurrentCommandLength=eae.usFromEmuDataBufLength;
	    eae.ucCurrentArgumentLength=0;
	    return 1;
}
int Emu_Engine::CopyApduArgumentToEngine(){
       eae.ucCurrentArgumentLength=eae.usFromEmuDataBufLength;
      memcpy(eae.ucCurrentArgument,eae.ucFromEmuDataBuf,eae.usFromEmuDataBufLength);
	    return 1;
}

/**************************************************************************** 
* 函数名称 : TransferEaeToEmu
* 功能描述 : 将数据准备传送给Emu
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 : 成功 返回 1 不正确  返回 -1 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::TransferEaeToEmu(unsigned char *buf, unsigned short length ){
int tmpCounter;	
	   if ( length> APDU_BUF_MAX_LENGTH){
	   	      std::cout<<RED<<"Length Overflow"<<RESET<<std::endl<<std::flush;
	   	      return -1;
	   	}
	   	else{
	   		  memcpy(eae.ucToEmuDataBuf,buf,length);	
	        eae.usToEmuDataBufLength=length;	       
	        eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_DATA_TO_EMU_IS_READY;
	    }
	    return 1;
}

// 将INS准备传送给Emu 
int Emu_Engine::TransferEaeToEmu_INS(){
	    TransferEaeToEmu(&eae.ucCurrentCommand[1],1);
      return 1;
}


int Emu_Engine::TransferEaeToNet_NetApi_Select(){
	     eae.ucToNetDataBuf[0]=APDU_COMMAND_SELECT;
	     eae.ucToNetDataBuf[1]=eae.ucCurrentSimCardIdentifier[0];
	     eae.ucToNetDataBuf[2]=eae.ucCurrentSimCardIdentifier[1];
	     eae.ucToNetDataBuf[3]=eae.ucExpectIdentifier[0];
	     eae.ucToNetDataBuf[4]=eae.ucExpectIdentifier[1];
	     eae.usToNetDataBufLength=5;
	     eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	     eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  
	     return 1;
}	
// 带路径传输命令中， 路径选择原则： 传送的路径没有歧义。 实现方法：不存在重名文件时直接传送文件名，以减少网络传输数据量；
// 存在重名文件或非标文件时，路径选择以能唯一识别为标准； 文件解析由Bank完成。
/**************************************************************************** 
* 函数名称 : TransferEaeToNet
* 功能描述 : 产生NetApi 命令， 准备传送给网络侧
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 : 成功 返回 1 不正确  返回 -1 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::TransferEaeToNet(int netpaiproperty){ 
	   unsigned char get_response[5]={0xa0,0xc0,0,0,0}; 
	    switch (netpaiproperty){
	    	case NETAPI_LASTCMD_LASTARGU_CURRENTCMD:
	    		   if (NETAPIBUFMAXLENGTH<eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	
	    		   else{
	    		   			memcpy(eae.ucToNetDataBuf,eae.ucLastCommand,eae.ucLastCommandLength);
	    	    			memcpy(&(eae.ucToNetDataBuf[eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  eae.usToNetDataBufLength=eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;
	    	    		  
	    	     }
	    	     break;
	    	case NETAPI_SELECT_CURRENTIDENTIFIER_LASTCMD_LASTARGU_CURRENTCMD:       //2017.5.26
	    		   if (NETAPIBUFMAXLENGTH<5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	
	    		   else{
	    		   			memcpy(eae.ucToNetDataBuf,command_select,5);
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,2);
	    	    			memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucLastCommand,eae.ucLastCommandLength);
	    		   		  memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
	    	    			memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  eae.usToNetDataBufLength=5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;    	    		  
	    	     }
	    	     break;     
	      case NETAPI_CURRENTCMD:	 
	      	   memcpy(eae.ucToNetDataBuf,eae.ucCurrentCommand,eae.ucCurrentCommandLength); 
	      	   eae.usToNetDataBufLength= eae.ucCurrentCommandLength;
	      	   break;
	    	
	    	case NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU:
	    		   if (NETAPIBUFMAXLENGTH<5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	     
	    		   			//std::cout<<std::hex <<" ucExpectIdentifier:"<<(unsigned short)eae.ucExpectIdentifier[0]<<" "<<(unsigned short)eae.ucExpectIdentifier[1]<<std::endl; 
	    		   			memcpy(eae.ucToNetDataBuf,command_select,5);
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,2);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;    	    		  
	    	     }
	    	     break;
	    	     
	    	 case NETAPI_SELECT_CURRENTIDENTIFIER_GET_RESPONSE_CURRENTCMD_CURRENTARGU:
	    		   if (NETAPIBUFMAXLENGTH<5+2+5+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	     
	    		   			//std::cout<<std::hex <<" ucExpectIdentifier:"<<(unsigned short)eae.ucExpectIdentifier[0]<<" "<<(unsigned short)eae.ucExpectIdentifier[1]<<std::endl; 
	    		   			memcpy(eae.ucToNetDataBuf,command_select,5);
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,2);
	    	    			get_response[4]=eae.ucCurrentExpectFileHeaderLength;
	    	    			memcpy(&(eae.ucToNetDataBuf[5+2]),get_response,5);
	    	    			 
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2+5+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+2+5+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;    	    		  
	    	     }
	    	     break;     
	    	case NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD:
	    		   if (NETAPIBUFMAXLENGTH<5+2+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{
	    		   			memcpy(eae.ucToNetDataBuf,command_select,5);
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,2);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		 // memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+2+eae.ucCurrentCommandLength;    	    		  
	    	     }
	    	     break;   
	    	     
	    	case NETAPI_CURRENTCMD_CURRENTARGU:	 
	      	   memcpy(eae.ucToNetDataBuf,eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	      	   memcpy(&(eae.ucToNetDataBuf[eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	      	   eae.usToNetDataBufLength=eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength; 
	      	   break;
	      	   
	      case NETAPI_SELECT_PATH_FIRSTLEVEL_SELECT_CURRENTCMD_CURRENTARGU:
	    		   if (NETAPIBUFMAXLENGTH<5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	     
	    		   			//std::cout<<std::hex <<" ucExpectIdentifier:"<<(unsigned short)eae.ucExpectIdentifier[0]<<" "<<(unsigned short)eae.ucExpectIdentifier[1]<<std::endl; 
	    		   			memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   			memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentIdWithPath.firstlevelid,2);
	    		   		//	memcpy(&eae.ucToNetDataBuf[5+2],command_select,5);
	    	    		//	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.ucCurrentSimCardIdentifier,2);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;    	    		  
	    	     }
	    	     break;
	    	 case NETAPI_SELECT_PATH_FIRSTLEVEL_SECONDLEVEL_CURRENTCMD_CURRENTARGU:
	    		   if (NETAPIBUFMAXLENGTH<5+2+5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	     
	    		   			//std::cout<<std::hex <<" ucExpectIdentifier:"<<(unsigned short)eae.ucExpectIdentifier[0]<<" "<<(unsigned short)eae.ucExpectIdentifier[1]<<std::endl; 
	    		   			memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   			memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentIdWithPath.firstlevelid,2);
	    		   			memcpy(&eae.ucToNetDataBuf[5+2],command_select,5);
	    		   			memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentIdWithPath.secondlevelid,2);
	    		   		//	memcpy(&eae.ucToNetDataBuf[5+2+5+2],command_select,5);
	    	    		//	memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5]),eae.ucCurrentSimCardIdentifier,2);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+2+5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+2+5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;    	    		  
	    	     }
	    	     break;
	    	     
	    	     case NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD:
	    	     	if  (eae.CurrentIdWithPath.secondlevelid[0]==FILE_1ST_ID_2LEVEL_DF){
	    	     		   if (NETAPIBUFMAXLENGTH<5+2+5+2+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		         }	      	 
	    		         else{
	    		   					memcpy(eae.ucToNetDataBuf,command_select,5);
	    	    					memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentIdWithPath.firstlevelid,2);
	    	    					memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    	    					memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentIdWithPath.secondlevelid,2);
	    	    				  memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    				 // memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    				  eae.usToNetDataBufLength=5+2+5+2+eae.ucCurrentCommandLength;    	    		  
	    	           }	    	     		
	    	     		}    	      
	    	     break;  
	    	     
	    	     case NETAPI_SELECT_ALLTYPEPATH_CURRENTCMD_CURRENTARGU:
	    	     	  
	    		   if (NETAPIBUFMAXLENGTH<eae.CurrentId_AllType.length*7+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	    if (eae.CurrentId_AllType.length==1){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	    		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		      eae.usToNetDataBufLength=5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;
	    	    		  }
	    	    		  else 
	    	    		  if (eae.CurrentId_AllType.length==2){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	 
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentId_AllType.identifier2,2);	       		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		      eae.usToNetDataBufLength=5+2+5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;
	    	    		  }
	    	    		  if (eae.CurrentId_AllType.length==3){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	 
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentId_AllType.identifier2,2);	 
	    		   		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5]),eae.CurrentId_AllType.identifier3,2);	       		      		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		      eae.usToNetDataBufLength=5+2+5+2+5+2+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;
	    	    		  }        	    		  
	    	     }
	    	     break;
	    	     	    	     
	    	     case NETAPI_SELECT_ALLTYPEPATH__LASTCMD_LASTARGU_CURRENTCMD:
	    	     	  
	    		   if (NETAPIBUFMAXLENGTH<eae.CurrentId_AllType.length*7+\
	    		   	eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	    if (eae.CurrentId_AllType.length==1){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	    		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucLastCommand,eae.ucLastCommandLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      eae.usToNetDataBufLength=5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;
	    	    		  }
	    	    		  else 
	    	    		  if (eae.CurrentId_AllType.length==2){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	 
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentId_AllType.identifier2,2);	  
	    		   		     	
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),eae.ucLastCommand,eae.ucLastCommandLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      eae.usToNetDataBufLength=5+2+5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;

	    	    		  }
	    	    		  if (eae.CurrentId_AllType.length==3){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	 
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentId_AllType.identifier2,2);	 
	    		   		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5]),eae.CurrentId_AllType.identifier3,2);	
	    		   		     	
	    		   		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5+2]),eae.ucLastCommand,eae.ucLastCommandLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5+2+eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      eae.usToNetDataBufLength=5+2+5+2+5+2+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;
	    	    		  }        	    		  
	    	     }
	    	     break;
	    	     
	    	     case NETAPI_SELECT_ALLTYPEPATH_CURRENTCMD:
	    	     	  
	    		   if (NETAPIBUFMAXLENGTH<eae.CurrentId_AllType.length*7+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  
	    		   	    if (eae.CurrentId_AllType.length==1){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	    		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		    //  memcpy(&(eae.ucToNetDataBuf[5+2+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		      eae.usToNetDataBufLength=5+2+eae.ucCurrentCommandLength;
	    	    		  }
	    	    		  else 
	    	    		  if (eae.CurrentId_AllType.length==2){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	 
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentId_AllType.identifier2,2);	       		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      eae.usToNetDataBufLength=5+2+5+2+eae.ucCurrentCommandLength;
	    	    		  }
	    	    		  if (eae.CurrentId_AllType.length==3){ 
	    		   		  		memcpy(eae.ucToNetDataBuf,command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5]),eae.CurrentId_AllType.identifier1,2);	 
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5]),eae.CurrentId_AllType.identifier2,2);	 
	    		   		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2]),command_select,5);
	    		   		     	memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5]),eae.CurrentId_AllType.identifier3,2);	       		      		   			
	    	    		      memcpy(&(eae.ucToNetDataBuf[5+2+5+2+5+2]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		      eae.usToNetDataBufLength=5+2+5+2+5+2+eae.ucCurrentCommandLength;
	    	    		  }        	    		  
	    	     }
	    	     break;
	    	      	    	     	      	   
	    	}
	    	
	    eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	    eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  	   
	    return 1;
}
int Emu_Engine::TransferEaeToNetSide_NoDeliver(int netpaiproperty){ 	
	    if (	netpaiproperty== NETAPI_CURRENTCMD){   		      
	      	   memcpy(eae.ucToNetDataBuf,eae.ucCurrentCommand,eae.ucCurrentCommandLength); 
	      	   eae.usToNetDataBufLength= eae.ucCurrentCommandLength;	
	      	   eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_NODELIVER2NET;
	           eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;
	    	}	
	     return 1;    	    		   
}

////sx
int Emu_Engine::IsCurrentIdentifierMF(){
	if (!eae.ucCurrentProcessLogicalChannel){
	    if ((eae.ucCurrentSimCardIdentifierLength==2)&&(eae.ucCurrentSimCardIdentifier[0]==0x3f)&&(eae.ucCurrentSimCardIdentifier[1]==0x0))
	    	  return 1;
	    else
	    	 return 0;
	 }
	 else{
	 	     if ((eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength==2)&&\
	 	     	(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==0x3f)&&\
	 	     	(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1]==0x0))
	    	     return 1;
	       else
	    	    return 0;	 	
	 }
}	

int Emu_Engine::TransferEaeToNet_Usim(int netpaiproperty){ 
	     
	    switch (netpaiproperty){
	    	case NETAPI_LASTCMD_LASTARGU_CURRENTCMD:
	    		   if (NETAPIBUFMAXLENGTH<eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	
	    		   else{
	    		   			memcpy(eae.ucToNetDataBuf,eae.ucLastCommand,eae.ucLastCommandLength);
	    	    			memcpy(&(eae.ucToNetDataBuf[eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  eae.usToNetDataBufLength=eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;
	    	    		  
	    	     }
	    	     break;
	    	   
	    	case NETAPI_SELECT_CURRENTIDENTIFIER_LASTCMD_LASTARGU_CURRENTCMD:       //2017.5.26
	    		   if (!eae.ucCurrentProcessLogicalChannel){
           		   if (NETAPIBUFMAXLENGTH<5+eae.ucCurrentSimCardIdentifierLength+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength){
           		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
           		   	      return -1;
           		   }	      	
           		   else{
           		   		//wx	usimselect_ByFIReturnFcpForSelectMF
           		   			if(IsCurrentIdentifierMF())
           		   				  memcpy(eae.ucToNetDataBuf,usimselect_ByFIReturnFcpForSelectMF,5);
           		   			else{	  
           		   		      	memcpy(eae.ucToNetDataBuf,usimselect_pathfromMfReturnFcp,4);
           		   		      	eae.ucToNetDataBuf[4]=eae.ucCurrentSimCardIdentifierLength;
           		   		  }
           	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,eae.ucCurrentSimCardIdentifierLength);
           	    			memcpy(&(eae.ucToNetDataBuf[5+eae.ucCurrentSimCardIdentifierLength]),eae.ucLastCommand,eae.ucLastCommandLength);
           		   		  memcpy(&(eae.ucToNetDataBuf[5+eae.ucCurrentSimCardIdentifierLength+eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
           	    			memcpy(&(eae.ucToNetDataBuf[5+eae.ucCurrentSimCardIdentifierLength+eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
           	    		  eae.usToNetDataBufLength=5+eae.ucCurrentSimCardIdentifierLength+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;    	    		  
           	     }
	    	     }
	    	     else{
	    	     	        if (NETAPIBUFMAXLENGTH<5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength){
           		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
           		   	      return -1;
           		        }	      	
           		        else{
           		   		
           		   			if(IsCurrentIdentifierMF())
           		   				  memcpy(eae.ucToNetDataBuf,usimselect_ByFIReturnFcpForSelectMF,5);           		   				  
           		   				
           		   			else{	  
           		   		      	memcpy(eae.ucToNetDataBuf,usimselect_pathfromMfReturnFcp,4);
           		   		      	eae.ucToNetDataBuf[4]=eae.ucCurrentSimCardIdentifierLength;
           		   		  }
           		   		  eae.ucToNetDataBuf[0]|=eae.ucCurrentProcessLogicalChannel; //逻辑通道号
           		   		  
           	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength);
           	    			memcpy(&(eae.ucToNetDataBuf[5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength]),eae.ucLastCommand,eae.ucLastCommandLength);
           		   		  memcpy(&(eae.ucToNetDataBuf[5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucLastCommandLength]),eae.ucLastArgument,eae.ucLastArgumentLength);
           	    			memcpy(&(eae.ucToNetDataBuf[5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucLastCommandLength+eae.ucLastArgumentLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
           	    		  eae.usToNetDataBufLength=5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucLastCommandLength+eae.ucLastArgumentLength+eae.ucCurrentCommandLength;    	    		  
           	     }
	    	     }
	    	     	
	    	     break;     
	      case NETAPI_CURRENTCMD:	 
	      	   memcpy(eae.ucToNetDataBuf,eae.ucCurrentCommand,eae.ucCurrentCommandLength); 
	      	   eae.usToNetDataBufLength= eae.ucCurrentCommandLength;
	      	   break;
	    	
	    	case NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU:
	    		 if (!eae.ucCurrentProcessLogicalChannel){
	    		   if (NETAPIBUFMAXLENGTH<5+eae.ucCurrentSimCardIdentifierLength+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{  	    		   	     	    		   			
	    		   			if(IsCurrentIdentifierMF())
	    		   				  memcpy(eae.ucToNetDataBuf,usimselect_ByFIReturnFcpForSelectMF,5);
	    		   			else{	 
	    		   			     memcpy(eae.ucToNetDataBuf,usimselect_pathfromMfReturnFcp,4);
	    		   			     eae.ucToNetDataBuf[4]=eae.ucCurrentSimCardIdentifierLength;
	    		   	   	}
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,eae.ucCurrentSimCardIdentifierLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+eae.ucCurrentSimCardIdentifierLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+eae.ucCurrentSimCardIdentifierLength+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+eae.ucCurrentSimCardIdentifierLength+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;    	    		  
	    	     }
	    	   }
	    	   else{
	    	   	     if (NETAPIBUFMAXLENGTH<5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		       }	      	 
	    		       else{  	    		   	     	    		   			
	    		   			     if(IsCurrentIdentifierMF())
	    		   				      memcpy(eae.ucToNetDataBuf,usimselect_ByFIReturnFcpForSelectMF,5);	    		   				 	    		   				
	    		   			     else{	 
	    		   			           memcpy(eae.ucToNetDataBuf,usimselect_pathfromMfReturnFcp,4);
	    		   			         // eae.ucToNetDataBuf[4]=eae.ucCurrentSimCardIdentifierLength;	   
	    		   			         eae.ucToNetDataBuf[4]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength;	 		   			     
	    		   	   	     }
	    		   	   	eae.ucToNetDataBuf[0]|=eae.ucCurrentProcessLogicalChannel; //逻辑通道号
	    		   	   	
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	    	    		  eae.usToNetDataBufLength=5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength;    	    		  
	    	     }	    	   	
	    	   	}
	    	     break;
	    	case NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD:
	    		if (!eae.ucCurrentProcessLogicalChannel){
	    		   if (NETAPIBUFMAXLENGTH<5+eae.ucCurrentSimCardIdentifierLength+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		   }	      	 
	    		   else{
	    		   			if(IsCurrentIdentifierMF())
	    		   				  memcpy(eae.ucToNetDataBuf,usimselect_ByFIReturnFcpForSelectMF,5);
	    		   			else{	 
	    		   			     memcpy(eae.ucToNetDataBuf,usimselect_pathfromMfReturnFcp,4);
	    		   			     eae.ucToNetDataBuf[4]=eae.ucCurrentSimCardIdentifierLength;
	    		   			}
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.ucCurrentSimCardIdentifier,eae.ucCurrentSimCardIdentifierLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+eae.ucCurrentSimCardIdentifierLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  eae.usToNetDataBufLength=5+eae.ucCurrentSimCardIdentifierLength+eae.ucCurrentCommandLength;    	    		  
	    	     }
	    	   }
	    	   else{
	    	         if (NETAPIBUFMAXLENGTH<5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucCurrentCommandLength){
	    		   	      std::cout<<RED<<"Net Api Length Overflow"<<RESET<<std::endl<<std::flush;
	    		   	      return -1;
	    		       }	      	 
	    		   else{
	    		   			if(IsCurrentIdentifierMF())
	    		   				  memcpy(eae.ucToNetDataBuf,usimselect_ByFIReturnFcpForSelectMF,5);
	    		   			else{	 
	    		   			     memcpy(eae.ucToNetDataBuf,usimselect_pathfromMfReturnFcp,4);
	    		   			     eae.ucToNetDataBuf[4]=eae.ucCurrentSimCardIdentifierLength;
	    		   			}
	    		   			eae.ucToNetDataBuf[0]|=eae.ucCurrentProcessLogicalChannel; //逻辑通道号
	    		   				
	    	    			memcpy(&(eae.ucToNetDataBuf[5]),eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength);
	    	    		  memcpy(&(eae.ucToNetDataBuf[5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength]),eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	    	    		  eae.usToNetDataBufLength=5+eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength+eae.ucCurrentCommandLength;    	    		  
	    	     }
	    	   }
	    	     break;   
	    	     
	    	case NETAPI_CURRENTCMD_CURRENTARGU:	 	    		
	      	   memcpy(eae.ucToNetDataBuf,eae.ucCurrentCommand,eae.ucCurrentCommandLength);
	      	   memcpy(&(eae.ucToNetDataBuf[eae.ucCurrentCommandLength]),eae.ucCurrentArgument,eae.ucCurrentArgumentLength);
	      	   eae.usToNetDataBufLength=eae.ucCurrentCommandLength+eae.ucCurrentArgumentLength; 	      	
	      	   break;
      
	    	}
	    	
	    eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	    eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;
	    return 1;
}		    
	    
/**************************************************************************** 
* 函数名称 : TransferEaeToNet_NetApi_GetResponse
* 功能描述 : 将来自网络侧的NetApi响应 传送给Eae
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :  返回 1 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::TransferEaeToNet_NetApi_GetResponse(){
	     eae.ucToNetDataBuf[0]=APDU_COMMAND_GET_RESPONSE;
	     eae.ucToNetDataBuf[1]=eae.ucLastCommand[1];
	     eae.ucToNetDataBuf[2]=eae.ucCurrentSimCardIdentifier[0];
	     eae.ucToNetDataBuf[3]=eae.ucCurrentSimCardIdentifier[1];
	     eae.ucToNetDataBuf[4]=eae.ucCurrentCommand[OFFSET_P3];
	     eae.usToNetDataBufLength=5;
	     eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	     eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  
	     return 1;
}	

int Emu_Engine::TransferEaeToNet_NetApi_Read_Binary(){
	     eae.ucToNetDataBuf[0]=APDU_COMMAND_READ_BINARY;
	     eae.ucToNetDataBuf[1]=eae.ucCurrentSimCardIdentifier[0];
	     eae.ucToNetDataBuf[2]=eae.ucCurrentSimCardIdentifier[1];
	     eae.ucToNetDataBuf[3]=eae.ucCurrentCommand[OFFSET_P1];
	     eae.ucToNetDataBuf[4]=eae.ucCurrentCommand[OFFSET_P2];
	     eae.ucToNetDataBuf[5]=eae.ucCurrentCommand[OFFSET_P3];
	     eae.usToNetDataBufLength=6;
	     eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	     eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  
	     return 1;
}	

int Emu_Engine::TransferEaeToNet_NetApi_Status(){
	     eae.ucToNetDataBuf[0]=APDU_COMMAND_STATUS;
	     eae.ucToNetDataBuf[1]=eae.ucCurrentSimCardIdentifier[0];
	     eae.ucToNetDataBuf[2]=eae.ucCurrentSimCardIdentifier[1];
	     eae.ucToNetDataBuf[3]=eae.ucCurrentCommand[OFFSET_P3];
	     eae.usToNetDataBufLength=4;
	     eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	     eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  
	     return 1;
}


int Emu_Engine::TransferEaeToNet_NetApi_Run_Gsm_Algorithm(){
	     eae.ucToNetDataBuf[0]=APDU_COMMAND_RUN_GSM_ALGORITHM;
	     eae.ucToNetDataBuf[1]=eae.ucCurrentSimCardIdentifier[0];
	     eae.ucToNetDataBuf[2]=eae.ucCurrentSimCardIdentifier[1];
	     if (eae.ucLastArgumentLength!=0x10){
	     	    std::cout<<RED<<"Run_Gsm_Algorithm Argument Length is Wrong"<<RESET<<std::endl<<std::flush;
	   	      return -1;
	     }
	     memcpy(&eae.ucToNetDataBuf[3],eae.ucCurrentArgument,eae.ucLastArgumentLength);
	     eae.usToNetDataBufLength=0x13;
	     eae.ucToNetDataProperty=DATA_PROPERTY_IS_NETAPI_COMMAND;
	     eae.ucEmuApduNetApiStatus=  EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY;  
	     return 1;
}

// 根据参数设置当前Identifieer全路径
int  Emu_Engine::SetCurrentIdentifierPath(unsigned char identifier[2] ){
	  
	  if  (identifier[0]==FILE_1ST_ID_MF){
	  	  eae.CurrentIdWithPath.mfid[0]=identifier[0];
	  	  eae.CurrentIdWithPath.mfid[1]=identifier[1];
	  	  eae.CurrentIdWithPath.firstlevelid[0]=0;
		  eae.CurrentIdWithPath.firstlevelid[1]=0;
	  	  eae.CurrentIdWithPath.secondlevelid[0]=0;
		  eae.CurrentIdWithPath.secondlevelid[1]=0;
		  eae.CurrentIdWithPath.thirdlevelid[0]=0;
		  eae.CurrentIdWithPath.thirdlevelid[1]=0;	  	  
	  }
	  else
	  if  ((identifier[0]==FILE_1ST_ID_UNDER_MF_EF)||(identifier[0]==FILE_1ST_ID_1LEVEL_DF)){
	  	  eae.CurrentIdWithPath.firstlevelid[0]=identifier[0];
	  	  eae.CurrentIdWithPath.firstlevelid[1]=identifier[1];	  	  
	  	  eae.CurrentIdWithPath.secondlevelid[0]=0;
		  eae.CurrentIdWithPath.secondlevelid[1]=0;
	  	  eae.CurrentIdWithPath.thirdlevelid[0]=0;
		  eae.CurrentIdWithPath.thirdlevelid[1]=0;	  	  
	  }	
	  else
	  if  ((identifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(identifier[0]==FILE_1ST_ID_2LEVEL_DF)){
	  	  eae.CurrentIdWithPath.secondlevelid[0]=identifier[0];
	  	  eae.CurrentIdWithPath.secondlevelid[1]=identifier[1];	  	  	  	  
	  	  eae.CurrentIdWithPath.thirdlevelid[0]=0;
		  eae.CurrentIdWithPath.thirdlevelid[1]=0;
	  }	
	  else
	  if  (identifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
	  	  eae.CurrentIdWithPath.thirdlevelid[0]=identifier[0];
	  	  eae.CurrentIdWithPath.thirdlevelid[1]=identifier[1];	  	  	  	  	  	    	  
	  }
	  else
	  	return -1;
	  return 1;			  	  
	}

// 根据参数设置当前非0逻辑通道Identifieer全路径
int  Emu_Engine::SetCurrentLogicChannelIdentifierPath(unsigned char lch,unsigned char identifier[2] ){
	  
	  if  (identifier[0]==FILE_1ST_ID_MF){
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.mfid[0]=identifier[0];
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.mfid[1]=identifier[1];
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.firstlevelid[0]=0;
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.firstlevelid[1]=0;
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=0;
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.secondlevelid[1]=0;
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=0;	  	  
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=0;	  	  
	  }
	  else
	  if  ((identifier[0]==FILE_1ST_ID_UNDER_MF_EF)||(identifier[0]==FILE_1ST_ID_1LEVEL_DF)){
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.firstlevelid[0]=identifier[0];
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.firstlevelid[1]=identifier[1];	  	  
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=0;
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.secondlevelid[1]=0;
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=0;	  	  
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=0;	  	  
	  }	
	  else
	  if  ((identifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(identifier[0]==FILE_1ST_ID_2LEVEL_DF)){
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=identifier[0];
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.secondlevelid[1]=identifier[1];	  	  	  	  
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=0;	  	  
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=0;	  	  
	  }	
	  else
	  if  (identifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=identifier[0];
	  	  eae.Lcm[lch].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=identifier[1];	  	  	  	  	  	    	  
	  }
	  else
	  	return -1;
	  return 1;			  	  
	}	
	
void  Emu_Engine::TransferEaeToNetPathAndExpectId(){
	  
	   if ((eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)||(eae.ucExpectIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)){
 	   					TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);
 	    }
     else
     if ((eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(eae.ucExpectIdentifier[0]==FILE_1ST_ID_2LEVEL_DF)){
     	    TransferEaeToNet(NETAPI_SELECT_PATH_FIRSTLEVEL_SELECT_CURRENTCMD_CURRENTARGU);
     	} 
     else	
     if (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
     	    TransferEaeToNet(NETAPI_SELECT_PATH_FIRSTLEVEL_SECONDLEVEL_CURRENTCMD_CURRENTARGU);
     	}
     else{
     	     std::cout<<YELLOW<<"UNRECONIZED IDENTIFIER.Do not deliver to Net"<<RESET<<std::endl<<std::flush;	
     	} 
	}	
		
void  Emu_Engine::UpdatePath_SimAndUim(unsigned char identifier[2] ){
	  
	 //  if (eae.ucExpectIdentifier[0]==FILE_1ST_ID_MF){
	     if (identifier[0]==FILE_1ST_ID_MF){
     	      eae.CurrentIdWithPath.mfid[0]=identifier[0]	;
 	   				eae.CurrentIdWithPath.mfid[1]=identifier[1]	;
 	   				eae.CurrentIdWithPath.firstlevelid[0]=0;
					eae.CurrentIdWithPath.firstlevelid[1]=0;
					eae.CurrentIdWithPath.secondlevelid[0]=0;
 	   				eae.CurrentIdWithPath.secondlevelid[1]=0;
					eae.CurrentIdWithPath.thirdlevelid[0]=0;
 	   				eae.CurrentIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.CurrentId_AllType.length=1;
 	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.mfid[0];
 	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.mfid[1];
     	}
     else
	   if ((identifier[0]==FILE_1ST_ID_UNDER_MF_EF)||(identifier[0]==FILE_1ST_ID_1LEVEL_DF)){
 	   				eae.CurrentIdWithPath.firstlevelid[0]=identifier[0]	;
 	   				eae.CurrentIdWithPath.firstlevelid[1]=identifier[1]	;
 	   				eae.CurrentIdWithPath.secondlevelid[0]=0;
					eae.CurrentIdWithPath.secondlevelid[1]=0;
					eae.CurrentIdWithPath.thirdlevelid[0]=0;
 	   				eae.CurrentIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.CurrentId_AllType.length=1;
 	   				eae.CurrentId_AllType.identifier1[0]=identifier[0];
 	   				eae.CurrentId_AllType.identifier1[1]=identifier[1];
 	    }
     else
     if ((identifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(identifier[0]==FILE_1ST_ID_2LEVEL_DF)){
     	      eae.CurrentIdWithPath.secondlevelid[0]=identifier[0]	;
 	   				eae.CurrentIdWithPath.secondlevelid[1]=identifier[1]	;
 	   				eae.CurrentIdWithPath.thirdlevelid[0]=0;
					eae.CurrentIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.CurrentId_AllType.length=2;
 	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.firstlevelid[0];
 	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.firstlevelid[1];
 	   				eae.CurrentId_AllType.identifier2[0]=identifier[0];
 	   				eae.CurrentId_AllType.identifier2[1]=identifier[1];
 	   				
     	} 
     else	
     if (identifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
     	      eae.CurrentIdWithPath.thirdlevelid[0]=identifier[0]	;
 	   				eae.CurrentIdWithPath.thirdlevelid[1]=identifier[1]	;
 	   				
 	   				eae.CurrentId_AllType.length=3;
 	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.firstlevelid[0];
 	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.firstlevelid[1];
 	   				eae.CurrentId_AllType.identifier2[0]=eae.CurrentIdWithPath.secondlevelid[0];
 	   				eae.CurrentId_AllType.identifier2[1]=eae.CurrentIdWithPath.secondlevelid[1];
 	   				eae.CurrentId_AllType.identifier3[0]=identifier[0];
 	   				eae.CurrentId_AllType.identifier3[1]=identifier[1];
     	}
     else{
     	     
     	     std::cout<<RED<<"UNRECONIZED IDENTIFIER"<<RESET<<std::endl<<std::flush;	
     	} 
	}	
void  Emu_Engine::UpdatePath_Usim(unsigned char identifierLength, unsigned char* identifier){
	  
  if (!eae.ucCurrentProcessLogicalChannel) {
	     if (*identifier==FILE_1ST_ID_MF){
     	      eae.CurrentIdWithPath.mfid[0]=*identifier;
 	   				eae.CurrentIdWithPath.mfid[1]=*(identifier+1);
 	   				eae.CurrentIdWithPath.firstlevelid[0]=0;
 	   				eae.CurrentIdWithPath.firstlevelid[1]=0;
 	   				eae.CurrentIdWithPath.secondlevelid[0]=0;
 	   				eae.CurrentIdWithPath.secondlevelid[1]=0;
 	   				eae.CurrentIdWithPath.thirdlevelid[0]=0;
 	   				eae.CurrentIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.CurrentId_AllType.length=1;
 	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.mfid[0];
 	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.mfid[1];
     	}
     else
	   if ((*identifier==FILE_1ST_ID_UNDER_MF_EF)||(*identifier==FILE_1ST_ID_1LEVEL_DF)){
 	   				  eae.CurrentIdWithPath.firstlevelid[0]=*identifier;
 	   				  eae.CurrentIdWithPath.firstlevelid[1]=*(identifier+1);
 	   				
 	   					if (identifierLength>=4){
 	   					   eae.CurrentIdWithPath.secondlevelid[0]=*(identifier+2);
 	   				     eae.CurrentIdWithPath.secondlevelid[1]=*(identifier+3);
 	   					}
 	   				  else{	  
						  eae.CurrentIdWithPath.secondlevelid[0]=0;
						  eae.CurrentIdWithPath.secondlevelid[1]=0;
 	   				  }
 	   				  
 	   				  if (identifierLength>=6){
 	   					   eae.CurrentIdWithPath.thirdlevelid[0]=*(identifier+4);
 	   				     eae.CurrentIdWithPath.thirdlevelid[1]=*(identifier+5);
 	   					}
 	   				  else{	  
						  eae.CurrentIdWithPath.thirdlevelid[0]=0;
						  eae.CurrentIdWithPath.thirdlevelid[1]=0;
 	   				  }
 	   				 
 	   				 if (identifierLength==2){     				
 	   				   eae.CurrentId_AllType.length=1;
 	   			     eae.CurrentId_AllType.identifier1[0]=*identifier;
 	   				   eae.CurrentId_AllType.identifier1[1]=*(identifier+1);   				   
 	   			   }
 	   			   else
 	   			  if (identifierLength==4){
 	   				   eae.CurrentId_AllType.length=2;
 	   			     eae.CurrentId_AllType.identifier1[0]=*identifier;
 	   				   eae.CurrentId_AllType.identifier1[1]=*(identifier+1);   
 	   				   eae.CurrentId_AllType.identifier2[0]=*(identifier+2);   
 	   				   eae.CurrentId_AllType.identifier2[1]=*(identifier+3);     				   
 	   			  }
 	   			  else
 	   			  if (identifierLength==6){
 	   				   eae.CurrentId_AllType.length=3;
 	   			     eae.CurrentId_AllType.identifier1[0]=*identifier;
 	   				   eae.CurrentId_AllType.identifier1[1]=*(identifier+1);   
 	   				   eae.CurrentId_AllType.identifier2[0]=*(identifier+2);
 	   				   eae.CurrentId_AllType.identifier2[1]=*(identifier+3);  
 	   				   eae.CurrentId_AllType.identifier3[0]=*(identifier+4);  
 	   				   eae.CurrentId_AllType.identifier3[1]=*(identifier+5);   	   				 
 	   			  } 	   	
 	    }
     else
     if ((*identifier==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(*identifier==FILE_1ST_ID_2LEVEL_DF)){
     	      eae.CurrentIdWithPath.secondlevelid[0]=*identifier;	
 	   				eae.CurrentIdWithPath.secondlevelid[1]=*(identifier+1); 
 	   				if (identifierLength==4){
 	   					 eae.CurrentIdWithPath.thirdlevelid[0]=*(identifier+2);
 	   					 eae.CurrentIdWithPath.thirdlevelid[1]=*(identifier+3); 
 	   				} 
 	   				else{
						eae.CurrentIdWithPath.thirdlevelid[0]=0;
						eae.CurrentIdWithPath.thirdlevelid[1]=0;
 	   				}
 	 				
 	   				if (identifierLength==2){
    	   				eae.CurrentId_AllType.length=2;
    	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.firstlevelid[0];
    	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.firstlevelid[1];
    	   				eae.CurrentId_AllType.identifier2[0]=*identifier;	
    	   				eae.CurrentId_AllType.identifier2[1]=*(identifier+1);     	   		
 	   			  }
 	   			  else
 	   			 if (identifierLength==4){
    	   				eae.CurrentId_AllType.length=3;
    	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.firstlevelid[0];
    	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.firstlevelid[1];
    	   				eae.CurrentId_AllType.identifier2[0]=*identifier;	
    	   				eae.CurrentId_AllType.identifier2[1]=*(identifier+1);    
    	   				eae.CurrentId_AllType.identifier3[0]=*(identifier+2);    
    	   			  eae.CurrentId_AllType.identifier3[1]=*(identifier+3);    
 	   			  } 	   			   	   			  	   	
 
     	} 
     else	
     if (*identifier==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
     	      eae.CurrentIdWithPath.thirdlevelid[0]=identifier[0]	;
 	   				eae.CurrentIdWithPath.thirdlevelid[1]=identifier[1]	;
 	   				
 	   				
 	   				eae.CurrentId_AllType.length=3;
 	   				eae.CurrentId_AllType.identifier1[0]=eae.CurrentIdWithPath.firstlevelid[0];
 	   				eae.CurrentId_AllType.identifier1[1]=eae.CurrentIdWithPath.firstlevelid[1];
 	   				eae.CurrentId_AllType.identifier2[0]=eae.CurrentIdWithPath.secondlevelid[0];
 	   				eae.CurrentId_AllType.identifier2[1]=eae.CurrentIdWithPath.secondlevelid[1];
 	   				eae.CurrentId_AllType.identifier3[0]=*identifier;
 	   				eae.CurrentId_AllType.identifier3[1]=*(identifier+1); 
     	}
     else{
     	     
     	     std::cout<<RED<<"UNRECONIZED IDENTIFIER"<<RESET<<std::endl<<std::flush;	
     	} 
 }
 else{
	     if (*identifier==FILE_1ST_ID_MF){
     	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.mfid[0]=*identifier;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.mfid[1]=*(identifier+1);
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[0]=0;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[1]=0;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=0;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=0;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=0;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=1;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.mfid[0];
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.mfid[1];
     	}
     else
	   if ((*identifier==FILE_1ST_ID_UNDER_MF_EF)||(*identifier==FILE_1ST_ID_1LEVEL_DF)){
 	   				  eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[0]=*identifier;
 	   				  eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[1]=*(identifier+1);
 	   				
 	   					if (identifierLength>=4){
 	   					   eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=*(identifier+2);
 	   				     eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[1]=*(identifier+3);
 	   					}
 	   				  else{	  
						  eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=0;
						  eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[1]=0;
 	   				  }
 	   				  
 	   				  if (identifierLength>=6){
 	   					   eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=*(identifier+4);
 	   				     eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=*(identifier+5);
 	   					}
 	   				  else{	  
						  eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=0;
						  eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=0;
 	   				  }
 	   				 
 	   				 if (identifierLength==2){     				
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=1;
 	   			     eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=*identifier;
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=*(identifier+1);   				   
 	   			   }
 	   			   else
 	   			  if (identifierLength==4){
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=2;
 	   			     eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=*identifier;
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=*(identifier+1);   
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[0]=*(identifier+2);   
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[1]=*(identifier+3);     				   
 	   			  }
 	   			  else
 	   			  if (identifierLength==6){
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=3;
 	   			     eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=*identifier;
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=*(identifier+1);   
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[0]=*(identifier+2);
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[1]=*(identifier+3);  
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier3[0]=*(identifier+4);  
 	   				   eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier3[1]=*(identifier+5);   	   				 
 	   			  } 	   	
 	    }
     else
     if ((*identifier==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(*identifier==FILE_1ST_ID_2LEVEL_DF)){
     	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[0]=*identifier;	
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[1]=*(identifier+1); 
 	   				if (identifierLength==4){
 	   					 eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=*(identifier+2);
 	   					 eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=*(identifier+3); 
 	   				} 
 	   				else{
						eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=0;
						eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=0;
 	   				}
 	 				
 	   				if (identifierLength==2){
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=2;
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[0];
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[1];
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[0]=*identifier;	
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[1]=*(identifier+1);     	   		
 	   			  }
 	   			  else
 	   			 if (identifierLength==4){
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=3;
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[0];
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[1];
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[0]=*identifier;	
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[1]=*(identifier+1);    
    	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier3[0]=*(identifier+2);    
    	   			  eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier3[1]=*(identifier+3);    
 	   			  } 	   			   	   			  	   	
 
     	} 
     else	
     if (*identifier==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
     	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[0]=identifier[0]	;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.thirdlevelid[1]=identifier[1]	;
 	   				
 	   				
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.length=3;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[0];
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier1[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.firstlevelid[1];
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[0];
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier2[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath.secondlevelid[1];
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier3[0]=*identifier;
 	   				eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType.identifier3[1]=*(identifier+1); 
     	}
     else{
     	     
     	     std::cout<<RED<<"UNRECONIZED IDENTIFIER"<<RESET<<std::endl<<std::flush;	
     	} 
 }
	}		
void  Emu_Engine::UpdateExpectPath_SimAndUim( ){
	     eae.ExpectIdWithPath=eae.CurrentIdWithPath;
	     eae.ExpectId_AllType=eae.CurrentId_AllType;
	     
	     if (eae.ucExpectIdentifier[0]==FILE_1ST_ID_MF){
     	      eae.ExpectIdWithPath.mfid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.mfid[1]=eae.ucExpectIdentifier[1]	;
 	   				eae.ExpectIdWithPath.firstlevelid[0]=0;
					eae.ExpectIdWithPath.firstlevelid[1]=0;
 	   				eae.ExpectIdWithPath.secondlevelid[0]=0;
					eae.ExpectIdWithPath.secondlevelid[1]=0;
 	   				eae.ExpectIdWithPath.thirdlevelid[0]=0;
					eae.ExpectIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.ExpectId_AllType.length=1;
 	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.mfid[0];
 	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.mfid[1];
     	}
     else
	   if ((eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)||(eae.ucExpectIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)){
 	   				eae.ExpectIdWithPath.firstlevelid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.firstlevelid[1]=eae.ucExpectIdentifier[1]	;
 	   				eae.ExpectIdWithPath.secondlevelid[0]=0;
					eae.ExpectIdWithPath.secondlevelid[1]=0;
 	   				eae.ExpectIdWithPath.thirdlevelid[0]=0;
					eae.ExpectIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.ExpectId_AllType.length=1;
 	   				eae.ExpectId_AllType.identifier1[0]=eae.ucExpectIdentifier[0];
 	   				eae.ExpectId_AllType.identifier1[1]=eae.ucExpectIdentifier[1];
 	    }
     else
     if ((eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(eae.ucExpectIdentifier[0]==FILE_1ST_ID_2LEVEL_DF)){
     	      eae.ExpectIdWithPath.secondlevelid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.secondlevelid[1]=eae.ucExpectIdentifier[1]	;
 	   				eae.ExpectIdWithPath.thirdlevelid[0]=0;
					eae.ExpectIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.ExpectId_AllType.length=2;
 	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.firstlevelid[0];
 	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.firstlevelid[1];
 	   				eae.ExpectId_AllType.identifier2[0]=eae.ucExpectIdentifier[0];
 	   				eae.ExpectId_AllType.identifier2[1]=eae.ucExpectIdentifier[1];
 	   				
     	} 
     else	
     if (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
     	      eae.ExpectIdWithPath.thirdlevelid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.thirdlevelid[1]=eae.ucExpectIdentifier[1]	;
 	   				
 	   				eae.ExpectId_AllType.length=3;
 	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.firstlevelid[0];
 	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.firstlevelid[1];
 	   				eae.ExpectId_AllType.identifier2[0]=eae.ExpectIdWithPath.secondlevelid[0];
 	   				eae.ExpectId_AllType.identifier2[1]=eae.ExpectIdWithPath.secondlevelid[1];
 	   				eae.ExpectId_AllType.identifier3[0]=eae.ucExpectIdentifier[0];
 	   				eae.ExpectId_AllType.identifier3[1]=eae.ucExpectIdentifier[1];
     	}
     else{
     	     
     	     std::cout<<RED<<"UNRECONIZED IDENTIFIER"<<RESET<<std::endl<<std::flush;	
     	} 
	}
	

void  Emu_Engine::UpdateExpectPath_Usim(unsigned char* ExpectIdWithPathLength,unsigned char* ExpectIdWithPath ){
	   
	   if (!eae.ucCurrentProcessLogicalChannel) {
	     eae.ExpectIdWithPath=eae.CurrentIdWithPath;
	     eae.ExpectId_AllType=eae.CurrentId_AllType;
	   }
	   else{
	   	     eae.ExpectIdWithPath=eae.Lcm[eae.ucCurrentProcessLogicalChannel].CurrentLogical_ChannelIdWithPath;
	         eae.ExpectId_AllType=eae.Lcm[eae.ucCurrentProcessLogicalChannel].Logical_ChannelId_AllType;	   	
	   	} 
	     if (eae.ucExpectIdentifier[0]==FILE_1ST_ID_MF){
     	      eae.ExpectIdWithPath.mfid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.mfid[1]=eae.ucExpectIdentifier[1]	;
 	   				eae.ExpectIdWithPath.firstlevelid[0]=0;
 	   				eae.ExpectIdWithPath.firstlevelid[1]=0;
 	   				eae.ExpectIdWithPath.secondlevelid[0]=0;
 	   				eae.ExpectIdWithPath.secondlevelid[1]=0;
 	   				eae.ExpectIdWithPath.thirdlevelid[0]=0;
 	   				eae.ExpectIdWithPath.thirdlevelid[1]=0;
 	   				
 	   				eae.ExpectId_AllType.length=1;
 	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.mfid[0];
 	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.mfid[1];
 	   				
 	   				*ExpectIdWithPathLength=2;
 	   				*ExpectIdWithPath=eae.ucExpectIdentifier[0]	;
 	   				*(ExpectIdWithPath+1)=eae.ucExpectIdentifier[1]	;
     	}
     else
	   if ((eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)||(eae.ucExpectIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)){
 	   				eae.ExpectIdWithPath.firstlevelid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.firstlevelid[1]=eae.ucExpectIdentifier[1]	;
 	   				if (eae.ucCurrentArgumentLength>=4){
 	   					eae.ExpectIdWithPath.secondlevelid[0]=eae.ucExpectIdentifier[2]	;
 	   				  eae.ExpectIdWithPath.secondlevelid[1]=eae.ucExpectIdentifier[3]	;
 	   					}
 	   				else{	  
 	   				   eae.ExpectIdWithPath.secondlevelid[0]=0;
 	   				   eae.ExpectIdWithPath.secondlevelid[1]=0;
 	   				}
 	   				   
 	   				if (eae.ucCurrentArgumentLength>=6){
 	   					eae.ExpectIdWithPath.thirdlevelid[0]=eae.ucExpectIdentifier[4]	;
 	   				  eae.ExpectIdWithPath.thirdlevelid[1]=eae.ucExpectIdentifier[5]	;
 	   					}
 	   				else{	
 	   				   	eae.ExpectIdWithPath.thirdlevelid[0]=0;
						eae.ExpectIdWithPath.thirdlevelid[1]=0;
 	   				}
 	   				
 	   				if (eae.ucCurrentArgumentLength==2){
 	   				   eae.ExpectId_AllType.length=1;
 	   			     eae.ExpectId_AllType.identifier1[0]=eae.ucExpectIdentifier[0];
 	   				   eae.ExpectId_AllType.identifier1[1]=eae.ucExpectIdentifier[1];
 	   				   *ExpectIdWithPathLength=2;
 	   				   *ExpectIdWithPath=eae.ucExpectIdentifier[0]	;
 	   				   *(ExpectIdWithPath+1)=eae.ucExpectIdentifier[1]	;
 	   			  }
 	   			  else
 	   			  if (eae.ucCurrentArgumentLength==4){
 	   				   eae.ExpectId_AllType.length=2;
 	   			     eae.ExpectId_AllType.identifier1[0]=eae.ucExpectIdentifier[0];
 	   				   eae.ExpectId_AllType.identifier1[1]=eae.ucExpectIdentifier[1];
 	   				   eae.ExpectId_AllType.identifier2[0]=eae.ucExpectIdentifier[2];
 	   				   eae.ExpectId_AllType.identifier2[1]=eae.ucExpectIdentifier[3];
 	   				   *ExpectIdWithPathLength=4;
 	   				   *ExpectIdWithPath=eae.ucExpectIdentifier[0]	;
 	   				   *(ExpectIdWithPath+1)=eae.ucExpectIdentifier[1]	;
 	   				   *(ExpectIdWithPath+2)=eae.ucExpectIdentifier[2]	;
 	   				   *(ExpectIdWithPath+3)=eae.ucExpectIdentifier[3]	;
 	   			  }
 	   			  else
 	   			  if (eae.ucCurrentArgumentLength==6){
 	   				   eae.ExpectId_AllType.length=3;
 	   			     eae.ExpectId_AllType.identifier1[0]=eae.ucExpectIdentifier[0];
 	   				   eae.ExpectId_AllType.identifier1[1]=eae.ucExpectIdentifier[1];
 	   				   eae.ExpectId_AllType.identifier2[0]=eae.ucExpectIdentifier[2];
 	   				   eae.ExpectId_AllType.identifier2[1]=eae.ucExpectIdentifier[3];
 	   				   eae.ExpectId_AllType.identifier3[0]=eae.ucExpectIdentifier[4];
 	   				   eae.ExpectId_AllType.identifier3[1]=eae.ucExpectIdentifier[5];
 	   				   
 	   				   *ExpectIdWithPathLength=6;
 	   				   *ExpectIdWithPath=eae.ucExpectIdentifier[0]	;
 	   				   *(ExpectIdWithPath+1)=eae.ucExpectIdentifier[1]	;
 	   				   *(ExpectIdWithPath+2)=eae.ucExpectIdentifier[2]	;
 	   				   *(ExpectIdWithPath+3)=eae.ucExpectIdentifier[3]	;
 	   				   *(ExpectIdWithPath+4)=eae.ucExpectIdentifier[4]	;
 	   				   *(ExpectIdWithPath+5)=eae.ucExpectIdentifier[5]	;
 	   			  }
 	   				
 	    }
     else
     if ((eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||(eae.ucExpectIdentifier[0]==FILE_1ST_ID_2LEVEL_DF)){
     	      eae.ExpectIdWithPath.secondlevelid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.secondlevelid[1]=eae.ucExpectIdentifier[1]	;
 	   				if (eae.ucCurrentArgumentLength==4){
 	   					 eae.ExpectIdWithPath.thirdlevelid[0]=eae.ucExpectIdentifier[2]	;
 	   					 eae.ExpectIdWithPath.thirdlevelid[1]=eae.ucExpectIdentifier[3]	;
 	   				} 
 	   				else{
						eae.ExpectIdWithPath.thirdlevelid[1]=0;
 	   				   eae.ExpectIdWithPath.thirdlevelid[0]=0;
					   }
 	   				
 	   				if (eae.ucCurrentArgumentLength==2){
    	   				eae.ExpectId_AllType.length=2;
    	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.firstlevelid[0];
    	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.firstlevelid[1];
    	   				eae.ExpectId_AllType.identifier2[0]=eae.ucExpectIdentifier[0];
    	   				eae.ExpectId_AllType.identifier2[1]=eae.ucExpectIdentifier[1];
    	   				
    	   			  *ExpectIdWithPathLength=4; 	 	   			  	   
    	   		    *ExpectIdWithPath=eae.ExpectIdWithPath.firstlevelid[0];
 	   				    *(ExpectIdWithPath+1)=eae.ExpectIdWithPath.firstlevelid[1];
 	   				    *(ExpectIdWithPath+2)=eae.ucExpectIdentifier[0]	;
 	   				    *(ExpectIdWithPath+3)=eae.ucExpectIdentifier[1]	;
    	   		 
 	   			  }
 	   			  else
 	   			  if (eae.ucCurrentArgumentLength==4){
    	   				eae.ExpectId_AllType.length=3;
    	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.firstlevelid[0];
    	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.firstlevelid[1];
    	   				eae.ExpectId_AllType.identifier2[0]=eae.ucExpectIdentifier[0];
    	   				eae.ExpectId_AllType.identifier2[1]=eae.ucExpectIdentifier[1];
    	   				eae.ExpectId_AllType.identifier3[0]=eae.ucExpectIdentifier[2];
    	   				eae.ExpectId_AllType.identifier3[1]=eae.ucExpectIdentifier[3];
    	   				
    	   				*ExpectIdWithPathLength=6; 	 	   			  	   
    	   		    *ExpectIdWithPath=eae.ExpectIdWithPath.firstlevelid[0];
 	   				    *(ExpectIdWithPath+1)=eae.ExpectIdWithPath.firstlevelid[1];
 	   				    *(ExpectIdWithPath+2)=eae.ucExpectIdentifier[0];
 	   				    *(ExpectIdWithPath+3)=eae.ucExpectIdentifier[1]	;
 	   				    *(ExpectIdWithPath+4)=eae.ucExpectIdentifier[2];
 	   				    *(ExpectIdWithPath+5)=eae.ucExpectIdentifier[3]	;
 	   				    
 	   			  } 	   			   	   			  	   				   				 
     	} 
     else	
     if (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){
     	      eae.ExpectIdWithPath.thirdlevelid[0]=eae.ucExpectIdentifier[0]	;
 	   				eae.ExpectIdWithPath.thirdlevelid[1]=eae.ucExpectIdentifier[1]	;
 	   				
 	   				eae.ExpectId_AllType.length=3;
 	   				eae.ExpectId_AllType.identifier1[0]=eae.ExpectIdWithPath.firstlevelid[0];
 	   				eae.ExpectId_AllType.identifier1[1]=eae.ExpectIdWithPath.firstlevelid[1];
 	   				eae.ExpectId_AllType.identifier2[0]=eae.ExpectIdWithPath.secondlevelid[0];
 	   				eae.ExpectId_AllType.identifier2[1]=eae.ExpectIdWithPath.secondlevelid[1];
 	   				eae.ExpectId_AllType.identifier3[0]=eae.ucExpectIdentifier[0];
 	   				eae.ExpectId_AllType.identifier3[1]=eae.ucExpectIdentifier[1];
 	   				
			    	*ExpectIdWithPathLength=6; 	 	   			  	   
   		      *ExpectIdWithPath=eae.ExpectIdWithPath.firstlevelid[0];
				    *(ExpectIdWithPath+1)=eae.ExpectIdWithPath.firstlevelid[1];
				    *(ExpectIdWithPath+2)=eae.ExpectIdWithPath.secondlevelid[0];
				    *(ExpectIdWithPath+3)=eae.ExpectIdWithPath.secondlevelid[1]	;
				    *(ExpectIdWithPath+4)=eae.ucExpectIdentifier[0];
				    *(ExpectIdWithPath+5)=eae.ucExpectIdentifier[1]	;
     	}
     else{
     	     
     	     std::cout<<RED<<"UNRECONIZED IDENTIFIER"<<RESET<<std::endl<<std::flush;	
     	} 
	}	
/**************************************************************************** 
* 函数名称 : SetDefaultCommandIdentifier
* 功能描述 : 在Eae中设置当前Sim 文件
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :  void 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	

void Emu_Engine::SetDefaultCommandIdentifier(){	
	     //在SIM卡激活时，根目录MF是隐含选中的目录，所以允许GET RESPONSE指令作为激活后的第1条指令；
	 
	   eae.ucCurrentSimCardIdentifier[0]=0x3F;
  		eae.ucCurrentSimCardIdentifier[1]=0;
  		eae.ucCurrentSimCardIdentifierLength=2;
  		
  		eae.ucLastSimCardIdentifier[0]=0x3F;
  		eae.ucLastSimCardIdentifier[1]=0;
  		eae.ucLastSimCardIdentifierLength=2;
  		
      eae.ucCurrentDirIdentifier[0]=0x3F;
  		eae.ucCurrentDirIdentifier[1]=0;  
  		eae.ucCurrentDirIdentifierLength=2;
  	//	memcpy(eae.ucCurrentCommand,command_select ,5);	将command 移到以后再赋值
  		eae.ucCurrentCommandLength=5;
  		eae.ucCurrentArgument[0]=0x3F;
  		eae.ucCurrentArgument[1]=0;
  		eae.ucCurrentArgumentLength=2;
  		
  		SetCurrentIdentifierPath(eae.ucCurrentSimCardIdentifier);
  		for (unsigned char tmplchno=1;tmplchno<MAX_LOGICAL_CHANNELS_NUMBER;tmplchno++)
  		   SetCurrentLogicChannelIdentifierPath(tmplchno,eae.ucCurrentSimCardIdentifier);
 
  		eae.ucLastCommandLength=5;
  		eae.ucLastArgument[0]=0x3F;
  		eae.ucLastArgument[1]=0;
  		eae.ucLastArgumentLength=2; 
  		  
  		eae.iCurrentVcardIndexListSerialnumber=	VCARDINDEXLISTSERIALNUMBER_UNKOWN;	
  			//std::cout<<"SetDefaultCommandIdentifier"<<std::endl;
  			
  	 eae.ucCurrentChv1Status=eae.ucDefaultChv1Status;
  	 //20180709
  	 memset(&eae.Lcm, 0, sizeof(emu_apdu_logic_channel_t)*MAX_LOGICAL_CHANNELS_NUMBER);	
  	 
  	 
  	for(int i=0;i<MAX_LOGICAL_CHANNELS_NUMBER;i++) {
  	  eae.Lcm[i].ucCurrentLogical_Channel_Identifier[0]=0x3F;
  		eae.Lcm[i].ucCurrentLogical_Channel_Identifier[1]=0;
  		eae.Lcm[i].ucCurrentLogical_Channel_IdentifierLength=2;
  		
  		eae.Lcm[i].ucLastLogical_Channel_Identifier[0]=0x3F;
  		eae.Lcm[i].ucLastLogical_Channel_Identifier[1]=0;
  		eae.Lcm[i].ucLastLogical_Channel_IdentifierLength=2;
  		
      eae.Lcm[i].ucCurrentLogical_Channel_DirIdentifier[0]=0x3F;
  		eae.Lcm[i].ucCurrentLogical_Channel_DirIdentifier[1]=0;  
  		eae.Lcm[i].ucCurrentLogical_Channel_DirIdentifierLength=2;
 	
  	//	SetCurrentIdentifierPath(eae.ucCurrentSimCardIdentifier);
  		eae.Lcm[i].LogicalChanneliCurrentIndexSerialnumber=	VCARDINDEXLISTSERIALNUMBER_UNKOWN;	
  	 }
  	
	   
}

void Emu_Engine::SetCurrentEngineStatusWhenUncertain(){	
if (eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_USIM_UIM_ENBALE){
     	 if (eae.ucFromEmuDataBuf[OFFSET_CLA]==0XA0){
     	 	
     	 	//   if (eae.ucCurrentEaeCapability&VC_DATA_SIMMODE_MAINTAIN) 
     	 	//20200330 处理Invaild card
     	 	  if (eae.ucCurrentEaeCapability&VG_HANDLEICC_SIMMODE_MAINTAIN) 
     	   	          eae.ucCurrentEngineStatus=ENGINESTATUS_SIM_ENBALE_ONLY;
     	   	 else {
     	   	 	     // if (eae.ucCurrentEaeCapability&VC_DATA_UIMMODE_MAINTAIN) 
     	   	 	     //20200330 处理Invaild card
     	 	            if (eae.ucCurrentEaeCapability&VG_HANDLEICC_UIMMODE_MAINTAIN) 
     	   	 	            eae.ucCurrentEngineStatus=ENGINESTATUS_UIM_ENBALE_ONLY;
     	   	 	}         
     	   	 memcpy(eae.ucCurrentCommand,command_select ,5);
     	   	 memcpy(eae.ucLastCommand,command_select ,5);
     	 }
 	   else{
 	   	     if ((eae.ucFromEmuDataBuf[OFFSET_CLA]==0X0)||\
 	   	       	      (eae.ucFromEmuDataBuf[OFFSET_CLA]==0X80)){
 	   	       	eae.ucCurrentEngineStatus=ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED;
 	   	        memcpy(eae.ucCurrentCommand,usimselect_pathfromMfReturnFcp ,4);
 	   	        eae.ucCurrentCommand[4]=2;
 	   	        memcpy(eae.ucLastCommand,usimselect_pathfromMfReturnFcp ,4);
 	   	        eae.ucLastCommand[4]=2;
 	   	      }
 	   	}	   
  }
}

int Emu_Engine::IsUsimModeEnable(){	
  if (
       (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)||\    
       (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)
      )
  return 1;
  else
  	return 0;    
 }   
 	
int Emu_Engine::IsSimModeEnable(){	
   if (eae.ucCurrentEngineStatus==ENGINESTATUS_SIM_ENBALE_ONLY)
    return 1;
  else
  	return 0;    
 }  
 
int Emu_Engine::IsUimModeEnable(){	
   if (eae.ucCurrentEngineStatus==ENGINESTATUS_UIM_ENBALE_ONLY)
    return 1;
  else
  	return 0;    
 } 	
int Emu_Engine::SubProcessSelectArgumentSim(vcard_file_property_t file1){
	 return (SubProcessSelectArgumentUim( file1));
}

int Emu_Engine::SubProcessUpdateBinaryWithSfiUsim( ){  
int vcardindexlistserialnumber	;    
unsigned char tmpresponse[2]; 
unsigned char tmpilengh;       
unsigned char tmpbuf2[2];	
int tmpbodyfile;  

     

     if (!eae.ucCurrentProcessLogicalChannel)	{
     	 if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	  TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	    eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_UPDATE_FORK_FAIL;   
     	 	}
     	 else{
       if ((vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,eae.ucCurrentRidType))<0){  	     		  	                          	      	
	         TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	    eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;    	
       }  
	            	   										        		      	     				  
	     else{
	        
	        if (GetFileBodyResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
     	        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	        eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
     	      }
          else{
              	     
            if (GetFileBodyLength(vcardindexlistserialnumber)<0){
              	TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
              	eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;             	
              }
            else{
            	    if (SetFileBodyContext(vcardindexlistserialnumber, (unsigned short) eae.ucCurrentCommand[OFFSET_P2],\
            	    	eae.ucCurrentArgument, (unsigned short) eae.ucCurrentCommand[OFFSET_P3])<0){  	                	       
                    TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
                    eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                  }
                  else{
                  	                       
      	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	   memcpy(tmpbuf2,tmpresponse ,2);	
          	         else{
          	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	    tmpbuf2[0]=0x91;
          	       	      	    tmpbuf2[1]=eae.ucCurrentCatLength;
          	       	     }
          	         }
      	       	      TransferEaeToEmu(tmpbuf2, 2 ) ;
      	       	      
      	     	      std::cout<<GREEN<<std::hex <<"Emu Side Current PATH CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<<" (SFI Tran) :";	 
	   	    	    	  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	            eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
	   	    	        }
                    std::cout<<std::endl;                    	
       	    	      eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength; 	
       	    	      eae.iCurrentVcardIndexListSerialnumber=vcardindexlistserialnumber;	
      	    	      
       	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
       	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	       	    	      
       	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
               }
            }	     
         }     	
       }
       }
    }
    else{
    	   if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	  TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	     eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_UPDATE_FORK_FAIL;   
     	 	}
     	  else
    	   {
    	      if ((vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
    	      	              eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type))<0){  	     		  	                     
      	      	
	              TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	           eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
     	
             }             	   										        		      	     				  
	          else{
	        
	        if (GetFileBodyResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
     	       TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	        eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
     	      }
          else{
              	     
                   if (GetFileBodyLength(vcardindexlistserialnumber)<0){
                     	TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
                     	eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;             	
                     }
                   else{
                   	  if (SetFileBodyContext(vcardindexlistserialnumber, (unsigned short) eae.ucCurrentCommand[OFFSET_P2],\
            	         	eae.ucCurrentArgument, (unsigned short) eae.ucCurrentCommand[OFFSET_P3])<0){
                     // if (GetFileBodyContext(vcardindexlistserialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P3] )<0){
                          TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
                           eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                       }
                      else{
                           
             	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
                 	       	    memcpy(tmpbuf2,tmpresponse ,2);	
                 	         else{
                 	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
                 	       	      	    tmpbuf2[0]=0x91;
                 	       	      	    tmpbuf2[1]=eae.ucCurrentCatLength;
                 	       	     }
                 	         }
             	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
             	       	      
             	     	       std::cout<<GREEN<<std::hex <<"Emu Side Current PATH CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<<" (SFI Tran) :";	 
       	   	    	    	   for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
       	   	    	            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
       	   	    	            std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
       	   	    	         }
                           std::cout<<std::endl;                    	
              	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength; 	
              	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=vcardindexlistserialnumber;	
             	    	      
              	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
              	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	       	    	      
              	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
                      }
                   }	     
                }     	
      }
    
    }
  }       
}

int Emu_Engine::SubProcessReadBinaryWithSfiUsim( ){  
int vcardindexlistserialnumber	;    
unsigned char tmpresponse[2]; 
unsigned char tmpilengh;       
//unsigned char tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH];	
unsigned char tmpbuf1[256*2],tmpbuf2[256*2+3];	  
     if (!eae.ucCurrentProcessLogicalChannel)	{
      if ((vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,eae.ucCurrentRidType))<0){  	     		  	                     
      	      	
	         TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
     	    eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
     	
      }  
	            	   										        		      	     				  
	    else{
	        
	        if (GetFileBodyResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
     	        TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
     	        eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
     	      }
          else{
              	     
            if (GetFileBodyLength(vcardindexlistserialnumber)<0){
              	TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
              	eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;             	
              }
            else{
              // if (GetFileBodyContext(vcardindexlistserialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P3] )<0){
               if (GetFileBodyContext(vcardindexlistserialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P2]+eae.ucCurrentCommand[OFFSET_P3] )<0){
                    TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
                    eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                }
               else{
                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
      	       	      memcpy(&tmpbuf2[1],&tmpbuf1[eae.ucCurrentCommand[OFFSET_P2]] ,eae.ucCurrentCommand[OFFSET_P3]);	 
      	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
          	         else{
          	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
          	       	     }
          	         }
      	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
      	       	      
      	     	      std::cout<<GREEN<<std::hex <<"Emu Side Current PATH CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<<" (SFI Tran) :";	 
	   	    	    	  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	            eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
	   	    	        }
                    std::cout<<std::endl;                    	
       	    	      eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength; 	
       	    	      eae.iCurrentVcardIndexListSerialnumber=vcardindexlistserialnumber;	
      	    	      
       	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
       	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	       	    	      
       	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
               }
            }	     
         }     	
      }
    }
    else{
    	      if ((vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
    	      	              eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type))<0){  	     		  	                     
      	      	
	               TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
     	           eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
     	
             }             	   										        		      	     				  
	          else{
	        
	        if (GetFileBodyResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
     	        TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
     	        eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
     	      }
          else{
              	     
                   if (GetFileBodyLength(vcardindexlistserialnumber)<0){
                     	TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
                     	eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;             	
                     }
                   else{
                    //  if (GetFileBodyContext(vcardindexlistserialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P3] )<0){
                     if (GetFileBodyContext(vcardindexlistserialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P2]+eae.ucCurrentCommand[OFFSET_P3] )<0){
                           TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
                           eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                       }
                      else{
                            memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
             	       	   //   memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
             	       	       memcpy(&tmpbuf2[1],&tmpbuf1[eae.ucCurrentCommand[OFFSET_P2]] ,eae.ucCurrentCommand[OFFSET_P3]);	
             	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
                 	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
                 	         else{
                 	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
                 	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
                 	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
                 	       	     }
                 	         }
             	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
             	       	      
             	     	       std::cout<<GREEN<<std::hex <<"Emu Side Current PATH CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<<" (SFI Tran) :";	 
       	   	    	    	   for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
       	   	    	            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
       	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
       	   	    	         }
                           std::cout<<std::endl;                    	
              	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength; 	
              	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=vcardindexlistserialnumber;	
             	    	      
              	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
              	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	       	    	      
              	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
                      }
                   }	     
                }     	
      }
    
    }         
}

int Emu_Engine::SubProcessReadRecordWithSfiUsim( ){  
int vcardindexlistserialnumber	;    
unsigned char tmpresponse[2]; 
unsigned char tmpilengh;       
unsigned char tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH];	
 if (!eae.ucCurrentProcessLogicalChannel)	{
 	
 	   if( ( vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,\
 	   	    eae.ucExpectIdentifier,eae.ucCurrentRidType))<0) { //如果本地没有Fid         	
	       TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
     	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
   }  
	            	   										        		      	     				  
	 else{
	 	
	        if (GetFileRecordResponse(vcardindexlistserialnumber,   &tmpresponse)<0){	    
     	        TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
     	        eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
     	      }
          else{
          if (GetFileRecord(vcardindexlistserialnumber,  eae.ucCurrentCommand[OFFSET_P1],  tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0) { 	     
              if (eae.ucReadRecordStatus==READRECORDSTATUS_NUMBER_OVERFLOW){
              	//Record not found
              	 tmpbuf2[0]=SW1_6A;
              	 tmpbuf2[1]=0x83;
              	 TransferEaeToEmu(tmpbuf2, 2 ) ;             	 
              	}
              else{	
              	TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
              	eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
              }
          }
          else{
                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
      	       	      memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
      	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
          	         else{
          	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
          	       	     }
          	         }
      	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
      	       	      
      	     	      std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";	 
	   	    	    	  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	            eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
	   	    	        }
                    std::cout<<std::endl;
       	    	      eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength; 	
       	    	      
       	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
       	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	
       	    	      eae.iCurrentVcardIndexListSerialnumber=vcardindexlistserialnumber;	
       	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
               
            }	     
         }     	
   } 
  }
  else{
              if( ( vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
             	   	    eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type))<0) { //如果本地没有Fid         	
            	       TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
                 	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
               }  
            	            	   										        		      	     				  
            	 else{
            	 	
            	        if (GetFileRecordResponse(vcardindexlistserialnumber,   &tmpresponse)<0){	    
                 	        TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
                 	        eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                 	      }
                      else{
                      if (GetFileRecord(vcardindexlistserialnumber,  eae.ucCurrentCommand[OFFSET_P1],  tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0) { 	     
                            if (eae.ucReadRecordStatus==READRECORDSTATUS_NUMBER_OVERFLOW){
                                  	//Record not found
                             	 tmpbuf2[0]=SW1_6A;
                             	 tmpbuf2[1]=0x83;
                            	 TransferEaeToEmu(tmpbuf2, 2 ) ;             	 
                          	}
                            else{	
                          	     TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim();
                                	eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                             } 
                       }
                      else{
                                  memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
                  	       	      memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
                  	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
                      	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
                      	         else{
                      	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
                      	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
                      	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
                      	       	     }
                      	         }
                  	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
                  	       	      
                  	     	      std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";	 
            	   	    	    	  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
            	   	    	            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
            	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
            	   	    	        }
                                std::cout<<std::endl;
                   	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength; 	
                   	    	      
                   	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
                   	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	
                   	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=vcardindexlistserialnumber;	
                   	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
                           
                        }	     
                     }     	
               } 
              
              
  }      
}
int Emu_Engine::SubProcessUpdateRecordWithSfiUsim( ){  
int vcardindexlistserialnumber	;    
unsigned char tmpresponse[2]; 
unsigned char tmpilengh;    
unsigned char tmpbuf2[2];	   

 if (!eae.ucCurrentProcessLogicalChannel)	{
 	   if( ( vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,\
 	   	    eae.ucExpectIdentifier,eae.ucCurrentRidType))<0) { //如果本地没有Fid         	
	      TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
     }  
	            	   										        		      	     				  
	   else{
	 	      if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	       TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();  
     	 	       eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_UPDATE_FORK_FAIL;   	        
           }
          else 
	 	      {
	        if (GetFileRecordResponse(vcardindexlistserialnumber,   &tmpresponse)<0){	    
     	        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
     	        eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
     	      }
          else{
               if (SetFileRecord(vcardindexlistserialnumber,  eae.ucCurrentCommand[OFFSET_P1],  eae.ucCurrentArgument, eae.ucCurrentCommand[OFFSET_P3])<0) { 	     
         
              	TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
              	eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
               }
               else
                {
                                            
      	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	   memcpy(tmpbuf2,tmpresponse ,2);	
          	         else{
          	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	    tmpbuf2[0]=0x91;
          	       	      	    tmpbuf2[1]=eae.ucCurrentCatLength;
          	       	     }
          	         }
      	       	      TransferEaeToEmu(tmpbuf2, 2 ) ;
                     
      	       	      
      	     	      std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";	 
	   	    	    	  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	            eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
	   	    	        }
                    std::cout<<std::endl;
       	    	      eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength; 	
       	    	      
       	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
       	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	
       	    	      eae.iCurrentVcardIndexListSerialnumber=vcardindexlistserialnumber;	
       	    	      eae.ucCurrentReadMode=READMODE_COMMON; 
       	    	  }      	    	  	               
          }	     
     }     	
   }
  }
  else{
              if( ( vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
             	   	    eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type))<0) { //如果本地没有Fid         	
            	      TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
                 	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
               }  
            	            	   										        		      	     				  
            	 else{
            	 	     if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	                 TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();  
     	 	                 eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_UPDATE_FORK_FAIL;   	        
                      }
                    else 
            	 	      {
            	        if (GetFileRecordResponse(vcardindexlistserialnumber,   &tmpresponse)<0){	    
                 	        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
                 	        eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                 	      }
                      else{
                      if (SetFileRecord(vcardindexlistserialnumber,  eae.ucCurrentCommand[OFFSET_P1],  eae.ucCurrentArgument, eae.ucCurrentCommand[OFFSET_P3])<0) { 	     
                     
                          	TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim();
                          	eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_READ_FAIL_IN_VCARD;
                      }
                      else{
                                 
                                 if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	                  memcpy(tmpbuf2,tmpresponse ,2);	
          	                     else{
          	       	                   if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	              tmpbuf2[0]=0x91;
          	       	      	               tmpbuf2[1]=eae.ucCurrentCatLength;
          	       	                 }           
          	                     }
      	       	                 TransferEaeToEmu(tmpbuf2, 2 ) ;
                  	       	      
                  	     	      std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";	 
            	   	    	    	  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
            	   	    	            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
            	   	    	           std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
            	   	    	        }
                                std::cout<<std::endl;
                   	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength; 	
                   	    	      
                   	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
                   	    	       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;	
                   	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=vcardindexlistserialnumber;	
                   	    	      eae.ucCurrentReadMode=READMODE_COMMON;       	    	  	
                           
                        }	     
                     }
                }     	
               } 
              
              
  }      
}



//TODO
int Emu_Engine::IsSameDir(unsigned char IdentifierA_Length,unsigned char * IdentifierA,unsigned char IdentifierB_Length,unsigned char * IdentifierB){

unsigned char dirdeepA,dirdeepB,dirdeep;	
unsigned char tmpDirA[IDENTIFIERDEEPTH],tmpDirB[IDENTIFIERDEEPTH],tmpDirALen,tmpDirBLen;
	  if (!(
	 	((IdentifierA_Length==2)||(IdentifierA_Length==4)||(IdentifierA_Length==6))&&\
	 	((IdentifierB_Length==2)||(IdentifierB_Length==4)||(IdentifierB_Length==6))
	 	))
	 	return 0;
	 	
	 	if (IdentifierA_Length==2){
	 		 if ((*(IdentifierA)==FILE_1ST_ID_1LEVEL_DF)){
	 		 	   
	 		 	    tmpDirA[0]=*(IdentifierA);
	 		 	    tmpDirA[1]=*(IdentifierA+1);
	 		 	}
	 		 	else{
	 		 		    tmpDirA[0]=0x3f;
	 		 	      tmpDirA[1]=0; 		 		
	 		 		}
	 		  tmpDirALen=2;	 
	 	}
	 	else
	 	if ((IdentifierA_Length==4)||(IdentifierA_Length==6)){
	 		 if ((*(IdentifierA+2)==FILE_1ST_ID_2LEVEL_DF)){
	 		 	   
	 		 	    tmpDirA[0]=*(IdentifierA);
	 		 	    tmpDirA[1]=*(IdentifierA+1);
	 		 	    tmpDirA[2]=*(IdentifierA+2);
	 		 	    tmpDirA[3]=*(IdentifierA+3);
	 		 	    tmpDirALen=4;	
	 		 	}
	 		 	else{
	 		 		    tmpDirA[0]=*(IdentifierA);;
	 		 	      tmpDirA[1]=*(IdentifierA+1); 	
	 		 	      tmpDirALen=2;		 		
	 		 		} 		  
	 	}
	 	
	 	if (IdentifierB_Length==2){
	 		 if ((*(IdentifierB)==FILE_1ST_ID_1LEVEL_DF)){
	 		 	   
	 		 	    tmpDirB[0]=*(IdentifierB);
	 		 	    tmpDirB[1]=*(IdentifierB+1);
	 		 	}
	 		 	else{
	 		 		    tmpDirB[0]=0x3f;
	 		 	      tmpDirB[1]=0; 		 		
	 		 		}
	 		  tmpDirBLen=2;	 
	 	}
	 	else
	 	if ((IdentifierB_Length==4)||(IdentifierB_Length==6)){
	 		 if ((*(IdentifierB+2)==FILE_1ST_ID_2LEVEL_DF)){
	 		 	   
	 		 	    tmpDirB[0]=*(IdentifierB);
	 		 	    tmpDirB[1]=*(IdentifierB+1);
	 		 	    tmpDirB[2]=*(IdentifierB+2);
	 		 	    tmpDirB[3]=*(IdentifierB+3);
	 		 	    tmpDirBLen=4;	
	 		 	}
	 		 	else{
	 		 		    tmpDirB[0]=*(IdentifierB);;
	 		 	      tmpDirB[1]=*(IdentifierB+1); 	
	 		 	      tmpDirBLen=2;		 		
	 		 		} 		  
	 	}
	 	if (tmpDirBLen!=tmpDirALen)
	 		 return 0;
	 	for (dirdeep=0;dirdeep<tmpDirBLen;dirdeep++)
	 	   if (tmpDirB[dirdeep]!=tmpDirA[dirdeep])
	 	   	 return 0;
	 	
	 	return 1;
	 	
	}

int Emu_Engine::SubProcessSelectArgumentUsim( ){   //USIM 为带路径选择，不需要判断重名
int vcardindexlistserialnumber	;    
unsigned char tmpresponse[2]; 
unsigned char tmpilengh;       
	
     //20181102  for (tmpilengh=0;tmpilengh< eae.ucCurrentArgumentLength;tmpilengh++)  	  	        	   										        		      	     				      										        		      	     				   
	   //20181102  	     eae.ucExpectIdentifier[tmpilengh]=eae.ucCurrentArgument[tmpilengh];
	    
	    if (!eae.ucCurrentProcessLogicalChannel)     
	    	  vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,eae.ucCurrentRidType);
	    else 
	    	   vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type);
	    if (vcardindexlistserialnumber<0) {  	   
	 //   if( (vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
   //      	      eae.ucCurrentRidType))<0) { //如果本地没有Fid 
           TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);	  //不再增加多余路径     2018.1.8  
	             if (eae.SyncLocal_Remote!=SYNCLOCAL_REMOTE_YES) 
     	  	     eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
     	  	 if (!eae.ucCurrentProcessLogicalChannel)   
     	  	    eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
     	  	 else
     	  	     eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
  	  }  
	            	   										        		      	     				  
	    else{
	        
	        if (GetIdentifierResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
	   	         TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);	 //不再增加多余路径      2018.1.8    	
	   	         if (eae.SyncLocal_Remote!=SYNCLOCAL_REMOTE_YES)
     	  	        eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;	
     	  	     if (!eae.ucCurrentProcessLogicalChannel)     	
     	  	         eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_RESPONSEBYTE_IN_VCARD;
     	  	      else
     	  	         eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=	VCARDINDEXLISTSERIALNUMBER_NO_RESPONSEBYTE_IN_VCARD;						        		      	     				   	    
	   	    }
	   	    else{	     	  	        	   										        		      	     				   	    	     	   										        		      	     				   	    	    
	   	    	    if (eae.ucExpectSelectResponseStatus==CURRENTSELECTRESPONSESTATUS_NODATA_RETURN){
	   	    	   	   tmpresponse[0]=SW1_90;
	   	    	   	   tmpresponse[1]=0;
	   	    	   	   TransferEaeToEmu(tmpresponse, 2 ) ;
	   	    	    	
	   	    	    }	
	   	    	    TransferEaeToEmu(tmpresponse, 2 ) ;
	   	    	   // if (SW1_61==tmpresponse[0]){
	   	    	    if ((SW1_61==tmpresponse[0])||(SW1_90==tmpresponse[0])){	
	   	    	    	  //logical ch 0    缺省  ucCurrentSimCardIdentifier 对应 logical ch 0 ； ucExpectIdentifier 与AID结合
	   	    	    	 if (!eae.ucCurrentProcessLogicalChannel){
       	   	    	    	if (IsSameDir(eae.ucCurrentSimCardIdentifierLength,eae.ucCurrentSimCardIdentifier,eae.ucExpectIdentifierLength,eae.ucExpectIdentifier))
       	   	    	    		 eae.SyncLocal_Remote=SYNC_DIR_LOCAL_REMOTE_YES_FOR_SFI;
       	   	    	    		else 
       	   	    	    		 eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;       	   	    	    		 
       	   	    	    	std::cout<<GREEN<<std::hex <<"Emu Side Current PATH Lch 0:";	 
       	   	    	    	for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
       	   	    	        eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
       	   	    	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
       	   	    	      }
       	   	    	      std::cout<<std::endl ; 
       	   	    	      eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength; 	
       	   	    	      UpdatePath_Usim(eae.ucCurrentSimCardIdentifierLength, eae.ucCurrentSimCardIdentifier);
       	   	    	      eae.ucCurrentExpectFileHeaderLength= tmpresponse[1]; 	
       	   	    	      
       	   	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	
       	   	    	       eae.iCurrentVcardIndexListSerialnumber=vcardindexlistserialnumber;		
	   	    	      } 
	   	    	      else {
	   	    	      	     if (IsSameDir(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength,\
	   	    	      	                 	eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier,\
	   	    	      	     	            eae.ucExpectIdentifierLength,eae.ucExpectIdentifier))
       	   	    	    		 eae.SyncLocal_Remote=SYNC_DIR_LOCAL_REMOTE_YES_FOR_SFI;
       	   	    	    		else 
       	   	    	    		 eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_NO;
       	   	    	    	std::cout<<GREEN<<std::hex <<"Emu Side Current PATH Lch "<<(unsigned short)eae.ucCurrentProcessLogicalChannel<<"  ";	 
       	   	    	    	for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
       	   	    	           eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
       	   	    	          std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
       	   	    	      }
       	   	    	      std::cout<<std::endl ; 
       	   	    	      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength; 	
       	   	    	      UpdatePath_Usim(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength,\
       	   	    	       eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier);
       	   	    	      eae.ucCurrentExpectFileHeaderLength= tmpresponse[1]; 	
       	   	    	      
       	   	    	       eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;	       	   	    	    
       	   	    	       eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=vcardindexlistserialnumber;	
	   	    	      	
	   	    	      }
	   	    	       							        		      	     				   	    	        
	   	    	    } 
              
	   	    }                     	 
	   } 
}   										        		      	     				   
int Emu_Engine::SubProcessSelectArgumentUim(vcard_file_property_t file1){	
int vcardindexlistserialnumber	;
unsigned char tmpresponse[2];
alltype_vcard_identifier_t alltype_vcard_identifier;
vcard_file_property_t right_gfile;
  // eae.ucCurrentSimCardIdentifierProperty=file1.property;
      if (file1.property&CAN_FORK)
         eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_AVAILABLE;
         
   if (!((file1.property&EXIST_SAME_EF_NAME))||((file1.property&EXIST_SAME_DF_NAME))) //不是重名文件
    {
   		if( (vcardindexlistserialnumber= GetIdentifierIndex(eae.ucExpectIdentifier))<0){
             //   TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);	
                TransferEaeToNet(NETAPI_CURRENTCMD_CURRENTARGU);	
                
                eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
           }          	  	        	   										        		      	     				  
      else{
      if (GetIdentifierResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
 	        //  TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU); 
 	          TransferEaeToNet(NETAPI_CURRENTCMD_CURRENTARGU);
 	          eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_RESPONSEBYTE_IN_VCARD;
 	        }      										        		      	     				   	    
 	    else{
 	    	    TransferEaeToEmu(tmpresponse, 2 ) ;
 	    	    if (SW1_9F==tmpresponse[0]){
 	    	    	  
 	    	    	  eae.ucLastSimCardIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];
 	    	        eae.ucLastSimCardIdentifier[1]=eae.ucCurrentSimCardIdentifier[1]; 	
 	    	     //   eae.ucLastSimCardIdentifierLength=	eae.ucCurrentSimCardIdentifierLength;			
 	    	    	  
 	    	        eae.ucCurrentSimCardIdentifier[0]=eae.ucExpectIdentifier[0];
 	    	        eae.ucCurrentSimCardIdentifier[1]=eae.ucExpectIdentifier[1]; 	
 	    	        eae.ucCurrentExpectFileHeaderLength= tmpresponse[1]; 				
 	    	        UpdatePath_SimAndUim(eae.ucCurrentSimCardIdentifier);	
 	    	        std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";
 	    	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.mfid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.mfid[1]<<" " ; 	  
	     	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.firstlevelid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.firstlevelid[1]<<" " ; 	     	          	          	     	      
	     	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.secondlevelid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.secondlevelid[1]<<" " ;
	     	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.thirdlevelid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.thirdlevelid[1]<<" " ; 	      	     	       
	     	        std::cout<<std::endl ; 
 	    	        eae.iCurrentVcardIndexListSerialnumber=	vcardindexlistserialnumber;		
 	    	        eae.ucCurrentSimCardIdentifierProperty=file1.property;
 	    	        eae.CurrentTransfer2Net_Status=TRANSFER2NET_STATUS_SINGLE_ID;		        		      	     				   	    	        
 	    	    } 	   										      		   	                            
 	    }         
    } 
   }
   else{  //同名文件时   
   	    //20181030 同名文件时ExpectTransfer2Net_Status 改为传路径
   	    eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_CURRENT_PATH;
   	   if( (vcardindexlistserialnumber= GetIdentifierIndexWithPath(eae.ucExpectIdentifier))<0){
               // TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);	
               TransferEaeToNetPathAndExpectId(); 
               
            //20181030     eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_CURRENT_PATH;
                    	  	  
               eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_VCARD;
             }      	   										        		      	     				  
      else{
      if (GetIdentifierResponse(vcardindexlistserialnumber,   &tmpresponse)<0){
 	         // TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);
 	         TransferEaeToNetPathAndExpectId();  
 	        //20181030    eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_CURRENT_PATH;
 	         eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_RESPONSEBYTE_IN_VCARD;
 	       }      										        		      	     				   	    
 	    else{
 	    	    TransferEaeToEmu(tmpresponse, 2 ) ;
 	    	    if (SW1_9F==tmpresponse[0]){
 	    	    	  
 	    	    	  eae.ucLastSimCardIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];
 	    	        eae.ucLastSimCardIdentifier[1]=eae.ucCurrentSimCardIdentifier[1]; 	
 	    	     //   eae.ucLastSimCardIdentifierLength=	eae.ucCurrentSimCardIdentifierLength;			
 	    	    	  
 	    	        eae.ucCurrentSimCardIdentifier[0]=eae.ucExpectIdentifier[0];
 	    	        eae.ucCurrentSimCardIdentifier[1]=eae.ucExpectIdentifier[1]; 	
 	    	        eae.ucCurrentExpectFileHeaderLength= tmpresponse[1]; 				
 	    	        UpdatePath_SimAndUim(eae.ucCurrentSimCardIdentifier);	
 	    	       // eae.CurrentTransfer2Net_Status=TRANSFER2NET_STATUS_SINGLE_ID;		  
 	    	        eae.CurrentTransfer2Net_Status=TRANSFER2NET_STATUS_CURRENT_PATH;	     
 	    	        std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";
 	    	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.mfid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.mfid[1]<<" " ; 	  
	     	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.firstlevelid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.firstlevelid[1]<<" " ; 	     	          	          	     	      
	     	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.secondlevelid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.secondlevelid[1]<<" " ;
	     	        std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.thirdlevelid[0]\
	     	          	           <<" "<<(unsigned short)eae.CurrentIdWithPath.thirdlevelid[1]<<" " ; 	      	     	       
	     	        std::cout<<std::endl ; 
 	    	        
 	    	        eae.iCurrentVcardIndexListSerialnumber=	vcardindexlistserialnumber;	
 	    	        //定位无歧义文件 	    	       
 	    	        if (GetVcardFilePropertyWithPath_Uim(eae.CurrentId_AllType, &right_gfile)<0)                        
     	                return -1; 
 	    	        else
 	    	        	eae.ucCurrentSimCardIdentifierProperty=right_gfile.property;					        		      	     				   	    	        
 	    	    } 	   										      		   	                            
 	    }         
    } 
   	
   	}  
 } 

int Emu_Engine::SubProcessGetResponseLastCommandIsSelectUimSim(){
int vcardindexlistserialnumber	;
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH];	
  	if (eae.iCurrentVcardIndexListSerialnumber>0xf00)
     	   TransferEaeToNet(NETAPI_CURRENTCMD);
    else	   
    if (GetFileHeaderResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0)  //如果本地没有文件头
	          	    TransferEaeToNet(NETAPI_CURRENTCMD);
	          else{
	      	       if (GetFileHeaderContext( eae.iCurrentVcardIndexListSerialnumber, tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0) //如果取文件头错误
	      	       	    TransferEaeToNet(NETAPI_CURRENTCMD);
	      	       else	{
	      	       	      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	      	       	      memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
	      	       	      if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
	      	       	            memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
	      	       	      else{
	      	       	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
	      	       	      	     	}
	      	       	      	}
	      	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
	      	       	      
	      	       	}    
	          }    	     
}

int Emu_Engine::SubProcessGetResponseLastCommandIsSelectUsim(){
int vcardindexlistserialnumber	;                                                              
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH];	       
   
   if (!eae.ucCurrentProcessLogicalChannel)
      vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucCurrentSimCardIdentifierLength,eae.ucCurrentSimCardIdentifier,eae.ucCurrentRidType);
   else
   	  vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength,\
   	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type);   
   
   if(vcardindexlistserialnumber<0)
   //if( (vcardindexlistserialnumber= GetIdentifierIndex_Usim(eae.ucCurrentSimCardIdentifierLength,eae.ucCurrentSimCardIdentifier))<0)  //如果本地没有Fid
      	      TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
      else	{
	          if (GetFileHeaderResponse(vcardindexlistserialnumber,   &tmpresponse)<0)  //如果本地没有文件头
	          	    TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
	          else{
	      	       if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0) //如果取文件头错误
	      	       	    TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
	      	       else	{
	      	       	      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	      	       	      memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
	      	       	      if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
	      	       	            memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
	      	       	      else{
	      	       	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
	      	       	      	     	}
	      	       	      	}
	      	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
	      	       	      
	      	       	}    
	          }	     
    	}	
}

int Emu_Engine::SubProcessGetResponseLastCommandIsSeekUsim(){
int vcardindexlistserialnumber	;                                                              
unsigned char tmpbuf2[APDU_BUF_MAX_LENGTH];	       
  
	      	       	      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	      	       	      memcpy(&tmpbuf2[1],eae.ucMeetSeekRecordNo ,eae.ucCurrentCommand[OFFSET_P3]);	 
	      	       	      if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD){
	      	       	                  tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x90;
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=0;
	      	       	       }
	      	       	      else{
	      	       	      	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
	      	       	      	     	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
	      	       	      	     	}
	      	       	      	}
	      	       	      TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
	      	       	      
}

int Emu_Engine::GetCurrentDir(unsigned char *identifier ){
	  //前2级目录都假设无重名
	  if ((eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_MF)||(eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)){
	  	//  *identifier_length=2;
	  	  *identifier=eae.ucCurrentSimCardIdentifier[0];
	  	  *(identifier+1)=eae.ucCurrentSimCardIdentifier[1];
	  	}
	  else
	  if (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)	{
	  //	*identifier_length=2;
	  	*identifier=0x3f;
	  	*(identifier+1)=0;
	  	}
	  else
	  if (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)	{ 
	  	//firstlevelid应在之前被系统设置好，为DF文件,并假设firstlevelid是无重名文件
	  //	*identifier_length=2;
	  	*identifier=eae.CurrentIdWithPath.firstlevelid[0];
	  	*(identifier+1)=eae.CurrentIdWithPath.firstlevelid[1];
	  }
	  else
	  if (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_2LEVEL_DF)	{ 
	  //	*identifier_length=2;
	  	*identifier=eae.ucCurrentSimCardIdentifier[0];
	  	*(identifier+1)=eae.ucCurrentSimCardIdentifier[1];
	  }
	  else	  
	   if (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)	{ 
	  //	*identifier_length=2;
	  	*identifier=eae.CurrentIdWithPath.secondlevelid[0];
	  	*(identifier+1)=eae.CurrentIdWithPath.secondlevelid[1];
	  }	
	  else{
	  	    std::cout<<RED<<"Unregonizde DirIdentifier "<<RESET<<std::endl<<std::flush;
             return -1;
	  	}
	  return 1;	 
}

void Emu_Engine::TransferToNetPathPendingSyncStatus_CurrentCmd_Usim(){
	  if (eae.SyncLocal_Remote==SYNCLOCAL_REMOTE_YES)
            TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
     else	{ 
     	  	  TransferEaeToNet_Usim(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
     	  	  eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
  	 }  
}
void Emu_Engine::TransferToNetPathPendingSyncStatus_CurrentCmdForSFI_Usim(){
	  if (eae.SyncLocal_Remote==SYNCLOCAL_REMOTE_YES)
            TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
    else
    if (eae.SyncLocal_Remote==SYNC_DIR_LOCAL_REMOTE_YES_FOR_SFI){
            TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
             eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
     }
     else	{ 
     	  	  TransferEaeToNet_Usim(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
     	  	  eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
  	 }  
}

void Emu_Engine::TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim(){
	  if (eae.SyncLocal_Remote==SYNCLOCAL_REMOTE_YES)
            TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);
     else	{ 
     	  	  TransferEaeToNet_Usim(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);
     	  	  eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
  	 }  
}

void Emu_Engine::TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArguForSFI_Usim(){
	  if (eae.SyncLocal_Remote==SYNCLOCAL_REMOTE_YES)
            TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);
    else
    if (eae.SyncLocal_Remote==SYNC_DIR_LOCAL_REMOTE_YES_FOR_SFI){
            TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);
            eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
    } 
     else	{ 
     	  	  TransferEaeToNet_Usim(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);
     	  	  eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_ENABLE;
  	 }  
}


int Emu_Engine::SubProcessGetResponseLastCommandIsStatusUsim(){  //GET RESPONSE要求直接跟在前一功能后面，在两条功能之间不能插入其他功能   //在 Usim 应用中，STATUS 内容指向当前目录或应用	 // 20180620
int vcardindexlistserialnumber	;                                                              
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpidentifier[IDENTIFIERDEEPTH];	 
unsigned char tmpilengh;         
  if ((eae.ucLastCommand[OFFSET_P3]==0)&&(eae.ucToEmuDataBuf[0]==SW1_61)&&(eae.usToEmuDataBufLength==2)){  //以此判断模块准备读7fff文件头
 	    	 
 	    	 if (!eae.ucCurrentProcessLogicalChannel)
 	    	     vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucCurrentDirIdentifierLength,eae.ucCurrentDirIdentifier,eae.ucCurrentRidType);
 	    	  else
 	    	  	vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifierLength,\
 	    	  	eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type);
 	    	 
 	    	  if (vcardindexlistserialnumber<0)   
	    //   if( (vcardindexlistserialnumber= GetIdentifierIndex_Usim(eae.ucCurrentDirIdentifierLength,eae.ucCurrentDirIdentifier))<0)
	                TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	                 
	        else{
	    	     if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
                  TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
  	   	     else{
                  if (GetFileHeaderLength(vcardindexlistserialnumber,&tmpilengh)>0)  
                     if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, tmpilengh)<0)    	 
									      TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();           	       	
                      else{	         					        		      	       	     	          	      	        
          	      	             memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
   	     	          	       	    memcpy(&tmpbuf2[1],tmpbuf1 ,tmpilengh);	 

   	     	          	       	    if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
	       	     	          	       	   memcpy(&tmpbuf2[tmpilengh+1],tmpresponse ,2);	
	       	     	          	      else
	       	     	          	      if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
	       	     	          	       	      	    tmpbuf2[tmpilengh+1]=0x91;
	       	     	          	       	      	    tmpbuf2[tmpilengh+2]=eae.ucCurrentCatLength;
	       	     	          	      }       	     	          	      
   	     	          	       	   TransferEaeToEmu(tmpbuf2, tmpilengh+3 ) ;        	        					        		      	       	     	          	    
   	     	          	      
                      }
             } 
         }
  }
}



int Emu_Engine::SubProcessReadBinaryCommandSim(){
	  return (SubProcessReadBinaryCommandUim());
	}
	
int Emu_Engine::SubProcessReadRecordCommandSim(){
	  return (SubProcessReadRecordCommandUim());
	}
int Emu_Engine::SubProcessReadRecordCommandUim(){  
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH];	   

   if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
      	   TransferEaeToNet(NETAPI_CURRENTCMD);
   else	{   
  	     	if (GetFileRecordResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0)
  	     	       TransferEaeToNet(NETAPI_CURRENTCMD);
  	     	else{ 	   										        		      	       	   	              
  	   	       if (eae.ucCurrentCommand[OFFSET_P2]!=4)  //  04：ABSOLUTE模式/CURRENT模式
  	   	       	     TransferEaeToNet(NETAPI_CURRENTCMD);
  	   	       else{
  	   	       	     	//保证文件      
  	   	            if (GetFileRecordWithCurrentFileProperty(eae.iCurrentVcardIndexListSerialnumber,  eae.ucCurrentCommand[OFFSET_P1],  tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0){  	                                                                    
                        if (eae.ucReadRecordStatus==READRECORDSTATUS_NUMBER_OVERFLOW ){
                        	//记录超出范围
                               	 tmpbuf2[0]=SW1_94;
                               	 tmpbuf2[1]=0x02;
                               	 TransferEaeToEmu(tmpbuf2, 2 ) ; 
                        	} 
                        	else
                           TransferEaeToNet(NETAPI_CURRENTCMD);
                    }
                    else{
                         memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
  	   	       	          memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
  	   	       	       
  	   	       	         if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
	     	          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
	     	          	       else{
	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
	     	          	       	     }
	     	          	       } 
  	   	       	          TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
                    }
               }          
              	  
	     	     	}	     
   } 
   return 1;
}  
  	
int Emu_Engine::SubProcessReadBinaryCommandUsim(){
//	int vcardindexlistserialnumber, tmpfilecounter,tmplength;
 unsigned char tmpilengh;
 //unsigned char tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpresponse[2];
 unsigned char tmpbuf2[READ_BIANARY_BUF_MAX_LENGTH],tmpresponse[2];
 unsigned char tmpbuf1[READ_BIANARY_BUF_MAX_LENGTH];
 unsigned short readoffset,tmpreadfilelength;  
 unsigned short tmpfilebodylength;
 unsigned short tmp_p3;
  if (!eae.ucCurrentCommand[OFFSET_P3])
     tmp_p3=0x100;
  else
     tmp_p3=eae.ucCurrentCommand[OFFSET_P3];
     
  if (!eae.ucCurrentProcessLogicalChannel){
   if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
   	   TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();  	  
      else	{   
 	         if (GetFileBodyResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0)
 	     	       
 	     	        TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
 	         else{
 	   	       
 	   	        tmpfilebodylength=   GetFileBodyLength(eae.iCurrentVcardIndexListSerialnumber);
                   if (tmpfilebodylength<0)     	     
     	          	    TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
     	       else{  
               //   if (GetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P3] )<0)     
                   if (GetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, tmpbuf1,tmpfilebodylength)<0)     //20180620          
                        TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                   else{
                       readoffset= eae.ucFromEmuDataBuf[OFFSET_P1]*256+eae.ucFromEmuDataBuf[OFFSET_P2];  
                         if (! (eae.ucCurrentSimCardIdentifierProperty&LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0)){                         
                         //   if ((readoffset+eae.ucFromEmuDataBuf[OFFSET_P3])>tmpfilebodylength)
                              if ((readoffset+tmp_p3)>tmpfilebodylength) {
                            	    std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;
                            	    TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                            	 }	
                            else	 {   	                                   
                                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
                                      
           	       	                  memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmp_p3);	           	       	                            	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(&tmpbuf2[tmp_p3+1],tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[tmp_p3+1]=0x91;
    	          	       	      	                   tmpbuf2[tmp_p3+2]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, tmp_p3+3 ) ;
           	       	        }
           	       	     }
           	       	     else{      //LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0 文件      	     	     	       	                                   
                                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
                                      
                                      if (readoffset>=tmpfilebodylength)
                                      	    memset(&tmpbuf2[1], 0, tmp_p3);	
                                      else
                                      if ((readoffset+tmp_p3)>tmpfilebodylength){
                                      	  memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmpfilebodylength-readoffset);
                                      	  memset(&tmpbuf2[1+tmpfilebodylength-readoffset], 0, \
                                      	  readoffset+tmp_p3-tmpfilebodylength);	
                                      	}
                                      else
           	       	                      memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmp_p3);
           	       	                      	           	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(&tmpbuf2[tmp_p3+1],tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[tmp_p3+1]=0x91;
    	          	       	      	                   tmpbuf2[tmp_p3+2]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, tmp_p3+3 ) ;
           	       	           
           	       	     	
           	       	     	}
          	       	    
                   }
              }	  
   	       }	     
    }
  }
  else{
  	  if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber>0xf00)  
   	     TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();  	  
      else	{   
 	         if (GetFileBodyResponse(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber,   &tmpresponse)<0)
 	     	       
 	     	        TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
 	         else{
 	   	       
 	   	        tmpfilebodylength=   GetFileBodyLength(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber);
                   if (tmpfilebodylength<0)     	     
     	          	    TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
     	       else{                
                   if (GetFileBodyContext(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber, tmpbuf1,tmpfilebodylength)<0)     //20180620          
                        TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                   else{
                       readoffset= eae.ucFromEmuDataBuf[OFFSET_P1]*256+eae.ucFromEmuDataBuf[OFFSET_P2];  
                         if (! (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChannelCurrentIdentifierProperty&LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0)){                         
                            if ((readoffset+tmp_p3)>tmpfilebodylength){
                            	    std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;	
                         /*         // Wrong parameter(s) P1-P2
                                  if (readoffset>=tmpfilebodylength){
                                  	  tmpbuf2[0]=0x6b;
                                  	  tmpbuf2[1]=0x0;
                                  }
                                  else{   //The terminal shall wait for a second procedure byte then immediately
                                          //repeat the previous command header to the UICC using a length of
                                          //'XX', where 'XX' is the value of the second procedure byte (SW2).
                                  	  tmpbuf2[0]=0x6c;
                                  	  tmpbuf2[1]=tmpfilebodylength-readoffset;                                 	
                                  	}
                                  TransferEaeToEmu(tmpbuf2, 2) ;
                            */
                              TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                            }
                            else	 {   	                                   
                                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
           	       	                  memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmp_p3);	           	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(&tmpbuf2[tmp_p3+1],tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[tmp_p3+1]=0x91;
    	          	       	      	                   tmpbuf2[tmp_p3+2]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, tmp_p3+3 ) ;
           	       	        }
           	       	     }
           	       	     else{      //LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0 文件      	     	     	       	                                   
                                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
                                      
                                      if (readoffset>=tmpfilebodylength)
                                      	    memset(&tmpbuf2[1], 0, tmp_p3);	
                                      else
                                      if ((readoffset+tmp_p3)>tmpfilebodylength){
                                      	  memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmpfilebodylength-readoffset);
                                      	  memset(&tmpbuf2[1+tmpfilebodylength-readoffset], 0, \
                                      	  readoffset+tmp_p3-tmpfilebodylength);	
                                      	}
                                      else
           	       	                      memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmp_p3);
           	       	                      	           	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(&tmpbuf2[tmp_p3+1],tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[tmp_p3+1]=0x91;
    	          	       	      	                   tmpbuf2[tmp_p3+2]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, tmp_p3+3 ) ;
           	       	           
           	       	     	
           	       	     	}
          	       	    
                   }
              }	  
   	       }	     
    }
  	
  }
}	
int Emu_Engine::SubProcessUpdateBinaryCommandArgumentUsim(){
//	int vcardindexlistserialnumber, tmpfilecounter,tmplength;
 unsigned char tmpilengh;
 //unsigned char tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpresponse[2];
// unsigned char tmpbuf2[READ_BIANARY_BUF_MAX_LENGTH],tmpresponse[2];
  unsigned char tmpbuf2[2],tmpresponse[2];
 unsigned char tmpbuf1[READ_BIANARY_BUF_MAX_LENGTH];
 unsigned short readoffset,tmpreadfilelength;  
 unsigned short tmpfilebodylength;
 if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	       TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();     	        
 }
 else 
 {
  if (!eae.ucCurrentProcessLogicalChannel){
   if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
   	   TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();     	   	  
      else	{ 
      	    
 	           if (GetFileBodyResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0) 	     	       
 	     	        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
 	           else{
 	   	       
 	   	        tmpfilebodylength=   GetFileBodyLength(eae.iCurrentVcardIndexListSerialnumber);
                   if (tmpfilebodylength<0)     	     
     	          	    TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
     	       else{  
               //   if (GetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, tmpbuf1,eae.ucCurrentCommand[OFFSET_P3] )<0)     
                   if (GetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, tmpbuf1,tmpfilebodylength)<0)     //20180620          
                        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                   else{
                       readoffset= eae.ucCurrentCommand[OFFSET_P1]*256+eae.ucCurrentCommand[OFFSET_P2];  
                       //  if (! (eae.ucCurrentSimCardIdentifierProperty&LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0)){                         
                            if ((readoffset+eae.ucCurrentCommand[OFFSET_P3])>tmpfilebodylength){
                            	    std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;	
                            	    TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                            }
                            else	 {
                            	   if (SetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, readoffset,\
            	    	                  eae.ucCurrentArgument, (unsigned short) eae.ucCurrentCommand[OFFSET_P3])<0){  	                	       
                                      TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                    
                                 }   
                            	   else{	                                   
                                               	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(tmpbuf2,tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[0]=0x91;
    	          	       	      	                   tmpbuf2[1]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, 2 ) ;
           	       	             }    
           	       	        }
           	       	    // }           	       	              	       	    
                   }
              }	  
   	       }
   	     	     
    }
  }
  else{
  	  if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber>0xf00)  
   	     TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();  	  
      else	{        	    
 	           
 	         if (GetFileBodyResponse(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber,   &tmpresponse)<0)
 	     	       
 	     	        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
 	         else{
 	   	       
 	   	        tmpfilebodylength=   GetFileBodyLength(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber);
                   if (tmpfilebodylength<0)     	     
     	          	    TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
     	       else{                
                   if (GetFileBodyContext(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber, tmpbuf1,tmpfilebodylength)<0)     //20180620          
                        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                   else{
                       readoffset= eae.ucCurrentCommand[OFFSET_P1]*256+eae.ucCurrentCommand[OFFSET_P2];  
                        // if (! (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChannelCurrentIdentifierProperty&LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0)){                         
                            if ((readoffset+eae.ucCurrentCommand[OFFSET_P3])>tmpfilebodylength)
                            	{
                            	    std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;	
                            	    TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                              }                            
                            else	 {   
                            	       if (SetFileBodyContext(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber, readoffset,\
            	    	                  eae.ucCurrentArgument, (unsigned short) eae.ucCurrentCommand[OFFSET_P3])<0){  	                	       
                                      TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                    
                                     }  
                                     else {	                                   
                                       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(tmpbuf2,tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[0]=0x91;
    	          	       	      	                   tmpbuf2[1]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                    TransferEaeToEmu(tmpbuf2, 2) ;
           	       	               }
           	       	        }
           	       	  //   }
           	       	    
                   }
              }	  
   	       }	
   	          
    }
  	
  }
 } 
}	

int Emu_Engine::SubProcessUpdateBinaryCommandArgumentSimOrUim(){
 unsigned char tmpilengh;
 unsigned char tmpbuf2[2],tmpresponse[2];
 unsigned char tmpbuf1[READ_BIANARY_BUF_MAX_LENGTH];
 unsigned short readoffset,tmpreadfilelength;  
 unsigned short tmpfilebodylength;
    if ((eae.CurrentIdWithPath.firstlevelid[0]==0x7f)&&(eae.CurrentIdWithPath.firstlevelid[1]==0x25))
    	 int eew=1;
 if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	        Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();    	        
 }
 else 
 {
  
   if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
   	   Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();     	   	  
      else	{ 
      	    
 	           if (GetFileBodyResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0) 	     	       
 	     	        Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();
 	           else{
 	   	       
 	   	        tmpfilebodylength=   GetFileBodyLength(eae.iCurrentVcardIndexListSerialnumber);
                   if (tmpfilebodylength<0)     	     
     	          	    Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();
     	       else{                     
                   if (GetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, tmpbuf1,tmpfilebodylength)<0)     //20180620          
                        Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();
                   else{
                            readoffset= eae.ucCurrentCommand[OFFSET_P1]*256+eae.ucCurrentCommand[OFFSET_P2];                                          
                            if ((readoffset+eae.ucCurrentCommand[OFFSET_P3])>tmpfilebodylength){
                            	    std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;	
                            	    Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();
                            }
                            else	 {
                            	   if (SetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber, readoffset,\
            	    	                  eae.ucCurrentArgument, (unsigned short) eae.ucCurrentCommand[OFFSET_P3])<0){  	                	       
                                      Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();                    
                                 }   
                            	   else{	                                   
                                               	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(tmpbuf2,tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[0]=0x91;
    	          	       	      	                   tmpbuf2[1]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, 2 ) ;
           	       	             }    
           	       	        }       	       	              	       	    
                   }
              }	  
   	       }
   	     	     
    }
  
 
 } 
}	

int Emu_Engine::SubProcessReadRecordCommandUsim(){
//	int vcardindexlistserialnumber, tmpfilecounter,tmplength;
 unsigned char tmpilengh;
 unsigned char tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpresponse[2];
 unsigned short readoffset,tmpreadfilelength;  
 unsigned short tmpfilebodylength;
 
 
 if (!eae.ucCurrentProcessLogicalChannel){
     if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
   	   	       	 TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	   else	{   
    	     	if (GetFileRecordResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0)
   	     	       TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
    	     	else{ 	   										        		      	       	   	              
    	   	       if (eae.ucCurrentCommand[OFFSET_P2]!=4)  //  04：ABSOLUTE模式/CURRENT模式
    	   	       	     TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
    	   	       else{	     
    	   	            if (GetFileRecord(eae.iCurrentVcardIndexListSerialnumber,  eae.ucCurrentCommand[OFFSET_P1], tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0){
                             if (eae.ucReadRecordStatus==READRECORDSTATUS_NUMBER_OVERFLOW){
                               	//Record not found
                               	 tmpbuf2[0]=SW1_6A;
                               	 tmpbuf2[1]=0x83;
                               	 TransferEaeToEmu(tmpbuf2, 2 ) ;             	 
                            	}
                             else	
                                 TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                           }
                      else{
                           memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
    	   	       	          memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
    	   	       	          if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
	 	     	          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
	 	     	          	      else{
	 	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
	 	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
	 	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
	 	     	          	       	     }
	 	     	          	      } 
    	   	       	          TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
                      }
                }                         	  
      	    }	 
     } 
  }
  else{
      	       if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber>0xf00)  
       	   	       	 TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
    	         else	{   
        	     	         if (GetFileRecordResponse(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber,   &tmpresponse)<0)
       	     	                TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
        	               	else{ 	   										        		      	       	   	              
        	   	                   if (eae.ucCurrentCommand[OFFSET_P2]!=4)  //  04：ABSOLUTE模式/CURRENT模式
        	   	       	                 TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
        	   	                   else{	     
        	   	                     if (GetFileRecord(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber,  eae.ucCurrentCommand[OFFSET_P1],  tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0){  	                                                                    
                                         if (eae.ucReadRecordStatus==READRECORDSTATUS_NUMBER_OVERFLOW){
                               	            //Record not found
                                           	 tmpbuf2[0]=SW1_6A;
                               	             tmpbuf2[1]=0x83;
                               	             TransferEaeToEmu(tmpbuf2, 2 ) ;             	 
                            	           }
                                         else	
                                           TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                                   }        
                                   else{
                                         memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
        	   	       	                   memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
        	   	       	                   if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	 	     	          	       	              memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
    	 	     	          	               else{
    	 	     	          	       	                if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	 	     	          	       	      	               tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
    	 	     	          	       	      	               tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
    	 	     	          	       	                }
    	 	     	          	               } 
        	   	       	                   TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
                                   }
                                 }                         	  
          	              }	 
               } 
      	
  }
}	

int Emu_Engine::SubProcessUpdateRecordCommandArgumentUsim(){
//	int vcardindexlistserialnumber, tmpfilecounter,tmplength;
 unsigned char tmpilengh;
 unsigned char tmpbuf2[2],tmpresponse[2];
 unsigned short readoffset,tmpreadfilelength;  
 unsigned short tmpfilebodylength;
 
 if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	       TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();       	 	      	        
 }
 else { 
 if (!eae.ucCurrentProcessLogicalChannel){
     if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
   	     TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim(); 
	   else	{ 
	   	     {
    	     	if (GetFileRecordResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0)
   	     	       TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
    	     	else{ 	   										        		      	       	   	              
    	   	       if (eae.ucCurrentCommand[OFFSET_P2]!=4)  //  04：ABSOLUTE模式/CURRENT模式
    	   	       	     TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim(); 
    	   	       else{	     
    	   	            if (SetFileRecord(eae.iCurrentVcardIndexListSerialnumber,  eae.ucCurrentCommand[OFFSET_P1],  eae.ucCurrentArgument, eae.ucCurrentCommand[OFFSET_P3])<0)  	                                                                    
                            TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                      else{
                            if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	            memcpy(tmpbuf2,tmpresponse ,2);	
          	                else{
          	       	              if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	        tmpbuf2[0]=0x91;
          	       	      	        tmpbuf2[1]=eae.ucCurrentCatLength;
          	       	              }
          	                }
      	       	            TransferEaeToEmu(tmpbuf2, 2 ) ;
                      }
                }                         	  
      	    }	 
     }
   } 
  }
  else{
      	       if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber>0xf00)  
       	   	       	 TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
    	         else	{   
        	     	         if (GetFileRecordResponse(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber,   &tmpresponse)<0)
       	     	                TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
        	               	else{ 	   										        		      	       	   	              
        	   	                   if (eae.ucCurrentCommand[OFFSET_P2]!=4)  //  04：ABSOLUTE模式/CURRENT模式
        	   	       	                 TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
        	   	                   else{	     
        	   	                     if (SetFileRecord(eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber,  eae.ucCurrentCommand[OFFSET_P1],  eae.ucCurrentArgument, eae.ucCurrentCommand[OFFSET_P3])<0)  	                                                                    
                                           TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
                                   else{
                                        if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	                       memcpy(tmpbuf2,tmpresponse ,2);	
          	                            else{
          	       	                          if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	                      tmpbuf2[0]=0x91;
          	       	      	                      tmpbuf2[1]=eae.ucCurrentCatLength;
          	       	                          }
          	                            }
      	       	                        TransferEaeToEmu(tmpbuf2, 2 ) ;
                                   }
                                 }                         	  
          	              }	 
               } 
      	
  }
 }
}	
int Emu_Engine::SubProcessUpdateRecordCommandArgumentSimOrUim(){

 unsigned char tmpilengh;
 unsigned char tmpbuf2[2],tmpresponse[2];
 unsigned short readoffset,tmpreadfilelength;  
 unsigned short tmpfilebodylength;
 
 if (eae.ucCurrentFilePrority!=CURRENTFILEPRORITY_FORK_AVAILABLE){
     	 	        Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();    	 	      	        
 }
 else { 

     if (eae.iCurrentVcardIndexListSerialnumber>0xf00)  
   	      Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
	   else	{ 
	   	     {
    	     	if (GetFileRecordResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0)
   	     	        Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
    	     	else{ 	   										        		      	       	   	              
    	   	       if (eae.ucCurrentCommand[OFFSET_P2]!=4)  //  04：ABSOLUTE模式/CURRENT模式
    	   	       	      Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();  
    	   	       else{	     
    	   	            if (SetFileRecord(eae.iCurrentVcardIndexListSerialnumber,  eae.ucCurrentCommand[OFFSET_P1],  eae.ucCurrentArgument, eae.ucCurrentCommand[OFFSET_P3])<0)  	                                                                    
                             Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
                      else{
                            if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
          	       	            memcpy(tmpbuf2,tmpresponse ,2);	
          	                else{
          	       	              if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
          	       	      	        tmpbuf2[0]=0x91;
          	       	      	        tmpbuf2[1]=eae.ucCurrentCatLength;
          	       	              }
          	                }
      	       	            TransferEaeToEmu(tmpbuf2, 2 ) ;
                      }
                }                         	  
      	    }	 
     }
   } 

 }
}	

int Emu_Engine::SubProcessReadBinaryCommandUim(){                                                             
unsigned char tmpresponse[2],tmpbuf1[READ_BIANARY_BUF_MAX_LENGTH],tmpbuf2[READ_BIANARY_BUF_MAX_LENGTH];	 
unsigned short tmpfilebodylength;
unsigned short readoffset,tmpreadfilelength;  
unsigned short tmp_p3;

if (!eae.ucCurrentCommand[OFFSET_P3])
     tmp_p3=0x100;
  else
     tmp_p3=eae.ucCurrentCommand[OFFSET_P3];
         
  	
     if (eae.iCurrentVcardIndexListSerialnumber>0xf00)
     	   TransferEaeToNet(NETAPI_CURRENTCMD);
     else	{   
        	if (GetFileBodyResponse(eae.iCurrentVcardIndexListSerialnumber,   &tmpresponse)<0){
  	     	       TransferEaeToNet(NETAPI_CURRENTCMD);
  	     	     }
  	     	else{
  	     		   
  	   	       tmpfilebodylength=(unsigned short)GetFileBodyLength(eae.iCurrentVcardIndexListSerialnumber);          	      
      	       if (tmpfilebodylength<0){
      	          	 TransferEaeToNet(NETAPI_CURRENTCMD);
      	          	}
      	       else{
      	       	    if (GetFileBodyContext(eae.iCurrentVcardIndexListSerialnumber,tmpbuf1,tmpfilebodylength )<0){
                          TransferEaeToNet(NETAPI_CURRENTCMD);
                        }
                    else{
                         readoffset= eae.ucFromEmuDataBuf[OFFSET_P1]*256+eae.ucFromEmuDataBuf[OFFSET_P2];  
                         if (! (eae.ucCurrentSimCardIdentifierProperty&LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0)){                         
                            if ((readoffset+tmp_p3)>tmpfilebodylength){
                            	    std::cout<<RED<<"Warning! length overflow"<<RESET<<std::endl<<std::flush;	
                            	    TransferEaeToNet(NETAPI_CURRENTCMD);
                            }
                            else	 {   	                                   
                                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
           	       	                  memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmp_p3);	           	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(&tmpbuf2[tmp_p3+1],tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[tmp_p3+1]=0x91;
    	          	       	      	                   tmpbuf2[tmp_p3+2]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, tmp_p3+3 ) ;
           	       	        }
           	       	     }
           	       	     else{      //LONG_TRANSPARENT_EF_CUT_TAIL_DEFAULT_0 文件      	     	     	       	                                   
                                      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
                                      
                                      if (readoffset>=tmpfilebodylength)
                                      	    memset(&tmpbuf2[1], 0, tmp_p3);	
                                      else
                                      if ((readoffset+tmp_p3)>tmpfilebodylength){
                                      	  memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmpfilebodylength-readoffset);
                                      	  memset(&tmpbuf2[1+tmpfilebodylength-readoffset], 0, \
                                      	  readoffset+tmp_p3-tmpfilebodylength);	
                                      	}
                                      else
           	       	                      memcpy(&tmpbuf2[1],&tmpbuf1[readoffset] ,tmp_p3);
           	       	                      	           	       	    
           	       	                  if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
    	          	       	                   memcpy(&tmpbuf2[tmp_p3+1],tmpresponse ,2);	
    	          	                      else{
    	          	       	                     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
    	          	       	                     	    tmpbuf2[tmp_p3+1]=0x91;
    	          	       	      	                   tmpbuf2[tmp_p3+2]=eae.ucCurrentCatLength;
    	          	       	                      }
    	          	                      }
           	       	                 TransferEaeToEmu(tmpbuf2, tmp_p3+3 ) ;
           	       	           
           	       	     	
           	       	     	}
           	       	     
           	       	        
                    }
               }	  
    	    }	     
     }
     return 1;
}

int Emu_Engine::SubProcessStatusCommandUim(){ 
int vcardindexlistserialnumber	;
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpidentifier[IDENTIFIERDEEPTH];	 
unsigned char currentdir[2];
 vcard_file_property_t file1;
    if (GetCurrentDir(currentdir )!=1)  //如果无法找到当前的目录文件
    	  return -1;
    if (GetVcardFileProperty_Uim(currentdir, &file1)<0){ 
   	    	 TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);
   	    	 std::cout<<RED<<"The Identifier is not include in  3GPP2 C.S0023-C, deliver status to Uim card "<<RESET<<std::endl<<std::flush;
             return -1;
    }
    
    if (!((file1.property&EXIST_SAME_EF_NAME))||((file1.property&EXIST_SAME_DF_NAME))){ //不是重名文件 	  
        if( (vcardindexlistserialnumber= GetIdentifierIndex(file1.identifier))<0)
      	    TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
        else	{
	              if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
	          	        TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
	              else{
	          	         if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0)
	          	       	     TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
	          	         else{
	          	       	       memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	          	       	       memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
	          	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
  	     	          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
  	     	          	       else{
  	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
  	     	          	       	     }
  	     	          	       }
	          	       	   TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
	          	    }    
	          }	     
        }   
    }
    else{
    	    if( (vcardindexlistserialnumber= GetIdentifierIndexWithPath(currentdir))<0)   //同名文件 但Vcard 中找不到
                TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);	  	        	   										        		      	     				  
         else{
               if (GetIdentifierResponse(vcardindexlistserialnumber,   &tmpresponse)<0)
 	                TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);	      //同名文件 但Vcard 中找不到响应字    										        		      	     				   	    
 	             else{	     	  	        	   										        		      	     				   	    	     	   										        		      	     				   	    	    
 	    	               if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0)
	          	       	    TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);	//同名文件但Vcard 中找不到文件头  
	          	         else{
	          	       	      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	          	       	      memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
	          	        	    if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
  	     	          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
  	     	          	      else{
  	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
  	     	          	       	     }
  	     	          	      }
	          	       	   TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;				        		      	     				   	    	        
 	    	               } 	   										      		   	                            
 	             }         
         }    	
    	}
    	return 1;
}

void Emu_Engine::SubProcessStatusCurrentAidUsim(unsigned char ucCurrentProcessLogicalChannel ){
int vcardindexlistserialnumber,tmpt	;
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpidentifier[IDENTIFIERDEEPTH];	 
//unsigned char currentdir[2];
 vcard_file_property_t file1;
 unsigned char aidbuf[2];
unsigned char tmpilengh;
		
         	         aidbuf[0]=0x7f;
         	         aidbuf[1]=0xff;
         	         if (!ucCurrentProcessLogicalChannel)
         	         	  vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(2,aidbuf,eae.ucCurrentSimCardRidType);
         	         else
         	         	   vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(2,aidbuf,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type);
       	           if (vcardindexlistserialnumber<0)    		  	   	      	          		  	   	      		      	
  	      	            TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
  	      	       else{
  	      	    	       if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
	 	     	          	        TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	   else{
	 	     	          		    if (GetFileHeaderLength(vcardindexlistserialnumber,&tmpilengh)>0){
	 	     	          		         if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, tmpilengh)<0)
	 	     	          	       	       TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	           else{
	 	     	          	      	        tmpt= GetTagLocation_inFCP(TAG_RESONSE_DF_NAME_WITH_FCP_TEMPLATE,tmpbuf1, tmpilengh-2);	 	 	 	 	     	    	    	    
	                                    if ((tmpt==-1)||((tmpt+0x12)>tmpilengh))
	                                    	TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	      	        else{	 	     	          	      	        
	 	     	          	      	             memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	 	     	          	      	             
   	       	     	          	       	     memcpy(&tmpbuf2[1],&(tmpbuf1[tmpt]) ,0x12);	 
      	  
   	       	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
		      	       	     	          	       	   memcpy(&tmpbuf2[0x12+1],tmpresponse ,2);	
		      	       	     	          	     else{
		      	       	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
		      	       	     	          	       	      	    tmpbuf2[0x12+1]=0x91;
		      	       	     	          	       	      	    tmpbuf2[0x12+2]=eae.ucCurrentCatLength;
		      	       	     	          	       	     }
		      	       	     	          	     }
   	       	     	          	       	    TransferEaeToEmu(tmpbuf2, 0x12+3 ) ;  	       	     	          	        
  	          	      	            }
	 	     	          	      	  }   
	 	     	          	      }		 	 	     	          		
	 	     	          		}      	      	    	
  	      	    }          	     	
}

int Emu_Engine::SubProcessStatusCurrentAidDfLengthUsim(unsigned char ucCurrentProcessLogicalChannel ){
int vcardindexlistserialnumber,tmpt	;
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpidentifier[IDENTIFIERDEEPTH];	 
//unsigned char currentdir[2];
 vcard_file_property_t file1;
 unsigned char aidbuf[2];
unsigned char tmpilengh;
		
         	         aidbuf[0]=0x7f;
         	         aidbuf[1]=0xff;
         	         if (!ucCurrentProcessLogicalChannel)
         	         	  vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(2,aidbuf,eae.ucCurrentSimCardRidType);
         	         else
         	         	   vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(2,aidbuf,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type);
       	           if (vcardindexlistserialnumber<0)    		  	   	      	          		  	   	      		      	
  	      	            return -1;
  	      	       else{
  	      	    	       if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
	 	     	          	       return -1;
	 	     	          	   else{
	 	     	          		    if (GetFileHeaderLength(vcardindexlistserialnumber,&tmpilengh)>0){
	 	     	          		         if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, tmpilengh)<0)
	 	     	          	       	       return -1;
	 	     	          	           else{
	 	     	          	      	        tmpt= GetTagLocation_inFCP(TAG_RESONSE_DF_NAME_WITH_FCP_TEMPLATE,tmpbuf1, tmpilengh-2);	 	 	 	 	     	    	    	    
	                                    if ((tmpt==-1)||((tmpt+0x12)>tmpilengh))
	                                    	return -1;
	 	     	          	      	        else{	 	     	          	      	        
	 	     	          	      	             return (tmpbuf1[tmpt+1]+2);
	 	     	          	      	               	       	     	          	        
  	          	      	            }
	 	     	          	      	  }   
	 	     	          	      }		 	 	     	          		
	 	     	          		}      	      	    	
  	      	    }          	     	
}
int Emu_Engine::SubProcessStatusCommandUsim(){ 
int vcardindexlistserialnumber,tmpt	;
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpidentifier[IDENTIFIERDEEPTH];	 
//unsigned char currentdir[2];
 vcard_file_property_t file1;
 unsigned char aidbuf[2];
unsigned char tmpilengh;
      if (!eae.ucCurrentProcessLogicalChannel){ 
      	       //No indication                           No data returned                        Empty,
      	// if ((eae.ucFromEmuDataBuf[OFFSET_P1]==0X0)&&(eae.ucFromEmuDataBuf[OFFSET_P2]==0Xc)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0X0)){
      	   if ((eae.ucFromEmuDataBuf[OFFSET_P2]==0Xc)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0X0)){
                if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD){
 	          	       	   tmpbuf2[0]=0x90;
 	          	       	   tmpbuf2[1]=0x0;
 	          	       	 }
 	          	   else{
 	          	        if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
 	          	       	      	    tmpbuf2[0]=0x91;
 	          	       	      	    tmpbuf2[1]=eae.ucCurrentCatLength;
 	          	       	}
 	          	   }
                 TransferEaeToEmu(tmpbuf2, 2 ) ;
      	 }
      	 else
      	 {     	
         if (!((eae.ucFromEmuDataBuf[OFFSET_P1]==0X0)&&(eae.ucFromEmuDataBuf[OFFSET_P2]==0X0))){ 
         	    //返回当前应用
         	   //if ((eae.ucFromEmuDataBuf[OFFSET_P1]==1)&&(eae.ucFromEmuDataBuf[OFFSET_P2]==1)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0x12)){
         	   if ((eae.ucFromEmuDataBuf[OFFSET_P2]==1)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0x12)){
                     SubProcessStatusCurrentAidUsim(eae.ucCurrentProcessLogicalChannel);
         	    }    
         	    else{   //返回aid 文件长度
         	    	  if ((eae.ucFromEmuDataBuf[OFFSET_P2]==1)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0)){
         	    	  	  tmpt=SubProcessStatusCurrentAidDfLengthUsim(eae.ucCurrentProcessLogicalChannel);
         	    	  	  if (tmpt>0){
         	    	  	  	    tmpbuf2[0]=SW1_6C; // resend command with P3= xx
         	    	  	  	    tmpbuf2[1]=tmpt;
         	    	  	  	    TransferEaeToEmu(tmpbuf2, 2) ;
         	    	  	  	}
         	    	  	  else	
                         TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
         	         }     	   	
         	   	    else	   								
  	       	      TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
  	       	  }    
  	      }
  	      else{      		  	   	      	        		  	   	      	     
  	      	    //找出当前目录或应用
  	      	    switch (eae.ucCurrentSimCardIdentifierLength) {
  	      	    	case 2: 
  	      	    	       if ( (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_MF)||(eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)){
  	      	    	  	  	  eae.ucCurrentDirIdentifierLength=2;
  	      	    	  	  	  eae.ucCurrentDirIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];  // 20180620 
  	      	                eae.ucCurrentDirIdentifier[1]=eae.ucCurrentSimCardIdentifier[1];  // 20180620 
  	      	    	       }
  	      	    	       else {
  	      	    	  	          if (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)
  	      	    	  	            {
  	      	    	  	          	  eae.ucCurrentDirIdentifierLength=2;
  	      	    	  	    	        eae.ucCurrentDirIdentifier[0]=FILE_1ST_ID_MF;  // 20180620 
  	      	                        eae.ucCurrentDirIdentifier[1]=0;  // 20180620 
  	      	    	                } 	   										        		  	   	      	    	  	   	   										        		  	   	      	    	  	
  	      	    	             else{
  	      	    	  	  	          std::cout<<RED<<"UNKNOW uccurrentsimcardidentifier under StatusCommand !"<<RESET<<std::endl<<std::flush;	
	                                  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
	                                  return -1;	
  	      	    	  	  	     }	
  	      	    	       }
  	      	    	       break;
  	      	    	  case 4: 
  	      	    	       if ( (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)&&(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)){
  	      	    	  	  	  eae.ucCurrentDirIdentifierLength=2;
  	      	    	  	  	  eae.ucCurrentDirIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];  // 20180620 
  	      	                eae.ucCurrentDirIdentifier[1]=eae.ucCurrentSimCardIdentifier[1];  // 20180620 
  	      	    	       }
  	      	    	       else {
  	      	    	  	           if ( (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)&&(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_2LEVEL_DF))
  	      	    	  	            {
  	      	    	  	          	   eae.ucCurrentDirIdentifierLength=4;
  	      	    	  	    	         eae.ucCurrentDirIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];  // 20180620 
  	      	                         eae.ucCurrentDirIdentifier[1]=eae.ucCurrentSimCardIdentifier[1];  // 20180620 
  	      	                         eae.ucCurrentDirIdentifier[2]=eae.ucCurrentSimCardIdentifier[2];  // 20180620 
  	      	                         eae.ucCurrentDirIdentifier[3]=eae.ucCurrentSimCardIdentifier[3];  // 20180620 
  	      	    	                } 	   										        		  	   	      	    	  	   	   										        		  	   	      	    	  	
  	      	    	             else{
  	      	    	  	  	          std::cout<<RED<<"UNKNOW uccurrentsimcardidentifier under StatusCommand !"<<RESET<<std::endl<<std::flush;	
	                                  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
	                                  return -1;	
  	      	    	  	  	     }	
  	      	    	       }
  	      	    	       break;
  	      	    	   case 6: 
  	      	    	       if ( (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)&&\
  	      	    	       	(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_2LEVEL_DF)&&(eae.ucCurrentSimCardIdentifier[4]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)){
  	      	    	  	  	   eae.ucCurrentDirIdentifierLength=4;
  	      	    	  	  	   eae.ucCurrentDirIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];  // 20180620 
  	      	                 eae.ucCurrentDirIdentifier[1]=eae.ucCurrentSimCardIdentifier[1];  // 20180620 
  	      	                 eae.ucCurrentDirIdentifier[2]=eae.ucCurrentSimCardIdentifier[2];  // 20180620 
  	      	                 eae.ucCurrentDirIdentifier[3]=eae.ucCurrentSimCardIdentifier[3];  // 20180620 
  	      	    	       }
  	      	    	        										        		  	   	      	    	  	   	   										        		  	   	      	    	  	
  	      	    	       else{
  	      	    	  	  	          std::cout<<RED<<"UNKNOW uccurrentsimcardidentifier under StatusCommand !"<<RESET<<std::endl<<std::flush;	
	                                  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
	                                  return -1;	
  	      	    	  	  }	
  	      	    	       
  	      	    	       break;    
  	      	    	
  	      	    	}    	      	    
  	      	     if( (vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucCurrentDirIdentifierLength,eae.ucCurrentDirIdentifier,eae.ucCurrentRidType))<0)    		  	   	      	          		  	   	      	
  	      	 //  if( (vcardindexlistserialnumber= GetIdentifierIndex_Usim(eae.ucCurrentDirIdentifierLength,eae.ucCurrentDirIdentifier))<0)
  	      	         TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
  	      	    else{
  	      	    	    if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
	 	     	          	    TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	else{
	 	     	          		    if (GetFileHeaderLength(vcardindexlistserialnumber,&tmpilengh)>0){
	 	     	          		      if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, tmpilengh)<0)
	 	     	          	       	   TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	        else{
	 	     	          	      	        if (eae.ucFromEmuDataBuf[OFFSET_P3]){
	 	     	          	      	             memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
   	       	     	          	       	    memcpy(&tmpbuf2[1],tmpbuf1 ,tmpilengh);	 
      	  
   	       	     	          	       	    if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
		      	       	     	          	       	   memcpy(&tmpbuf2[tmpilengh+1],tmpresponse ,2);	
		      	       	     	          	      else{
		      	       	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
		      	       	     	          	       	      	    tmpbuf2[tmpilengh+1]=0x91;
		      	       	     	          	       	      	    tmpbuf2[tmpilengh+2]=eae.ucCurrentCatLength;
		      	       	     	          	       	     }
		      	       	     	          	      }
   	       	     	          	       	   TransferEaeToEmu(tmpbuf2, tmpilengh+3 ) ;
   	       	     	          	      }
   	       	     	          	      else   {  // 向EMU报告文件头长度 	   
	 	     	          	      	                 tmpbuf2[0]=SW1_61;      //TS 102.221 Table 7.1 Procedure byte coding
	 	     	          	      	                 tmpbuf2[1]=tmpilengh;  
	 	     	          	      	                 TransferEaeToEmu(tmpbuf2, 2 ) ; 
	 	     	          	      	                 
  	          	      	              }
	 	     	          	      	} 
	 	     	          	      }		 	 	     	          		
	 	     	          		}      	      	    	
  	      	    }   
  	      	}
  	     } 
  	} 
  	else{
  		    if ((eae.ucFromEmuDataBuf[OFFSET_P1]==0X0)&&(eae.ucFromEmuDataBuf[OFFSET_P2]==0Xc)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0X0)){
                if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD){
 	          	       	   tmpbuf2[0]=0x90;
 	          	       	   tmpbuf2[1]=0x0;
 	          	       	 }
 	          	   else{
 	          	        if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
 	          	       	      	    tmpbuf2[0]=0x91;
 	          	       	      	    tmpbuf2[1]=eae.ucCurrentCatLength;
 	          	       	}
 	          	   }
                 TransferEaeToEmu(tmpbuf2, 2 ) ;
      	 }
      	 else
  		    {
  		    if (!((eae.ucFromEmuDataBuf[OFFSET_P1]==0X0)&&(eae.ucFromEmuDataBuf[OFFSET_P2]==0X0))) {
  			 	   //返回当前应用
         	   if ((eae.ucFromEmuDataBuf[OFFSET_P1]==1)&&(eae.ucFromEmuDataBuf[OFFSET_P2]==1)&&(eae.ucFromEmuDataBuf[OFFSET_P3]==0x12)){
                     SubProcessStatusCurrentAidUsim(eae.ucCurrentProcessLogicalChannel);
         	    }         	   	
         	   	else	 								
  	       	      TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
          }  	    
  	      else{      		  	   	      	        		  	   	      	     
  	      	    //找出当前目录或应用  	      	    
  	      	    switch (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength) {
  	      	    	case 2: 
  	      	    	       if ( (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_MF)||(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_1LEVEL_DF)){
  	      	    	  	  	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifierLength=2;
  	      	    	  	  	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];  // 20180620 
  	      	                eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];  // 20180620 
  	      	    	       }
  	      	    	       else {
  	      	    	  	          if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_UNDER_MF_EF)
  	      	    	  	            {
  	      	    	  	          	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifierLength=2;
  	      	    	  	    	        eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[0]=FILE_1ST_ID_MF;  // 20180620 
  	      	                        eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[1]=0;  // 20180620 
  	      	    	                } 	   										        		  	   	      	    	  	   	   										        		  	   	      	    	  	
  	      	    	             else{
  	      	    	  	  	          std::cout<<RED<<"CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<< " UNKNOW uccurrentsimcardidentifier under StatusCommand !"<<RESET<<std::endl<<std::flush;	
	                                  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
	                                  return -1;	
  	      	    	  	  	     }	
  	      	    	       }
  	      	    	       break;
  	      	    	  case 4: 
  	      	    	       if ( (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_1LEVEL_DF)&&(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)){
  	      	    	  	  	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifierLength=2;
  	      	    	  	  	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];  // 20180620 
  	      	                eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];  // 20180620 
  	      	    	       }
  	      	    	       else {
  	      	    	  	           if ( (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_1LEVEL_DF)&&(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2]==FILE_1ST_ID_2LEVEL_DF))
  	      	    	  	            {
  	      	    	  	          	   eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifierLength=4;
  	      	    	  	    	         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];  // 20180620 
  	      	                         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];  // 20180620 
  	      	                         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[2]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2];  // 20180620 
  	      	                         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[3]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[3];  // 20180620 
  	      	    	                } 	   										        		  	   	      	    	  	   	   										        		  	   	      	    	  	
  	      	    	             else{
  	      	    	  	  	          std::cout<<RED<<"CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<<" UNKNOW uccurrentsimcardidentifier under StatusCommand !"<<RESET<<std::endl<<std::flush;	
	                                  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
	                                  return -1;	
  	      	    	  	  	     }	
  	      	    	       }
  	      	    	       break;
  	      	    	   case 6: 
  	      	    	       if ( (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_1LEVEL_DF)&&\
  	      	    	       	(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2]==FILE_1ST_ID_2LEVEL_DF)&&(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[4]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)){
  	      	    	  	  	   eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifierLength=4;
  	      	    	  	    	         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];  // 20180620 
  	      	                         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];  // 20180620 
  	      	                         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[2]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2];  // 20180620 
  	      	                         eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_DirIdentifier[3]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[3];  // 20180620   	      	    	                
  	      	    	       }
  	      	    	        										        		  	   	      	    	  	   	   										        		  	   	      	    	  	
  	      	    	       else{
  	      	    	  	  	          std::cout<<RED<<"CH"<<(unsigned short) eae.ucCurrentProcessLogicalChannel<<" UNKNOW uccurrentsimcardidentifier under StatusCommand !"<<RESET<<std::endl<<std::flush;	
	                                  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
	                                  return -1;
  	      	    	  	  }	
  	      	    	       
  	      	    	       break;    
  	      	    	
  	      	    	}    	      	    
  	      	     if( (vcardindexlistserialnumber= GetIdentifierIndex_UsimWithRidType(eae.ucCurrentDirIdentifierLength,eae.ucCurrentDirIdentifier,eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type))<0)    		  	   	      	          		  	   	      	
  	      	 //  if( (vcardindexlistserialnumber= GetIdentifierIndex_Usim(eae.ucCurrentDirIdentifierLength,eae.ucCurrentDirIdentifier))<0)
  	      	         TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
  	      	    else{
  	      	    	    if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
	 	     	          	    TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	else{
	 	     	          		    if (GetFileHeaderLength(vcardindexlistserialnumber,&tmpilengh)>0){
	 	     	          		      if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, tmpilengh)<0)
	 	     	          	       	   TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
	 	     	          	        else{
	 	     	          	      	        if (eae.ucFromEmuDataBuf[OFFSET_P3]){
	 	     	          	      	             memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
   	       	     	          	       	    memcpy(&tmpbuf2[1],tmpbuf1 ,tmpilengh);	 
      	  
   	       	     	          	       	    if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
		      	       	     	          	       	   memcpy(&tmpbuf2[tmpilengh+1],tmpresponse ,2);	
		      	       	     	          	      else{
		      	       	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
		      	       	     	          	       	      	    tmpbuf2[tmpilengh+1]=0x91;
		      	       	     	          	       	      	    tmpbuf2[tmpilengh+2]=eae.ucCurrentCatLength;
		      	       	     	          	       	     }
		      	       	     	          	      }
   	       	     	          	       	   TransferEaeToEmu(tmpbuf2, tmpilengh+3 ) ;
   	       	     	          	      }
   	       	     	          	      else   {  // 向EMU报告文件头长度 	   
	 	     	          	      	                 tmpbuf2[0]=SW1_61;      //TS 102.221 Table 7.1 Procedure byte coding
	 	     	          	      	                 tmpbuf2[1]=tmpilengh;  
	 	     	          	      	                 TransferEaeToEmu(tmpbuf2, 2 ) ; 
	 	     	          	      	                 
  	          	      	              }
	 	     	          	      	} 
	 	     	          	      }		 	 	     	          		
	 	     	          		}      	      	    	
  	      	    }   
  	      	} 
  	      }
  	}      	     	
}

// 根据SFI 和CurrentIdentifier 产生ExpectedIdentifier 通过vcard_file_property_list_Usim
int Emu_Engine::GenExpectedFidWithSfi_Usim(){   //SFI 是EF文件, 假设Usim 是带路径选择
vcard_file_property_t tmpfile;
unsigned char tmpCounter,tmpExpectIdentifierLength=0;
unsigned char tmpridtype;
	int iListCounter;	
eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_UNAVAILABLE;
	
if (!eae.ucCurrentProcessLogicalChannel){	  //通道0
	          tmpridtype=eae.ucCurrentRidType;
          	if (eae.ucCurrentSimCardIdentifierLength==2){
          		  if((eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_MF)||(eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)){
          		  	tmpfile.parent_identifier[0]=0x3F;
          		  	tmpfile.parent_identifier[1]=0;
          		  	tmpfile.grandfather_identifier[0]=0x3F;
          		  	tmpfile.grandfather_identifier[1]=0x0;
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0x0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=2;
          		  }
                else
          		  if(eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF){
          		  	tmpfile.parent_identifier[0]=eae.ucCurrentSimCardIdentifier[0];
          		  	tmpfile.parent_identifier[1]=eae.ucCurrentSimCardIdentifier[1];
          		  	tmpfile.grandfather_identifier[0]=0x3F;
          		  	tmpfile.grandfather_identifier[1]=0x0;
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0x0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=4;
          		  }
          		  else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  	}
          		  		
          	}
          	else
          	if (eae.ucCurrentSimCardIdentifierLength==4){
          		  if(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF){ 
          		  	tmpfile.parent_identifier[0]=eae.ucCurrentSimCardIdentifier[0];
          		  	tmpfile.parent_identifier[1]=eae.ucCurrentSimCardIdentifier[1];
          		  	tmpfile.grandfather_identifier[0]=0x3F;
          		  	tmpfile.grandfather_identifier[1]=0x0;
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0x0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=4;
          		  }	
          		  else
          		  if(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_2LEVEL_DF){ 
          		  	tmpfile.parent_identifier[0]=eae.ucCurrentSimCardIdentifier[2];
          		  	tmpfile.parent_identifier[1]=eae.ucCurrentSimCardIdentifier[3];
          		  	tmpfile.grandfather_identifier[0]=eae.ucCurrentSimCardIdentifier[0];
          		  	tmpfile.grandfather_identifier[1]=eae.ucCurrentSimCardIdentifier[1];
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0x0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=6;
          		  }
          		  else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  }
          	}
          	else
          	if (eae.ucCurrentSimCardIdentifierLength==6){
          		  if(eae.ucCurrentSimCardIdentifier[4]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){ 
          		  	tmpfile.parent_identifier[0]=eae.ucCurrentSimCardIdentifier[2];
          		  	tmpfile.parent_identifier[1]=eae.ucCurrentSimCardIdentifier[3];
          		  	tmpfile.grandfather_identifier[0]=eae.ucCurrentSimCardIdentifier[0];
          		  	tmpfile.grandfather_identifier[1]=eae.ucCurrentSimCardIdentifier[1];
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0x0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=6;
          		  }			  
          		  else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  }
          	}
          	else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  }
	}	
	else{      //通道非0
		       tmpridtype=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type;
		       
		       if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength==2){
          		  if((eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_MF)|| \
          		  	(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_UNDER_MF_EF)){
					tmpfile.parent_identifier[0]=0x3F;
					tmpfile.parent_identifier[1]=0;
          		  	tmpfile.grandfather_identifier[0]=0x3F;
          		  	tmpfile.grandfather_identifier[1]=0x0;
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=2;
          		  }
                else
          		  if(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]==FILE_1ST_ID_1LEVEL_DF){
          		  	tmpfile.parent_identifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];
          		  	tmpfile.parent_identifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];
          		  	tmpfile.grandfather_identifier[0]=0x3F;
          		  	tmpfile.grandfather_identifier[1]=0x0;
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=4;
          		  }
          		  else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  	}
          		  		
          	}
          	else
          	if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength==4){
          		  if(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF){ 
          		  	tmpfile.parent_identifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];
          		  	tmpfile.parent_identifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];
          		  	tmpfile.grandfather_identifier[0]=0x3F;
          		  	tmpfile.grandfather_identifier[1]=0x0;
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=4;
          		  }	
          		  else
          		  if(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2]==FILE_1ST_ID_2LEVEL_DF){ 
          		  	tmpfile.parent_identifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2];
          		  	tmpfile.parent_identifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[3];
          		  	tmpfile.grandfather_identifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];
          		  	tmpfile.grandfather_identifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=6;
          		  }
          		  else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  }
          	}
          	else
          	if (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength==6){
          		  if(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[4]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){ 
          		  	tmpfile.parent_identifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[2];
          		  	tmpfile.parent_identifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[3];
          		  	tmpfile.grandfather_identifier[0]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0];
          		  	tmpfile.grandfather_identifier[1]=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1];
          		  	tmpfile.great_grandfather_identifier[0]=0x3F;
          		  	tmpfile.great_grandfather_identifier[1]=0;
          		  	tmpfile.sfi=eae.ucCurrentSFI;
          		  	tmpExpectIdentifierLength=6;
          		  }			  
          		  else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  }
          	}
          	else{
          		  	   std::cout<<RED<<" GenExpectedFidWithSfi_Usim fail  "<<RESET<<std::endl<<std::flush;
          		  	   return -1;
          		  }
		
		}
	//	  
	
		
 // 找到SFI 对应的identifier
		for (iListCounter=0;iListCounter<vcard_file_property_list_Usim_num;iListCounter++){			 			 
			  	 if ((vcard_file_property_list_Usim[iListCounter].parent_identifier[0]== tmpfile.parent_identifier[0])\
			  	   && (vcard_file_property_list_Usim[iListCounter].parent_identifier[1]== tmpfile.parent_identifier[1]) \
			  	   && (vcard_file_property_list_Usim[iListCounter].grandfather_identifier[0]== tmpfile.grandfather_identifier[0]) \
			  	   && (vcard_file_property_list_Usim[iListCounter].grandfather_identifier[1]== tmpfile.grandfather_identifier[1]) \
			  	   && (vcard_file_property_list_Usim[iListCounter].great_grandfather_identifier[0]== tmpfile.great_grandfather_identifier[0]) \
			  	   && (vcard_file_property_list_Usim[iListCounter].great_grandfather_identifier[1]== tmpfile.great_grandfather_identifier[1]) \
			  	    && (vcard_file_property_list_Usim[iListCounter].sfi== tmpfile.sfi)\
			  	    && (vcard_file_property_list_Usim[iListCounter].application_id_type== tmpridtype) 
			  	   // && (vcard_file_property_list_Usim[iListCounter].application_id_type== eae.ucCurrentRidType) 
			  	   
			  	   ){
			  	    tmpfile.identifier[0]=vcard_file_property_list_Usim[iListCounter].identifier[0];
			  	    tmpfile.identifier[1]=vcard_file_property_list_Usim[iListCounter].identifier[1];
			  	   break;
			  	   
			      }
		}
	  if (iListCounter==vcard_file_property_list_Usim_num){
			
			 return -1;
	  }
	  else{
	  	    if (vcard_file_property_list_Usim[iListCounter].property&CAN_FORK)
	  	    	  eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_AVAILABLE;
	  	
	  }
	  
    eae.ucExpectIdentifierLength=tmpExpectIdentifierLength;
    if (eae.ucExpectIdentifierLength==2){
    	  eae.ucExpectIdentifier[0]=tmpfile.identifier[0];
    	  eae.ucExpectIdentifier[1]=tmpfile.identifier[1];
    	}
    if (eae.ucExpectIdentifierLength==4){
    	  eae.ucExpectIdentifier[0]=tmpfile.parent_identifier[0];
    	  eae.ucExpectIdentifier[1]=tmpfile.parent_identifier[1];
    	  eae.ucExpectIdentifier[2]=tmpfile.identifier[0];
    	  eae.ucExpectIdentifier[3]=tmpfile.identifier[1];
    }
    if (eae.ucExpectIdentifierLength==6){
    	  eae.ucExpectIdentifier[0]=tmpfile.grandfather_identifier[0];
    	  eae.ucExpectIdentifier[1]=tmpfile.grandfather_identifier[1];
    	  eae.ucExpectIdentifier[2]=tmpfile.parent_identifier[0];
    	  eae.ucExpectIdentifier[3]=tmpfile.parent_identifier[1];
    	  eae.ucExpectIdentifier[4]=tmpfile.identifier[0];
    	  eae.ucExpectIdentifier[5]=tmpfile.identifier[1];
    }

	  return 1;	
    
}	

// 根据SFI 和CurrentIdentifier 产生ExpectedIdentifier    通过VcardIndexList_Usim
int Emu_Engine::GenExpectedFidWithSfiAccordingIndexList_Usim(){   //SFI 是EF文件, 假设Usim 是带路径选择
vcard_file_property_t file1;
unsigned char tmpCounter,tmpExpectIdentifierLength=0;
unsigned char tmpridtype;
	int iListCounter;	
	int idlength,filenum;
 eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_UNAVAILABLE;
  
  // if ((!eae.ucCurrentProcessLogicalChannel)&&(eae.ucCurrentSFI)){ 
	    for ( filenum=0;filenum<MAX_VCARDINDEX;filenum++)
    	    if (VcardIndexList_Usim[filenum].ptr_identifier!=0)
    	    	 if (
    	    	     ((!eae.ucCurrentProcessLogicalChannel)&&(eae.ucCurrentSFI)&&(eae.ucCurrentSFI==VcardIndexList_Usim[filenum].ucSfi)&&\
    	    	     (eae.ucCurrentRidType==((*VcardIndexList_Usim[filenum].ptr_access_flag)>>5)))||\
    	    	     ((eae.ucCurrentProcessLogicalChannel)&&(eae.ucCurrentSFI)&&(eae.ucCurrentSFI==VcardIndexList_Usim[filenum].ucSfi)&&\
    	    	     (eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type==((*VcardIndexList_Usim[filenum].ptr_access_flag)>>5)))
    	    		  )    	    	
    	    	 	 {	  	
    	    	  	  if (eae.ucCurrentSimCardIdentifierLength==2){
    	    	  	  	  if((eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_MF)||(eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)){
          		  	       if ((*VcardIndexList_Usim[filenum].ptr_identifier)==FILE_1ST_ID_UNDER_MF_EF)          		  	       	   
    	                        break;  	       	  
          		           
          		        }
          		        else
          		        if(eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_1LEVEL_DF){
          		  	       if (
          		  	       	     ((*VcardIndexList_Usim[filenum].ptr_identifier)==FILE_1ST_ID_1LEVEL_DF)&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+1)==eae.ucCurrentSimCardIdentifier[1])
          		  	       	  )
          		  	       	
    	                        break;    	                   
          		        }    	    	  	       
    	    	      }
    	    	      else
    	    	      if (eae.ucCurrentSimCardIdentifierLength==4){
    	    	  	  	   if(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF){ 
          		  	         if (
          		  	       	     ((*VcardIndexList_Usim[filenum].ptr_identifier)==eae.ucCurrentSimCardIdentifier[0])&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+1)==eae.ucCurrentSimCardIdentifier[1])
          		  	       	  )
          		  	       	  break;
          		         }	
          		         else
          		         if(eae.ucCurrentSimCardIdentifier[2]==FILE_1ST_ID_2LEVEL_DF){ 
          		  	          if (
          		  	       	     ((*VcardIndexList_Usim[filenum].ptr_identifier)==eae.ucCurrentSimCardIdentifier[0])&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+1)==eae.ucCurrentSimCardIdentifier[1])&&\
          		  	       	     ((*VcardIndexList_Usim[filenum].ptr_identifier+2)==eae.ucCurrentSimCardIdentifier[2])&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+3)==eae.ucCurrentSimCardIdentifier[3])
          		  	       	  )
          		  	       	  break;
          		  	
          		         }          		   
    	    	      }
    	    	      else
    	    	      if (eae.ucCurrentSimCardIdentifierLength==6){
          		         if(eae.ucCurrentSimCardIdentifier[4]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF){ 
          		  	         if (
          		  	       	     (*VcardIndexList_Usim[filenum].ptr_identifier==eae.ucCurrentSimCardIdentifier[0])&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+1)==eae.ucCurrentSimCardIdentifier[1])&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+2)==eae.ucCurrentSimCardIdentifier[2])&&\
          		  	       	     (*(VcardIndexList_Usim[filenum].ptr_identifier+3)==eae.ucCurrentSimCardIdentifier[3])
          		  	       	  )
          		  	       	  break;
          		         }			  
          		  
          	      }
    	    	  }
  // }
   
    		
    	if (filenum==MAX_VCARDINDEX){
    		   std::cout<<RED<<" GenExpectedFidWithSfiAccordingIndexList_Usim SFI no exist  "<<RESET<<std::endl<<std::flush;
    		   return -1;
      }
    		
    	if (!VcardIndexList_Usim[filenum].ptr_identifier){
    		   std::cout<<RED<<" GenExpectedFidWithSfiAccordingIndexList_Usim fail  "<<RESET<<std::endl<<std::flush;
    		   return -1;
      }
    	else{
    		    eae.ucExpectIdentifierLength=*VcardIndexList_Usim[filenum].ptr_identifier_length;
          	for (idlength=0;idlength<(*VcardIndexList_Usim[filenum].ptr_identifier_length);idlength++)
          		  eae.ucExpectIdentifier[idlength]=*(VcardIndexList_Usim[filenum].ptr_identifier+idlength);    		
    		} 
     
    if (!eae.ucCurrentProcessLogicalChannel) {
      if (GetVcardFileProperty_WithUsimRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
      	     		  	             eae.ucCurrentRidType,&file1) <1)
           return -1;
      else
      	   if (file1.property&CAN_FORK) 
      	   	 eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_AVAILABLE; 
    }
    else
    {
      if (GetVcardFileProperty_WithUsimRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
      	     		  	             eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type,&file1) <1)
           return -1;
      else
      	   if (file1.property&CAN_FORK) 
      	   	 eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_AVAILABLE; 
    }	  	   	 
     
     return 1;	
}

int Emu_Engine::SubProcessStatusCommandSim(){ 
int vcardindexlistserialnumber	;
unsigned char tmpresponse[2],tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpidentifier[IDENTIFIERDEEPTH];	 
unsigned char currentdir[2];
 vcard_file_property_t file1;
    if (GetCurrentDir(currentdir )!=1)  //如果无法找到当前的目录文件
    	  return -1;
    if (GetVcardFileProperty(currentdir, &file1)<0){ 
   	    	 TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);
   	    	 std::cout<<RED<<"The Identifier is not include in GSM11.11, deliver status to Uim card "<<RESET<<std::endl<<std::flush;
             return -1;
    }
    
    if (!((file1.property&EXIST_SAME_EF_NAME))||((file1.property&EXIST_SAME_DF_NAME))){ //不是重名文件 	  
        if( (vcardindexlistserialnumber= GetIdentifierIndex(file1.identifier))<0)
      	    TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
        else	{
	              if (GetFileHeaderResponse(vcardindexlistserialnumber,&tmpresponse)<0)
	          	        TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
	              else{
	          	         if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0)
	          	       	     TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
	          	         else{
	          	       	       memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	          	       	       memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
	          	       	   //memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
	          	       	       if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
  	     	          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
  	     	          	       else{
  	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
  	     	          	       	     }
  	     	          	       }
	          	       	   TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;
	          	    }    
	          }	     
        }   
    }
    else{
    	    if( (vcardindexlistserialnumber= GetIdentifierIndexWithPath(currentdir))<0)   //同名文件 但Vcard 中找不到
               TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);	  	        	   										        		      	     				  
         else{
               if (GetIdentifierResponse(vcardindexlistserialnumber,   &tmpresponse)<0)
 	               TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);	      //同名文件 但Vcard 中找不到响应字    										        		      	     				   	    
 	             else{	     	  	        	   										        		      	     				   	    	     	   										        		      	     				   	    	    
 	    	               if (GetFileHeaderContext( vcardindexlistserialnumber, tmpbuf1, eae.ucCurrentCommand[OFFSET_P3])<0)
	          	       	    TransferEaeToNet(NETAPI_SELECT_PATH_UNAMBIGUOUSDIR_CURRENTCMD);	//同名文件但Vcard 中找不到文件头  
	          	         else{
	          	       	      memcpy(tmpbuf2,&eae.ucCurrentCommand[1] ,1);	
	          	       	      memcpy(&tmpbuf2[1],tmpbuf1 ,eae.ucCurrentCommand[OFFSET_P3]);	 
	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_VCARD)
  	     	          	       	   memcpy(&tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1],tmpresponse ,2);	
  	     	          	      else{
  	     	          	       	     if (eae.ucCurrentFheaderFbodyResponsePath==FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT){
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+1]=0x91;
  	     	          	       	      	    tmpbuf2[eae.ucCurrentCommand[OFFSET_P3]+2]=eae.ucCurrentCatLength;
  	     	          	       	     }
  	     	          	      }
	          	       	   TransferEaeToEmu(tmpbuf2, eae.ucCurrentCommand[OFFSET_P3]+3 ) ;				        		      	     				   	    	        
 	    	               } 	   										      		   	                            
 	             }         
         }    	
    	}
    	return 1;	

}
void Emu_Engine::Transfer2Net_PathDependingAcutualStatus_CurrentCmd()
 {        										        		      	     				 		   
    if (eae.CurrentTransfer2Net_Status==TRANSFER2NET_STATUS_SINGLE_ID)  	   										      
		      TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);
    else
    if (eae.CurrentTransfer2Net_Status==TRANSFER2NET_STATUS_CURRENT_PATH)
		      TransferEaeToNet(NETAPI_SELECT_ALLTYPEPATH_CURRENTCMD);  
 }  
 
void Emu_Engine::Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu()
 {        										        		      	     				 		   
    if (eae.CurrentTransfer2Net_Status==TRANSFER2NET_STATUS_SINGLE_ID)  	   										      
		      TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);
    else
    if (eae.CurrentTransfer2Net_Status==TRANSFER2NET_STATUS_CURRENT_PATH)
		      TransferEaeToNet(NETAPI_SELECT_ALLTYPEPATH_CURRENTCMD_CURRENTARGU);  
 }
 
 void Emu_Engine::Transfer2Net_PathDependingAcutualStatus_LastCmd_LastArgu_CurrentCmd()
 {        										        		      	     				 		   
    if (eae.CurrentTransfer2Net_Status==TRANSFER2NET_STATUS_SINGLE_ID)  	   										      
      TransferEaeToNet(NETAPI_SELECT_CURRENTIDENTIFIER_LASTCMD_LASTARGU_CURRENTCMD);
    else
    if (eae.CurrentTransfer2Net_Status==TRANSFER2NET_STATUS_CURRENT_PATH)
      TransferEaeToNet(NETAPI_SELECT_ALLTYPEPATH__LASTCMD_LASTARGU_CURRENTCMD);  
 } 
void Emu_Engine::DeliverReadCommandWithUnkownSfi_Usim(){
	 if (eae.SyncLocal_Remote==SYNCLOCAL_REMOTE_YES)
        TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
   else	 
       //由于GenExpectedFidWithSfi_Usim 报错，直接带路径传送，并且由于不知道expectId，不再同步
        TransferEaeToNet_Usim(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD);                                    
}
void Emu_Engine::DeliverUpdateCommandArgumentWithUnkownSfi_Usim(){
	 if (eae.SyncLocal_Remote==SYNCLOCAL_REMOTE_YES)
        TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);
   else	 
       //由于GenExpectedFidWithSfi_Usim 报错，直接带路径传送，并且由于不知道expectId，不再同步
        TransferEaeToNet_Usim(NETAPI_SELECT_CURRENTIDENTIFIER_CURRENTCMD_CURRENTARGU);                                    
}


int Emu_Engine::SubProcessFromEmuCommand()
 {        										        		      	     				 		   
   vcard_file_property_t file1;
   int tmpfileproperty;
   SetCurrentEngineStatusWhenUncertain();  // 通过Emu 来的数据判断模块正在以Usim 还是Sim 方式访问
   CopyApduCommandToEngine();
   eae.ulApduCommandCounter++;
   switch (eae.ucCurrentCommand[1]){
        case APDU_COMMAND_SELECT:
        	    eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	    TransferEaeToEmu_INS(); 
        	    eae.ucExpectIdentifierLength=eae.ucCurrentCommand[4]; 
        	    //20181031
        	      //0xx011xx  No data returned
        	    if (    
        	    	    (eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]>>7==0)&&\
        	    	    ((eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]&0x10)==0)&&\
        	    	    ((eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]&0x8))&&\
        	    	    ((eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]&0x4))
        	    	)
        	       eae.ucExpectSelectResponseStatus=CURRENTSELECTRESPONSESTATUS_NODATA_RETURN;
        	    else
        	    	 eae.ucExpectSelectResponseStatus=CURRENTSELECTRESPONSESTATUS_NORMAL;  
        	    	 
        	    //判断是否准备访问AID 以激活对话
        	    if ( (eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)\
        	     	&&
        	     	  (eae.ucAdfSessionStatus==ADFSESSIONSTATUS_DISABLE)
        	     	) {
        	     		  if (
        	     		  	 //Selection by DF name
        	     		  	(eae.ucCurrentCommand[APDU_CMD_OFFSET_P1]==4)\     
        	     		  	 // No data returned                                //Return FCP template   
        	     		  	&&((eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]==0xC)||(eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]==0x4))\
        	     		  	   
        	     		  	/*&&(eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]==0x10)*/
        	     		  	&& ((eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]>=0x7)&& (eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]<=0x10))
        	     		  	){
        	     		  	   eae.ucAdfSessionStatus=ADFSESSIONSTATUS_COMMAND_OK;
        	     		  	   std::cout<<RED<<"ADFSESSIONSTATUS_COMMAND_OK"<<RESET<<std::endl<<std::flush;
        	     		  	  }  	   										        		      	     		  	  	   										        		      	     		  		
        	     }
        	    //2018.07.10 假设 ADF激活后， 又在另一个逻辑通道准备激活另一个ADF
        	    else{
        	    	   if ((eae.ucCurrentProcessLogicalChannel)&& (eae.ucAdfSessionStatus==ADFSESSIONSTATUS_ENABLE)&&
						        	(eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED)){
						        		  if (
        	     		  	          //Selection by DF name
        	     		  	          (eae.ucCurrentCommand[APDU_CMD_OFFSET_P1]==4)\     
        	     		  	           // No data returned                                //Return FCP template   
        	     		             	&&((eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]==0xC)||(eae.ucCurrentCommand[APDU_CMD_OFFSET_P2]==0x4))\       	     		  	   
       	     		  	          /*&&(eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]==0x10)*/
       	     		  	          && ((eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]>=0x7)&& (eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]<=0x10))
        	     		  	       ){
        	     		  	            eae.ucAdfSessionStatus=ADFSESSIONSTATUS_ADDITONAL_COMMAND_OK;
        	     		  	             std::cout<<RED<<"ADFSESSIONSTATUS_ADDITONAL_COMMAND_OK"<<RESET<<std::endl<<std::flush;
        	     		  	        }  
						        	}        	    	
        	    	}        	             	         	   	                                        	   	   	                                        	         		   	          
     	         break;
        	
        case APDU_COMMAND_UPDATE_BINARY:       	
        	     if (IsUsimModeEnable()){         		   	  	         		   	  	             		        	            	 	   
           	 	     if (((eae.ucFromEmuDataBuf[OFFSET_P1])>>5)==READ_UPDATE_BINARY_P1_SFI_SUPPORT_B8_B7_B6){  //SFI	
         	  	  	  eae.ucCurrentUpdateMode=UPDATEMODE_SFI;
         	  	      eae.ucCurrentSFI=(eae.ucFromEmuDataBuf[OFFSET_P1])&0x1F; 
         	  	    //  if (GenExpectedFidWithSfi_Usim()<0)
         	  	      if (GenExpectedFidWithSfiAccordingIndexList_Usim()<0)	
         	  	      
         	  	         	eae.ucCurrentSFIForUpdatePrority=SFI_FOR_UPDATE_UNKNOWN;           		   	  	      	  
         	  	      else
         	  	          eae.ucCurrentSFIForUpdatePrority=SFI_FOR_UPDATE_KNOWN;
         	  	    }
         	  	    else    //正常Update   
         	  	          eae.ucCurrentUpdateMode=UPDATEMODE_COMMON;                 	            	 					        		   	  	  
         	     } 
 
        	     eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	     TransferEaeToEmu_INS();     		   	          
     	         break;
        	
        case APDU_COMMAND_UPDATE_RECORD:
        	    if (IsUsimModeEnable()){      
        	       		   	  	         		   	  	             		        	            	 	   
           	 	     if (((eae.ucFromEmuDataBuf[OFFSET_P2])>>3)>0){  //SFI	
         	  	  	  eae.ucCurrentUpdateMode=UPDATEMODE_SFI;         	  	  	
         	  	      eae.ucCurrentSFI=(eae.ucFromEmuDataBuf[OFFSET_P2])>>3; 
         	  	    //  if (GenExpectedFidWithSfi_Usim()<0)
         	  	      if (GenExpectedFidWithSfiAccordingIndexList_Usim()<0)	
         	  	         	eae.ucCurrentSFIForUpdatePrority=SFI_FOR_UPDATE_UNKNOWN;           		   	  	      	  
         	  	      else
         	  	          eae.ucCurrentSFIForUpdatePrority=SFI_FOR_UPDATE_KNOWN;
         	  	    }
         	  	    else    //正常Update   
         	  	          eae.ucCurrentUpdateMode=UPDATEMODE_COMMON;                 	            	 					        		   	  	  
         	     } 
 
        	     eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	     TransferEaeToEmu_INS();     		   	          
     	         break;
        case APDU_COMMAND_SEEK:
        case APDU_COMMAND_INCREASE:	  	   										        		     
        case APDU_COMMAND_CHANGE_CHV:  	   										        		      
        case APDU_COMMAND_DISABLE_CHV:
        case APDU_COMMAND_ENABLE_CHV:
        case APDU_COMMAND_RUN_GSM_ALGORITHM:  	   										        		     
        case APDU_COMMAND_ENVELOPE:
        case APDU_COMMAND_TERMINAL_RESPONSE: 	   										        		      	   										        		      	 	   										        		      
     	  case APDU_COMMAND_TERMINAL_PROFILE:  	   										        		   	  	     
     	   	     eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	     TransferEaeToEmu_INS();     		   	          
     	         break;
     	    
     	        
     	 case APDU_COMMAND_UNBLOCK_CHV:    
     	  	   if (eae.ucFromEmuDataBuf[OFFSET_P3]!=0){
     	  	   	   eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	       TransferEaeToEmu_INS();  	   										        		   	  	   
     	  	   	}	
     	  	   	else{
     	  	   	      //SIM UIM 不支持 UNBLOCK_RETRYTIMES
     	  	   	     
     	  	   	      if (IsUsimModeEnable()){
     	  	   	   
     	  	   	          TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
     	  	   	      //    eae.ucCurrentChv1Status=CHV1_STATUS_CHV_RETRYTIMES_PENDING;	
     	  	   	         }
     	  	   	       } 
     	  	   	 
     	  	    break;   
     	  //VERIFY CHV命令不应该在MF上执行，而应在相关的应用目录中实现 	    
     	 case APDU_COMMAND_VERIFY_CHV:  	//TODO 
     	  	   if (eae.ucFromEmuDataBuf[OFFSET_P3]!=0){
     	  	   	   eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	       TransferEaeToEmu_INS();  	   										        		   	  	   
     	  	   	}	
     	  	   	else{
     	  	   	      //SIM UIM 不支持 CHV_RETRYTIMES
     	  	   	     
     	  	   	      if (IsUsimModeEnable()){
     	  	   	       // TransferEaeToNet(NETAPI_CURRENTCMD);
     	  	   	          TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
     	  	   	          eae.ucCurrentChv1Status=CHV1_STATUS_CHV_RETRYTIMES_PENDING;	
     	  	   	         }
     	  	   	       } 
     	  	   	 
     	  	    break; 
     	 case APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION:
     	 case APDU_COMMAND_UIM_STORE_ESN_ME:
     	 case APDU_COMMAND_UIM_GENERATE_KEY_VPM:
     	  	//Sim USIM 暂不支持
     	  	//   if (IsUimModeEnable()){
     	  	//Sim  暂不支持
     	  	    if (IsUimModeEnable()||IsUsimModeEnable()){
     	  	   	    eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	        TransferEaeToEmu_INS();   	   										        		   	  	   	
     	  	   	}
     	  	    break;   
     	  	  //  int debug=0;
     	 case APDU_COMMAND_UIM_BASE_STATION_CHALLENGE:
     	  	//Sim USIM 暂不支持
     	  	//   if (IsUimModeEnable()){
     	  	//Sim  暂不支持
		 std::cout<<RED<<"enter APDU_COMMAND_UIM_BASE_STATION_CHALLENGE "<<APDU_COMMAND_UIM_BASE_STATION_CHALLENGE<<std::endl<<std::flush;	
     	  	    if (IsUimModeEnable()||IsUsimModeEnable()){
     	  	   	    eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	        TransferEaeToEmu_INS();   
     	  	   	}
     	  	    break;   
     	  	  //  int debug=0;
     	  //Add  Command in C.S0023-0_v4.0  //20200102	  
        case APDU_COMMAND_GET_CHALLENGE:
        case APDU_COMMAND_UIM_CONFIRM_SSD: 
        case APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY:   
        case APDU_COMMAND_UIM_KEY_GENERATION_REQUEST:
        case APDU_COMMAND_UIM_CONFIGURATION_REQUEST:
        case APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST:   
        case APDU_COMMAND_UIM_OTAPA_REQUEST:          
        case APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST: 
        case APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST:      
        case APDU_COMMAND_UIM_VALID:                  
             	  	  
     	  	  //Sim  暂不支持
     	  	    if (IsUimModeEnable()||IsUsimModeEnable()){
     	  	   	    eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
        	        TransferEaeToEmu_INS();   	   										        		   	  	   	
     	  	   	}
     	  	    break;
     	 case APDU_COMMAND_SLEEP:
     	 case APDU_COMMAND_FETCH:   //经验证 fetch 和路径无依赖关系
     	  	   if (IsSimModeEnable()||IsUimModeEnable())
        	        TransferEaeToNet(NETAPI_CURRENTCMD);  										        		      	      
             else{
          	 if (IsUsimModeEnable()) 	   										        		        	    
          	        TransferEaeToNet_Usim(NETAPI_CURRENTCMD); 	   										        		        	
          	    } 	     
        	   break; 
       //Add  Command in C.S0023-0_v4.0  //20200102 	 未验证  和路径有无依赖关系 ，暂按无依赖处理
       case APDU_COMMAND_UIM_COMMIT:
        	   if (IsUimModeEnable())
        	        TransferEaeToNet(NETAPI_CURRENTCMD);  										        		      	      
             else{
          	 if (IsUsimModeEnable()) 	   										        		        	    
          	        TransferEaeToNet_Usim(NETAPI_CURRENTCMD); 	   										        		        	
          	    } 	     
        	   break; 	
        	   
        case APDU_COMMAND_INVALIDATE:    //使当前EF无效 ,和路径有依赖关系
        case APDU_COMMAND_REHABILITATE:  //使当前无效的EF恢复有效状态	，和路径有依赖关系
          
            if (IsUimModeEnable()||IsSimModeEnable())
         		{
         		
                 if  (       										        		      	     				 		   
         	     				 (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)||\        										        		      	     				 		   
         	     				 (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\        										        		      	     				 		   
         	     				 (eae.ucCurrentSimCardIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
         	      )
         	      {        										        		      	     				 		   
                    
                 		   Transfer2Net_PathDependingAcutualStatus_CurrentCmd();       
                  }
                  else	{
                 	       	 std::cout<<RED<<"The Current Identifier is not EF (INVALIDATE/REHABILITATE "<<RESET<<std::endl<<std::flush;	
         	       	  	   eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
                 	} 
                  		  	   										        		               		                	                                                  	                                            
               }
              
            if (IsUsimModeEnable()) 	
         		{
         		   //TOMODIFY
         		 //  if (GetVcardFileProperty_Usim(eae.ucCurrentSimCardIdentifierLength,eae.ucCurrentSimCardIdentifier, &file1)<0){ 
         		   if (!eae.ucCurrentProcessLogicalChannel) 
         		       tmpfileproperty=GetVcardFileProperty_WithUsimRidType(eae.ucCurrentSimCardIdentifierLength,eae.ucCurrentSimCardIdentifier, eae.ucCurrentRidType,&file1);
         		   else
         		      tmpfileproperty=GetVcardFileProperty_WithUsimRidType(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength,\
         		      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier, eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type,&file1);
         		   if  (tmpfileproperty<0){  
         		 //  if (GetVcardFileProperty_Usim(eae.ucCurrentSimCardIdentifierLength,eae.ucCurrentSimCardIdentifier, &file1)<0){ 
         		   	     TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                 }
                 else {
                 	       if (file1.property&FILE_TYPE_EF){
                 	       	TransferToNetPathPendingSyncStatus_CurrentCmd_Usim();
                 	       }
                 	       else	{
                 	       	      std::cout<<RED<<"The Current Identifier is not EF (INVALIDATE/REHABILITATE "<<RESET<<std::endl<<std::flush;	
         	       	  	        eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
                 	       	}     
                 }
               }          
                 break;  
        case APDU_COMMAND_MANAGE_SECURE_CHANNEL :    	                                    	               
        case APDU_COMMAND_MANAGE_CHANNEL :
                   if (IsUsimModeEnable())  {
                   	     TransferEaeToNet_Usim(NETAPI_CURRENTCMD);   	                                               	   
                   	} 
                   	
                    break;   
      	 case APDU_COMMAND_GET_RESPONSE:
      	  	 if (IsSimModeEnable()||IsUimModeEnable())
         		{
      	  	     switch (eae.ucLastCommand[1]){
      	  	          case APDU_COMMAND_SELECT:  	   										        		   	  	          	
      	  	          	  SubProcessGetResponseLastCommandIsSelectUimSim();    										        		   	  	       		  										        		      	       	 
         	       	     	break; 
         	       	     	// SEEK ,INCREASE 也需要带路径  2018.1.2
         	       	 case APDU_COMMAND_SEEK:    //TODO local search         	       	 	    
         	      	 case APDU_COMMAND_INCREASE:   
         	      	 	    //在执行该指令之前，必须先选择GSM目录或者GSM目录下的子目录作为当前目录 2017.5.26
         	       	 	//20180620    Transfer2Net_PathDependingAcutualStatus_LastCmd_LastArgu_CurrentCmd();
         	       	 	   TransferEaeToNet(NETAPI_CURRENTCMD);  //20180620  LastCmd 已经完成了Emu Bank 端的路径同步
         	       	 	    break; 	
         	       	 case	APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION:	    
         	       	 case APDU_COMMAND_UIM_GENERATE_KEY_VPM:	    
         	       	 case APDU_COMMAND_UIM_STORE_ESN_ME:	    
         	       	 case APDU_COMMAND_RUN_GSM_ALGORITHM:
         	       	 case APDU_COMMAND_UIM_BASE_STATION_CHALLENGE:    //2019/12/27 出现该指令
         	       	 	
         	       	  //Add  Command in C.S0023-0_v4.0  //20200102	  
                   case APDU_COMMAND_GET_CHALLENGE:
                   case APDU_COMMAND_UIM_CONFIRM_SSD: 
                   case APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY:   
                   case APDU_COMMAND_UIM_KEY_GENERATION_REQUEST:
                   case APDU_COMMAND_UIM_CONFIGURATION_REQUEST:
                   case APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST:   
                   case APDU_COMMAND_UIM_OTAPA_REQUEST:          
                   case APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST: 
                   case APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST:      
                   case APDU_COMMAND_UIM_VALID:    	
                    	       	 	
         	       	 	    //在执行该指令之前，必须先选择GSM目录或者GSM目录下的子目录作为当前目录 2017.5.26
         	       	 	//20180620    Transfer2Net_PathDependingAcutualStatus_LastCmd_LastArgu_CurrentCmd()  
         	       	 	    if  (eae.ucLastCommand[1]==APDU_COMMAND_UIM_STORE_ESN_ME)
         	       	 	    	   int erp=0;   	       	 	        	       	 	    
         	       	 	    if (eae.ucOptimizationMode&EMU_TRANSLAOTR_OPTIMIZATION_FORK_GET_RESPONSE_WITH_RUNAUTH){
         	       	 	       if (eae.ucExpectRunAuthResLength==eae.ucFromEmuDataBuf[APDU_CMD_OFFSET_P3])         	       	 	    	         	       	 		   
 	   										       TransferEaeToNetSide_NoDeliver(NETAPI_CURRENTCMD);
 	   							          else   	       	 	  
         	       	 	          TransferEaeToNet(NETAPI_CURRENTCMD); 
         	       	 	    } 
         	       	 	    else
         	       	 		     TransferEaeToNet(NETAPI_CURRENTCMD); 
         	       	 	    
         	       	 	    break;
         	       	 	    
         	       	 	    
         	       	 case APDU_COMMAND_ENVELOPE:
         	      	  	  TransferEaeToNet(NETAPI_LASTCMD_LASTARGU_CURRENTCMD);
         	       	  	  break; 
         	     /*
         	       //	 case APDU_COMMAND_UIM_STORE_ESN_ME:
         	       	 case	APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION:
         	       //	 case APDU_COMMAND_UIM_GENERATE_KEY_VPM:
         	       	 	 	  TransferEaeToNet(NETAPI_CURRENTCMD);
         	       	  	  break; 		
         	       	  	  */							        		      	       	   
         	       	 default:
         	       	  	   //用于返回RUN GSM ALOGRITHM、SELECT，SEEK（类型2）、INCREASE和ENVELOPE等指令的响应数据
         	       	  	  //GET RESPONSE要求直接跟在前一功能后面，在两条功能之间不能插入其他功能
         	       	  	  std::cout<<RED<<"The Last Command is mismatch with GET_RESPONSE "<<RESET<<std::endl<<std::flush;	
         	       	  	  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
         	       	  		return -1;	   	      
      	  	     } 
      	  	  }
      	  	 if (IsUsimModeEnable())
         		  {
         		 	   switch (eae.ucLastCommand[1]){
      	  	       case APDU_COMMAND_SELECT:  	
      	  	          	/*  if (eae.ucCurrentProcessLogicalChannel)   // 逻辑通道不在本地处理
      	  	          	  	     			TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
      	  	          	  else 	
      	  	          	*/     										        		   	  	          	
         	       	     	   SubProcessGetResponseLastCommandIsSelectUsim();
         	       	     	break; 
         	       	 case APDU_COMMAND_SEEK:       //20180620   //TODO local search
         	       	 	  if ((eae.ucCurrentSeekStatus==SEEKSTATUS_ENABLE)&&(eae.ucMeetSeekVolume!=0))
         	       	 	  	  SubProcessGetResponseLastCommandIsSeekUsim();
         	       	 	  else	  
         	       	 	    TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
         	       	 	    break;
         	         case APDU_COMMAND_INCREASE:	  //20180620   	
         	       //	 case APDU_COMMAND_UIM_STORE_ESN_ME:  //Usim 中出现  Uim指令 //20180620
         	     //  	 case	APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION://Usim 中出现  Uim指令 //20180620
         	       	// case APDU_COMMAND_UIM_GENERATE_KEY_VPM:	 //Usim 中出现  Uim指令 //20180620
         	       	 	    TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
         	       	 	    break;
         	       	 case	APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION://Usim 中出现  Uim指令 //20180620
         	       	 case APDU_COMMAND_UIM_GENERATE_KEY_VPM:	 //Usim 中出现  Uim指令 //20180620	    
         	       	 case APDU_COMMAND_UIM_STORE_ESN_ME:  //Usim 中出现  Uim指令 //20180620	    
         	       	 case APDU_COMMAND_RUN_GSM_ALGORITHM:
         	       	 case APDU_COMMAND_UIM_BASE_STATION_CHALLENGE:    //2019/12/27 出现该指令
         	       	 	
         	       	  //Add  Command in C.S0023-0_v4.0  //20200102	  
                   case APDU_COMMAND_GET_CHALLENGE:
                   case APDU_COMMAND_UIM_CONFIRM_SSD: 
                   case APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY:   
                   case APDU_COMMAND_UIM_KEY_GENERATION_REQUEST:
                   case APDU_COMMAND_UIM_CONFIGURATION_REQUEST:
                   case APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST:   
                   case APDU_COMMAND_UIM_OTAPA_REQUEST:          
                   case APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST: 
                   case APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST:      
                   case APDU_COMMAND_UIM_VALID:   	
         	       	 	
         	       	 	if (eae.ucOptimizationMode&EMU_TRANSLAOTR_OPTIMIZATION_FORK_GET_RESPONSE_WITH_RUNAUTH){
         	       	 	     if (
         	       	 	    	((eae.ucCurrentProcessLogicalChannel==0)&&(eae.ucExpectRunAuthResLength==eae.ucFromEmuDataBuf[APDU_CMD_OFFSET_P3]))||\
         	       	 	    	((eae.ucCurrentProcessLogicalChannel!=0)&&(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucLogical_Channel_ExpectRunAuthResLength==eae.ucFromEmuDataBuf[APDU_CMD_OFFSET_P3]))
         	       	 		    )
         	       	 		 
 	   										   TransferEaeToNetSide_NoDeliver(NETAPI_CURRENTCMD);
 	   							      else   	       	 	  
         	       	 	       TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
         	       	 	} 
         	       	 	else
         	       	 		     TransferEaeToNet_Usim(NETAPI_CURRENTCMD); 
         	       	 	    break;
         	       	 	    //在执行该指令之前，必须先选择GSM目录或者GSM目录下的子目录作为当前目录 2017.5.26
         	       	 case APDU_COMMAND_ENVELOPE:
         	      //20180620 	 case APDU_COMMAND_SEEK:
         	      //20180620   	 case APDU_COMMAND_INCREASE:				 
         	       	  	  TransferEaeToNet_Usim(NETAPI_LASTCMD_LASTARGU_CURRENTCMD);
         	       	  	  break;
         	       	 case APDU_COMMAND_STATUS: 	 
         	       	 	    SubProcessGetResponseLastCommandIsStatusUsim();
         	       	 	    break;   	   									        		      	       	   
         	       	 default:
         	       	  	   //用于返回RUN GSM ALOGRITHM、SELECT，SEEK（类型2）、INCREASE和ENVELOPE等指令的响应数据
         	       	  	  //GET RESPONSE要求直接跟在前一功能后面，在两条功能之间不能插入其他功能
         	       	  	  std::cout<<RED<<"The Last Command is mismatch with GET_RESPONSE "<<RESET<<std::endl<<std::flush;	
         	       	  	  eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
         	       	  		return -1;	   	      
      	  	       }   	   										        		      		 	  	   										        		      		 	
         		 	}
      	  	        	     	          	   										        		      	      								        		      	      
      	         break; 
      	 case APDU_COMMAND_STATUS: 
     	  	 
       	   if (IsSimModeEnable()) {
                 SubProcessStatusCommandSim();  
          	}
          	
       	   if (IsUimModeEnable()) {     	   										        		   	  	 	
          	   SubProcessStatusCommandUim();
          	}
       
           // 20180620 if (IsUsimModeEnable()){  //在 Usim 应用中，如果应用被激活， 通常STATUS 内容指向应用
           if (IsUsimModeEnable()){  //在 Usim 应用中，STATUS 内容指向当前目录或应用	 // 20180620 
           	    SubProcessStatusCommandUsim();  	   										        		       	   
          	}
     	  	    break; 
     	 case APDU_COMMAND_READ_BINARY:
    		   	  	  if (IsSimModeEnable())
    		   	  	  	SubProcessReadBinaryCommandSim();
    		   	  	  if (IsUimModeEnable()){
    		   	  	  	SubProcessReadBinaryCommandUim();        	   										        		   	  	  
      		        }
      		      
      		        if (IsUsimModeEnable()){
      		        	 if (eae.ucCurrentProcessLogicalChannel)   {
      		        	 	//  TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
      		        	 	   if (((eae.ucFromEmuDataBuf[OFFSET_P1])>>5)==READ_UPDATE_BINARY_P1_SFI_SUPPORT_B8_B7_B6){  //SFI
      		        	 	   	//  if ((eae.ucFromEmuDataBuf[2]==0x8b)&&(eae.ucFromEmuDataBuf[3]==0x0)&&(eae.ucFromEmuDataBuf[4]==0xb))
      		        	 	   	//    int i=1;
      		        	 	   	    
           		   	  	  	  eae.ucCurrentReadMode=READMODE_SFI;
           		   	  	      eae.ucCurrentSFI=(eae.ucFromEmuDataBuf[OFFSET_P1])&0x1F; 
           		   	  	     // if (GenExpectedFidWithSfi_Usim()<0){
           		   	  	      if (GenExpectedFidWithSfiAccordingIndexList_Usim()<0){
           		   	  	      	     DeliverReadCommandWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI                                        
                                  eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                     
           		   	  	      }
           		   	  	      else
           		   	  	           SubProcessReadBinaryWithSfiUsim();
           		   	  	           
           		   	  	  }
           		   	  	  else{    //正常读             		   	  	             		      	   
           		      	    	        SubProcessReadBinaryCommandUsim();
           		      	  } 
      		        	 	}  
      		        	 	 
      		        	 else	  {
      		        	 	     // if ((eae.ucCurrentCommand[0]==0x0)&&(eae.ucCurrentCommand[2]==0x8c)&&(eae.ucCurrentCommand[3]==0x0)&&(eae.ucCurrentCommand[4]==0xe))
    	                     //      int tmpilengh=3;
           		   	  	  if (((eae.ucFromEmuDataBuf[OFFSET_P1])>>5)==READ_UPDATE_BINARY_P1_SFI_SUPPORT_B8_B7_B6){  //SFI
           		   	  	  	  eae.ucCurrentReadMode=READMODE_SFI;
           		   	  	      eae.ucCurrentSFI=(eae.ucFromEmuDataBuf[OFFSET_P1])&0x1F; 
           		   	  	     // if (GenExpectedFidWithSfi_Usim()<0){
           		   	  	      if (GenExpectedFidWithSfiAccordingIndexList_Usim()<0){
           		   	  	      	  DeliverReadCommandWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI        
                                 eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;  
                                    
           		   	  	      }
           		   	  	      else
           		   	  	           SubProcessReadBinaryWithSfiUsim();
           		   	  	  }
           		   	  	  else{    //正常读             		   	  	
           		      	   
           		      	    	SubProcessReadBinaryCommandUsim();
           		      	  } 
           		     } 	    
      		       } 
      		              	   										        		   	  	   
    		   	  	  break;
      	 case APDU_COMMAND_READ_RECORD:    	   										        		   	 	        
      		      	if (IsSimModeEnable())
      		          	SubProcessReadRecordCommandSim();
      		        if (IsUimModeEnable())
      		          	SubProcessReadRecordCommandUim(); 
      		      	if (IsUsimModeEnable()){   
      		   	  	     //// debug
      		   	  	    //SFI TODO
      		   	  	  //  0 b2 1 44 2
      		   	  	 /*   if (
      		   	  	    	  (eae.ucFromEmuDataBuf[OFFSET_P2]==0x44)&&\
      		   	  	    	  (eae.ucFromEmuDataBuf[OFFSET_P1]==0x1)
      		   	  	    	  )
      		   	  	    	  int errr=1;
      		   	  	 */   	
      		   	  	    if (eae.ucCurrentProcessLogicalChannel) {  // 逻辑通道不在本地处理  
      		        	 	//  TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
      		        	 	    if (((eae.ucFromEmuDataBuf[OFFSET_P2])>>3)>0){  //SFI
           		   	  	  	  eae.ucCurrentReadMode=READMODE_SFI;
           		   	  	      eae.ucCurrentSFI=(eae.ucFromEmuDataBuf[OFFSET_P2])>>3; 
           		   	  	    //  if (GenExpectedFidWithSfi_Usim()<0){
           		   	  	    if (GenExpectedFidWithSfiAccordingIndexList_Usim()<0){
           		   	  	      	  DeliverReadCommandWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI        
                                // eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;  
                                eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                  
           		   	  	      }
           		   	  	      else
           		   	  	           SubProcessReadRecordWithSfiUsim();
           		   	  	    }
           		   	  	    else{    //正常读   
           		   	  	     
           		      	    	SubProcessReadRecordCommandUsim();
           		      	   }
      		        	 }
      		        	 else	  {
           		   	  	    if (((eae.ucFromEmuDataBuf[OFFSET_P2])>>3)>0){  //SFI
           		   	  	  	  eae.ucCurrentReadMode=READMODE_SFI;
           		   	  	      eae.ucCurrentSFI=(eae.ucFromEmuDataBuf[OFFSET_P2])>>3; 
           		   	  	    //  if (GenExpectedFidWithSfi_Usim()<0){
           		   	  	    if (GenExpectedFidWithSfiAccordingIndexList_Usim()<0){
           		   	  	      	   DeliverReadCommandWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI        
                                 eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                      
           		   	  	      }
           		   	  	      else
           		   	  	           SubProcessReadRecordWithSfiUsim();
           		   	  	    }
           		   	  	    else{    //正常读   
           		   	  	     
           		      	    	SubProcessReadRecordCommandUsim();
           		      	   }
           		     } 	           	   										        		   	  	  
      		      	}   
      		      	      	   										        		   	  	 
      	          break;
      	/*          
      	 case APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION: 
      	      	  	if (IsUimModeEnable()){
      	      	  		TransferEaeToEmu_INS();
      	      	  		 eae.ucCurrentExpect=EXPECT_IS_ARGUMENT_DATA;
      	      	  	}
      	      	  	break;	
      	  */    	  		
      	 default:  	   										        		      	       	  	   
        	    std::cout<<RED<<"Unrecognised APDU Command "<<RESET<<std::endl<<std::flush;	
        	    eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
        	    return -1;	        	   										        		   	   	      	   	                                                                  	                                                                 	        	     	  	        	   										        		   	                                     
   	}
 	return 1;   										        		
 } 

int Emu_Engine::SubProcessFromEmu_Argument() 
	{
	 vcard_file_property_t file1;  
	 int tmpfileproperty;
	 unsigned char  tmpresponse[2],tmpwrongrespnse[2],tmpnumber,direct2remoteflag=0,invalidid_usim=0, selectformmf=0; 
	 unsigned char SeekResponse[2];
	 alltype_vcard_identifier_t  tmpId_AllType= eae.CurrentId_AllType;
	 unsigned char  ExpectIdWithPathLength;
   unsigned char  ExpectIdWithPath[20];   	     		   	         
	                             
      CopyApduArgumentToEngine(); 
     eae.ucCurrentExpect=EXPECT_IS_COMMAND;
      switch (eae.ucCurrentCommand[1]){
				   case APDU_COMMAND_SELECT:	
				 	      eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_UNAVAILABLE;
				 	      
     	     		 if (IsSimModeEnable()){
     	     				 eae.ucExpectIdentifier[0]=eae.ucCurrentArgument[0];
     	     				 eae.ucExpectIdentifier[1]=eae.ucCurrentArgument[1];  
     	     				
     	     				 UpdateExpectPath_SimAndUim();										        		      	     				 
     	     				 eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_SINGLE_ID;
     	     			//	 if (GetVcardFileProperty(eae.ucExpectIdentifier, &file1)<0){  //如果是运营商自定义的EF 或有意的错误EF
     	     		if  (!(

     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_MF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	     				 		   )){
     	     				 		   	  std::cout<<RED<<"Wrong identifier "<<RESET<<std::endl<<std::flush;
     	     				 		   	  
     	     				 		   	  tmpwrongrespnse[0]=SW1_94;     //表示标识符没被选中
     	     				 		   	  tmpwrongrespnse[1]=04;
     	     				 		   	  TransferEaeToEmu(tmpwrongrespnse,2);
     	     				 		   	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_UNKOWN;		 
     	     				 	   }
     	     				 	   else{
             	     					if (GetVcardFilePropertyWithPath_Sim(eae.ExpectId_AllType, &file1)<0){  //如果是运营商自定义的EF 或有意的错误EF 	      	     				 	 
             	     				 	   	  //路径+期待的文件
             	     				     	  TransferEaeToNetPathAndExpectId();  
             	     				     	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;
             	     				     	  eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_CURRENT_PATH;
             	     				    
             	     				  }
             	     				  else        										        		      	     				 	   
             	     				     SubProcessSelectArgumentSim(file1);
             	     		 }     	     				   	  										        		      	     				     
     	     		 }     										        		      	     		   										        		      	     		 		 
     	     		 if (IsUimModeEnable()){
     	     		 	//   if ( eae.ucFromEmuDataBuf[1]==0x44)
     	     			//	 	  int eee=1;
     	     				 eae.ucExpectIdentifier[0]=eae.ucCurrentArgument[0];
     	     				 eae.ucExpectIdentifier[1]=eae.ucCurrentArgument[1];
     	     				 UpdateExpectPath_SimAndUim();
     	     				 eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_SINGLE_ID;
     	     			//	 if (GetVcardFileProperty_Uim(eae.ucExpectIdentifier, &file1)<0){  //如果是运营商自定义的EF 或有意的错误EF       
     	     			   if  (!(

     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_MF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_MF_EF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_1LEVEL_DF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_1LEVEL_DF_EF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_2LEVEL_DF)||\
     	     				 		   (eae.ucExpectIdentifier[0]==FILE_1ST_ID_UNDER_2LEVEL_DF_EF)
     	     				 		   )){
     	     				 		   	  std::cout<<RED<<"Wrong identifier "<<RESET<<std::endl<<std::flush;
     	     				 		   	  
     	     				 		   	  tmpwrongrespnse[0]=SW1_94;     //表示标识符没被选中
     	     				 		   	  tmpwrongrespnse[1]=04;
     	     				 		   	  TransferEaeToEmu(tmpwrongrespnse,2);
     	     				 		   	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_UNKOWN;		 
     	     				 	   }
     	     				 	   else{
          	     			      if (GetVcardFilePropertyWithPath_Uim(eae.ExpectId_AllType, &file1)<0){  //如果是运营商自定义的EF 或有意的错误EF
          	     				 	   	  //路径+期待的文件
          	     				     	  TransferEaeToNetPathAndExpectId();  
          	     				     	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;
          	     				     	  eae.ExpectTransfer2Net_Status=TRANSFER2NET_STATUS_CURRENT_PATH;
          	     				     }
          	     				 
          	     		         else  {      										        		      	     				 	        	     				        
          	     				       // if (eae.ucExpectIdentifier[1]==0x48)
          	     				      //  	int erd=0;
          	     				        SubProcessSelectArgumentUim(file1);
          	     			       }
     	     				  }    
     	     				   	  										        		      	     				     
     	     		 }      										        		      	     		   
     	     		 if (IsUsimModeEnable()){ 
     	     		 	  	
     	     		 	     //检查是否为合法AID
     	     		 	     for (tmpnumber=0;tmpnumber<sizeof(ridset)/sizeof(rid_inform_t);tmpnumber++){
                 	        if ((!char_arrayncmp ( &(eae.ucCurrentArgument[0]), &(ridset[tmpnumber].rid[0]), 5 )))
                 	  	      break;
                 	   }
                 	     
    						 	  if (   // 处理 AID select
    						 	  	((eae.ucCurrentEngineStatus==ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_UNACTIVATED)\
      	     	     &&
      	     	     (eae.ucAdfSessionStatus==ADFSESSIONSTATUS_COMMAND_OK))
      	     	     ||(eae.ucAdfSessionStatus==ADFSESSIONSTATUS_ADDITONAL_COMMAND_OK)
      	     	    )       	     	          	     	  
      	     	    {   //检查是否为 AID
      	     	   //第1个 select AID  出现后， 视同所有AID被激活，包括其他逻辑通道的AID ，TODO
                               
                             if (tmpnumber<sizeof(ridset)/sizeof(rid_inform_t))  {
                 	                     eae.ucAdfSessionStatus=ADFSESSIONSTATUS_DATA_OK;
      	     		  	                   std::cout<<RED<<"USIMSESSIONSTATUS_DATA_OK"<<RESET<<std::endl<<std::flush;
      	     		  	                 //  eae.ucExpectIdentifier[0]=0x7f;
      	     		  	                 //  eae.ucExpectIdentifier[1]=0xff;
      	     		  	                   
      	     		  	                   if (!eae.ucCurrentProcessLogicalChannel){  // 通道0
      	     		  	  	                   
      	     		  	  	                    eae.ucCurrentSimCardRidType=ridset[tmpnumber].ridtype;
      	     		  	  	                    eae.ucCurrentRidType=eae.ucCurrentSimCardRidType;
      	     		  	  	                  }
      	     		  	                  else {  //非0通道
      	     		  	 	                      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_RidType=ridset[tmpnumber].ridtype;
      	     		  	 	                      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_RidType;
      	     		  	 	                      direct2remoteflag=1;
      	     		  	                  }
                 	           }               	              
      	     		  	         else  { 
      	     		  	                  eae.ucAdfSessionStatus=ADFSESSIONSTATUS_DISABLE;
      	     		  	                  std::cout<<RED<<"ADFSESSIONSTATUS_DISABLE"<<RESET<<std::endl<<std::flush;	
      	     		  	         }   										        		      	     		  		
      	     		   }
      	     		   else{  //处理 非AID
      	     		   	     // if (tmpnumber==sizeof(ridset)/sizeof(rid_inform_t))  { 
      	     		   	             //7f 10 5f 3a 4f 62  
      	     		   	           if  (eae.ucCurrentArgumentLength<=6){
      	     		   	             	  for (int tmpk=0;tmpk<eae.ucCurrentArgumentLength;tmpk++)
      	     		   	             	    eae.ucExpectIdentifier[tmpk]=eae.ucCurrentArgument[tmpk];      	     		   	             	
      	     		   	           } 
      	     		   	           if ((eae.ucExpectIdentifier[4]==0x4f)&&(eae.ucExpectIdentifier[5]==0x30))
      	     		   	           	int ss=1;  
      	     		   	           //有些类型的模块会发出错误的ID
      	     		   	        
      	     		   	           if (!IsValidIdentifier(eae.ucCurrentArgumentLength,eae.ucExpectIdentifier))     	     		   	           
      	     		   	              invalidid_usim=1;
      	     		   	           else{	 
      	     		   	           	   // 有些类型的模块发送Usimmode Select指令时 并未发送全路径ID，需要再生全路径指令      	     		   	           	  
      	     		   	           	//  if (eae.ucExpectIdentifier[1]==0xe2)
      	     		   	          // 	  	int ddsds=1;
      	     		   	           	  UpdateExpectPath_Usim(&ExpectIdWithPathLength,ExpectIdWithPath );
      	     		   	           	  //当文件长度小于实际路径时，要全路径选择
      	     		   	           	  if (eae.ucCurrentArgumentLength<ExpectIdWithPathLength)
      	     		   	           	  	 selectformmf=1;
      	     		   	           	  for (int tmpk=0;tmpk<ExpectIdWithPathLength;tmpk++)
      	     		   	             	    eae.ucExpectIdentifier[tmpk]=ExpectIdWithPath[tmpk];
      	     		   	             	eae.ucExpectIdentifierLength=ExpectIdWithPathLength;
      	     		   	             	//再生当前Select命令的 P3值      	
      	     		   	           	  eae.ucCurrentCommand[APDU_CMD_OFFSET_P3]=eae.ucExpectIdentifierLength;
      	     		   	           	  if (selectformmf)
      	     		   	           	  	 eae.ucCurrentCommand[APDU_CMD_OFFSET_P1]=8;
      	     		   	           	  // 再生当前argment
      	     		   	           	   for (int tmpk=0;tmpk<eae.ucExpectIdentifierLength;tmpk++)
      	     		   	             	    eae.ucCurrentArgument[tmpk]=eae.ucExpectIdentifier[tmpk];
      	     		   	             	 eae.ucCurrentArgumentLength=eae.ucExpectIdentifierLength;
      	     		   	             	    
           	     		  	          if (!eae.ucCurrentProcessLogicalChannel){  // 通道0
           	     		  	          	if (!((eae.ucExpectIdentifier[0]==0x7f)&&(eae.ucExpectIdentifier[1]==0xff))){ //不是AID 下的文件
           	     		  	  	                    eae.ucCurrentRidType=RIDTYPE_NONEEDAID;      	     		  	  	                 
           	     		  	  	        }
           	     		  	  	        else{ //是AID下的文件
           	     		  	  	        	   eae.ucCurrentRidType=eae.ucCurrentSimCardRidType;
           	     		  	  	                   
           	     		  	  	        }
           	     		  	  	        tmpfileproperty=GetVcardFileProperty_WithUsimRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier, eae.ucCurrentRidType,&file1);
           	     		  	  	      }
           	     		  	          else {  //非0通道
           	     		  	          	 if ((!(eae.ucExpectIdentifier[0]==0x7f)&&(eae.ucExpectIdentifier[1]==0xff))){     		  	 	                      
          		  	 	                      eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type=RIDTYPE_NONEEDAID;
          		  	 	                   }
          		  	 	                    else  //是AID下的文件
           	     		  	  	        	    eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type=eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_RidType;   
           	     		  	             tmpfileproperty=GetVcardFileProperty_WithUsimRidType(eae.ucExpectIdentifierLength,eae.ucExpectIdentifier,\
           	     		  	             eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type,&file1);
           	     		  	          }
           	     		  	          
           	     		  	          if (tmpfileproperty!=-1){
           	     		  	          	    if (!eae.ucCurrentProcessLogicalChannel)
           	     		  	          	    	    eae.ucCurrentSimCardIdentifierProperty=file1.property;
           	     		  	          	    else
           	     		  	          	    	    eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChannelCurrentIdentifierProperty=file1.property;
           	     		  	          	    if (file1.property&CAN_FORK)
           	     		  	          	    	 eae.ucCurrentFilePrority=CURRENTFILEPRORITY_FORK_AVAILABLE;
           	     		  	          	}
           	     		  	       }
      	     		  	     //  }
                 	                 	               		   	
      	     		   }  
      	     		   
      	     		 //  if (eae.ucCurrentProcessLogicalChannel)   // 逻辑通道不在本地处理 
      	     		 if (invalidid_usim){
      	     		 	         std::cout<<RED<<"Wrong identifier Usim "<<RESET<<std::endl<<std::flush;
     	     				 		   	  
     	     				 		   	  tmpwrongrespnse[0]=SW1_6A;     //表示标识符没被选中
     	     				 		   	  tmpwrongrespnse[1]=SW2_82;
     	     				 		   	  TransferEaeToEmu(tmpwrongrespnse,2);
     	     				 		   	  eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_UNKOWN;		 
      	     		 	}
      	     		 else{ 
      	     		       if (direct2remoteflag)  
   		        	 	            TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU); 
   		        	 	     else        
    						 	            SubProcessSelectArgumentUsim(); 
    						 }                    										        		     						 	     
     	     		}                    	   										        		   	       				    
    				     break;
    			 case APDU_COMMAND_RUN_GSM_ALGORITHM:
    			   	/*  if (IsSimModeEnable()){                   //sim mode 下 -> a0 88 xx xx xx  <-88 -> yy yy .....  <- 9f 0c, ==>  select id a0 88 xx xx xx yy yy ... a0 c0 0 0 c
    			 	      tmpresponse[0]=0x9f;
    			 	      tmpresponse[1]=0xc;
    			 	      TransferEaeToEmu(tmpresponse, 2 ) ; 
    			 	   }
    			 	   
    			 	    if (IsUimModeEnable()){                //Uim mode 下 -> a0 88 xx xx xx  <-88 -> yy yy .....  <- 9f 03, ==> select id a0 88 xx xx xx yy yy ... a0 c0 0 0 3
    			 	     tmpresponse[0]=0x9f;
    			 	      tmpresponse[1]=0x3;
    			 	      TransferEaeToEmu(tmpresponse, 2 ) ;   							        		   	       			 	 
    			 	   }*/
    			 	   if (IsSimModeEnable()||IsUimModeEnable())
    			 	       Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu();
    			 	    if (IsUsimModeEnable()){         //  //USim mode 下 -> 0 88 xx xx xx  <-88 -> yy yy .....  ==> select id a0 88 xx xx xx yy yy 
 		           TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();
 		          }

    			 	    break;  
    			 	     //VERIFY CHV命令不应该在MF上执行，而应在相关的应用目录中实现   										        		   	  
    			 case APDU_COMMAND_VERIFY_CHV:	//TODO
    			 	    if (IsSimModeEnable()||IsUimModeEnable()){
 		          		 //EMU 端支持错误Pin 判断
 		          	 if (IsUnsuitableInputAtEnablePin(eae.ucCurrentArgumentLength)){  //远端卡没有设置PIN时，发送错误PIN，应返回错误状态码
 		          	 	   tmpresponse[0]=SW1_98;                     
    			 	           tmpresponse[1]=SW2_08;
    			 	           TransferEaeToEmu(tmpresponse, 2 ) ; //简化设计
 		          	 	}
 		          	 else
 		          	 	
 		             if (IsRightPin(eae.ucCurrentArgumentLength,eae.ucCurrentArgument)){  //Valid PIN 后，发到Bank，同步CHV 
 		             	   Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
 		                  eae.ucCurrentChv1Status=CHV1_STATUS_WAITING_PENDING;	
 		             }
 		             else  {        //远端卡设置PIN时,EMU发送错误的PIN，则直接反馈错误代码 
 		                       tmpresponse[0]=SW1_98;                     
    			 	               tmpresponse[1]=SW2_04;//简化设计
    			 	               TransferEaeToEmu(tmpresponse, 2 ) ; 
 		                } 								        	  								        		      		          		 								        		      		          		
 		          		
 		          }		
 		     
 		          if (IsUsimModeEnable()){
 		          	  //EMU 端支持错误Pin 判断
 		          	 if (IsUnsuitableInputAtEnablePin(eae.ucCurrentArgumentLength)){  //远端卡没有设置PIN时，发送错误PIN，应返回错误状态码
 		          	 	   tmpresponse[0]=SW1_69;                     
    			 	           tmpresponse[1]=0x84;
    			 	           TransferEaeToEmu(tmpresponse, 2 ) ; //简化设计
 		          	 	}
 		          	 else
 		          	 	
 		             if (IsRightPin(eae.ucCurrentArgumentLength,eae.ucCurrentArgument)){  //Valid PIN 后，发到Bank，同步CHV 
 		             	   TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();	
 		                  eae.ucCurrentChv1Status=CHV1_STATUS_WAITING_PENDING;	
 		             }
 		             else  {        //远端卡设置PIN时,EMU发送错误的PIN，则直接反馈错误代码 ，retry times 减少
 		                    tmpresponse[0]=SW1_63;                     
    			 	              tmpresponse[1]=0xc2;//简化设计
    			 	               TransferEaeToEmu(tmpresponse, 2 ) ; 
 		                } 								        		      		                  
 		            }
			   	    break; 
    			 	
    			 	    
    		  case APDU_COMMAND_CHANGE_CHV:  							        		   	       			 	
 				  case APDU_COMMAND_UNBLOCK_CHV:	
			    case APDU_COMMAND_DISABLE_CHV:
		      case APDU_COMMAND_ENABLE_CHV: 	
		     
 		          if (IsSimModeEnable())
 		          		Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 		
 		          if (IsUimModeEnable())
 		          		Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
 		          if (IsUsimModeEnable())
 		              TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();		
			   	    break; 
		     	  
		     	 										        		  				     		     	   										        		   	       		   
          case APDU_COMMAND_SLEEP:        	
       //   case APDU_COMMAND_FETCH:        	 	   										        		               	       	   										        		               	     
		  //   case APDU_COMMAND_INVALIDATE:   
		 //    case APDU_COMMAND_REHABILITATE: 
		      case APDU_COMMAND_TERMINAL_PROFILE:    
		      case APDU_COMMAND_ENVELOPE:
			    case APDU_COMMAND_TERMINAL_RESPONSE:  
			  // case APDU_COMMAND_GET_RESPONSE:
			   	
 		          TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);	
			   	    break; 
			    case APDU_COMMAND_UPDATE_BINARY:  					//TODO SYNC SFI ,ID		
			    	     if (IsSimModeEnable()||IsUimModeEnable())
          		     // Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
             		     SubProcessUpdateBinaryCommandArgumentSimOrUim();        
 		             if (IsUsimModeEnable()){   
      		   	  	     //// debug
      		   	  	    //SFI TODO
      		   	  	    
      		   	  	    if (eae.ucCurrentProcessLogicalChannel) {  // 逻辑通道不在本地处理  
      		        	 	//  TransferEaeToNet_Usim(NETAPI_CURRENTCMD);
      		        	 	    if (eae.ucCurrentUpdateMode==UPDATEMODE_SFI){  //SFI           		   	  	  	          		   	  	     
           		   	  	       if (eae.ucCurrentSFIForUpdatePrority==SFI_FOR_UPDATE_UNKNOWN){
           		   	  	      	    DeliverUpdateCommandArgumentWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI                                        
                                  eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                  
           		   	  	       }
           		   	  	        else
           		   	  	            SubProcessUpdateBinaryWithSfiUsim();
           		   	  	    }
           		   	  	    else    //正常Update 
           		   	  	     
           		      	    	 SubProcessUpdateBinaryCommandArgumentUsim();          		      	   
      		        	 }
      		        	 else	  {
           		   	  	    if (eae.ucCurrentUpdateMode==UPDATEMODE_SFI){  //SFI            		   	  	  	  
           		   	  	        if (eae.ucCurrentSFIForUpdatePrority==SFI_FOR_UPDATE_UNKNOWN){
           		   	  	      	    DeliverUpdateCommandArgumentWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI                                          
                                 eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                      
           		   	  	        }
           		   	  	        else
           		   	  	            SubProcessUpdateBinaryWithSfiUsim();
           		   	  	    }
           		   	  	    else{    //正常Update 
           		   	  	            SubProcessUpdateBinaryCommandArgumentUsim();     
           		      	   }
           		     } 	           	   										        		   	  	  
      		      	} 
             
 		             break;   ///wx
			    	
			    			        		  					   	      
          case APDU_COMMAND_UPDATE_RECORD:           //TODO SYNC SFI ,ID		
          	   if (IsSimModeEnable()||IsUimModeEnable())
          		    //  Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
          		    if ((eae.ucCurrentCommand[2]==0)&&(eae.ucCurrentCommand[3]==3)&&(eae.ucCurrentCommand[4]==0x1c))
          		    	  int errr=0;
             		    SubProcessUpdateRecordCommandArgumentSimOrUim();         
 		             if (IsUsimModeEnable()){   
      		   	  	     //// debug
      		   	  	    //SFI TODO
      		   	  	    
      		   	  	    if (eae.ucCurrentProcessLogicalChannel) {  // 逻辑通道不在本地处理  
      		        	 	
      		        	 	    if (eae.ucCurrentUpdateMode==UPDATEMODE_SFI){  //SFI           		   	  	  	          		   	  	     
           		   	  	       if (eae.ucCurrentSFIForUpdatePrority==SFI_FOR_UPDATE_UNKNOWN){
           		   	  	      	    DeliverUpdateCommandArgumentWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI                                        
                                  eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                  
           		   	  	       }
           		   	  	        else
           		   	  	            SubProcessUpdateRecordWithSfiUsim();
           		   	  	    }
           		   	  	    else    //正常Update 
           		   	  	     
           		      	    	 SubProcessUpdateRecordCommandArgumentUsim();          		      	   
      		        	 }
      		        	 else	  {
           		   	  	    if (eae.ucCurrentUpdateMode==UPDATEMODE_SFI){  //SFI            		   	  	  	  
           		   	  	        if (eae.ucCurrentSFIForUpdatePrority==SFI_FOR_UPDATE_UNKNOWN){
           		   	  	      	    DeliverUpdateCommandArgumentWithUnkownSfi_Usim();
                                  //没有在标准中找到该SFI                                          
                                 eae.iCurrentVcardIndexListSerialnumber=VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD;                                      
           		   	  	        }
           		   	  	        else
           		   	  	            SubProcessUpdateRecordWithSfiUsim();
           		   	  	    }
           		   	  	    else{    //正常Update 
           		   	  	            SubProcessUpdateRecordCommandArgumentUsim();     
           		      	   }
           		     } 	           	   										        		   	  	  
      		      	} 
             
 		             break;   ///wx
			    	
          case APDU_COMMAND_INCREASE:  								        		                 								        		               	  
          		   if (IsSimModeEnable()||IsUimModeEnable())
          		      Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
                 if (IsUsimModeEnable())  								        		      		          
 		               TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();		
 		             break;   ///wx
 		      case APDU_COMMAND_SEEK:
          	     if (IsSimModeEnable()||IsUimModeEnable())
          		      Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 
                 if (IsUsimModeEnable()){                  	 
                 //	 if ((eae.ucCurrentCommand[OFFSET_P1]==0x1)&&(eae.ucCurrentCommand[OFFSET_P2]==0x4)&&(SeekLocalOk_Usim(SeekResponse)==1)){
                  if ((eae.ucCurrentCommand[OFFSET_P1]!=0x0)&&(eae.ucCurrentCommand[OFFSET_P2]==0x4)&&(SeekLocalOk_Usim(eae.ucCurrentCommand[OFFSET_P1],SeekResponse)==1)){
                 	 	 	eae.ucCurrentSeekStatus=SEEKSTATUS_ENABLE;
                 	 	 	TransferEaeToEmu(SeekResponse, 2 );
                 	 }	 	
                 	 else	{ 								        		      		          	                     
 		                     eae.ucCurrentSeekStatus=SEEKSTATUS_DISABLE;
 		                     TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();	
 		               }
 		             } 	
 		             break;   ///wx
 		     case APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION:	//路径依赖        
 		     case APDU_COMMAND_UIM_GENERATE_KEY_VPM:	
 		     case 	APDU_COMMAND_UIM_STORE_ESN_ME:    //路径依赖
 		     case APDU_COMMAND_UIM_BASE_STATION_CHALLENGE:    //2019/12/27 出现该指令
 		     	
 		      //Add  Command in C.S0023-0_v4.0  //20200102	  
                   case APDU_COMMAND_GET_CHALLENGE:
                   case APDU_COMMAND_UIM_CONFIRM_SSD: 
                   case APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY:   
                   case APDU_COMMAND_UIM_KEY_GENERATION_REQUEST:
                   case APDU_COMMAND_UIM_CONFIGURATION_REQUEST:
                   case APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST:   
                   case APDU_COMMAND_UIM_OTAPA_REQUEST:          
                   case APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST: 
                   case APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST:      
                   case APDU_COMMAND_UIM_VALID:   	
 		     	
 		     	      if (IsUimModeEnable()){
 		     	     	  Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 	  								        		      		     	     						        		               		      	    
          		   }    
          		   if (IsUsimModeEnable()){  //发现Usim 中出现过逻辑通道操作STORE_ESN_ME  81 de 1 0 8 TODO 带路径
          		   	if (eae.ucCurrentProcessLogicalChannel)
 		     	     	  TransferEaeToNet_Usim(NETAPI_CURRENTCMD_CURRENTARGU);   								        		      		     	     						        		               		      	    
          		   }  
          		             		   
          		   
          		    break;   
         /* 		    
 		     case APDU_COMMAND_UIM_COMPUTE_IP_AUTHENTICATION:	//路径依赖
 		    // case APDU_COMMAND_UIM_GENERATE_KEY_VPM:	
 		     	     if (IsUimModeEnable())							        		      		     	  
 		     	     	  Transfer2Net_PathDependingAcutualStatus_CurrentCmd_Argu(); 	
 		     	     if (IsUsimModeEnable())  //20180630  								        		      		     	     						        		               		      	    
          		        TransferToNetPathPendingSyncStatus_CurrentCmd_CurrentArgu_Usim();		
          		    break; 
         */  								        		              
			   default:  	   										        		      	       	  	   
 	            std::cout<<RED<<"The APDU Command should not have Argument !"<<RESET<<std::endl<<std::flush;	
 	            eae.ucEmuApduNetApiStatus=EMU_APDU_NETAPI_STATUS_ERROR	;
 	            return -1;		     		    
		 } 

		}


void Emu_Engine::SubProcessFromNet() {

 unsigned char tmpilengh;
 unsigned char  currentIsAid=0;	
	
	switch (eae.ucCurrentCommand[1]){
  	   									  case APDU_COMMAND_SELECT: 
  	   									  	   if (IsSimModeEnable()||IsUimModeEnable())	         	   			
  	   									  	     if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==SW1_9F)){	   
  	   									  	     	   eae.ucLastSimCardIdentifier[0]=eae.ucCurrentSimCardIdentifier[0];
 	   										             eae.ucLastSimCardIdentifier[1]=eae.ucCurrentSimCardIdentifier[1];
  	   									  	     										  	
 	   										             eae.ucCurrentSimCardIdentifier[0]=eae.ucExpectIdentifier[0];
 	   										             eae.ucCurrentSimCardIdentifier[1]=eae.ucExpectIdentifier[1];
 	   										             
 	   										             eae.CurrentTransfer2Net_Status=eae.ExpectTransfer2Net_Status;
 	   										             
 	   										             UpdatePath_SimAndUim(eae.ucCurrentSimCardIdentifier);		
 	   										             std::cout<<GREEN<<std::hex <<"Emu Side Current PATH :";
 	    	                			       std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.mfid[0]\
	     	     				     	           <<" "<<(unsigned short)eae.CurrentIdWithPath.mfid[1]<<" " ; 	  
	     	 											       std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.firstlevelid[0]\
	     	    			      	           <<" "<<(unsigned short)eae.CurrentIdWithPath.firstlevelid[1]<<" " ; 	     	          	          	     	      
	     												       std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.secondlevelid[0]\
	     	          	  			         <<" "<<(unsigned short)eae.CurrentIdWithPath.secondlevelid[1]<<" " ;
	     	    										     std::cout<<GREEN<<std::hex <<(unsigned short)eae.CurrentIdWithPath.thirdlevelid[0]\
	     	          				           <<" "<<(unsigned short)eae.CurrentIdWithPath.thirdlevelid[1]<<" " ; 	      	     	       
	     	      											 std::cout<<std::endl ; 		
 	   										         }
 	   										          	   										         	   										         
 	   										       if (IsUsimModeEnable())
  	   										        {
  	   										        
  	   										        		  if (eae.ucAdfSessionStatus==ADFSESSIONSTATUS_DATA_OK)  {
						        		      	     		     if (
						        		      	     		  	      ((eae.ucFromNetDataBuf[0]==SW1_90)&&(eae.ucCurrentArgument[1]==SW2_00))\
						        		      	     		           ||(eae.ucFromNetDataBuf[0]==SW1_91)\
						        		      	     		           // select P2 = Return FCP template时
						        		      	     		            ||(eae.ucFromNetDataBuf[0]==SW1_61)
						        		      	     		  	    ){
						        		      	     		  	       eae.ucAdfSessionStatus=ADFSESSIONSTATUS_ENABLE;
						        		      	     		  	       eae.ucCurrentEngineStatus=ENGINESTATUS_USIM_ENBALE_ONLY_SESSION_ACTIVATED;
						        		      	     		  	       std::cout<<GREEN<<"USIM_ENBALE_ONLY_SESSION_ACTIVATED ,USIMSESSIONSTATUS_ENABLE"<<RESET<<std::endl<<std::flush;
						        		      	     		  	  
						        		      	     		  	   //   if  (eae.ucExpectIdentifierLength==0x10)   //2020/3/10 部分AID 长度可变
						        		      	     		  	    if  ((eae.ucExpectIdentifierLength>=0x7)&& (eae.ucExpectIdentifierLength<=0x10))
						        		      	     		  	      if (   ((eae.ucCurrentRidType!=RIDTYPE_NONEEDAID)&&(!eae.ucCurrentProcessLogicalChannel))||\
						        		      	     		  	      	     ((eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Type!=RIDTYPE_NONEEDAID)&&(eae.ucCurrentProcessLogicalChannel)) 						        		      	     		  	      	     						        	     	     		  	      	     
						        		      	     		  	      	 ){ //AID
						        		      	     		  	   
	     	      	   		                              // 传递的是 ADFUSIM
	     	      	   		                              eae.ucExpectIdentifier[0]=0x7f;
	     	      	   		                              eae.ucExpectIdentifier[1]=0xff;
	     	      	   		                              eae.ucExpectIdentifierLength=2;
	     	      	   		                              if (!eae.ucCurrentProcessLogicalChannel){ //如果是通道0
	     	      	   		                                eae.ucCurrentSimCardIdentifierLength=2;
	     	      	   		                                eae.ucCurrentSimCardIdentifier[0]=0x7F;
	     	      	   		                                eae.ucCurrentSimCardIdentifier[1]=0xFF;
	     	      	   		                                UpdatePath_Usim(eae.ucCurrentSimCardIdentifierLength, eae.ucCurrentSimCardIdentifier);
	     	      	   		                                currentIsAid=1;
	     	      	   		                                std::cout<<GREEN<<std::hex <<"Emu Side Current PATH  CH0 by remote response :";	
	     	      	   		                                for (tmpilengh=0;tmpilengh< 2;tmpilengh++)    	    	    	   	    	                                             
	   	    	                                              std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	                                          
	   	    	                                          std::cout<<std::endl;   
	     	      	   		                             }
	     	      	   		                             else{
	     	      	   		                             	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=2;
	     	      	   		                                eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[0]=0x7F;
	     	      	   		                                eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[1]=0xFF;
	     	      	   		                                UpdatePath_Usim(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength,\
	     	      	   		                                 eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier);
	     	      	   		                                currentIsAid=1;
	     	      	   		                                std::cout<<GREEN<<std::hex <<"Emu Side Current PATH  CH"<<(unsigned short)eae.ucCurrentProcessLogicalChannel<<" remote response :";		     	      	   		                             	     	      	   		                             	
	     	      	   		                             	  for (tmpilengh=0;tmpilengh< 2;tmpilengh++)    	    	    	   	    	                                             
	   	    	                                              std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	                                          
	   	    	                                          std::cout<<std::endl;  
	     	      	   		                             	}   	
	     	      	   		                              
	     	      	   		                                
	     	      	   	                              }
						        		      	     		  	     	if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		                  eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                           eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                                        }
						        		      	     		  	     	
						        		      	     		  	     }
						        		      	     		  	     else  { 
						        		      	     		  	       eae.ucAdfSessionStatus=ADFSESSIONSTATUS_DISABLE;	
						        		      	     		  	        std::cout<<RED<<"ADFSESSIONSTATUS_DISABLE"<<RESET<<std::endl<<std::flush;
						        		      	     		  	     }   										        		      	     		  		
						        		      	     		   }    
 	 
  	   										      //20181031     if (((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==SW1_61))&&(!currentIsAid)) {	 
  	   										     if (
  	   										     	  (
  	   										     	  ((eae.ucFromNetDataBuf[0]==SW1_61)&&(eae.ucExpectSelectResponseStatus==CURRENTSELECTRESPONSESTATUS_NORMAL))||\
  	   										     	  ((eae.ucFromNetDataBuf[0]==SW1_90)&&(eae.ucExpectSelectResponseStatus==CURRENTSELECTRESPONSESTATUS_NODATA_RETURN))
  	   										     	  )\
  	   										     	  &&(eae.usFromNetDataBufLength==2)&&(!currentIsAid)  	   										     	  
  	   										     	  ) {	  
  	   										        	     std::cout<<GREEN<<std::hex <<"Emu Side Current PATH by remote response CH"<<(unsigned short)eae.ucCurrentProcessLogicalChannel<<" : ";	
  	   										        	     	
  	   										        	     	     if (!eae.ucCurrentProcessLogicalChannel){ //如果是通道0
	   	    	    	                                  for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	                                            eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	                                            std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	   
	   	    	                                         } 
	   	    	                                        std::cout<<std::endl;
 	   										        		                eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength; 
 	   										        		                UpdatePath_Usim(eae.ucCurrentSimCardIdentifierLength, eae.ucCurrentSimCardIdentifier);
 	   										        		           }
 	   										        		           else{
 	   										        		           	    for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	                                            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	                                            std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	   
	   	    	                                         } 
	   	    	                                        std::cout<<std::endl;
 	   										        		                eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength;  	   										        		           	
 	   										        		                UpdatePath_Usim(eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength, \
 	   										        		                eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier);
 	   										        		           	}
 	   										        		       
 	   										        		   if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
 	   										        		       
 	   										        		 } 		

  	   										        	}
 	   										        
 	   										       break;  
 	   										  case APDU_COMMAND_READ_RECORD:     
 	   										  case APDU_COMMAND_READ_BINARY:          
 	   										  	    if (IsUsimModeEnable()){
 	   										  	   
 	   										  	    	  if (eae.ucCurrentReadMode==READMODE_COMMON){
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
     	  	                          }
     	  	                          else
     	  	                          if (eae.ucCurrentReadMode==READMODE_SFI){     	  	                       
     	  	                          	  eae.ucCurrentReadMode=READMODE_COMMON; // 无论正常与否， 置位ucCurrentReadMode
     	  	                          	  if (   //读到数据说明 选择文件操作正常
						        		      	     		  	       //20180620
						        		      	     		  	      ((eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-2]==SW1_90)&&(eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-1]==SW2_00))\       
						        		      	     		           ||(eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-2]==SW1_91)
						        		      	     		   ){  
						        		      	     		   std::cout<<GREEN<<std::hex <<"Emu Side Current PATH by remote Response CH"<<(unsigned short)eae.ucCurrentProcessLogicalChannel<<": ";	
						        		      	     		   if (!eae.ucCurrentProcessLogicalChannel){ 
						        		      	     		   	   if ( eae.iCurrentVcardIndexListSerialnumber==VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD){
						        		      	     		   		      std::cout<<GREEN<<std::hex <<" unrecognized"<<std::endl;
						        		      	     		   	   }
						        		      	     		   	   else{
	   	    	    	                              for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	                                       eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	                                       std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
	   	    	                                    }	   	    	                                   
	   	    	                                    std::cout<<std::endl;
 	   										        		            eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength;
 	   										        		           }
 	   										        		       }
 	   										        		       else{
 	   										        		       	   if ( eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber==VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD){
						        		      	     		   		      std::cout<<GREEN<<std::hex <<" unrecognized"<<std::endl;
						        		      	     		   	   }
						        		      	     		   	   else{
 	   										        		       	    for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	                                       eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	                                       std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
	   	    	                                    }
	   	    	                                    std::cout<<std::endl;
 	   										        		            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength;
 	   										        		           } 
 	   										        		       }		  	                          	
  	   									     	            if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		         eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                   eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                                }

						        		      	         }
     	  	                          	
     	  	                          }	
     	  	                    
  	   									  	     }     	
 	   										  	
 	   										  	      break;
 	   										   case APDU_COMMAND_RUN_GSM_ALGORITHM:	      
 	   										  	      if (IsUsimModeEnable()){  	   										  	    
 	   										  	           if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==0x61)&&(eae.ucFromNetDataBuf[1]!=0)){
 	   										  	           	   if (!eae.ucCurrentProcessLogicalChannel) 
 	   										  	           	   	  eae.ucExpectRunAuthResLength=eae.ucFromNetDataBuf[1];
 	   										  	           	   else 
 	   										  	           	   	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucLogical_Channel_ExpectRunAuthResLength=eae.ucFromNetDataBuf[1];  
 	   										  	           }
 	   										  	           else{
 	   										  	           	      if (!eae.ucCurrentProcessLogicalChannel) 
 	   										  	           	   	     eae.ucExpectRunAuthResLength=0;
 	   										  	           	      else 
 	   										  	           	   	   eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucLogical_Channel_ExpectRunAuthResLength=0;   	   										  	           
 	   										  	           	}
 	   										  	           	
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
  	   									  	      }  
  	   									  	       if (IsSimModeEnable()||IsUimModeEnable()){  	   										  	    
 	   										  	           if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==SW1_9F)&&(eae.ucFromNetDataBuf[1]!=0)) 	   									
 	   										  	           	   	  eae.ucExpectRunAuthResLength=eae.ucFromNetDataBuf[1]; 	   										  	            
 	   										  	           else
 	   										  	           	   	  eae.ucExpectRunAuthResLength=0; 	   										  	           
  	   									  	      }     	
  	   									  	         	
  	   									  	      break;  
  	   									  case APDU_COMMAND_STATUS:           
  	   									//  case APDU_COMMAND_READ_BINARY:
  	   								//	  case APDU_COMMAND_READ_RECORD:
  	   									  case APDU_COMMAND_SEEK: 	       	   									
  	   									  case APDU_COMMAND_SLEEP:            
  	   									  case APDU_COMMAND_GET_RESPONSE:     
  	   									  case APDU_COMMAND_TERMINAL_RESPONSE: 
  	   									  case APDU_COMMAND_ENVELOPE:         
  	   									         if (IsUsimModeEnable()){   
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
  	   									  	     }     	
  	   									  	    break;  
  	   									  	
  	   									  case APDU_COMMAND_VERIFY_CHV:  
  	   									  	       if (IsUsimModeEnable()){   
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
     	  	                            if  (eae.ucCurrentChv1Status==CHV1_STATUS_CHV_RETRYTIMES_PENDING){
     	  	                            	  if (   //读到数据说明 选择文件操作正常
  	   									  	       	  	          (eae.usFromNetDataBufLength==2)&&
						        		      	     		  	      (((eae.ucFromNetDataBuf[0]==SW1_63)&&(eae.ucCurrentArgument[1]==0xc0))\
						        		      	     		           ||((eae.ucFromNetDataBuf[0]==SW1_69)&&(eae.ucCurrentArgument[1]==0x83)))
						        		      	     		      )
						        		      	     		      eae.ucCurrentChv1Status= CHV1_STATUS_DISABLE; 
						        		      	     		  else
						        		      	     		  if (   //读到数据说明 选择文件操作正常
  	   									  	       	  	          (eae.usFromNetDataBufLength==2)&&
						        		      	     		  	      ((eae.ucFromNetDataBuf[0]==SW1_63)&&((eae.ucFromNetDataBuf[1]&0xf0)==0xc0)&&((eae.ucFromNetDataBuf[1]&0xf)!=0))\
						        		      	     		           
						        		      	     		      )
						        		      	     		      eae.ucCurrentChv1Status= CHV1_STATUS_ENABLE; 
						        		      	     		      
     	  	                            }
     	  	                            else
     	  	                            if (eae.ucCurrentChv1Status==CHV1_STATUS_WAITING_PENDING){
  	   									  	       	       if (   //读到数据说明 选择文件操作正常
  	   									  	       	  	          (eae.usFromNetDataBufLength==2)&&
						        		      	     		  	      (((eae.ucFromNetDataBuf[0]==SW1_90)&&(eae.ucFromNetDataBuf[1]==SW2_00))\
						        		      	     		           ||(eae.ucFromNetDataBuf[0]==SW1_91))
						        		      	     		      )
						        		      	     		         eae.ucCurrentChv1Status= CHV1_STATUS_ENABLE;
						        		      	     	     else	
						        		      	     	 	     eae.ucCurrentChv1Status= CHV1_STATUS_DISABLE;  
  	   									  	       	
  	   									  	       	   }	
  	   									  	       }
  	   									  	       if (IsUimModeEnable()||IsSimModeEnable()){ 
  	   									  	       	// Uim/Sim 暂时不考虑用于SFI的 同步
  	   									  	       	 
  	   									     	      //   if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		//       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                         //        eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                          //   }
     	  	                             
     	  	                            
     	  	                            if (eae.ucCurrentChv1Status==CHV1_STATUS_WAITING_PENDING){
  	   									  	       	       if (   //读到数据说明 选择文件操作正常
  	   									  	       	  	          (eae.usFromNetDataBufLength==2)&&
						        		      	     		  	      (((eae.ucFromNetDataBuf[0]==SW1_90)&&(eae.ucFromNetDataBuf[1]==SW2_00))\
						        		      	     		           ||(eae.ucFromNetDataBuf[0]==SW1_91))
						        		      	     		      )
						        		      	     		         eae.ucCurrentChv1Status= CHV1_STATUS_ENABLE;
						        		      	     	     else	
						        		      	     	 	     eae.ucCurrentChv1Status= CHV1_STATUS_DISABLE;  
  	   									  	       	
  	   									  	       	   }	
  	   									  	       }
  	   									  	       
  	   									  	       break;	
  	   									  case APDU_COMMAND_MANAGE_CHANNEL :
  	   									  	       if (IsUsimModeEnable()){ 
  	   									  	       	  if (eae.ucCurrentCommand[OFFSET_P1]==0) //open channel
  	   									  	       	       if ((eae.ucFromNetDataBuf[0]==APDU_COMMAND_MANAGE_CHANNEL)&&	(eae.ucFromNetDataBuf[1]<MAX_LOGICAL_CHANNELS_NUMBER)){//20180702
  	   									  	       	  	        eae.Lcm[eae.ucFromNetDataBuf[1]].LogicalChannelStatus=LOGICALCHANNELSTATUS_OPEN;
  	   									  	       	  	   }           
  	   									  	       }
  	   									  	       break;
  	   									  	
  	   									  case APDU_COMMAND_INCREASE:  	
  	   									  	   if (IsUsimModeEnable()){   
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
  	   									  	     }    
  	   									  	    break;   									  	      	      
  	   									  case APDU_COMMAND_UPDATE_BINARY:      	      //TODO SFI SYnc ID							  
  	   									  case APDU_COMMAND_UPDATE_RECORD:              //TODO SFI SYnc	ID			               
  	   									      
  	   									      if (IsUsimModeEnable()){
 	   										  	    
 	   										  	    	  if (eae.ucCurrentUpdateMode==UPDATEMODE_COMMON){
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
     	  	                          }
     	  	                          else
     	  	                          if (eae.ucCurrentUpdateMode==UPDATEMODE_SFI){     	  	                       
     	  	                          	  eae.ucCurrentUpdateMode=UPDATEMODE_COMMON; // 无论正常与否， 置位ucCurrentReadMode
     	  	                          	  if (   //读到数据说明 选择文件操作正常
						        		      	     		  	       //20180620
						        		      	     		  	      ((eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-2]==SW1_90)&&(eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-1]==SW2_00))\       
						        		      	     		           ||(eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-2]==SW1_91)
						        		      	     		   ){  
						        		      	     		   std::cout<<GREEN<<std::hex <<"After remote Update,Emu Side Current PATH by remote Response CH"<<(unsigned short)eae.ucCurrentProcessLogicalChannel<<": ";	
						        		      	     		   if (!eae.ucCurrentProcessLogicalChannel){ 
						        		      	     		   	   if ( eae.iCurrentVcardIndexListSerialnumber==VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD){
						        		      	     		   		      std::cout<<GREEN<<std::hex <<" unrecognized"<<std::endl;
						        		      	     		   	   }
						        		      	     		   	   else{
	   	    	    	                              for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	                                       eae.ucCurrentSimCardIdentifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	                                       std::cout<<GREEN<<std::hex <<(unsigned short)eae.ucCurrentSimCardIdentifier[tmpilengh];     	          	           
	   	    	                                    }	   	    	                                   
	   	    	                                    std::cout<<std::endl;
 	   										        		            eae.ucCurrentSimCardIdentifierLength=eae.ucExpectIdentifierLength;
 	   										        		           }
 	   										        		       }
 	   										        		       else{
 	   										        		       	   if ( eae.Lcm[eae.ucCurrentProcessLogicalChannel].LogicalChanneliCurrentIndexSerialnumber==VCARDINDEXLISTSERIALNUMBER_NO_EXIST_IN_STANDARD){
						        		      	     		   		      std::cout<<GREEN<<std::hex <<" unrecognized"<<std::endl;
						        		      	     		   	   }
						        		      	     		   	   else{
 	   										        		       	    for (tmpilengh=0;tmpilengh< eae.ucExpectIdentifierLength;tmpilengh++) { 	   	    	    
	   	    	                                       eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh]=eae.ucExpectIdentifier[tmpilengh]; 
	   	    	                                       std::cout<<GREEN<<std::hex <<(unsigned short)eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_Identifier[tmpilengh];     	          	           
	   	    	                                    }
	   	    	                                    std::cout<<std::endl;
 	   										        		            eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucCurrentLogical_Channel_IdentifierLength=eae.ucExpectIdentifierLength;
 	   										        		           } 
 	   										        		       }		  	                          	
  	   									     	            if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		         eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                   eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                                }

						        		      	         }
     	  	                          	
     	  	                          }	
     	  	                      
  	   									  	     } 
  	   									      break;
  	   									  case APDU_COMMAND_UIM_GENERATE_KEY_VPM:
  	   									  case APDU_COMMAND_UIM_STORE_ESN_ME:
  	   									  case APDU_COMMAND_UIM_BASE_STATION_CHALLENGE:    //2019/12/27 出现该指令
  	   									  	
  	   									     //Add  Command in C.S0023-0_v4.0  //20200102	  
                   case APDU_COMMAND_GET_CHALLENGE:
                   case APDU_COMMAND_UIM_CONFIRM_SSD: 
                   case APDU_COMMAND_UIM_GENERATE_PUBLIC_KEY:   
                   case APDU_COMMAND_UIM_KEY_GENERATION_REQUEST:
                   case APDU_COMMAND_UIM_CONFIGURATION_REQUEST:
                   case APDU_COMMAND_UIM_KEY_DOWNLOAD_REQUEST:   
                   case APDU_COMMAND_UIM_OTAPA_REQUEST:          
                   case APDU_COMMAND_UIM_SSPR_CONFIGURATION_REQUEST: 
                   case APDU_COMMAND_UIM_SSPR_DOWNLOAD_REQUEST:      
                   case APDU_COMMAND_UIM_VALID:    	
  	   									  	
  	   									  
  	   									  	  if (IsUsimModeEnable()){
  	   									            if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==0x61)&&(eae.ucFromNetDataBuf[1]!=0)){
 	   										  	           	   if (!eae.ucCurrentProcessLogicalChannel) 
 	   										  	           	   	  eae.ucExpectRunAuthResLength=eae.ucFromNetDataBuf[1];
 	   										  	           	   else 
 	   										  	           	   	  eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucLogical_Channel_ExpectRunAuthResLength=eae.ucFromNetDataBuf[1];  
 	   										  	        }
 	   										  	        else{
 	   										  	           	      if (!eae.ucCurrentProcessLogicalChannel) 
 	   										  	           	   	     eae.ucExpectRunAuthResLength=0;
 	   										  	           	      else 
 	   										  	           	   	   eae.Lcm[eae.ucCurrentProcessLogicalChannel].ucLogical_Channel_ExpectRunAuthResLength=0;   	   										  	           
 	   										  	        }
 	   										  	        if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                          } 	   										  	           	
 	   										  	  }
 	   										  	  if (IsUimModeEnable()){  	   										  	    
 	   										  	           if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==SW1_9F)&&(eae.ucFromNetDataBuf[1]!=0)) 	   									
 	   										  	           	   	  eae.ucExpectRunAuthResLength=eae.ucFromNetDataBuf[1]; 	   										  	            
 	   										  	           else
 	   										  	           	   	  eae.ucExpectRunAuthResLength=0; 	   										  	           
  	   									  	  }     
 	   										  	  break;        	
  	   									  case APDU_COMMAND_CHANGE_CHV:     
  	   									  case APDU_COMMAND_DISABLE_CHV:    
  	   									  case APDU_COMMAND_ENABLE_CHV:       
  	   									  case APDU_COMMAND_UNBLOCK_CHV:      
  	   									  case APDU_COMMAND_INVALIDATE:        
  	   									  case APDU_COMMAND_REHABILITATE: 
  	   									 // case APDU_COMMAND_UIM_STORE_ESN_ME:
  	   									  	//TODO
  	   									  	     if (IsUsimModeEnable()){   
  	   									     	         if (eae.ExpectSyncLocal_Remote==EXPECTSYNCLOCAL_REMOTE_ENABLE){
 	   										        		       eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
     	  	                                 eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;
     	  	                             }
  	   									  	     }    
  	   									  	    break;
  	   									  case APDU_COMMAND_TERMINAL_PROFILE: 
  	   									  	 //   if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]=0x91)){	
  	   									  	    if ((eae.usFromNetDataBufLength==2)&&(eae.ucFromNetDataBuf[0]==0x91)){		//20180702
  	   									  	    	  eae.ucCurrentCatLength=eae.ucFromNetDataBuf[1];
  	   									  	    	 // if (!IsUimModeEnable())    //观察UIM 实际通信过程，发现COMMAND_TERMINAL_PROFILE 未影响后续的response
  	   									  	           eae.ucCurrentFheaderFbodyResponsePath=FHEADERFBODY_RESPONSE_PATH_FROM_EMU_CAT;
  	   									  	      }
  	   									  	      
  	   									  	      
  	   									  	      break;
  	   									  case APDU_COMMAND_FETCH: 
  	   									  	    if ((eae.usFromNetDataBufLength>=2)&&(eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-1]==0x0)&&\
  	   									  	    	
  	   									  	    	(eae.ucFromNetDataBuf[eae.usFromNetDataBufLength-2]==0x90)){	 	   									  	    	  
  	   									  	        eae.ucCurrentFheaderFbodyResponsePath=FHEADERFBODY_RESPONSE_PATH_FROM_VCARD;
  	   									  	      }
  	   									  	        	   									  	      
  	   									  	      break;
  	   									  	          
  	   									  default:         	    
         	                      break;     	
 					        	 		}	 					        	 	 		        	      
 	                 TransferEaeToEmu(eae.ucFromNetDataBuf,eae.usFromNetDataBufLength );
	
	}
/**************************************************************************** 
* 函数名称 : ProcessEmuDataCmd
* 功能描述 : 处理进入Eae中的数据，产生新的状态和数据，外部操作者据此操作
* 参    数 : 
* 参    数 :  
* 参    数 : 
* 参    数 :  
* 返 回 值 :  成功 返回 1 不正确  返回 -1 
* 作    者 : 王翔
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/	
int Emu_Engine::ProcessEmuDataCmd( ){
	
 unsigned long tmpTimeCounter;
  vcard_file_property_t file[SIMFILEPATHDEEPTH],file1;
 int vcardindexlistserialnumber, tmpfilecounter,tmplength;
 unsigned char tmpilengh,tmpidentLength;
 unsigned char tmpbuf1[APDU_BUF_MAX_LENGTH],tmpbuf2[APDU_BUF_MAX_LENGTH],tmpresponse[2],tmpidentifier[IDENTIFIERDEEPTH],tmpwrongrespnse[2];
 unsigned short readoffset;
 int tmpfilebodylength;
 unsigned char aid_rid_3gpp[]={0xa0,0,0,0,0x87};
 unsigned char aid_rid_3gpp2[]={0xa0,0,0,3,0x43};
 
 unsigned char  currentIsAid=0;	
	     if (eae.ucEmuEngineStatus==EMU_ENGINE_WAITING_EMU_DELIVER_RST_ICC){
   	      switch(eae.ucEmuApduNetApiStatus){
    	  	  case EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY:
    	  	        if (eae.ucFromEmuDataProperty==DATA_PROPERTY_IS_REQ_RST_ICC){
    	  	        	   eae.ucCurrentEngineStatus=ENGINESTATUS_SIM_USIM_UIM_ENBALE;
    	  	        	   eae.ucAdfSessionStatus=ADFSESSIONSTATUS_DISABLE;
    	  	        	   std::cout<<RED<<"ADFSESSIONSTATUS_DISABLE"<<RESET<<std::endl<<std::flush;
    	  	        	   
    	  	        	   eae.ucWaitGetInteractiveStartTime=GetTickCount();	 
    	  	        	   eae.ulApduCommandCounter=0;
    	  	        	   DirectTransfer_EmuToNet();		     	  	        	      	  	        	  
    	  	        }	
    	  	        break;
    	  	  case EMU_APDU_NETAPI_STATUS_ICC_RST_DELIVER_TO_REMOTE:
    	  	  	   tmpTimeCounter=GetTickCount();
 					     if (tmpTimeCounter<eae.ucWaitGetInteractiveStartTime)
 					             tmpTimeCounter+=0xFFFFFFFF-eae.ucWaitGetInteractiveStartTime;
 					     else
 					             tmpTimeCounter=tmpTimeCounter-eae.ucWaitGetInteractiveStartTime;
 					     if(tmpTimeCounter>EMUIATIMEOUT){
 					             std::cout<<RED<<"Warning! ICC_RST_DELIVER_TO_REMOTE Timeout"<<RESET<<std::endl<<std::flush;	
 					             eae.ucEmuApduNetApiStatus= EMU_APDU_NETAPI_STATUS_ERROR;
 					             return -1;
 					     }
 					     else{
 					             eae.ucEmuApduNetApiStatus= EMU_APDU_NETAPI_STATUS_IDLE; 
 					             eae.ucEmuEngineStatus=EMU_ENGINE_WORKING;
 					             SetDefaultCommandIdentifier(); 
 					             
 					             eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;				             
 					             eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
 					             eae.ucCurrentFheaderFbodyResponsePath=	FHEADERFBODY_RESPONSE_PATH_FROM_VCARD;					                                              						             
 					     }
 					     break;	
    	  	  	
    	  	  default:         	    
         	     break;
            	             	     	  	        	     	  	        	 	     	  	        	              	  	
	     	  }	     	     
	     } 
	     else{	     	      
    	       if (eae.ucEmuEngineStatus==EMU_ENGINE_WORKING){
    	          switch(eae.ucEmuApduNetApiStatus){
		              case EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY:  
		              	  eae.ucWaitGetInteractiveStartTime=GetTickCount();
		                  if (eae.ucFromEmuDataProperty==DATA_PROPERTY_IS_REQ_RST_ICC){	
		                  	     eae.ucCurrentEngineStatus=ENGINESTATUS_SIM_USIM_UIM_ENBALE;
		                  	     eae.ucAdfSessionStatus=ADFSESSIONSTATUS_DISABLE;
		                  	     std::cout<<RED<<"ADFSESSIONSTATUS_DISABLE"<<RESET<<std::endl<<std::flush;		                  	     
		                  	     eae.ucCurrentExpect=EXPECT_IS_COMMAND; //wx
		                  	     eae.ulApduCommandCounter=0;  	     	  	                  	       	        	   					
		        	  					   DirectTransfer_EmuToNet();	   	     	  	        	  					 	     	  	        	  
		                  }	
		                  else  {
                	    if (eae.ucFromEmuDataProperty==DATA_PROPERTY_IS_DATA){ 
              	    	    if (eae.ucTransmissionProtocol==TRANSMISSION_PROTOCOL_T0){	     	  	                  	    	    	 
    	   										  if (VerifyDataFromEmu()>0){   //判断是否数据合理，包括当前选择Sim 还是Usim 是否合理      	   										  	
 	   										        	if (eae.ucCurrentExpect==EXPECT_IS_COMMAND){
 	   										        	 if (	SubProcessFromEmuCommand()==-1)
 	   										        	 	   return -1; 	   										        	
 	   										          }	
    										          else{
  							                  if (eae.ucCurrentExpect==EXPECT_IS_ARGUMENT_DATA){	
  							                  	     
  							                  	  if (	SubProcessFromEmu_Argument()==-1)
 	   										        	 	   return -1; 	  	  	        	   										                   							                  	 
  							                  }	     	  	        	   										                  
  										          }
    	   										  }
    	   									}
    	   									else     	  	        	   									           
                              DirectTransfer_EmuToNet();	     	  	        	   									 	  	        	  					 	     	  	        	  
                      }
                 }		                  	  
	  	                break;
	  	            case EMU_APDU_NETAPI_STATUS_ICC_RST_DELIVER_TO_REMOTE:
    	  	  	         tmpTimeCounter=GetTickCount();
 					             if (tmpTimeCounter<eae.ucWaitGetInteractiveStartTime)
 					                 tmpTimeCounter+=0xFFFFFFFF-eae.ucWaitGetInteractiveStartTime;
 					             else
 					                 tmpTimeCounter=tmpTimeCounter-eae.ucWaitGetInteractiveStartTime;
 					             if(tmpTimeCounter>EMUIATIMEOUT){
 					            		 std::cout<<RED<<"Warning! ICC_RST_DELIVER_TO_REMOTE Timeout"<<RESET<<std::endl<<std::flush;	
 					            		 eae.ucEmuApduNetApiStatus= EMU_APDU_NETAPI_STATUS_ERROR;
 					           		   return -1;
 					             }
 					             else{
 					                  eae.ucEmuApduNetApiStatus= EMU_APDU_NETAPI_STATUS_IDLE;  					             
 					                  SetDefaultCommandIdentifier(); 		
 					                  eae.ExpectSyncLocal_Remote=EXPECTSYNCLOCAL_REMOTE_DISABLE;				             
 					                  eae.SyncLocal_Remote=SYNCLOCAL_REMOTE_YES;
 					                  
 					                  eae.ucCurrentFheaderFbodyResponsePath=	FHEADERFBODY_RESPONSE_PATH_FROM_VCARD;						                                              						             
 					             }
 					             break;	
 					        case EMU_APDU_NETAPI_STATUS_DATA_FROM_NET_IS_READY: 
 					        	   SubProcessFromNet(); 
 					        		
 					             break;
 					        default:         	    
         	             break;         
    	  	       }          
    	      }
	     	      
	     	
	     }
	     	
	     
	     return 1;      	
	} 
	
unsigned long Emu_Engine::GetTickCount()  
{  
    struct timespec ts;  
  
    clock_gettime(CLOCK_MONOTONIC, &ts);  
  
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);  
}	

Emu_Engine::Emu_Engine(){
	  EmuApduEngineInit();  
	
	}		
	
// 获取准备发送给网络侧的数据	
int Emu_Engine::CopyEaeDataToDeliverNetBuf(unsigned char* DeliverNetBuf, unsigned short* lengthptr,unsigned char* ToNetDataPropertyPtr){
	   memcpy(DeliverNetBuf,eae.ucToNetDataBuf, eae.usToNetDataBufLength);	    
	   (*lengthptr)=eae.usToNetDataBufLength;
	   (*ToNetDataPropertyPtr)=eae.ucToNetDataProperty;
	   return 1;
}	

// 获取准备发送给Emu侧的数据	
int Emu_Engine::CopyNetDataToEaeBuf(unsigned char* netbuf, unsigned short length,unsigned char FromNetDataProperty){
	   memcpy(eae.ucFromNetDataBuf,netbuf,length);	    
	   eae.usFromNetDataBufLength=length;
	   eae.ucFromNetDataProperty=FromNetDataProperty;
	   return 1;
}

// 设置Atr状态
int Emu_Engine::SetEmuSimcardAtrReadyStatus(unsigned char ucEmuSimcardAtrReadystatus){
	  eae.ucEmuSimcardAtrReady=ucEmuSimcardAtrReadystatus ;
	  return 1;
	}		
unsigned char Emu_Engine::GetEmuSimcardAtrReadyStatus( ){
	  return eae.ucEmuSimcardAtrReady;	  
	}				
	
//设置传递Atr到Emu状态
int Emu_Engine::SetDeliverAtrToEmuStatus(unsigned char ucDeliverAtrToEmustatus){
	  eae.ucDeliverAtrToEmu=ucDeliverAtrToEmustatus ;
	  return 1;
	}		

//设置传输协议
int Emu_Engine::SetEmuTransmissionProtocol(unsigned char ucTransmissionProtocol){
	  eae.ucTransmissionProtocol=ucTransmissionProtocol ;
	  return 1;
	}	

//设置Eae 本地的Vcard状态	
int Emu_Engine::SetEmuLocoalVCardReadyStatus(unsigned char ucLocoalVCardReadystatus){
	  eae.ucLocoalVCardReady=ucLocoalVCardReadystatus ;
	  return 1;	  
	}			
	
// 设置来自Emu 的数据属性	
int  Emu_Engine::SetFromEmuDataProperty(unsigned char ucFromEmuDataProperty){
	  eae.ucFromEmuDataProperty=ucFromEmuDataProperty ;
	  return 1;	  
	}	

unsigned char Emu_Engine::GetFromEmuDataProperty( ){
	  return eae.ucFromEmuDataProperty;	  
	}	
	
//将来自Emu的数据传入Eae
int Emu_Engine::CopyFromEmuDataToEaeBuf(unsigned char* fromemubuf, unsigned short length,unsigned char FromEmuDataProperty){
	   memcpy(eae.ucFromEmuDataBuf,fromemubuf,length);	    
	   eae.usFromEmuDataBufLength=length;
	   eae.ucFromEmuDataProperty=FromEmuDataProperty;
	   return 1;
}	

//获取准备发送到网络侧的数据属性
unsigned char Emu_Engine::GetToNetDataProperty( ){
	  return eae.ucToNetDataProperty;	  
	}	
		
unsigned short Emu_Engine::GetToEmuDataLength( ){
	  return eae.usToEmuDataBufLength;	  
	}	
	
int Emu_Engine::CopyEaeDataToEmuBuf(unsigned char* ToEmuBuf, unsigned short* lengthptr){
	   memcpy(ToEmuBuf,eae.ucToEmuDataBuf, eae.usToEmuDataBufLength);	    
	   (*lengthptr)=eae.usToEmuDataBufLength;	   
	   return 1;
}	

