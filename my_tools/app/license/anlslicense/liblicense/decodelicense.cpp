#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>
#include <jsoncpp/json/json.h>

using namespace std;

string dec_uuid(string plain)
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
	if(value["Endtime"].isNull())
		return "";
	if(value["Uuid"].isNull()) 
		return "";	
	if(value["Uuid"].isString())
		return value["Uuid"].asString();
	else
		return "";
}

string dec_mac(string plain)
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
	if(value["Endtime"].isNull())
		return "";
	if(value["Mac"].isNull()) 
		return "";	
	if(value["Mac"].isString())
		return value["Mac"].asString();
	else
		return "";
}

string dec_endtime(string plain)
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
	if(value["Endtime"].isNull())
		return "";
	if(value["Endtime"].isString())
		return value["Endtime"].asString();
	else
		return "";
}

