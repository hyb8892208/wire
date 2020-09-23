//g++ gensecondarypair.cpp -lcrypto++ -o gensecondarypair

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

string GenKeyPair(AutoSeededRandomPool &rng, string pass)
{
	// InvertibleRSAFunction is used directly only because the private key
	// won't actually be used to perform any cryptographic operation;
	// otherwise, an appropriate typedef'ed type from rsa.h would have been used.
	InvertibleRSAFunction privkey;
	privkey.Initialize(rng, 1024*4);

	// With the current version of Crypto++, MessageEnd() needs to be called
	// explicitly because Base64Encoder doesn't flush its buffer on destruction.
	string privKeyStr;
	StringSink privKeyStrSink(privKeyStr);
	//Base64Encoder privkeysink(new FileSink("secondary-privkey.txt"));
	privkey.DEREncode(privKeyStrSink);
	privKeyStrSink.MessageEnd();
	 
	//Hash the pass phrase to create 128 bit key
	string hashedPass;
	RIPEMD128 hash;
	StringSource(pass, true, new HashFilter(hash, new StringSink(hashedPass)));

	// Generate a random IV
	byte iv[AES::BLOCKSIZE];
	rng.GenerateBlock(iv, AES::BLOCKSIZE);

	//Encrypt private key
	CFB_Mode<AES>::Encryption cfbEncryption((const unsigned char*)hashedPass.c_str(), hashedPass.length(), iv);
	byte encPrivKey[privKeyStr.length()];
	cfbEncryption.ProcessData(encPrivKey, (const byte*)privKeyStr.c_str(), privKeyStr.length());
	string encPrivKeyStr((char *)encPrivKey, privKeyStr.length());

	//Save private key to file
	StringSource encPrivKeySrc(encPrivKeyStr, true);
	Base64Encoder sink(new FileSink("secondary-privkey-enc"));
	encPrivKeySrc.CopyTo(sink);
	sink.MessageEnd();

	//Save hashed key to file
	StringSource encHashKeySrc(hashedPass, true);
	Base64Encoder sink1(new FileSink("secondary-privkey-hash"));
	encHashKeySrc.CopyTo(sink1);
	sink1.MessageEnd();


	//Save initialization vector key to file
	StringSource ivStr(iv, AES::BLOCKSIZE, true);
	Base64Encoder sink2(new FileSink("secondary-privkey-iv"));
	ivStr.CopyTo(sink2);
	sink2.MessageEnd();

	// Suppose we want to store the public key separately,
	// possibly because we will be sending the public key to a third party.
	RSAFunction pubkey(privkey);
	
	Base64Encoder pubkeysink(new FileSink("secondary-pubkey"));
	pubkey.DEREncode(pubkeysink);
	pubkeysink.MessageEnd();

	string pubkeyStr;
	Base64Encoder pubkeysink2(new StringSink(pubkeyStr));
	pubkey.DEREncode(pubkeysink2);
	pubkeysink2.MessageEnd();

	return pubkeyStr;
}

void SignSecondaryKey(AutoSeededRandomPool &rng, string strContents)
{
	//Read private key
	string encMasterPrivKey;
	StringSink encMasterPrivKeySink(encMasterPrivKey);
	FileSource file("master-privkey-enc", true, new Base64Decoder);
	file.CopyTo(encMasterPrivKeySink);

	//Read initialization vector
	byte iv[AES::BLOCKSIZE];
	CryptoPP::ByteQueue bytesIv;
	FileSource file2("master-privkey-iv", true, new Base64Decoder);
	file2.TransferTo(bytesIv);
	bytesIv.MessageEnd();
	bytesIv.Get(iv, AES::BLOCKSIZE);

	//Read private key
	string hashedPass;
	StringSink hashedPassSink(hashedPass);
	FileSource file1("master-privkey-hash", true, new Base64Decoder);
	file1.CopyTo(hashedPassSink);

	//Decrypt master key
	byte test[encMasterPrivKey.length()];
	CFB_Mode<AES>::Decryption cfbDecryption((const unsigned char*)hashedPass.c_str(), hashedPass.length(), iv);
	cfbDecryption.ProcessData(test, (byte *)encMasterPrivKey.c_str(), encMasterPrivKey.length());
	StringSource masterKey(test, encMasterPrivKey.length(), true, NULL);

	RSA::PrivateKey privateKey;
	privateKey.Load(masterKey);

	//Sign message
	RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);
    size_t length = privkey.MaxSignatureLength();

	SecByteBlock sbbSignature(privkey.SignatureLength());
	privkey.SignMessage(
		rng,
		(byte const*) strContents.data(),
		strContents.size(),
		sbbSignature);

	//Save result
	Base64Encoder enc(new FileSink("secondary-pubkey-sig"));
	enc.Put(sbbSignature, sbbSignature.size());
	enc.MessageEnd();
}

int main()
{

	cout << "Enter new secondary key password" << endl;
	string pass2;
	cin >> pass2;

	AutoSeededRandomPool rng;
	string pubkey = GenKeyPair(rng, pass2);
	SignSecondaryKey(rng, pubkey);
}


