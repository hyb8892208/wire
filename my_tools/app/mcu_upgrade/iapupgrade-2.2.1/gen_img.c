#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "aes.h"

/* This application generate firmware use for upgrade or download */
#define UPGRADE_PREFIX      "upgrade_"
#define DOWNLOAD_PREFIX     "download_"

#define BOOTLOADER_SIZE_MAX      (32 * 1024)
#define APP_SIZE_MAX             (64 * 1024)
#define APP_SHIFT_MAX            (0x8000)

#define PKT_LEN     (32)
/* Total size = 32 Byte */
struct apdu_t {
    char tag[8];        /* Unique flag */
    uint32_t mcu;       /* MCU model */
    char hw_type[8];    /* Hardware desc */
    uint32_t size;      /* App size */
    uint16_t crc16;     /* App CRC16*/
    uint8_t major;      /* App major number */
    uint8_t minor;      /* App minor number */
    uint8_t subminor;   /* App subminor number */
    uint8_t bugfix;     /* App bugfix number */
    uint16_t build;     /* App build number */
} __attribute__((packed));

enum IAP_UPDATE_RES{
	IAP_UPDATE_NONE = 'N',
	IAP_UPDATE_FAILED = 'F',
	IAP_UPDATE_AUTH_FAILED = 'A',
	IAP_UPDATE_RECV_FAILED = 'R',
	IAP_UPDATE_SUCCESS = 'S',
};

enum IAP_MODE{
	IAP_APP_MODE = 'A',    		 /* Jump to Appication */
	IAP_BOOT_MODE = 'B',
};

/* Total size = 32 Byte */
struct param_t {
    uint16_t mode;
    uint16_t update;
    uint16_t reserve[14];
} ;

static uint8_t key[] = { 0x3b, 0x7e, 0x25, 0x16, 0x22, 0xae, 0x2d, 0xa6, 0xab, 0xf7, 0x55, 0x88, 0x09, 0xcf, 0x4e, 0x3c };
static uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

#define TER_NONE             "\e[0m"
#define TER_BOLD             "\e[1m"
#define TER_RED              "\e[0;31m"
#define TER_LRED             "\e[1;31m"
#define TER_GREEN            "\e[0;32m"
#define TER_LGREEN           "\e[1;32m"
         
void show_wellcome(void)
{
    printf(
        "\n"
        "##########################################################################\n"
        "#      OpenVox IAP Wireless Gateway STM32 Firmware Rebuild Utility       #\n"
        "#                               V1.1.0                                   #\n"
        "# Firmware header packet size is 32 Byte, CRC16 checksum, AES encryption #\n"
        "#                     OpenVox Communication Co.,Ltd                      #\n"
        "#        Copyright (c) 2009-2020 OpenVox. All Rights Reserved.           #\n"
        "##########################################################################\n");
}

void usage(char *path)
{
    printf(
        "OpenVox IAP Wireless Gateway STM32 Firmware Rebuild Utility\n"
        "Firmware header packet size is 32 Byte, CRC16 checksum, AES encryption\n"
        "Usage:\n"
        "  %s  -h   --  get help\n"
        "  %s  -t <tag> -m <mcu> -n <hw_type> -a <app>  --  generate upgrade firmware\n"
        "  %s  -t <tag> -m <mcu> -n <hw_type> -a <app> -b <bootloader> -i <fwhead_shift> -s <app_shift>  --  generate download firmware\n"
        "\n"
        "Options:\n"
        "  -h      Show help\n"
        "  -t      tag (8 byte string)\n"
        "  -m      mcu number (0 ~ 65536)\n"
        "  -n      hw_type name (8 byte string)\n"
        "  -a      app source firmware\n"
        "  -b      bootloader source firmware\n"
        "  -i      fwhead shift from start (0 ~ 0x%x)\n"
        "  -s      app shift from start (0 ~ 0x%x)\n"
        "\n", path, path, path, APP_SHIFT_MAX, APP_SHIFT_MAX);
    exit(0);
}

// prints string as hexstatic
void phex(uint8_t* str, char *desc)
{
    printf("%s :\n", desc);
    unsigned char i;
    for(i = 0; i < 128; i++){
        printf("%02x", str[i]);
        if((i & 0x0F) == 0x0F)
            printf("\n");
        else
            printf(" ");
    }
    printf("\n");
}

char *mcuid2desc(uint32_t mcu)
{
    if(mcu < 0 || mcu > 65536){
        printf("the MCU ID is too big!\n");
        return NULL;
    }

    if(mcu == 42){
        return "STM32F042";
    }else if(mcu == 72){
        return "STM32F072";
    }else if(mcu == 103){
        return "STM32F103";
    }else{
        return "Unrecognized model";
    }
}

unsigned long calc_filesize(char *filename)
{
    struct stat statbuff;

    if(!filename || !strlen(filename))
        return 0;

    if((access(filename, F_OK)) != 0){
        printf("%s not exist\n", filename);
        return 0;
    }

    if(stat(filename, &statbuff) < 0)
        return 0;
    else
        return statbuff.st_size;
}

int get_file_modifydate(char *filename, char *str)
{
    struct stat statbuff;
    struct tm *tm_info;

    if(!filename || !strlen(filename))
        return -1;
    if((access(filename, F_OK)) != 0){
        printf("%s not exist\n", filename);
        return -1;
    }
    if(stat(filename, &statbuff) < 0)
        return -1;
    tm_info = localtime(&statbuff.st_mtim.tv_sec);
    snprintf(str, 16, "%4d-%02d-%02d", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
    return 0;
}

unsigned short calculate_crc(unsigned short crc, unsigned char data)
{
	crc  = (unsigned char)(crc >> 8) | (crc << 8);
	crc ^= data;
	crc ^= (unsigned char)(crc & 0xff) >> 4;
	crc ^= (crc << 8) << 4;
	crc ^= ((crc & 0xff) << 4) << 1;
	return crc;
}

unsigned short calc_filecrc16(FILE *fp)
{
    unsigned char send_buf[PKT_LEN];
    int i;
    int len = 0;
    unsigned short checksum = 0x0000;

    while((len = fread(send_buf, 1, PKT_LEN, fp))){
        for(i = 0; i < len; i++)
            checksum = calculate_crc(checksum, send_buf[i]);
        memset(send_buf, 0, PKT_LEN);
    }
    return checksum;
}

void show_fw_info(struct apdu_t *fw_head, char *fw_name, int fwhead_shift, int app_shift)
{
    char strdate[32] = {0}, strtag[32] = {0}, strhw[32] = {0};
    int idx = 0, line = 0;;
    
    if(NULL == fw_head)
        return;
    
    memcpy(strtag, fw_head->tag, sizeof(fw_head->tag));
    memcpy(strhw, fw_head->hw_type, sizeof(fw_head->hw_type));
    get_file_modifydate(fw_name, strdate);
    
    printf("List Firmware information:\n");
    printf("    Tag: " TER_LGREEN "%s\n" TER_NONE, strtag);
    printf("    MCU Name: " TER_LGREEN "%s\n" TER_NONE, mcuid2desc(fw_head->mcu));
    printf("    Hareware Type: " TER_LGREEN "%s\n" TER_NONE, strhw);
    printf("    Build Date: %s\n", strdate);
    printf("    Firmware Size: %u Bytes\n", fw_head->size);
    printf("    Firmware CRC16: 0x%04x\n", fw_head->crc16);
    printf("    Firmware bin: %s\n", fw_name);
    printf("    Firmware Header Shift: 0x%x\n", fwhead_shift);
    printf("    Firmware App Shift: 0x%x\n", app_shift);
    
    if((fwhead_shift == 0) && (app_shift == sizeof(struct apdu_t))) {  //upgrade file struct
        printf(TER_LGREEN "Upgrade" TER_NONE " File Structure:\n");
        printf(TER_LGREEN "**************************\n" TER_NONE);
        printf(TER_LGREEN "*  FW Head (%4d pkt)    *\n" TER_NONE, 1);
        printf(TER_LGREEN "**************************\n" TER_NONE);
        line = (fw_head->size >> 10) + 1;
        for(idx = 0; idx < line; idx++) {
            if(idx == (line >> 1))
                printf(TER_LGREEN "* Encrypt App (%4d pkt) *\n" TER_NONE, (fw_head->size + PKT_LEN - 1) / PKT_LEN);
            else
                printf(TER_LGREEN "*                        *\n" TER_NONE);
        }
        printf(TER_LGREEN "**************************\n" TER_NONE);
    }
    else {  //download file struct
        printf(TER_LGREEN "Download" TER_NONE " File Structure:\n");
        printf(TER_LGREEN "**************************\n" TER_NONE);
        line = (fwhead_shift >> 10) + 1;
        for(idx = 0; idx < line; idx++) {
            if(idx == (line >> 1))
                printf(TER_LGREEN "*  Bootloader (%4d Kb)  *\n" TER_NONE, fwhead_shift >> 10);
            else
                printf(TER_LGREEN "*                        *\n" TER_NONE);
        }
        printf(TER_LGREEN "**************************\n" TER_NONE);
        printf(TER_LGREEN "*   FW Head (%4d Kb)    *\n" TER_NONE, (app_shift + 0x3FF - fwhead_shift) >> 10);
        printf(TER_LGREEN "**************************\n" TER_NONE);
        line = (fw_head->size >> 10) + 1;
        for(idx = 0; idx < line; idx++) {
            if(idx == (line >> 1))
                printf(TER_LGREEN "*  Source App (%4d Kb)  *\n" TER_NONE, fw_head->size >> 10);
            else
                printf(TER_LGREEN "*                        *\n" TER_NONE);
        }
        printf(TER_LGREEN "**************************\n" TER_NONE);
    }
}

int main(int argc, char ** argv)
{
    struct apdu_t *fw_head = NULL;
    struct param_t *fw_param = NULL;
    unsigned char buffer[PKT_LEN] = {0};
    unsigned char crypt_buffer[PKT_LEN] = {0};
    char foutname[34] = {0};
    char *filename = NULL, *bootloader = NULL;
    long app_shift = sizeof(struct apdu_t), fwhead_shift = 0, fill_shift = 0;  //default output upgrade file
    FILE *fin_app = NULL, *fin_bootloader = NULL, *fout = NULL;
    int opt = 0;
    int flag_argv = 0;
    unsigned int pkt_cnt = 0;
    unsigned long fw_size, bootloader_size;
    int res = 0;
    char *end;
    
    show_wellcome();
    
    if(argc < 2) {
        usage(argv[0]);
    }
    
    if(PKT_LEN < sizeof(struct apdu_t)){
        printf(TER_LRED "sizeof(struct apdu_t) must be limited to PKT_LEN\n" TER_NONE);
        return -1;
    }
    fw_head = (struct apdu_t *)malloc(PKT_LEN);
    if(!fw_head){
        printf(TER_LRED "failed to allocate struct apdu_t.\n" TER_NONE);
        return -1;
    }
    memset(fw_head, 0xFF, PKT_LEN);   //mcu flash data default is 0xFF
    
    while((opt = getopt(argc, argv, "ht:m:n:a:b:i:s:")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                break;
            case 't':
                flag_argv |= 0x01;
                snprintf(fw_head->tag, sizeof(fw_head->tag) + 1, optarg);
                if(0 == strcmp(optarg, "xovnepo"))
                    fw_head->tag[7] = '\n';
                break;
            case 'm':
                flag_argv |= 0x02;
                fw_head->mcu = strtol(optarg, &end, 0);
                if(*end || !*optarg)
                    fw_head->mcu = -1;
                break;
            case 'n':
                flag_argv |= 0x04;
                snprintf(fw_head->hw_type, sizeof(fw_head->hw_type) + 1, optarg);
                break;
            case 'a':
                flag_argv |= 0x08;
                filename = optarg;
                break;
            case 'b':
                flag_argv |= 0x10;
                bootloader = optarg;
                break;
            case 'i':
                flag_argv |= 0x20;
                fwhead_shift = strtol(optarg, &end, 0);
                if(*end || !*optarg)
                    fwhead_shift = -1;
                break;
            case 's':
                flag_argv |= 0x40;
                app_shift = strtol(optarg, &end, 0);
                if(*end || !*optarg)
                    app_shift = -1;
                break;
            default:
                printf(TER_LRED "unknown option '%c'\n" TER_NONE, opt);
                break;
        }
    }
    if((flag_argv & 0x0F) != 0x0F) {
        printf("essential option is missing\n");
        usage(argv[0]);
        goto err;
    }
    
    if((flag_argv & 0x70) == 0x70) {  //if want to generate download firmware, check bootloader ,fwhead shift and app shift
        if(!bootloader && !strlen(bootloader)) {
            printf(TER_LRED "bootloader firmware is invalid!\n" TER_NONE);
            goto err;
        }
        bootloader_size = calc_filesize(bootloader);
        if((bootloader_size < 1) || (BOOTLOADER_SIZE_MAX < bootloader_size)) {
            printf(TER_LRED "Unsupport bootloader firmware size:%ld, limited to 1 ~ %d Byte\n" TER_NONE, bootloader_size, BOOTLOADER_SIZE_MAX);
            goto err;
        }
        //download file layout : [boot_start  boot_end] ~ [fwhead_start  fwhead_end] ~ [app_start  app_end]
        if((fwhead_shift < bootloader_size) || (APP_SHIFT_MAX < fwhead_shift)) {
            printf(TER_LRED "fwhead shift is invalid, limited to bootloader size ~ 0x%x\n" TER_NONE, APP_SHIFT_MAX);
            goto err;
        }
        //check intersection of bootloader/fwhead/app
        if((app_shift < (fwhead_shift + 1024) || (APP_SHIFT_MAX < app_shift))) {
            printf(TER_LRED "app shift is invalid, limited to bootloader size ~ 0x%x, and behind fwhead\n" TER_NONE, APP_SHIFT_MAX);
            goto err;
        }
    }
    else if((flag_argv & 0x70) != 0){  //-b -s must be specify at the same time
        printf(TER_LRED "If want to generate download firmware, option [bootloader],[fwhead_shift],[app_shift] must be specify\n" TER_NONE);
        usage(argv[0]);
        goto err;
    }
    
    if(!filename && !strlen(filename)) { //app firmware check
        printf(TER_LRED "app firmware is invalid!\n" TER_NONE);
        goto err;
    }
    
    fw_size = calc_filesize(filename);
    if((0 == fw_size) || (APP_SIZE_MAX <= fw_size)) {
        printf(TER_LRED "Unsupport app firmware size:%ld, limited to 1 ~ %d Byte\n" TER_NONE, fw_size, APP_SIZE_MAX);
        goto err;
    }
    fw_head->size = (uint32_t)fw_size;
    
    fin_app = fopen(filename, "rb");
    if(fin_app == NULL) {
        printf(TER_LRED "error opening '%s'\n" TER_NONE, filename);
        goto err;
    }
    
    fw_head->crc16 = calc_filecrc16(fin_app);
    if(0 == fw_head->crc16) {
        printf(TER_LRED "calculate source firmware %s crc16 error\n" TER_NONE, filename);
        goto err;
    }
    
    show_fw_info(fw_head, filename, fwhead_shift, app_shift);
    
    if(bootloader != NULL)
        snprintf(foutname, sizeof(foutname), "%s%s", DOWNLOAD_PREFIX, filename);
    else
        snprintf(foutname, sizeof(foutname), "%s%s", UPGRADE_PREFIX, filename);  //default generate upgrade file
    
    fout = fopen(foutname, "wb");
    if(fout == NULL) {
        printf(TER_LRED "error opening '%s'\n" TER_NONE,foutname);
        goto err;
    }
    
    if(bootloader != NULL) {  //write bootloader to output file have not encrypt
        fin_bootloader = fopen(bootloader, "rb");
        if(fin_bootloader == NULL) {
            printf(TER_LRED "error opening '%s'\n" TER_NONE, bootloader);
            goto err;
        }
        printf("Append the %s into the Image...\n", bootloader);
        if(fseek(fin_bootloader, 0, SEEK_SET)){
            printf(TER_LRED "File %s fseek start fail\n" TER_NONE, bootloader);
            goto err;
        }
        while(fread(buffer, 1, PKT_LEN, fin_bootloader)){
            //phex(buffer, "pkt");
            if(fwrite(buffer, PKT_LEN, 1, fout) != 1){
                printf(TER_LRED "failed to write bootloader!\n" TER_NONE);
                goto err;
            }
            memset(buffer, 0xFF, PKT_LEN); //mcu flash data default is 0xFF
        }
        //fill 0xFF in blank area
        fill_shift = ftell(fout);
        memset(buffer, 0xFF, PKT_LEN);
        while(fill_shift < fwhead_shift){
            if(fwrite(buffer, PKT_LEN, 1, fout) != 1){
                printf(TER_LRED "failed to write bootloader!\n" TER_NONE);
                goto err;
            }
            fill_shift += PKT_LEN;
        }
    }
    
    printf("Prepend the header into the Image...\n");
    printf("Firmware header packet = %d byte\n", PKT_LEN);
    
    if(fseek(fout, fwhead_shift, SEEK_SET)){  //set fw header shift in output file
        printf(TER_LRED "File %s fseek fwhead shift fail\n" TER_NONE, foutname);
        goto err;
    }
    
    //write fw_head to output file, fw_head position before app
    if(fwrite(fw_head, PKT_LEN, 1, fout) != 1){
        printf(TER_LRED "failed to write fw_head header!\n" TER_NONE);
        goto err;
    }
    pkt_cnt++; //header pkt
    
    if(bootloader != NULL){   //download file need to init param data
        fw_param = (struct param_t *)malloc(sizeof(struct param_t));
        if(!fw_param){
            printf(TER_LRED "failed to allocate struct param_t.\n" TER_NONE);
            goto err;
        }
        memset(fw_param, 0xFF, sizeof(struct param_t));   //mcu flash data default is 0xFF
        fw_param->mode = IAP_APP_MODE;
        fw_param->update = IAP_UPDATE_NONE;
        //write fw_head to output file, fw_head position before app
        if(fwrite(fw_param, PKT_LEN, 1, fout) != 1){
            printf(TER_LRED "failed to write fw_param!\n" TER_NONE);
            goto err;
        }
        //fill 0xFF in blank area
        fill_shift = ftell(fout);
        memset(buffer, 0xFF, PKT_LEN);
        while(fill_shift < app_shift){
            if(fwrite(buffer, PKT_LEN, 1, fout) != 1){
                printf(TER_LRED "failed to write bootloader!\n" TER_NONE);
                goto err;
            }
            fill_shift += PKT_LEN;
        }
    }
    
    printf("Append the %s into the Image...\n" TER_NONE, filename);
    if(fseek(fin_app, 0, SEEK_SET)){
        printf(TER_LRED "File %s fseek start fail\n" TER_NONE, filename);
        goto err;
    }
    
    if(fseek(fout, app_shift, SEEK_SET)){  //set app shift in output file
        printf(TER_LRED "File %s fseek app shift fail\n" TER_NONE, foutname);
        goto err;
    }

    while(fread(buffer, 1, PKT_LEN, fin_app)){
        //phex(buffer, "pkt");
        if(bootloader == NULL){ //upgrade file have encrypt
            AES128_CBC_encrypt_buffer(crypt_buffer, buffer, PKT_LEN, key, iv);
            memcpy(buffer, crypt_buffer, PKT_LEN);
        }
        
        if(fwrite(buffer, PKT_LEN, 1, fout) != 1){
            printf(TER_LRED "failed to write firmware!\n" TER_NONE);
            goto err;
        }
        pkt_cnt++;  //firmware pkt
        memset(buffer, 0xFF, PKT_LEN); //mcu flash data default is 0xFF
        memset(crypt_buffer, 0, PKT_LEN);
    }
    
    if(0 != (res = ferror(fin_app)))
        printf(TER_LRED "Fail" TER_NONE " : read %s, ferror(fin_app)=%d\n", filename, res);
    else if(0 != (res = ferror(fout)))
        printf(TER_LRED "Fail" TER_NONE " : write %s, ferror(fout)=%d\n", foutname, res);
    else
        printf(TER_LGREEN "Successful" TER_NONE " : image have %d pkt(%db/pkt)\n", pkt_cnt, PKT_LEN);
    
err:
    free(fw_head);
    free(fw_param);
    if(fin_app)
        fclose(fin_app);
    if(fout)
        fclose(fout);
    if(fin_bootloader)
        fclose(fin_bootloader);
    
    return 0;
}

