#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "uc15_upgrade.h"
//#define PRINT(fmt,...) printf(fmt, ##args)
#define PRINT printf

#define NORMAL_LINE_SIZE 45
#define LAST_LINE_SIZE 29
#define MIN_LINE_SIZE 13
#define NORMAL_FORMAT ":%08x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n"
#define LOAST_FORMT   ":%08x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n"
#define ADDR_FORMT    ":%08x%04x%02x\r\n"
/*
*描述：读取nprg文件中的内容
*输入参数：filename -- nprg文件名称
*          nprg_buf      -- nprg 
*          start_addr --第一行中的偏移量
*返回值: <0 error
*        >0 size
*/
#if 0
int load_nprg_hex(char *filename, unsigned char *nprg_buf, short *start_addr ){
#endif
int load_nprg_hex(struct firmware_file *file, short *start_addr){
	if(file->filename == NULL){
		return -1;
	}
	FILE *nprg_handle = fopen(file->filename, "rb");
	if(nprg_handle == NULL){
		PRINT("open %s failed!");
		return -2;
	}
	int file_size = 0;
	char *line;
	int i = 0;
	size_t size = 0;
	size_t read_size;
	char end_flag;
	int end_addr;
	
	file_size = get_file_size(file->filename);
	if(file_size <= 0)
		return -3;
	
	file->data = (unsigned char *)malloc(file_size + 1);
	
	read_size = getline(&line, &size, nprg_handle);
	if(read_size < 0){
		return -4;
	}else{
	//第一行中读取起始地址,end_addr没有实质性的作用，用于分割字符串
		sscanf(line, ADDR_FORMT, &end_addr, start_addr, &end_flag);
	}
	while((read_size = getline(&line, &size, nprg_handle)) > MIN_LINE_SIZE){
		if(read_size == NORMAL_LINE_SIZE){
			sscanf(line, NORMAL_FORMAT, &end_addr, &file->data[i],&file->data[i+1],&file->data[i+2],&file->data[i+3],
									&file->data[i+4],&file->data[i+5],  &file->data[i+6], &file->data[i+7], &file->data[i+8],
									&file->data[i+9],&file->data[i+10], &file->data[i+11], &file->data[i+12],&file->data[i+13],
									&file->data[i+14],&file->data[i+15], &end_flag);
			i = i + 16;
		}
		else if(read_size == LAST_LINE_SIZE){
			sscanf(line, LOAST_FORMT, &end_addr, &file->data[i],&file->data[i+1],&file->data[i+2],&file->data[i+3],
							&file->data[i+4],&file->data[i+5],  &file->data[i+6], &file->data[i+7],&end_flag);
			i = i + 8;
		}
		else
			continue;
	}
//	PRINT("size = %d\n", i);
	free(line);
	fclose(nprg_handle);

	file->size = i;	
	return i;
}

//#define LOAD_NPRG_TEST
#ifdef LOAD_NPRG_TEST
void load_nprg_test(void){
	char *filename = (char *)"NPRG6270.hex";
	char *nprg_buf = (char *)malloc(312712);
	short start_addr=0;
	if(nprg_buf == NULL){
		PRINT("malloc memory failed");
		return;
	}
	int size = load_nprg_hex(filename, nprg_buf, &start_addr);
	if(size > 0){
		printf("nprg size=%d, start_addr = %04x\n", size, start_addr);
	}else{
		return;
	}
	FILE* handle = fopen("new.hex", "w+");
	if(handle == NULL){
		return;
	}
	fwrite(nprg_buf, size, 1, handle);
	fclose(handle);
}

int main(int argc, char **argv){
	load_nprg_test();
	return 0;
}
#endif
