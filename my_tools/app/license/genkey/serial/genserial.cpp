//g++ genserial.cpp -lcrypto++ -o genserial -lpthread

#include <string>
using namespace std;
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/ripemd.h>
using namespace CryptoPP;

void GenSerial(string serial)
{

	//Read encrypt plain
	string encPlain;
	StringSink encPlainSink(encPlain);
	FileSource file("encrypt-enc", true, new Base64Decoder);
	file.CopyTo(encPlainSink);

	//Read initialization vector
	byte iv[AES::BLOCKSIZE];
	CryptoPP::ByteQueue bytesIv;
	FileSource file2("encrypt-iv", true, new Base64Decoder);
	file2.TransferTo(bytesIv);
	bytesIv.MessageEnd();
	bytesIv.Get(iv, AES::BLOCKSIZE);

	//Hash the pass phrase to create 128 bit key
	//string hashedPass;
	//RIPEMD128 hash;
	//StringSource(pass, true, new HashFilter(hash, new StringSink(hashedPass)));
	string hashedPass;
        StringSink hashedPassSink(hashedPass);
        FileSource file3("encrypt-hash", true, new Base64Decoder);
        file3.CopyTo(hashedPassSink);

	//Decrypt encrypt plain
	byte plain[encPlain.length()];
	CFB_Mode<AES>::Decryption cfbDecryption((const unsigned char*)hashedPass.c_str(), hashedPass.length(), iv);
	cfbDecryption.ProcessData(plain, (byte *)encPlain.c_str(), encPlain.length());

        //Save encrypt plain to file (Base64)
        string decryptPlainStr((char *)plain, encPlain.length());
        
        StringSource decryptPlainSrc(decryptPlainStr, true);
        Base64Encoder decryptPlainSink(new FileSink("encrypt-2"));
        decryptPlainSrc.CopyTo(decryptPlainSink);
        decryptPlainSink.MessageEnd();

	//Hash the serial phrase to create 128 bit key
	string hashedSerial;
	RIPEMD128 hash128;
	StringSource(serial, true, new HashFilter(hash128, new StringSink(hashedSerial)));
	
	//Encrypt plain with hased serial 
	CFB_Mode<AES>::Encryption cfbEncryption((const unsigned char*)hashedSerial.c_str(), hashedSerial.length(), iv);
	byte encPlain2[encPlain.length()];
	cfbEncryption.ProcessData(encPlain2, (const byte*)plain, encPlain.length());
	string encPlainStr2((char *)encPlain2, encPlain.length());

	//Save encrypt plain to file (Base64)
	StringSource encPlainSrc2(encPlainStr2, true);
	Base64Encoder encPlainsink2(new FileSink("serial-enc"));
	encPlainSrc2.CopyTo(encPlainsink2);
	encPlainsink2.MessageEnd();

	//Save encrypt plain to file (Binary)
	FileSink out("serial-enc.bin");
	out.Put((byte const*) encPlain2, encPlain.length());

	//Save serial to file
	FileSink out1("serial");
	out1.Put((byte const*) serial.data(), serial.size());
	

}

int main()
{

	cout << "Enter serial number" << endl;
	string serial;
	cin >> serial;

	GenSerial(serial);
}



