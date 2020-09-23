include $(shell_pwd)../../../path
targent = soap callmonitor_server callmonitor_cli

server_objs=./server/queue.o ./server/callmonitor.o ./server/ami_interface.o ./server/config.o ./server/redis_interface.o ./server/callmonitor_autocall.o ./server/callmonitor_log.o ./gsoap/gsoap_api.o ./gsoap/stdsoap2.o ./gsoap/soapServer.o ./gsoap/soapC.o

client_objs=./client/gsoap_cli.o ./client/callmonitor_client.o ./gsoap/soapClient.o ./gsoap/soapC.o ./gsoap/stdsoap2.o ./client/cli.o

CC ?= gcc -g
CPP ?= g++

LIBS=-L$(DEV_HOME)/basefs/usr/lib -lpthread -lhiredis

INCLUDE_DIR=-I./include

all:$(targent)

soap:
	cd gsoap&&./generate_apis.sh&&cd -

callmonitor_server:$(server_objs)
	$(CC) -g -o ./release/$@ $(server_objs) $(LIBS)

callmonitor_cli:$(client_objs)
	 $(CC) -g -o ./release/$@ $(client_objs) $(LIBS)

%.o:%.c
	$(CC) -g -o $@ -c $< $(INCLUDE_DIR)

clean:
	@rm -rf $(server_objs) $(client_objs) ./release/callmonitor_server ./release/callmonitor_cli
