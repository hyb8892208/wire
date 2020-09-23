#include "openssl/err.h" 
#include "openssl/des.h"
#include "openssl/rc4.h"
#include "openssl/md5.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"

#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/modes.h>
using namespace CryptoPP;

#include <iostream>  
#include <cstring>
#include <fstream>  
#include <iterator>

using namespace std;

typedef enum {
        GENERAL = 0,
        ECB,
        CBC,
        CFB,
        OFB,
        TRIPLE_ECB,
        TRIPLE_CBC
}CRYPTO_MODE;

int padding = RSA_PKCS1_PADDING;

RSA* createRSA(unsigned char* key, int flag)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf(key, -1);

	if (keybio == NULL) {
		printf("Failed to create key BIO");
		return NULL;
	}

	if (flag)
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	else
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

	if (rsa == NULL)
		printf("Failed to create RSA");

	return rsa;
}


int private_encrypt(unsigned char* data, int data_len, unsigned char* key, unsigned char* encrypted)
{
	RSA * rsa = createRSA(key, 0);
	int result = RSA_private_encrypt(data_len, data, encrypted, rsa, padding);
	return result;
}

int public_decrypt(unsigned char* enc_data, int data_len, unsigned char* key, unsigned char* decrypted)
{
	RSA * rsa = createRSA(key, 1);
	int  result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, padding);
	return result;
}

void printLastError(char *msg)
{
	char * err = (char*)malloc(130);;
	ERR_load_crypto_strings();
	ERR_error_string(ERR_get_error(), err);
	printf("%s ERROR: %s\n", msg, err);
	free(err);
}
string pub0 = "-----BEGIN PUBLIC KEY-----\n";
string pub2 = "DcTmnybr5UvgM8MlMIpWqsDoYdAnk/iyTEvMbDY+ZUQybfpZlr34cysBsnkGI5Uk\n";
string pub6 = "24khfsGxjv8DdAbSLq4Nki3mxBdc8i1k15ktJsuSNscfq4fxxrFTfKGXF4Z4iISa\n";
string pub7 = "lwIDAQAB\n";
string pub5 = "cOS2BDQ/r7Jg3k73pAbYHtzvAKL33b+bq2crQqiBX12QpgPP6f+DLWCXPh8XCETr\n";
string pub3 = "9G/msMpSacVGlZyiifLqcJzy70O0dGjImgx/zUJZaqpRUW06gdhO1rF8mMPKfGRD\n";
string pub4 = "phCDPTjfM5iJgHHPsqrl2TrWt1k0GGxAIXiMelCxp7ZSeOhWsr8DwIKYc8efWvIr\n";
string pub1 = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7pAaq+arLy+3INksKr5Q\n";
string pub8 = "-----END PUBLIC KEY-----";
int RSA_test1(string cleartext,string &plain)
{
	string plainText = cleartext;


    //ifstream inputFile("/data/license/public.pem");
    //string publicKey((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());
	string publicKey = pub0 + pub1 + pub2 + pub3 + pub4 + pub5 + pub6 + pub7 + pub8;


	unsigned char decrypted[1024] = {};

	int decrypted_length = public_decrypt((unsigned char*)plainText.c_str(), plainText.length(), (unsigned char*)publicKey.c_str(), decrypted);
	if (decrypted_length == -1) {
		printLastError((char *)"Public Decrypt failed");
		return -1;
	}
	
	string decryptedStr((char *)decrypted,decrypted_length);
	
	plain = decryptedStr;
	FileSink out1("/data/license/license-dec");
	out1.Put((byte const*) decryptedStr.data(), decryptedStr.size());

	return 0;
}


int decryptLicense(string con,string &plain)  
{  
       
	//ifstream inputFile("/data/license/license-enc.bin");
	//string cleartext((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());
    //if (cleartext.length() > 256) {  
	//		cout<<"cleartext too length!!!"<<endl;  
      //     	return -1;  
	//}  
	//return RSA_test1(cleartext);
	return RSA_test1(con,plain);
} 
