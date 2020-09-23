#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>
using namespace std;

#include "license_internal.h"
#define _BOARD_COUNT_	11
#define _UUID_LEN_ 24
#define _BOARD_UUID_CMD_ "/my_tools/bsp_cli module uid %d | grep uid | awk '{print($6)}' | awk -F: '{print($2)}'"

static char license_endtime[256];

int anaLicense1()
{
	string lic = "";
	if(verifyLicense(lic)) {
		cout << "Err: verify license fail" << endl;
		return -1;
	}
	string con = "";
	string serial = "";
	if(separateLicenseSerial(lic,con,serial)) {
		cout << "Err: can not separate license serial" << endl;
		return -1;
	}
	string plain = "";
	if(decryptLicense(con,plain)) {
		cout << "Err: decrypt license fail" << endl;
		return -1;	
	}
	return 0;
}

extern "C" int anaLicense()
{
	return anaLicense1();;
}


static int saveFile(string filename, string key)
{
	ofstream fout(filename, ios::out);	// | ios::binary

	if (fout.is_open()) {
//		cout << encSerial << endl;
//		fout << serial;
//		fout << " ";
//		fout << encSerial << endl;
		fout << key;
//		fout.write((char *)&encSerial, encSerial.length());
		fout.close();
	}
	return 0;
}

int get_local_uuid(int board_id, char *uuid)
{
	char get_uuid_cmd[256] = {0};
	int ret, real_len;
	sprintf(get_uuid_cmd, _BOARD_UUID_CMD_, board_id);
	
	FILE *handle = popen(get_uuid_cmd, "r");
	if(handle == NULL){
		perror("get uuid open error:");
		return -1;
	}
	ret = fread(uuid, 1, 256, handle);
	if(ret < 0){
		printf("read uuid error.\n");
		return -1;
	}
	
	fclose(handle);
	if(ret < _UUID_LEN_){
		printf("uuid len is err, uuid len=%d\n", strlen(uuid));
		return -1;
	}
	uuid[strlen(uuid) - 1]='\0';
	return 0;
}

int decodeLicense()
{
	char *tmp_endtime;
	string serial = "";
	int get_uuid_flag = 0;
	char uuid[256] = {0};
	int i;
	int pass_check = 0;

	FILE *pp = popen("/my_tools/prelicense", "r");
	if (!pp) {
		perror("popen error:");
		return -1;
	}

	char tmp[100]={0};
	fgets(tmp, sizeof(tmp), pp);
	string serialStr(tmp);
	pclose(pp);

	size_t found = serialStr.find("ERROR");
	if (found != string::npos) {
		cout << serialStr << endl;
		return -1;
	}
	serial = serialStr.substr(0, serialStr.size()-1);  // local serial
	cout << serial << endl;

	string lic = "";  //encrypted license
	if(verifyLicense(lic)) {
		cout << "Err: verify license fail" << endl;
		return -1;
	}
	saveFile("lic.bin", lic);
	string con = "";	// content: the first part of encrypted license
	string serial1 = ""; // serial: the second part of encrypted license
	if(separateLicenseSerial(lic,con,serial1)) {
		cout << "Err: can not separate license serial" << endl;
		return -1;
	}
	saveFile("con.bin", con);
	saveFile("serial1.bin", serial1);

	string serial2 = "";	// encrypted local serial
	for (i = 1; i <= _BOARD_COUNT_; i++) {
		if (get_uuid_flag) {
			if(get_local_uuid(i, uuid) < 0)
				continue;
			serial = uuid;
		}
		if(encryptSerial(serial,serial2)) {
			cout << "Err: encrypt serial fail" << endl;
			get_uuid_flag = 1;
			continue;
		}
		if(compareSerial(serial1,serial2)) {	// the second part of encrypted license ==  encrypted local serial ?
			cout << "Info: encrypt serial not match" << endl;
			get_uuid_flag = 1;
			continue;
		}
		cout << "INFO: match serial success. serial: \n" << serial << endl;
		pass_check = 1;
		break;
	};
	if (!pass_check) {
		cout << "Err: encrypt serial fail or encrypt serial not match" << endl;
		return -1;
	}
	string plain = "";		// cleartext of license
	if(decryptLicense(con,plain)) {
		cout << "Err: decrypt license fail" << endl;
		return -1;
	}

	string serial3 = dec_uuid(plain); // serial in cleartext of license
	if(serial3.empty()) {
		cout << "Err: license have no serial" << endl;
		return -1;
	}
	if(serial3.compare(serial)) { // serial in cleartext of license  == local serial ?
		cout << "Err: license have wrong serial" << endl;
		return -1;
	}

	string mac_plain = dec_mac(plain);
	if(mac_plain.empty()) {
		cout << "Err: license have no mac" << endl;
		return -1;
	}

	string endtime_plain = dec_endtime(plain);
	if(endtime_plain.empty()) {
		cout << "Err: license have no endtime" << endl;
		return -1;
	}
//	license_endtime = endtime_plain;
//	license_endtime[0] = '\0';
	tmp_endtime = (char *)endtime_plain.c_str();
	strcpy(license_endtime, tmp_endtime);

	return 0;
}

extern "C" int decode_license()
{
	license_endtime[0] = '\0';
	return decodeLicense();
}

extern "C" char *get_license_endtime()
{
	return license_endtime;
}


