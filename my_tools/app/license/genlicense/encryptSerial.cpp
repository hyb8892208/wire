#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <cstdlib>

#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/ripemd.h>

#include <jsoncpp/json/json.h>


using namespace CryptoPP;
using namespace std;

#include "bstrwrap.h"

extern CBString keypath;
extern CBString debugpath;
extern CBString respath;


string enc0 = "bjRp5hb+JiCH/yozAANl+HtJe51XoaInCXQ4iZgk1tP13IOj2DY3zeMCRQyvlf6iBrjgGLie\n";
string enc1 = "eInyhL6WeVQZdvjwvTgxIl7cuA/C4wG9MtXnfuyLx/LTVUXHilWhzTB2FKW5zhjo/6/E2ibf\n";
string hash0 = "6L+uuxR2FxSImNBOw38+EQ==";
string enc2 = "tSVk1hkbYTXi6hsSi6bemLe+xXb2r+BBlgE5+rJ4FXwSp00/OLIM0ICJkLF1vVikRrMqlyBN\n";
string enc3 = "otyxPeYu3mDh2y2NspYmO5wVW6wBcnoLv01eBh7i42PNX0p0oLqZXPIodezpnrXlaSWs5Tg4\n";
string enc4 = "3Aaa61b1rBE=";
string iv0 = "45mn5KsSG5KLL3CwVSK4tA==";

int encryptSerial(string serial1, string & serial2) {
	//Read encrypt plain
	string encPlain;
	string encPlain64 = enc0 + enc1 + enc2 + enc3 + enc4;
    StringSink encSink(encPlain);
    StringSource encSS(encPlain64,true,new Base64Decoder);
    encSS.TransferTo(encSink);

	//StringSink encPlainSink(encPlain);
	//FileSource file("/data/license/encrypt-enc", true, new Base64Decoder);
	//file.CopyTo(encPlainSink);

	//Read initialization vector
	byte iv[AES::BLOCKSIZE];
	CryptoPP::ByteQueue bytesIv;
	//FileSource file2("/data/license/encrypt-iv", true, new Base64Decoder);
	//file2.TransferTo(bytesIv);
	string ivs = iv0;
	StringSource ivss(ivs,true,new Base64Decoder);
    ivss.TransferTo(bytesIv);	
	bytesIv.MessageEnd();
	bytesIv.Get(iv, AES::BLOCKSIZE);

	// Read Hashed Key
	string hashedPass;
	StringSink hashedPassSink(hashedPass);
	//FileSource file3("/data/license/encrypt-hash", true, new Base64Decoder);
	//file3.CopyTo(hashedPassSink);
	StringSource hashSS(hash0,true,new Base64Decoder);
	hashSS.CopyTo(hashedPassSink);

	//Decrypt encrypt plain
	byte plain[encPlain.length()];
	CFB_Mode<AES>::Decryption cfbDecryption((const unsigned char*)hashedPass.c_str(), hashedPass.length(), iv);
	cfbDecryption.ProcessData(plain, (byte *)encPlain.c_str(), encPlain.length());

#ifdef SAVE_FILE
	//Save encrypt plain to file (Base64)
	string decryptPlainStr((char *)plain, encPlain.length());
	StringSource decryptPlainSrc(decryptPlainStr, true);
	Base64Encoder decryptPlainSink(new FileSink("/data/license/encrypt"));
	decryptPlainSrc.CopyTo(decryptPlainSink);
	decryptPlainSink.MessageEnd();
#endif

	//Hash the serial phrase to create 128 bit key
	string hashedSerial;
	RIPEMD128 hash128;
	StringSource(serial1, true, new HashFilter(hash128, new StringSink(hashedSerial)));

	//Encrypt plain with hased serial 
	CFB_Mode<AES>::Encryption cfbEncryption((const unsigned char*)hashedSerial.c_str(), hashedSerial.length(), iv);
	byte encPlain2[encPlain.length()];
	cfbEncryption.ProcessData(encPlain2, (const byte*)plain, encPlain.length());
	string encPlainStr2((char *)encPlain2, encPlain.length());
	serial2 = encPlainStr2;

#ifdef SAVE_FILE
	//Save encrypt plain to file (Base64)
	StringSource encPlainSrc2(encPlainStr2, true);
	Base64Encoder encPlainsink2(new FileSink("/data/license/serial-enc-2"));
	encPlainSrc2.CopyTo(encPlainsink2);
	encPlainsink2.MessageEnd();
#endif
	
//	//Save encrypt plain to file (Binary)
	FileSink out(debugpath+"./serial-enc.bin");
	out.Put((byte const*)encPlain2, encPlain.length());

#ifdef SAVE_FILE
	//Save serial to file
	FileSink out1("/data/license/serial-2");
	out1.Put((byte const*)serial.data(), serial.size());
#endif
	return 0;
}
