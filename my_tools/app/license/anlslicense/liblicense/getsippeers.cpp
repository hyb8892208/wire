#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>
using namespace std;

#include "license_internal.h"


int getSippeers1()
{

    FILE *pp = popen("/bin/prelicense", "r");
    if (!pp) {
		perror("popen error:");
        return -1;
    }
    char tmp[100]={0};
	
    fgets(tmp, sizeof(tmp), pp);
	string serialStr(tmp);
    pclose(pp);

	size_t found = serialStr.find("error");
	if (found != string::npos) {
		cout << serialStr << endl;
		return -1;	
	}
	//ifstream inputFile(serialStr);
    //string serialStr((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());

	
	string serial = serialStr.substr(2, serialStr.size()-3);  // local serial
	cout << serial << endl;

	string defaultserial = "odpeefnavuolxtsieprbixal"; // use for v1

	
	string lic = "";  //encrypted license
	if(verifyLicense(lic)) {
		cout << "Err: verify license fail" << endl;
		return -1;
	}
	string con = "";	// content: the first part of encrypted license
	string serial1 = ""; // serial: the second part of encrypted license
	if(separateLicenseSerial(lic,con,serial1)) {
		cout << "Err: can not separate license serial" << endl;
		return -1;
	}
	string serial2 = "";	// encrypted local serial
	if(encryptSerial(serial,serial2)) {  
		cout << "Err: encrypt serial fail" << endl;
		return -1;
	}
	if(compareSerial(serial1,serial2)) {	// the second part of encrypted license ==  encrypted local serial ?
		cout << "Info: encrypt serial not match" << endl;
		if(encryptSerial(defaultserial,serial2)) {
			cout << "Err: encrypt def serial fail" << endl;
			return -1;
		}
		if(compareSerial(serial1,serial2)) {
			cout << "Err: encrypt def serial not match" << endl;
			return -1;
		}else {
			cout << "Info:Use def serial" << endl;
			serial = defaultserial;			
		}
	}
	string plain = "";		// cleartext of license
	if(decryptLicense(con,plain)) {
		cout << "Err: decrypt license fail" << endl;
		return -1;	
	}
	string serial3 = anlsLicenseSerial(plain); // serial in cleartext of license
	if(serial3.empty()) {
		cout << "Err: license have no serial" << endl;
		return -1;
	}
	if(serial3.compare(serial)) { // serial in cleartext of license  == local serial ?
		cout << "Err: license have wrong serial" << endl;
		return -1;	
	}
	return anlsLicenseSippeers(plain);	// sip in cleartext of license
}

extern "C" int getSippeers()
{
	int sippeers = getSippeers1();

	if(sippeers <= 0)
		sippeers = 30;
	
	char tmp[64]={0};
	sprintf(tmp,"/bin/postlicense %d",sippeers);
    FILE *pp = popen(tmp, "r");
    if (!pp) {
        perror("popen error:");
        return -1;
    }
    pclose(pp);

	return sippeers;
}
