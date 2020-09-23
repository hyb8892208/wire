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
//#include "easylogging++.h"
//INITIALIZE_EASYLOGGINGPP
using namespace CryptoPP;
using namespace std;

#include "configini.h"
#include "queue.h"
#include "bstrwrap.h"
 
#define MAX_LINE_LEN 256		// max line length in config file.
CBString g_ConfigFileName("/etc/license/license.conf");
CBString plain="./license.txt";
CBString keypath="./keys/";
CBString debugpath="./debug/";
CBString respath="./";

extern int encryptLicense(string plainLicense,string & enclicense);
extern int encryptSerial(string serial,string &encserial);
extern int combineLicenseSerial(string enclicense, string serial,string &encryption);
extern int signLicense(string encryption);

// process arguments
static int ProcessArgs(int argc, char* argv[])
{
	while (argc-- > 0) {
		// Look for -c=<configfile> argument
		if ((strlen(argv[argc]) >= 4) && (argv[argc][0] == '-') && (argv[argc][1] == 'c') && (argv[argc][2] == '='))
			g_ConfigFileName = argv[argc] + 3;
	}
	//cout << "Config file =>" << g_ConfigFileName << endl;

	return 0;
}
int main(int argc,char*argv[])
{
	Config *cfg = NULL;
	char buf[MAX_LINE_LEN + 1] = {0};
	int ret;
	if (0 != ProcessArgs(argc, argv))
		return -1;

	if ((ret = ConfigReadFile(g_ConfigFileName, &cfg)) != CONFIG_OK) {
		cout << "ConfigOpenFile API failed for " << g_ConfigFileName << endl;
		return -2;
	}
	
	ConfigReadString(cfg, "general", "plainpath", buf, MAX_LINE_LEN, NULL);
	plain = buf;
	//cout << "plain: " << plain << endl;

	ConfigReadString(cfg, "general", "keypath", buf, MAX_LINE_LEN, NULL);
	keypath = buf;
	//cout <<  "keypath: " << keypath << endl;

    ConfigReadString(cfg, "general", "debugpath", buf, MAX_LINE_LEN, NULL);
    debugpath = buf;
	//cout << "debugpath: " << debugpath << endl;

    ConfigReadString(cfg, "general", "respath", buf, MAX_LINE_LEN, NULL);
    respath = buf;	
	//cout << "respath: " << respath << endl;

	// Read license 
	ifstream inputFile(plain);
	string plainLicense((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());

	// Parse license 
	if (plainLicense.length() > 256) {
		cout << "License too long!";
		return -1;
	}  

	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(plainLicense,value))  {
		cout << "License format error!";
		return -1;
	}

	if(value["Sip"].isNull()) {
		cout << "License error: no item \"Sip\"";
		return -1;
	}		
	if(value["Serial"].isNull()) {
		cout << "License error: no item \"Serial\"";
		return -1;
	}
	
/*
	// Confirm license 
	cout << "---------------------license------------------" << endl;
	cout << "Sip = " << value["Sip"].asInt() << endl;
	cout << "Serial = " << value["Serial"].asString() << endl;
	cout << "----------------------------------------------" << endl;
	cout << endl;
	cout << "Enter \"yes\" to confirm, \"no\" to cancel...\n";

	string confirm;
	cin >> confirm;
	if(confirm.compare("yes")){
		//LOG(INFO) << "Cancelled";
		exit(0);
	}		
*/
	// Encrypt license 
	string encLicense;
	encryptLicense(plainLicense,encLicense);
	// Encrypt Serial
	string serial = value["Serial"].asString();
	string encSerial;
	encryptSerial(serial,encSerial);
	// Combine license and serial
	string encryption;
	combineLicenseSerial(encLicense,encSerial,encryption);	
	// Sign license 
	signLicense(encryption);
	
	ConfigFree(cfg);	
	return 0;
}
