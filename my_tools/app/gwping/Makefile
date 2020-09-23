CC=gcc

all: gwpingd gwping rx_test tx_test

tests: rx_test

gwping: gwping.o RawEthSocket.o RawEthFrame.o 
	${CC} -o gwping gwping.o RawEthSocket.o RawEthFrame.o -lstdc++ -lsupc++ -luuid

gwping.o : gwping.cxx
	${CC} -c -g -O2 gwping.cxx


rx_test: rx_test.o RawEthFrame.o RawEthSocket.o 
	${CC} -o rx_test rx_test.o RawEthFrame.o RawEthSocket.o -lsupc++ -lstdc++

rx_test.o : rx_test.cxx RawEthFrame.hpp RawEthSocket.hpp eth_svc.hpp config.h 
	${CC} -c -g -O2 rx_test.cxx
	
tx_test: tx_test.o 	RawEthFrame.o RawEthSocket.o 
	${CC} -o tx_test tx_test.o RawEthFrame.o RawEthSocket.o -lsupc++ -luuid -lstdc++

tx_test.o : tx_test.cxx RawEthFrame.hpp RawEthSocket.hpp eth_svc.hpp config.h 
	${CC} -c -g tx_test.cxx
	

RawEthServer.o : RawEthServer.cxx eth_svc.hpp config.h 
	${CC} -c -g -O2 RawEthServer.cxx

gwpingd: RawEthServer.o RawEthSocket.o RawEthFrame.o iniparser.o dictionary.o
	${CC} -o gwpingd RawEthServer.o RawEthSocket.o RawEthFrame.o iniparser.o dictionary.o -lsupc++ -luuid -lstdc++

RawEthFrame.o : RawEthFrame.cxx RawEthFrame.hpp eth_svc.hpp config.h 
	${CC} -c -g -O2 RawEthFrame.cxx

RawEthSocket.o : RawEthSocket.cxx RawEthSocket.hpp eth_svc.hpp config.h 
	${CC} -c -g -O2 RawEthSocket.cxx


iniparser.o : iniparser.c
	${CC} -c -g -O2 iniparser.c

dictionary.o : dictionary.c
	${CC} -c -g -O2 dictionary.c

clean:
	rm *.o -f
	rm rx_test -f
	rm tx_test -f
	rm gwping -f
	rm gwpingd -f
