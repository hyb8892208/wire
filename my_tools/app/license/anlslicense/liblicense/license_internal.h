#ifndef _LIBLICENSE_H
#define _LIBLICENSE_H
int verifyLicense(string &lic);
int separateLicenseSerial(string &lic, string &con, string &serial);
int decryptLicense(string con,string &plain);
int encryptSerial(string serial1, string &serial2);
int compareSerial(string serial1, string serial2);
int anlsLicenseSippeers(string plain);
string anlsLicenseSerial(string plain);

string dec_uuid(string plain);
string dec_mac(string plain);
string dec_endtime(string plain);

#endif




