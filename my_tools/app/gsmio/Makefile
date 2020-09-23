all: server lib probe_mcu init_gsm gsmiocli


# need to exec source gcc_path.sh in shell first
# define max mcu, max chn use below
# default max mcu is 2, max chn is 32
# -DMAX_MCU=2 -DMAX_CHN=32
CFLAGS   = -Wall -g -DMAX_MCU=2 -DMAX_CHN=32
LDFLAGS	 = -shared -fpic
ARFLAGS  = -rc

CLIENT_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapClient.o client.o
SERVER_COBJS   = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapServer.o ./serial/serial.o server.o
GSMIOCLI_COBJS = ./gsoap/stdsoap2.o ./gsoap/soapC.o ./gsoap/soapClient.o gsmiocli.o

OBJS      = $(COBJS)
LIBS      = -L./czmq/lib -lczmq -lpthread
INCLUDES  = -I./czmq/include -I./serial/ -I./gsoap/

LD	  = ld
AR	  = ar

LIB_NAME   	= libgsmio
SHARE_LIB	= $(LIB_NAME).so
DYNAMIC_LIB	= $(LIB_NAME).a


server: $(SERVER_COBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -o gsmiosvr
$(SERVER_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

$(CLIENT_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

lib: $(CLIENT_COBJS)
	$(LD) $(LDFLAGS) -o $(SHARE_LIB) $(CLIENT_COBJS)
	${AR} ${ARFLAGS} $(DYNAMIC_LIB) $(CLIENT_COBJS)
	mv $(SHARE_LIB) ./lib
	mv $(DYNAMIC_LIB) ./lib
	cp ./gsmio.h ./lib

$(GSMIOCLI_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@

gsmiocli: $(GSMIOCLI_COBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -L./lib -lgsmio -o gsmiocli
	
probe_mcu:
	$(CC) -o probe_mcu probe_mcu.c 

init_gsm:
	$(CC) -o init_gsm init_gsm.c $(INCLUDES) -L./lib -lgsmio -L./czmq/lib -lczmq 

test:
	$(CC) -o test test.c $(INCLUDES) -L./lib -lgsmio -L./czmq/lib -lczmq 

clean: clean_server clean_lib clean_probe_mcu clean_init_gsm clean_gsmiocli

clean_server:
	rm -f $(SERVER_COBJS) gsmiosvr

clean_lib:
	rm -f $(CLIENT_COBJS) ./lib/*
	
clean_probe_mcu:
	rm -f ./probe_mcu ./probe_mcu.o

clean_gsmiocli:
	rm -rf $(GSMIOCLI_COBJS) ./gsmiocli

clean_probe_mcu:
	rm -f ./probe_mcu ./probe_mcu.o

clean_init_gsm:
	rm -f ./init_gsm

clean_test:
	rm -f ./test

.PHONY: clean
