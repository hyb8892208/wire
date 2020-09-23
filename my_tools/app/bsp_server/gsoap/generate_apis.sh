#!/bin/sh

SRC_API=./gsoap_api.h
DST_DIR=./
SOAP_CPP=./../../../../third/gsoap-2.8/gsoap/bin/linux386/soapcpp2
cp ../../../../third/gsoap-2.8/gsoap/stdsoap2.c ../../../../third/gsoap-2.8/gsoap/stdsoap2.h ./ -af

if [ ! -f $SRC_API ]; then
	echo "file not exist: $SRC_API"
	exit -1
fi

if [ ! -f $SOAP_CPP ]; then
	echo "file not exist: $SOAP_CPP"
	exit -1
fi

$SOAP_CPP -w -x -c -C $SRC_API -d $DST_DIR
$SOAP_CPP -w -x -c -S $SRC_API -d $DST_DIR

echo "generate gsoap server/client APIs complete!"

