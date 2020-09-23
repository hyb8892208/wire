#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>
#include <jsoncpp/json/json.h>

using namespace std;
string anlsLicenseSerial(string plain)
{
	//ifstream inputFile("/data/license/license-dec");
	//string plainLicense((istreambuf_iterator<char>(inputFile)),istreambuf_iterator<char>());
	string plainLicense = plain;

	// Parse license 
	if (plainLicense.length() > 256)
		return "";  
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(plainLicense,value))
		return "";
	if(value["Sip"].isNull())
		return "";
	if(value["Serial"].isNull()) 
		return "";	
	if(value["Serial"].isString())
		return value["Serial"].asString();
	else
		return "";
}
