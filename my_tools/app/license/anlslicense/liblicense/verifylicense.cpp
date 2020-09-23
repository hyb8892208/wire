//g++ verifylicense.cpp -lcrypto++ -o verifylicense
#include <string>
using namespace std;
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>

using namespace CryptoPP;
string sigStr16 = "L2SfQQeWq069d1PFXEbVWv3KIp3O+ivqDHDYh9fRks+tNaOx12SVxVC+kLoZe0mVt9bsb3N0\n";
string mp0  = "MIIEIDANBgkqhkiG9w0BAQEFAAOCBA0AMIIECAKCBAEAklt5wqoBf10FdO6PaFk9ATFmrsrS\n";
string sigTxt10 = "LyRIKmUCARE=\n";
string sigStr0 ="euu0A03TbpTC1o5YQ/i0Ra548R+b0qyUUX5PYWJyIIDrM0NE2BOcb4dH7a5lmKSeJZK7EwaI\n"; 
string sigStr8 = "Z5tcPfCTtkKgy+PwUa1rcsTIDZ0aZzXlBy7L+IKdeGZWmwqiJndPPTFRXgonNQouVSSE5koj\n";
string mp1  = "LNh/OnsxIs+TIXC1cL6DYdFDV9DGDS8yZri039h06PZzDqu/SFBY8K/bgX0YHIoC8nPwJOUd\n";
string sigStr1 = "DhFSPVcWIrqjqMxCgFgCXx1cpfP6XtRNQbJgHFPV6jLvDnP0bj1xMaAelj5uUJEA41ZkNFBi\n";
string mp19 = "X3Gq1dAaEFVCLjlcwNwhh4btnS9BtDAX0f9TK5Fb+QIBEQ==";
string sigTxt2 = "YraTIOPLsgAuWlAl4+Xe0FTuJKgdnloHwmdUeep4RMH+Grj1SOOeCFC6RHV8mYbs4AKtsNe1\n";
string sigStr5 = "c9620eExgaoMgAYi8ax/lVfI5OjZNpYsxKiyab03FrPhvwezIqor+U6Zi0vmVdlge75YxHZh\n";
string mp3  = "NbO9vJZIAyrA2GW2R+ZKtdbQbnpMS1XG5z/VoBTzfG2Sh+fdQhS4Mh/QdoPTB2NMHzb/k0Vi\n";
string sigStr9 = "jxm4Pyv/rXZ2L9ezYLMT0Uys+X3C/kY2joQSc9nK4nVxzW5M3fDA1YERDU1WlaDAHcYHW0H8\n";
string mp6  = "z1ZjAK/wrzqLmUO9Ykve+NUpyIekpkHT5r6vAp/MnF+98FQC6+TDjGBiJRhH3ae2PQueu0L+\n";
string sigStr17 = "R5XrK4j/WolSDmn0+aCixNMTkheE7m2reYKIdohwX+dPtyYjRO10XMVbv+anDVPEtDWlBDf7\n";
string sigTxt3 = "TcgUzLMpm9ivu6C9+EuN0Dv7rlTPH58PvO1i0T8uyb5/ix83LNW2qz0zvWXjbnV36iqj4AfC\n";
string mp2  = "af6qoMUhl/xQgdGUCX3Tfyb7hatKK9amm9rVUh2w7c7p4e6SGgVkU0x8JHYPHHp3xJwHm8vU\n";
string sigStr4 = "ysJQ0CEytEC+H0rIgkoumz4Knz+XzfcN1DNrVzAV0DQejAqHLnbZqsT21F2fOe3tf8tzcX+f\n";
string sigStr12 = "MFXVHIdTDq0YK1M+CmdIi9deToVEau2Rz5NiEaswxxDUXZcP70VNIUubHP4w6Jf/G555OOEK\n";
string sigTxt8 = "84lM4LzUA4xghGg+0Q+zymqIySXoTGRVhE7piEnLGC6pKZQ0eFz2gQ1gzIMIpVMyA8NcfQzJ\n";
string mp15 = "j0AU4wWfxMk0XTHcIjoLNSPvNAIUT4Rw9ktBNVG9KEFifDrfPjxwUg2aY24fA41TzkNx+l87\n";
string mp8  = "TF2MlfVbu9cW4R0sa9Ip7k3aO/iHnDq2vXLFZtRKkY7aXz1oMli7PZhKnFq52wkoK97JDAFq\n";
string sigTxt5 = "T4nSkuHDN/O0hlxnbme0Akhxlopn0V3izkp+7b3Px53bw+UhQALPNeIbIrTqpdja+pyJdmJP\n";
string sigStr18 = "XLB4yaq/REiIwF5SAdLQxdKZ6yPGJ1dU1JGwk8llMrIRJj+3v2M1PjxoHQlBcerMpgXDDA==";
string mp7  = "/ZF8xJw48PCsQOIyMjkBZ6slXW52lbdSMRo72gLQ5UzkTaWTpAXsYHhZwvlJyBpnb8og9tPp\n";
string sigStr13 = "VKQkt7DvSZVn37Xtc4XqEUsDyZoo7fwtoERnz9OyHkD1MSdwvIBDtB8HHp6+CEgVb7Fc2+xR\n";
string mp13 = "42kMCkF/M7Xp8k7MXNyLZUEXxpxP4Liratx1aaXS4fH8eu+ISwR8xLAJso0T+zWVJIAjgZHH\n";
string sigStr6 = "0FcrIZJL+Zdr9+kQd96k/fe0mLhQOOJIYp6pwPDhkRjvYTLN5mjUL+oRH7B1W64vOnnQbRK6\n";
string mp4  = "I/a2Fu9xm4R28byOOrZmj2kNSXmIDp2aEym2jG3USwyRQdththEmQ403010lryhP8VNoH+Wk\n";
string sigStr15 = "X531hwwSoLw0QhZm5bzWQDtGkSeKCTkaU7GNXLgdAOhJMeCJoJALGJV9MISZamecLnZEXP0r\n";
string sigTxt6 = "3h7S4HVDuuKkGgf4LENuH7uVUGLpv5Wjck1f5nYv9zyjH1bK+Kj20Amejt2lPauvctHKzQjD\n";
string mp5  = "adtJwqAJrKrrUlKjwLgX5dScFGMeCdxOTNZgAA/qkzznm/bw0Qa8wScejQrlIlHSmwH3qdOs\n";
string sigTxt7 = "jon5JDfDBFGnVGx8ojAZrd0CpE5kt381JXa0F6p2kUKKuXTmC3irh7gEMtMMlwzXOiqYNGvK\n";
string mp9  = "hXs+27oNOZ60VcxSG4VDYMEiNBDMmkfOmbrRgBet8qBGEGD2oRXh6w2qmDy4f1i7STojVRsm\n";
string mp17 = "mVXTYOGyhR7Yc4X3b5fsPH70SMjhM73AvHvb34qpW8zJLdCBPxDiuL8JgPH2UoqjaeqlWX1k\n";
string sigTxt4 = "Sf3/iKw7WBfV6/FSsdQwyYCb15FajE2PispCnIL5v/7GXsasgZxoSwCmm+w/EQDqBN2ogIuu\n";
string sigStr7 = "iUAFCKRRdOKJuvrrKjizoYckDoHFHcAilWxAToElfVrhdQ0UliUh7M7ENQgkDXeaNn7PJxEH\n";
string mp10 = "pKXbpQSJtwgfSpe3vYB92hKbtE8REVWZaZV26UmCD59i1quG5ZpWlLiVYCTralO31HAHZc/C\n";
string sigTxt9 = "xjcBMBIO8nByGPy5ynA1m2iUkF6xlkURYb/TsrRYZeB6sPmcluTC87L+R1AbbHSLdA7wZ/XR\n";
string mp14 = "YVqbfqWxcHmy7LcbNpokuSiZqu5qycPz3KKT+xFd94fQRHbL+tJ4I4LWYUjmDlMPRPyfiwsL\n";
string sigStr3 = "821D6E9dowKQPCYdmG1/i26Pv7aLPOugjB8YXW6eZ0qisey4795WqWv12g+zooP8Z7n8r21v\n";
string mp18 = "49I0fEYpcJqTIGVULiQtW4okmOPTKOO0AWz+3HTSq6/Xo/h0zEEAZJpS4QBla+vaEBagZykv\n";
string sigTxt1 = "J6dry8aaQZzSsB5ni2tUNzjhAFrNU9zkgmwkI/0geO1AxEZOidtYT5urRrySYK0pXieNBWXV\n";
string mp11 = "oSwkEIpz+XXmqYpZ8iyMj8o3zk2Wl8fhC3rR7vF8v0QE22oq9lcYiBeHBqzkoSfjFnJ1ZH6i\n";
string sigStr10 = "R8FtT0uyakWcZQ32m/FPGJU5yFvDpAfc/HVGbiTN6io8Ha7m+2rNtpH2jyBemefztLMI1QpU\n";
string mp16 = "X9+8LLtcjMrN78Ph/jW49Td/UbmgD2jFPA7J2++DZ0ZLXd2gi8mRkbQM+XVNyohDQSkM5KK2\n";
string sigStr11 = "nWsxzKpyFZnA3L6WGvNdK8kZHpdfu2QVa3cPlsQLHculXyWReqqkFBu47i3E30OtjXRerkmj\n";
string mp12 = "1Mmt5seBtpbKXUAiDFd+j61kIywCNr9r1XD6JkVB0SEG7UC2cGsAZgvehmqE82c6m78kLsYY\n";
string sigTxt0 = "MIICIDANBgkqhkiG9w0BAQEFAAOCAg0AMIICCAKCAgEAlJCq/S/VSzepaOmUx1WSaMheurZ7\n"; 
string sigStr2 = "fRr3X9/GEsRQeReZ7ZE8GvCqrNoRiLjieIdiOhLOktmWw47C7HjSnFBAAXIWXXkGYTJrfh0Q\n";
string sigStr14 = "8MrI7uXx+j1ld+M5UlFQ3h0Cl56PHsdWSsi+61LLcOOV7+mmjaDk4cHrj3JrMOKoLpn00FQx\n";


int VerifySecondaryKey()
{
	string mpub = mp0+mp1+mp2+mp3+mp4+mp5+mp6+mp7+mp8+mp9+mp10+mp11+mp12+mp13+mp14+mp15+mp16+mp17+mp18+mp19;
	//Read public key
	CryptoPP::ByteQueue bytes;
	//FileSource file("/data/license/master-pubkey", true, new Base64Decoder);
	//file.TransferTo(bytes);
	StringSource ss(mpub,true,new Base64Decoder);
	ss.TransferTo(bytes);
	bytes.MessageEnd();
	RSA::PublicKey pubKey;
	pubKey.Load(bytes);

	RSASSA_PKCS1v15_SHA_Verifier verifier(pubKey);

	//Read signed message
	//string signedTxt1;
	//FileSource("/data/license/secondary-pubkey", true, new StringSink(signedTxt1));
	//CryptoPP::ByteQueue sig;
	//FileSource sigFile("/data/license/secondary-pubkey-sig", true, new Base64Decoder);
	//string sigStr;
	//StringSink sigStrSink(sigStr);
	//sigFile.TransferTo(sigStrSink);
	
	string signedTxt64 = sigTxt0 + sigTxt1 + sigTxt2 + sigTxt3 + sigTxt4 + sigTxt5 + sigTxt6 + sigTxt7 + sigTxt8 + sigTxt9 + sigTxt10;
	string signedTxt;
	StringSource(signedTxt64,true,new StringSink(signedTxt));


	string sigStr64 = sigStr0 + sigStr1 + sigStr2 + sigStr3 + sigStr4 + sigStr5 + sigStr6 + sigStr7 + sigStr8 + sigStr9 + sigStr10 + sigStr11 + sigStr12 + sigStr13 + sigStr14 + sigStr15 + sigStr16 + sigStr17 + sigStr18;
	string sigStr;
	StringSink sigStrSink(sigStr);
	StringSource sigSS(sigStr64,true,new Base64Decoder);
	sigSS.TransferTo(sigStrSink);

	string combined(signedTxt);
	combined.append(sigStr);

	//Verify signature
	try
	{
		StringSource(combined, true,
			new SignatureVerificationFilter(
				verifier, NULL,
				SignatureVerificationFilter::THROW_EXCEPTION
		   )
		);
		cout << "Secondary Key OK" << endl;

	}
	catch(SignatureVerificationFilter::SignatureVerificationFailed &err)
	{
	
		cout << err.what() << endl;
		cout << "Secondary Key Error"<< endl;
		return -1;
	}
	return 0;
}


int verifyLicense(string &lic)
{

	if(VerifySecondaryKey())
		return -1;
	
	//Read public key
	string spub = sigTxt0 + sigTxt1 + sigTxt2 + sigTxt3 + sigTxt4 + sigTxt5 + sigTxt6 + sigTxt7 + sigTxt8 + sigTxt9 + sigTxt10;
	CryptoPP::ByteQueue bytes;
	//FileSource file("/data/license/secondary-pubkey", true, new Base64Decoder);
	//file.TransferTo(bytes);
	StringSource spubss(spub,true,new Base64Decoder);
    spubss.TransferTo(bytes);
	bytes.MessageEnd();
	RSA::PublicKey pubKey;
	pubKey.Load(bytes);

	RSASSA_PKCS1v15_SHA_Verifier verifier(pubKey);

	ifstream fin("/data/license/license.bas");
	if (!fin)
	{
		cout << "can not open file license.bas" << endl;
		return -1;
		
	}
	//Read signed message
	
    FileSource baseFile("/data/license/license.bas", true, new Base64Decoder);
    string signedTxt;
    StringSink baseSink(signedTxt);
    baseFile.TransferTo(baseSink);
	lic = signedTxt;

	    //Save encrypt plain to file (Binary)
	    FileSink out("/data/license/license.bin");
	    out.Put((byte const*) signedTxt.data(), signedTxt.size());

    fstream fin1("/data/license/license.sig");
    if (!fin1)
    {
        cout << "can not open file license.sig" << endl;
        return -1;

    }	

	CryptoPP::ByteQueue sig;
	FileSource sigFile("/data/license/license.sig", true, new Base64Decoder);
	string sigStr;
	StringSink sigStrSink(sigStr);
	sigFile.TransferTo(sigStrSink);
	
	string combined(signedTxt);
	combined.append(sigStr);

	//Verify signature
	try
	{
		StringSource(combined, true,
			new SignatureVerificationFilter(
				verifier, NULL,
				SignatureVerificationFilter::THROW_EXCEPTION
		   )
		);
		cout << "License Signature OK" << endl;

	}
	catch(SignatureVerificationFilter::SignatureVerificationFailed &err)
	{
		cout << err.what() << endl;
		cout << "License Sig Error"<< endl;
		return -1;
	}
	
	return 0;
}



