include $(shell pwd)/../../../path
CXX = g++
LD_FLAG = -lcgicc -lfcgi++ -lfcgi -I$(DEV_HOME)/basefs/usr/include -L$(DEV_HOME)/basefs/usr/lib  -I$(DEV_HOME)/third/hiredis-master -lhiredis -lsqlite3
#CXX_FLAG=-D_DEBUG_

all:service

service: service.cpp FCgiIO.cpp common.cpp sendsms.cpp astmanproxy.cpp chanstate.cpp sendussd.cpp smsstatus.cpp
	${CXX} $^ ${LD_FLAG} -o $@ 

.PHONY: clean install

clean:
	\rm -f service

install:
	\cp service ../../fs-nfs/webservice 
