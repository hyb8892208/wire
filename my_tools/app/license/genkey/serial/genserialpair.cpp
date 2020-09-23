//g++ genserialpair.cpp -lcrypto++ -o genserialpair -lpthread

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

#define  PLAIN_LENGTH 224 //should less than 256-11

void GenSerialPair()
{
	cout << "Enter new serial key password" << endl;
	string pass;
	cin >> pass;

	AutoSeededRandomPool rng;
	
	// Generate random plain
	byte plain[PLAIN_LENGTH];
	rng.GenerateBlock(plain,PLAIN_LENGTH);

	//Hash the pass phrase to create 128 bit key
	string hashedPass;
	RIPEMD128 hash;
	StringSource(pass, true, new HashFilter(hash, new StringSink(hashedPass)));
	
	// Generate a random IV
	byte iv[AES::BLOCKSIZE];
	rng.GenerateBlock(iv, AES::BLOCKSIZE);

	//Encrypt plain
	CFB_Mode<AES>::Encryption cfbEncryption((const unsigned char*)hashedPass.c_str(), hashedPass.length(), iv);
	byte encPlain[PLAIN_LENGTH];
	cfbEncryption.ProcessData(encPlain, (const byte*)plain,PLAIN_LENGTH);
	string encPlainStr((char *)encPlain, PLAIN_LENGTH);

	//Save encrypt plain to file (Base64)
	StringSource encPlainSrc(encPlainStr, true);
	Base64Encoder sink(new FileSink("encrypt-enc"));
	encPlainSrc.CopyTo(sink);
	sink.MessageEnd();
	
        //Save encrypt plain to file (Binary)
       	FileSink out("encrypt-enc.bin");
      	out.Put((byte const*) encPlain, PLAIN_LENGTH);
        

	//Save initialization vector to file
	StringSource ivStr(iv, AES::BLOCKSIZE, true);
	Base64Encoder sink3(new FileSink("encrypt-iv"));
	ivStr.CopyTo(sink3);
	sink3.MessageEnd();

	//Save plain to file (Base64)
        StringSource plainStr(plain,PLAIN_LENGTH, true);
        Base64Encoder sink4(new FileSink("encrypt"));
        plainStr.CopyTo(sink4);
        sink4.MessageEnd();	

        //Save plain to file (Binary)
        FileSink out1("encrypt.bin");
        out1.Put((byte const*) plain, PLAIN_LENGTH);

	//Save hash to file (Base64)
	StringSource hashSrc(hashedPass, true);
        Base64Encoder sink5(new FileSink("encrypt-hash"));
        hashSrc.CopyTo(sink5);
        sink5.MessageEnd();
	
}

int main()
{
	GenSerialPair();
}

