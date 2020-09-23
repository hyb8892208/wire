include $(shell pwd)/../../../../path
all: soap SimEmuSvr


# need to exec source gcc_path.sh in shell first
CC       ?= gcc
CFLAGS   ?= -Wall -g

CXXFLAGS ?= -Wall -g -DMAX_CHN=32 -DSLOT_NBR=8

SOAPOBJS  = webservice/src/emu_webservice.o webservice/gsoap/soapC.o webservice/gsoap/soapwebproxyService.o webservice/gsoap/stdsoap2.o webservice/gsoap/webproxy.o
CPPOBJS   = ../common/zprint.o ../common/msg.o ../common/config.o ../common/CSocketEx.o ../common/serial.o ../common/EmuVcard.o ../common/VcardGenerator_Translator.o SimEmuSvr.o
COBJS     = ../common/base32.o
OBJS      = $(CPPOBJS) $(COBJS) 
LIBS      = -L$(DEV_HOME)/basefs/usr/lib -L ../common/ -lpthread -lhiredis -lcrypto 
INCLUDES ?= -I ../common/ -I$(DEV_HOME)/third/hiredis-master 
#LDFLAGS   = -L../common/

SimEmuSvr: $(OBJS) $(SOAPOBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) $(INCLUDES) -o SimEmuSvr

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

$(SOAPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) -I$(THIRD_LIB_PATH)/gsoap-2.8/gsoap $< -o $@

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

soap:
	cd webservice;./make_soapservice.sh; cd ..
clean:
	rm -rf $(OBJS) SimEmuSvr  $(SOAPOBJS) webservice/gsoap

.PHONY: clean
