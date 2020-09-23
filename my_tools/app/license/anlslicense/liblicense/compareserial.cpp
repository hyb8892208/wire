#include <iostream>  
#include <cstring>
#include <string> 
#include <fstream>
#include <cstdlib>
#include <cmath>


using namespace std;

#define BLOCKSIZE 4096

int compareSerial(string str1, string str2){
//	ifstream fin1("/data/license/serial-enc.bin");
//	string str1((istreambuf_iterator<char>(fin1)),istreambuf_iterator<char>());
	
//	ifstream fin2("/data/license/serial-enc-2.bin");
//	string str2((istreambuf_iterator<char>(fin2)),istreambuf_iterator<char>());

	if(str1.compare(str2)) 
		return 1;
	return 0;
}



