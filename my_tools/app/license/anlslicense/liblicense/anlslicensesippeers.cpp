#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>
#include <jsoncpp/json/json.h>

using namespace std;

int anlsLicenseSippeers(string plain)
{
	//ifstream inputFile("/data/license/license-dec");
	//string plainLicense((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());
	string plainLicense = plain;

	// Parse license 
	if (plainLicense.length() > 256) {
		cout << "Err: license too long" << endl;
		return -1;  
    }
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(plainLicense,value)) {
		return -1;
	}
	if(value["Sip"].isNull()) {
		return -1;			
	}																	
	if(value["Serial"].isNull()) {
		return -1;	
	}
	if(value["Sip"].isInt())
		return value["Sip"].asInt();
	else
		return -1;
}
