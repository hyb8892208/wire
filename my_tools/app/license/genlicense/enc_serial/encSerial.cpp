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

#include "../bstrwrap.h"
 
string g_serial;	// uuid
CBString keypath="./keys/";
CBString debugpath="./debug/";
CBString respath="./";

extern int encryptSerial(string serial,string &encserial);

// process arguments
static int processArgs(int argc, char* argv[])
{
	while (argc-- > 0) {
		// Look for -c=<configfile> argument
		cout << "argc=";
		cout << argc;
		cout << ", argv[argc]=";
		cout << argv[argc] << endl;
		
		if ((strlen(argv[argc]) >= 4) && (argv[argc][0] == '-') && (argv[argc][1] == 's') && (argv[argc][2] == '=')) {
			g_serial = argv[argc] + 3;
			break;
		} else {
			cout << "Usage: \"process -s=uuid\"" << endl;
			return -1;
		}
	}
	//cout << "Config file =>" << g_ConfigFileName << endl;

	return 0;
}

static int saveUuidKeyFile(string filename, string serial, string encSerial)
{
	ofstream fout(filename, ios::out);	// | ios::binary

	if (fout.is_open()) {
//		cout << encSerial << endl;
//		fout << serial;
//		fout << " ";
//		fout << encSerial << endl;
		fout << encSerial;
//		fout.write((char *)&encSerial, encSerial.length());
		fout.close();
	}
	return 0;
}

int main(int argc,char*argv[])
{

	if (0 != processArgs(argc, argv))
		return -1;

	// Encrypt Serial
	string encSerial;
	encryptSerial(g_serial,encSerial);

	string filename;
	filename = "uuidKey-"+g_serial+".bin";
	saveUuidKeyFile(filename, g_serial, encSerial);

	return 0;
}

