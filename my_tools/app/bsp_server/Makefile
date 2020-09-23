include $(shell pwd)/../../../path
all:soap bsp_server bsp_api bsp_cli_path get_version get_dev script reg_operation


CFLAGS   = -Wall -fPIC 
LDFLAGS	 = -shared
ARFLAGS  = -rc

CLIENT_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapClient.o ./bsp_client/bsp_api.o
SERVER_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapServer.o gsoap_api.o bsp_server.o \
                 ./common/bsp_tools.o \
				 ./hw/mod_brd_hw_intf.o ./hw/m35_operation.o ./hw/sim6320c_operation.o ./hw/chan_brd_hw_intf.o \
				 ./hw/mod_brd.o ./hw/swg_device.o ./hw/swg_device_test.o ./hw/ec20f_operation.o ./hw/uc15_operation.o ./hw/bspsched.o ./hw/sim_device.o

VERSION_COBJS  = ./common/bsp_tools.o ./hw/get_version.o
DEV_COBJS  = ./common/bsp_tools.o ./hw/get_dev.o

REG_OBJS = ./common/bsp_tools.o ./hw/reg_operation.o ./bsp_cli/cli.o

RELEASE   = ./release/
LIBS      = -L$(DEV_HOME)/basefs/usr/lib -L./gsoap/ -lczmq -lpthread
INCLUDES  = -I./ -I$(DEV_HOME)/basefs/usr/include -I./gsoap/ -I./release/ -I./hw/ -I./common/

VERSION_LIBS  = -lpthread 
VERSION_INCLUDES  = -I./ -I./common/

LD	  = ld
AR	  = ar
CC    = gcc

$(SERVER_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -D SWG_TEST=0 -c $(INCLUDES) $(LIBS) $< -o $@

bsp_server: $(SERVER_COBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -o $(RELEASE)$@ -Wl,-Map,helloworld.map
	cp -rf bsp_server.sh ./release

$(CLIENT_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

$(VERSION_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(VERSION_INCLUDES) $(INCLUDES) $< -o $@ 

get_version: $(VERSION_COBJS)
	$(CC) $(CFLAGS) $^ $(VERSION_INCLUDES) $(VERSION_LIBS) $(LIBS)  -o $(RELEASE)$@ -lczmq

$(DEV_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(VERSION_INCLUDES) $(INCLUDES) $< -o $@
get_dev: $(DEV_COBJS)
	$(CC) $(CFLAGS) $^ $(VERSION_INCLUDES) $(VERSION_LIBS) $(LIBS) -o $(RELEASE)$@ -lczmq

reg_operation:$(REG_OBJS)
	$(CC) $(CFALGS) $^ $(VERSION_INCLUDES) $(VERSION_LIBS) $(LIBS) -o $(RELEASE)$@ -lczmq
script:
	\cp -rf  hw/10_opvx_mod_brd_plug.rules  hw/gen_device_link.sh $(RELEASE)
	\cp -rf  vs_usb_channel_set.sh $(RELEASE)


bsp_api: $(CLIENT_COBJS)
	$(AR) $(ARFLAGS) $(RELEASE)libbsp_api.a $(CLIENT_COBJS)
	${LD} $(LDFLAGS) -o $(RELEASE)libbsp_api.so $(CLIENT_COBJS)

soap:
	cd gsoap;./generate_apis.sh;cd ..


bsp_cli_path:
	make -C ./bsp_cli
clean:
	rm -f $(RELEASE)bsp_server
	rm -f $(RELEASE)bsp_cli
	rm -f $(RELEASE)get_dev
	rm -f $(RELEASE)get_version
	rm -f $(RELEASE)reg_operation
	rm -f $(RELEASE)libbsp_api.*
	rm -f ./gsoap/*.o
	rm -f ./bsp_client/*.o
	rm -f ./*.o
	rm -f ./hw/*.o
	rm -f ./common/*.o
	rm -f $(RELEASE)*.sh
	rm -f $(RELEASE)*.rules
	make clean -C ./bsp_cli
	cd gsoap;rm soap* ns.nsmap stdsoap2.*;cd ..

.PHONY: clean
