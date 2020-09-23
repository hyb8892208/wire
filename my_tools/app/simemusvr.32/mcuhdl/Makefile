all: server lib probe_mcu init_gsm restore_event_monitor test


# need to exec source gcc_path.sh in shell first
CFLAGS   = -Wall -g -DMAX_MCU=2 -DMAX_CHN=32
LDFLAGS	 = -shared -fpic
ARFLAGS  = -rc

CLIENT_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapClient.o client.o
SERVER_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapServer.o ./serial/serial.o server.o

OBJS      = $(COBJS)
LIBS      = -L./czmq/lib -lczmq -lpthread
INCLUDES  = -I./czmq/include -I./serial/ -I./gsoap/

LD	  = ld
AR	  = ar

LIB_NAME   	= libmcuhdl
SHARE_LIB	= $(LIB_NAME).so
DYNAMIC_LIB	= $(LIB_NAME).a


server: $(SERVER_COBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -o mcuhdlsvr
$(SERVER_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@


$(CLIENT_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

lib: $(CLIENT_COBJS)
	$(LD) $(LDFLAGS) -o $(SHARE_LIB) $(CLIENT_COBJS)
	${AR} ${ARFLAGS} $(DYNAMIC_LIB) $(CLIENT_COBJS)
	mv $(SHARE_LIB) ./lib
	mv $(DYNAMIC_LIB) ./lib
	cp ./mcuhdl.h ./lib
	
probe_mcu:
	$(CC) -o probe_mcu probe_mcu.c 

init_gsm:
	$(CC) -o init_gsm init_gsm.c $(INCLUDES) -L./lib -lmcuhdl -L./czmq/lib -lczmq 

restore_event_monitor:
	$(CC) -o restore_event_monitor restore_event_monitor.c $(INCLUDES) -L./lib -lmcuhdl

test:
	$(CC) -o test test.c $(INCLUDES) -L./lib -lmcuhdl -L./czmq/lib -lczmq 

clean: clean_server clean_lib clean_probe_mcu clean_init_gsm clean_restore_event_monitor

clean_server:
	rm -f $(SERVER_COBJS) mcuhdlsvr

clean_lib:
	rm -f $(CLIENT_COBJS) ./lib/*
	
clean_probe_mcu:
	rm -f ./probe_mcu ./probe_mcu.o

clean_init_gsm:
	rm -f ./init_gsm

clean_restore_event_monitor:
	rm -f ./restore_event_monitor

clean_test:
	rm -f ./test

.PHONY: clean
