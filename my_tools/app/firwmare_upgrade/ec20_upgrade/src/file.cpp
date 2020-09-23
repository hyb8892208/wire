#include "file.h"
#include "download.h"
#include "tinyxml.h"
#include "os_linux.h"
#include "quectel_common.h"
#include "quectel_log.h"
#include "quectel_crc.h"

extern void qdl_pre_download(void);
extern void qdl_post_download(void);

unsigned char * open_file(const char *filepath, uint32 *filesize) {
    unsigned char *filebuf;
    struct stat sb;
    int fd;

    if (filesize == NULL)
        return NULL;

    fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        QFLASH_LOGD("fail to open %s\n", filepath);
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        QFLASH_LOGD("fail to fstat %s\n", filepath);
        return NULL;
    }

#if 0 //some soc donnot support MMU, so donot support mmap
    filebuf = (byte *)mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (filebuf == MAP_FAILED) {
	close(fd);
        QFLASH_LOGD("fail to mmap %s\n", filepath);
        return NULL;
    }
 
    if (close(fd) == -1) {
        munmap(filebuf, sb.st_size);
        QFLASH_LOGD("fail to close %s\n", filepath);
        return NULL;
    }
#else
    if (sb.st_size > (1024*1024)) {
        close(fd);
        QFLASH_LOGD("%s %s 's size is %dKbytes, larger than 1MBytes\n!", __func__, filepath, (uint32)sb.st_size/(1024));
        return NULL;
    }

    filebuf = (unsigned char *)malloc(sb.st_size + 128);
    if (filebuf == NULL) {
        close(fd);
        QFLASH_LOGD("fail to malloc for %s\n", filepath);
        return NULL;
    }	

    if(read(fd, filebuf, sb.st_size) != sb.st_size) {
        close(fd);
        QFLASH_LOGD("fail to read for %s\n", filepath);
        return NULL;
    }

    close(fd);
#endif

    *filesize = sb.st_size;
    return filebuf;
}

void free_file(unsigned char *filebuf,uint32 filesize) {
    if (filebuf == NULL) return;
    
#if 0 //some soc donnot support MMU, so donot support mmap
    if (munmap(filebuf, filesize) == -1) {
        QFLASH_LOGD("fail to munmap %p %u\n", filebuf, filesize);
    }
#else
    free(filebuf);
#endif
}


bool GetNodePointerByName(TiXmlElement* pRootEle,const char* strNodeName,TiXmlElement* &Node)  
{  
	if (strcmp(strNodeName,pRootEle->Value())==0)  
	{  
		Node = pRootEle;  
		return true;  
	}  
	TiXmlElement* pEle = pRootEle;    
	
	for (pEle = pRootEle->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())    
	{    
		if(GetNodePointerByName(pEle,strNodeName,Node))  
			return true;  
	}    
	return false;  
}   

int retrieve_nrpg_enrpg_filename(const char* path, char** nrpg_filename, char **enrpg_filename)
{
	DIR *pdir;
	struct dirent* ent = NULL;
	pdir = opendir(path);
	if(pdir)
	{
		while((ent = readdir(pdir)) != NULL)
		{
			if(!strncmp(ent->d_name, "NPRG", 4))
			{
				*nrpg_filename = strdup(ent->d_name);
			}
			if(!strncmp(ent->d_name, "ENPRG", 5))
			{
				*enrpg_filename = strdup(ent->d_name);
			}
			
		}
		closedir(pdir);
		return 0;
	}else
	{
		return 1;
	}
	return 1;
}


int image_read(download_context *ctx) {
   
	//find contents.xml
	char *nrpg_filename = NULL;
	char *enrpg_filename = NULL;
	char *ptr = NULL;
	int ret = 0;
	TiXmlDocument *pDocNode = NULL;
	TiXmlDocument *pDoc = NULL;
	TiXmlElement *pRootEle = NULL;
	TiXmlElement *pNode = NULL;
	char* partition_nand_path;
	long long all_files_bytes = 0;
	vector<Ufile>::iterator iter; 
	
	asprintf(&ctx->contents_xml_path,"%s/%s", ctx->firmware_path, "contents.xml");
	if(access(ctx->contents_xml_path, F_OK))
	{
		QFLASH_LOGD("Not found contents.xml\n");
		return 0;
	}
	
	pDoc  = new TiXmlDocument();
	if (NULL==pDoc)  
	{  	
		return 0;  
	}  
	pDoc->LoadFile(ctx->contents_xml_path); 
	pRootEle = pDoc->RootElement();  
	if (NULL==pRootEle)  
	{  
		return 0;  
	}  
	
	if(GetNodePointerByName(pRootEle,"partition_file",pNode)==false)
		return 0;
	if (NULL!=pNode)  
	{  
		TiXmlElement *NameElement = pNode->FirstChildElement();
		asprintf(&partition_nand_path,"%s/%s%s",ctx->firmware_path,NameElement->NextSiblingElement()->GetText(),NameElement->GetText());
		ptr = ctx->firmware_path;
		asprintf(&ctx->firmware_path,"%s/%s",ctx->firmware_path,NameElement->NextSiblingElement()->GetText());
		if(ptr)
		{
			free(ptr);			
		}
		
	} 
	QFLASH_LOGD("%s\n",partition_nand_path);
	if(access(partition_nand_path, F_OK))
	{
		QFLASH_LOGD("Not found partition_nand.xml\n");
		ret = 0;
		goto __exit_image_read;
	}
	delete pDoc;
	pDocNode  = new TiXmlDocument();
	if (NULL==pDocNode)  
	{  
		ret = 0;
		goto __exit_image_read;  
	}  
	pDocNode->LoadFile(partition_nand_path);
	pRootEle= pDocNode->RootElement();
	if (NULL==pRootEle)  
	{  
		ret = 0;
		goto __exit_image_read; 
	}  	
	pNode = NULL;  
	if(GetNodePointerByName(pRootEle,"partitions",pNode)==false)
		return 0;
	if (NULL!=pNode)  
	{
		for (TiXmlElement * pEle = pNode->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())    
		{
			
			Ufile ufile = {0};
			int i = 0;
			for (TiXmlElement * tmp=pEle->FirstChildElement();tmp;tmp=tmp->NextSiblingElement())
			{
				if(strcmp("name",tmp->Value())==0)
				{
					asprintf(&ufile.name,"%s",tmp->GetText());
					i++;
					{
						char * p = strstr(ufile.name, ":");
						if(p == NULL)
						{
							QFLASH_LOGD("error, parse partition name failed!");
						}else
						{
							p++; //skip :
							asprintf(&ufile.partition_name, "%s", p);
						}
						
					}
				}
				if(strcmp("img_name",tmp->Value())==0)
				{
					asprintf(&ufile.img_name,"%s/%s",ctx->firmware_path,tmp->GetText());
					i++;
				}
			}
			if(i==2)
			{
				ctx->ufile_list.push_back(ufile);
			}
			else
			{			
				if(ufile.img_name)
				{
					free(ufile.img_name);
					ufile.img_name = NULL;
				}
				if(ufile.name)
				{
					free(ufile.name);
					ufile.name = NULL;
				}
				if(ufile.partition_name)
				{
					free(ufile.partition_name);
					ufile.partition_name = NULL;
				}
			}
		}
	}
	 
    for (iter=ctx->ufile_list.begin();iter!=ctx->ufile_list.end();iter++)  
    {  
		if(strcmp("0:MIBIB",((Ufile)*iter).name)==0)
		{
			asprintf(&ctx->partition_path,"%s",((Ufile)*iter).img_name);
		}		
		all_files_bytes += get_file_size(((Ufile)*iter).img_name);
    }
    transfer_statistics::getInstance()->set_total(all_files_bytes);
	if(retrieve_nrpg_enrpg_filename(ctx->firmware_path, &nrpg_filename, &enrpg_filename) != 0)
	{
		ret = 0;
		goto __exit_image_read;
	}
	if(nrpg_filename == NULL || enrpg_filename == NULL)
	{
		ret = 0;
		goto __exit_image_read;
	}	
	asprintf(&ctx->NPRG_path,"%s/%s",ctx->firmware_path,nrpg_filename);
	asprintf(&ctx->ENPRG_path,"%s/%s",ctx->firmware_path,enrpg_filename);
	ret = 1;
__exit_image_read:	
	delete pDocNode;
	delete partition_nand_path;
	if(nrpg_filename) delete nrpg_filename;
	if(enrpg_filename) delete enrpg_filename;
    return ret;
}

int image_close(download_context *ctx)
{
	delete ctx->firmware_path;
	ctx->firmware_path = NULL;
	delete ctx->contents_xml_path;
	ctx->contents_xml_path = NULL;
	delete ctx->NPRG_path;
	ctx->NPRG_path = NULL;
	delete ctx->ENPRG_path;
	ctx->ENPRG_path = NULL;
	delete ctx->partition_path;
	ctx->partition_path = NULL;
	vector<Ufile>::iterator iter;  
    for (iter=ctx->ufile_list.begin();iter!=ctx->ufile_list.end();iter++)  
    {  
		if( (*iter).name != NULL)
		{
			free((*iter).name);
		}
		if((*iter).img_name != NULL)
		{
			free((*iter).img_name);
		}
		if((*iter).partition_name != NULL)
		{
			free((*iter).partition_name);
		}
    }
    
	if(ctx->diag_port)
	{
		free(ctx->diag_port);
	}
	return 1;
}


