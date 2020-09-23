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
#include "bstrwrap.h"

extern CBString keypath;
extern CBString debugpath;
extern CBString respath;

RSA* createRSA(unsigned char* key, int flag)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf(key, -1);

	if (keybio == NULL) {
		cout << "Private Encrypt failed" << endl;
		exit(-1);

	}

	if (flag)
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	else
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

	if (rsa == NULL){
		cout << "Private Encrypt failed" << endl;
		exit(-1);
	}

	return rsa;
}


int private_encrypt(unsigned char* data, int data_len, unsigned char* key, unsigned char* encrypted)
{
	RSA * rsa = createRSA(key, 0);
	int result = RSA_private_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);
	return result;
}

int public_decrypt(unsigned char* enc_data, int data_len, unsigned char* key, unsigned char* decrypted)
{
	RSA * rsa = createRSA(key, 1);
	int  result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_PADDING);
	return result;
}

string pri14 = "RNNxjlv7zmDLLHAtUA+C5RUwUJSpbys8HCLjbhlb/+wGvzfvnStUXHsCgYEA9Ogj\n";
string pri6  = "8i1k15ktJsuSNscfq4fxxrFTfKGXF4Z4iISalwIDAQABAoIBAB813ydqMC8mMPEt\n";
string pri26 = "-----END RSA PRIVATE KEY-----";
string pri21 = "yo+ItYtG5Acfu4/BnMYvMrUDu5YgvrBhWT+mjLu+hXlEv9KeRJwb8ZOe+vwrMQUi\n";
string pri2  = "k/iyTEvMbDY+ZUQybfpZlr34cysBsnkGI5Uk9G/msMpSacVGlZyiifLqcJzy70O0\n";
string pri0  = "-----BEGIN RSA PRIVATE KEY-----\n"; 
string pri18 = "6R3HNmmLpY+57fvy6FvBStD3e57CLp9oTxdhrLKydATsyw3ClvRxbKKe1OfhKhf/\n"; 
string pri22 = "K/Ci1yoDjyZhKwdnoNsMmVtYGf3xDK6rHi09kPNWjRqKBTOdZQyuNmEWYSKB8gwo\n";
string pri15 = "g9mrJ3Tj0nyovvBcI2UTaQu0pZbcdxFtq81422/bPelY75IwhG3ZWLg7YX7Tumrz\n"; 
string pri10 = "vlYzLoeaVOn8/M47IBx/4l184xrb9U9gvAc/BagNYOhNyq3nmcOzxMGBSqXX91N+\n";
string pri1  = "MIIEowIBAAKCAQEA7pAaq+arLy+3INksKr5QDcTmnybr5UvgM8MlMIpWqsDoYdAn\n"; 
string pri8  = "+mShWIpaMXYjohCQbKGaMnGVxDIjdmzzEKLCKqxswjOaYJRh3+2lOqhOIlJYOhLK\n";
string pri12 = "xFOSmvECgYEA+V5nXk1c5dLNwrVNqZX48uhn3SA3hy+m42vJFUsk+PmqkOCTz1Ua\n";
string pri24 = "STbr1BMGZUqNKNEaWFZLemI/v4IgJG3/ev7n1Wes94yfb/eOkMDwQj13hxDWMs9b\n";
string pri19 = "2ExR6sh/xdNz5GPoe6OzETFiUWrojqKKMA19400xV6iWMsiwYHKYoSAy2ehEcDMf\n";  
string pri16 = "+PSgIJYOSPS3Z295Ihx7Vn+F4ix7J8qUHTlj64WNBAW3iaFwKXBBBHyDKxbpcKuy\n"; 
string pri4  = "GGxAIXiMelCxp7ZSeOhWsr8DwIKYc8efWvIrcOS2BDQ/r7Jg3k73pAbYHtzvAKL3\n";
string pri11 = "DYvQOybTxmgDZ56olxgRjY+67Qhhoc9OK4UBaHkLgU/KXssEm6XMSoIs4o4cVu39\n";
string pri25 = "OGpqhuA+SxQXxxuHBRk3AdEPr8iiWOxUK5AUiDuj3wL3wgJsWW8U\n";
string pri13 = "uOt5LYpNRFoyZ2wezP03OHf3Vw699J++8LzXs/VudSN0ADe6ACRWnKOlnEFFZ5Us\n";
string pri9  = "Wq1XeLo7ItBZpmnbpPGW8jNPxG5c0Zwggfvwve5b53Ihzbt+Ox77T9QztUhTqkmd\n";
string pri5  = "3b+bq2crQqiBX12QpgPP6f+DLWCXPh8XCETr24khfsGxjv8DdAbSLq4Nki3mxBdc\n";
string pri23 = "JHPNAoGBAJ3TeHd3SZJVLcvTS1HWs27fsb5FpGm7jWxiLz46c71FtkJe9kQgjHya\n";
string pri17 = "T6v/qzf9bERks8D58fceNIpMaDJY4ukuw3AXJZUCgYBqsUF4z5DXpAH1NK0tycTh\n";
string pri20 = "tq2oBaGfNdUNK1mR3WAS0QKBgE/Yy5N2STYRByIeIIo4JWIo1x49gcUnHXP4i7Ai\n";
string pri7  = "OGgHKfYDMqDTuLzw5k4fU7g8AQap4j0ZvHPRb5InwIDPv+4DtA1LWXktNVnr4Hb2\n";
string pri3  = "dGjImgx/zUJZaqpRUW06gdhO1rF8mMPKfGRDphCDPTjfM5iJgHHPsqrl2TrWt1k0\n";



int  encryptLicense(string plaintLicense, string & encLicense)
{
	//ifstream inputFile(keypath+"./private.pem");
    //string privateKey((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());
	
	string privateKey = pri0 + pri1 + pri2 + pri3 + pri4 + pri5 + pri6 + pri7 + pri8 +
						pri9 + pri10 + pri11 + pri12 + pri13 + pri14 + pri15 + pri16 + pri17 +
						pri18 + pri19 + pri20 + pri21 + pri22 + pri23 + pri24 + pri25 + pri26;

	unsigned char  encrypted[1024] = {};

	int encrypted_length = private_encrypt((unsigned char*)plaintLicense.c_str(), plaintLicense.length(), (unsigned char*)privateKey.c_str(), encrypted);
	if (encrypted_length == -1) {
		cout << "Private Encrypt failed" << endl;
		exit(-1);
	}
	string encryptedStr((char *)encrypted,encrypted_length);
	encLicense = encryptedStr;

#ifdef SAVE_FILE

	//Save encrypt function to file
	StringSource encryptedSrc(encryptedStr, true);
	Base64Encoder sink(new FileSink(debugpath+"./license-enc"));
	encryptedSrc.CopyTo(sink);
	sink.MessageEnd();
#endif	
	FileSink out(debugpath+"./license-enc.bin");
	out.Put((byte const*) encryptedStr.data(), encryptedStr.size());	
	return 0;

	return 0;
}


