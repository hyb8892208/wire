include $(shell pwd)/../../../path
all: soap rri_server rri_api rri_cli_path qdai tools_path


CFLAGS   = -Wall -g
LDFLAGS	 = -shared -fPIC
ARFLAGS  = -rc

CLIENT_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapClient.o ./rri_api.o
SERVER_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapServer.o gsoap_api.o rri_server.o

RELEASE   = ./release/

LIBS      = -L$(DEV_HOME)/basefs/usr/lib -L./gsoap/ -lczmq -lpthread
INCLUDES  = -I./ -I$(DEV_HOME)/basefs/usr/include -I./gsoap/ -I./release/

LD	  = ld
AR	  = ar
CC    = gcc

$(SERVER_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

rri_server: $(SERVER_COBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -o $(RELEASE)$@ -Wl,-Map,server.map

$(CLIENT_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

rri_api: $(CLIENT_COBJS)
	$(AR) $(ARFLAGS) $(RELEASE)librri_api.a $(CLIENT_COBJS)
	${LD} $(LDFLAGS) -o $(RELEASE)librri_api.so $(CLIENT_COBJS)
qdai:
	cp -rf module_set_qdai.sh $(RELEASE)

tools_path:
	make -C ./tools

soap:
	cd gsoap;./generate_apis.sh;cd ..

rri_cli_path:
	make -C ./rri_cli
clean:
	rm -f $(RELEASE)rri_server
	rm -f $(RELEASE)rri_cli
	rm -f $(RELEASE)librri_api.*
	rm -f ./gsoap/*.o
	rm -f ./*.o
	make clean -C ./rri_cli
	cd gsoap;rm soap* ns.nsmap stdsoap2.*;cd ..

.PHONY: clean
