#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/aes.h>
#include <string.h>

#include <openssl/pem.h>  
#include <openssl/bio.h>  
#include <openssl/evp.h>  
  
int base64_encode(char *in_str, int in_len, char *out_str)  
{  
    BIO *b64, *bio;  
    BUF_MEM *bptr = NULL;  
    size_t size = 0;  
  
    if (in_str == NULL || out_str == NULL)  
        return -1;  
  
    b64 = BIO_new(BIO_f_base64());  
    bio = BIO_new(BIO_s_mem());  
    bio = BIO_push(b64, bio);  
  
    BIO_write(bio, in_str, in_len);  
    BIO_flush(bio);  
  
    BIO_get_mem_ptr(bio, &bptr);  
    memcpy(out_str, bptr->data, bptr->length);  
    out_str[bptr->length] = '\0';  
    size = bptr->length;  
  
    BIO_free_all(bio);  
    return size;  
}  
  
int base64_decode(char *in_str, int in_len, char *out_str)  
{  
    BIO *b64, *bio;  
    BUF_MEM *bptr = NULL;  
    int counts;  
    int size = 0;  
  
    if (in_str == NULL || out_str == NULL)  
        return -1;  
  
    b64 = BIO_new(BIO_f_base64());  
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  
  
    bio = BIO_new_mem_buf(in_str, in_len);  
    bio = BIO_push(b64, bio);  
  
    size = BIO_read(bio, out_str, in_len);  
    out_str[size] = '\0';  
  
    BIO_free_all(bio);  
    return size;  
}  
int main(int argc,char **argv)
{
#if 0 
	if(argc != 3 )
	{
		printf("Usage:%s <password> <filename>\n",argv[0]);
		return -1;
	}
#else
	if(argc != 2 )
	{
		printf("Usage:%s <password> \n",argv[0]);
		return -1;
	}	
#endif	
    char userkey[AES_BLOCK_SIZE + 1] = {0};
	char data[1024] = {0};

	unsigned char *encrypt = malloc(1024);
    unsigned char *plain = malloc(1024);
    AES_KEY key;

	int nLen = strlen(argv[1]);
	int nBei = nLen / AES_BLOCK_SIZE + 1;
	int nTotal = nBei * AES_BLOCK_SIZE;
	int nNumber = 0;
	if (nLen % 16 > 0)
		nNumber = nTotal - nLen;
	else
		nNumber = 16;
	
	memset(data, nNumber, nTotal);
	memcpy(data, argv[1], nLen);

	
	strcpy(userkey,"Openvox is No.1.");
	
    memset((void*)encrypt, 0, 1024);
    memset((void*)plain, 0, 1024);

    /*设置加密key及密钥长度*/
    AES_set_encrypt_key(userkey, AES_BLOCK_SIZE*8, &key);

    int len = nTotal;
	char* src = data;
	char* dst = encrypt;
	/*加密，每次只能加密AES_BLOCK_SIZE长度的数据*/
    while (len)
    {
      AES_ecb_encrypt(src, dst, &key, AES_ENCRYPT); 
      len -= AES_BLOCK_SIZE;
      dst += AES_BLOCK_SIZE;
      src += AES_BLOCK_SIZE;
    }
	 


    /*设置解密key及密钥长度*/    
    AES_set_decrypt_key(userkey, AES_BLOCK_SIZE*8, &key);

    len = nTotal;
    /*解密*/
	src = encrypt;
	dst = plain;
	/*加密，每次只能加密AES_BLOCK_SIZE长度的数据*/
    while (len)
    {
      AES_ecb_encrypt(src, dst, &key, AES_DECRYPT); 
      len -= AES_BLOCK_SIZE;
      src += AES_BLOCK_SIZE;
      dst += AES_BLOCK_SIZE;
    }
       
#if 0
    /*解密后与原数据是否一致*/
    if(!memcmp(plain, data, nLen)){
        printf("decrypt success\n");    
    }else{
        printf("decrypt failed\n");   
		return -2;
    }

    printf("encrypt:\n");
    int i = 0;
    for(i = 0; i < nTotal ; i++){
        printf("%.2x ", encrypt[i]);
    }

	printf("\n");
	FILE *fp = fopen(argv[2],"w+");
	if(fp)
	{
		fwrite(encrypt,1,nTotal,fp);
		fclose(fp);
	}
	else{
		printf("open file %s error\n",argv[2]);
	}
#endif
    char outstr1[1024] = {0};  
    base64_encode(encrypt,nTotal,outstr1);  
    printf("%s",outstr1); 
	
    return 0;
}

