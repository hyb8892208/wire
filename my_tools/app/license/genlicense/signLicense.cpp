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
#include "bstrwrap.h"

extern CBString keypath;
extern CBString debugpath;
extern CBString respath;

string iv64 = "lPe1o1wNbQwtyWU6J6OYQA==";
string hash64 = "7dUJn1gtL9ps1Vq/3n1eBQ==";


string spri31 = "qM1U2jZCT5ADH3MjgzGdSh/Y6jEDiPu2hkmkHzH8P5QQSlMYkWQuswCGxVU3M1S89gNyEn+u\n";
string spri19 = "DNK5NLniXsco2oZ4AvNUNTSEDSJLlO3MlwjA9x+NjtJgsHtA1bSULRkhWJ/7yvtQNDZfIm60\n";
string spri8  = "plvAhciHpK7SR5q8MUehGpr2atMk4j000zwHdcTPd1eFPq2YN3fKR/pMkRMXZpy04wr2Xn/P\n";
string spri0  = "zA91q6PLeYwnEHsUyjt2PrRwMBlW/3qjgGd3DCSzB6mx7v6ZTBWc6WwkbfJefCUPMql8YqZt\n";
string spri1  = "4b+v3lvcJj6Wg+QIMFFsGOB1X4MkZhBUiOYARvvPgW2muQe/hSAzuuW1YI49XA4oUh//kcRz\n";
string spri17 = "nHTSlJecm/qlh1ed83PPYsqVz7gV6OC0+Jgkm4swtzNnts8EEdjUO+CzuQfYYF5cxSnwka4G\n";
string spri18 = "O1kH0KpGDuDp7MwLjIhGKsb9r3yDAk7B/WgQn4acL7Hcna9uK1ZkkBrGGCKwoie13Me5nHU5\n";
string spri25 = "NcFzfwoLQNIAgQHxUb2ALbu6Rw83H7qydut+EILXUhGyOqLhL0bfj28GuGDReQ7MHO++EFOH\n";
string spri26 = "6mZarWK7EgR6mrfmf3mo7yw7N1NMR2ZWTYaw/Y34WVe4EIjAocBbwAA2TyoA1oy5HB0tlFsR\n";
string spri42 = "gYjSbZq/YK8JxsRRYlFGC9hwS4yru2FsqoKG7OVAtrDDC+hu6/MpNU0neuZQYcdtGdsleOJX\n";
string spri43 = "Ue5B3YiPdUi9RGWXCs38bYAjPFji75mkViAjMwRw8rFUHyY9tbyTMj7y+VopHJJF26U=\n";
string spri32 = "1zyGPzGFh9WqzAb/Ka1oSIgbqSKlWRRQ/k7ChiSHnyVdNHWwXkhMvx+NbG7PZpcEAxesQyyc\n";
string spri33 = "Xtn6da8aQIir2x98nfqKsDEoqTuOkb+7eYT7VE+jlZEmolj/wmQOY5w2ouoSkIiF9l8VMcYI\n";
string spri15 = "v4JOiCCi+2vYH1Q13cpy5sd41OsJC8X5WFI4D0ulNrd8keqip/wQLlaFK4pV33D3xfiU2oIN\n";
string spri16 = "U8HCTuqzFYN20xLQCuWExdlOPw/yWypY2BL2608ayMcj1ydS1r5n8p55kQJLmSOucP0itjBc\n";
string spri5  = "1+8BQEAb+/ZOHWmIkMQKBbO1vNiPYYW0A8jtqG1Xk5UN5Oy1M1HaR2Mrl31oaCHSpvx+v0kj\n";
string spri6  = "bbFg+MJhaPcQRcTGLmFYXYWHoOY19UNrNklJr54c+Id9fND1I/wnX0HwEzveN1KUmJl7b8Go\n";
string spri23 = "yDnX1a5IYvRjqadMkhsSNhxGzJ080cM9tkPgM3S5fiQ5rYgxMAgCfXkZOJZiw389kEd84adb\n";
string spri24 = "cOH3yEPrWJrlbMnvZvm4odpiadkRrcioyeSbBBtKsApqV6sXUeSo8zKJw6HeXTDBK40L3iA5\n";
string spri37 = "ln9RtEsnyCLV6HOBl93/hyeg03U3SamXQpS7cBmsTSS2H1mM+PIDi1XgjUkC5B4osKsSApbN\n";
string spri38 = "xTFhksj8WBhtoEqbPOKAHL6UbWTA29Ltki9IkeXlId2o4+RUPXlehwSqkACjKDrSEAFw5mOC\n";
string spri2  = "t6gM4EQk9Q1SstNnBegfTNqEgCM/9S+tRmALu17kBianLHyH3lxhGscTlG2qlTHOaIjTEWuF\n";
string spri3  = "pJ0JRimH54Fj7Qs+Ke03w5rvxhrGC9XRzdoIw5VHQyBpHfMYuMuqqpueFlUGlNeQ+qUhpMwW\n";
string spri4  = "DEuoOMjWfhukxINIsJIU0Xe3tEevO1DqddeA3dYMgD4pbB0Ff5SkYc0JkRXK18pL1SVK0ODX\n";
string spri34 = "tGNBnVBKCOgKh+3JkRr2JKKCErHxcLBjWSDzpMD1iHHilaNOOA85Z0vSmdTJlpK716WsnMTA\n";
string spri39 = "1mo/HfNdGtYqxZ0MURO3EfPOzkBHTuaMb+wUIlNmApPVjsCs1KGuhF2dw/TTomV/lTCmieBM\n";
string spri40 = "y1OUMf0qTkcMXzbuGITDfXTztG+DTY/ogGzOoQed+X5mLpz9C0SGxlXEJex3LJrKmgSNJN2h\n";
string spri41 = "9JeflUqK/6h5ad8vpvCAl99TBZeOCaJoTwXA9gG5Ry5ZBEL51V1yDbuXH113MpsJeoO3OPBa\n";
string spri29 = "WULZRDyoa7nXAP7qQDBqod/D3D42CMJQ0sv43qxRY/if2O4T/t3rumTaiyDcI3WgD15hOwYX\n";
string spri30 = "RnDb5+WsY4TjRk5Hpnnd+T6fENuMwt/aadQqdUi9AsFrWrVSp5L3+LPjpWcfJjXasEePRY07\n";
string spri7  = "mjV5hq5RNFjiMmaSRo4Fu5ww51wwjrP9zbI5NSCvvio8zWj7skAB9Ej3tbI//fxuWAR0oyOm\n";
string spri13 = "tg2yHMc9LDa08tzyUwOrWCITaQZlEOLPNpr0EliZswlRUV98ODzjyc7dAdIKzblBdi4XuYbs\n";
string spri14 = "ENCCeQRRSonnN/WyxUma2vpVsBFXAkl2kR24TIo1vKjirOn560gv1q9nGxUxY7gzxNR7Ce/6\n";
string spri20 = "HQ1ODR3dGP5VWjDdJnCehiG53nIFT3vroUiPS3CWN/L72vo+JnmLJ/kkmI61aIQ1wfCPjwNe\n";
string spri21 = "XxQxDsKjBRihHpYAc1MqSuPxYcsOoOxLDYD8LGXlxB+EX8P05i706DY3F9nks89Q+LDkWuej\n";
string spri22 = "4QBXaM1O6rlH0z5bssjZwhZI6QMc+VWgqQalHL/YPVmaKsJFAFqjAieztnStJBGbXF2R3PK4\n";
string spri10 = "WtmY80rC0l3PXknrlkM/yfsPKHPxb1HPKmQkfDsFWXAhts/+hkSquPa4+Se0MjWBnlpkcq5N\n";
string spri11 = "VyBHYDRf4RdZTJJQhmVm9csP4lvcbpZWvu4SHFS1KPqI3+nvw1Z4mYHhiOwvMePg4h+DozIE\n";
string spri28 = "sLB479IGnnEB6ieQQL4DMkiYSn4iG1lyK2A5o5qsGEuQGWllBWUd1C8sEyBIYsl5xBeOsovI\n";
string spri9  = "p2P8ktzsGhbW3f/g/hN4aQUkz9PBZhc9OJiTfhRdGIt3vemzWDhmmGQWKf98KaaPNhWTHFrG\n";
string spri36 = "bsmF6DlRrjkwZ3bRFaD9BdxCerOBR1dCxqDQwCDs6DKNDBKMkX6Bwxsu9y7Gsl4TdRuW4zmS\n";
string spri27 = "aCw4SADo5OUqMvQ2d7ZVfnppSQqr8tySHeqIoogOBkKfAFhHkXrAgUsoMufg03qswXCzdhTk\n";
string spri12 = "jyA4ERosbsAfAqghjxe5AtE6bivpGw7SGfgl//SdFOt5qqjECkkT4z61XXqVtkVlVKlOK3Hn\n";
string spri35 = "D7llqfqK/cEDxDqH5Cbt6WoITNtJNGvME3T10QLjVoKGKDtqKzqLkDl3dKOcqxBMOmNLadsZ\n";




int signLicense(string encryption)
{
	AutoSeededRandomPool rng;
	//ifstream inputFile(debugpath+"./license.bin");
	//string license((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());
	string license = encryption;
	
	//Read private key
	string encPrivKey;
//	StringSink encPrivKeySink(encPrivKey);
//	FileSource file(keypath+"./secondary-privkey-enc", true, new Base64Decoder);
//	file.CopyTo(encPrivKeySink);

    string encPrivKey64 = spri0 + spri1 + spri2 + spri3 + spri4 + spri5 + spri6 + spri7 + spri8 + spri9 +
						  spri10 + spri11 + spri12 + spri13 + spri14 + spri15 + spri16 + spri17 + spri18 + spri19 +
						  spri20 + spri21 + spri22 + spri23 + spri24 + spri25 + spri26 + spri27 + spri28 + spri29 +
						  spri30 + spri31 + spri32 + spri33 + spri34 + spri35 + spri36 + spri37 + spri38 + spri39 +
						  spri40 + spri41 + spri42 + spri43;
    StringSink encPrivKeySink(encPrivKey);
    StringSource encSS(encPrivKey64,true,new Base64Decoder);
    encSS.TransferTo(encPrivKeySink);

	//Read initialization vector
	byte iv[AES::BLOCKSIZE];
	CryptoPP::ByteQueue bytesIv;
	//FileSource file2(keypath+"./secondary-privkey-iv", true, new Base64Decoder);
	//file2.TransferTo(bytesIv);
	string ivs = iv64;
    StringSource ivss(ivs,true,new Base64Decoder);
    ivss.TransferTo(bytesIv);
	bytesIv.MessageEnd();
	bytesIv.Get(iv, AES::BLOCKSIZE);

	//Read private key
	string hashedPass;
	StringSink hashedPassSink(hashedPass);
	//FileSource file1(keypath+"./secondary-privkey-hash", true, new Base64Decoder);
	//file1.CopyTo(hashedPassSink);
    StringSource hashSS(hash64,true,new Base64Decoder);
    hashSS.CopyTo(hashedPassSink);

	//Decrypt private key
	byte test[encPrivKey.length()];
	CFB_Mode<AES>::Decryption cfbDecryption((const unsigned char*)hashedPass.c_str(), hashedPass.length(), iv);
	cfbDecryption.ProcessData(test, (byte *)encPrivKey.c_str(), encPrivKey.length());
	StringSource privateKeySrc(test, encPrivKey.length(), true, NULL);

	//Decode key
	RSA::PrivateKey privateKey;
	privateKey.Load(privateKeySrc);
	
	//cout << test << endl;
	//Sign message
	RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);


	SecByteBlock sbbSignature(privkey.SignatureLength());
	privkey.SignMessage(
		rng,
		(byte const*) license.data(),
		license.size(),
		sbbSignature);

	//Save signed to file
	Base64Encoder enc(new FileSink(debugpath+"./license.sig"));
	enc.Put(sbbSignature, sbbSignature.size());
	enc.MessageEnd();

	// Save license bin to base64
	StringSource licenseSrc(license, true);
	Base64Encoder sink(new FileSink(debugpath+"./license.bas"));
	licenseSrc.CopyTo(sink);
	sink.MessageEnd();	
	
	
	CBString cmd;
	cmd.format("tar -zcvf %s./license.crt  -C %s license.bas license.sig >/dev/null 2>&1", (const char *)respath, (const char *)debugpath);
	system(cmd);
	return 0;
}

