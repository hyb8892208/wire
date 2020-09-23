all:
	g++  -g -O2  -std=gnu++11 -Wall -c -o encSerial.o  encSerial.cpp
#	g++  -g -O2  -std=gnu++11 -Wall  ../bstraux.o ../bstrlib.o ../bstrwrap.o ../encryptSerial.o ./encSerial.o -ljsoncpp -lssl -lcrypto -lcryptopp -lpthread -o encSerial
	g++  -g -O2  -std=gnu++11 -Wall  ../bstraux.o ../configini.o ../bsafe.o ../bstrlib.o ../bstrwrap.o ../encryptSerial.o ./encSerial.o -ljsoncpp -lssl -lcrypto -lcryptopp -lpthread -o encSerial

clean:
	rm -rf encSerial.o encSerial
