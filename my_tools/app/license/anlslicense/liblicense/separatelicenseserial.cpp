#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>


using namespace std;
int separateLicenseSerial(string &lic, string &con, string &serial)
{

#if 1
	con = lic.substr(0,256 );
	serial = lic.substr(256,224);
	return 0;

/*

	char bin[1000]= {0};

	std::ifstream fin1("/data/license/license.bin", std::ios::binary);
	fin1.read(bin, sizeof(char) * 256+224);
	fin1.close();
	
	std::ofstream fout1("/data/license/license-enc.bin", std::ios::binary);
	fout1.write(bin, sizeof(char) * 256);
	fout1.close();
	
	std::ofstream fout2("/data/license/serial-enc.bin", std::ios::binary);
	fout2.write(bin + 256, sizeof(char) * 224);
	fout2.close();
	return 0;
*/

#else
	char *fc = (char*)malloc(256);
	if(NULL==fc)
    {
    	LOG(ERROR) << "Malloc license memory fail!";
    	exit(0);
	}
	FILE *ff = fopen("debug/license-enc.bin", "r");
	if(NULL==ff)
    {
    	LOG(ERROR) << "Open debug/license-enc.bin fail!";
    	exit(0);
	}	
	fread(fc, sizeof(char), 256, ff);
	fclose(ff);

    char *sc = (char*)malloc(224);
	if(NULL==fc)
    {
    	LOG(ERROR) << "Malloc serial memory fail!";
    	exit(0);
	}

	FILE *sf = fopen("debug/serial-enc.bin", "r");
	if(NULL==ff)
    {
    	LOG(ERROR) << "Open debug/serial-enc.bin fail!";
    	exit(0);
	}

	fread(sc, sizeof(char), 256, sf);
    fclose(sf);

    FILE *out = fopen("debug/license.bin", "w");
	if(NULL==out )
    {
    	LOG(ERROR) << "Open debug/license.bin fail!";
    	exit(0);
	}	
    fwrite(fc, sizeof(char), 256, out);
    fwrite(sc, sizeof(char), 224, out);
    fclose(out);
	
    free(fc);
	free(sc);
	
	return 0;
#endif	
}

