#!/bin/bash

. ./../../../../../path

rm -rf gsoap
mkdir gsoap

DEV_HOME=./../../..
${THIRD_LIB_PATH}/gsoap-2.8/gsoap/src/soapcpp2 -d./gsoap/ -i ${THIRD_LIB_PATH}/gsoap-2.8/gsoap/import ./include/ns_emu_webservice.h

if [ x"$?" = x"0" ]; then
	xxd -i gsoap/webproxy.wsdl gsoap/webproxy.cpp
	cp ${THIRD_LIB_PATH}/gsoap-2.8/gsoap/stdsoap2.cpp ./gsoap
	make -C ./cli
fi
