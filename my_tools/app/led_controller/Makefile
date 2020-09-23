include $(shell pwd)/../../../path

all: lib led_server

CC = gcc

AR = ar
ARFLAGS = -rc 
CFLAGS = -Wall -g

LED_COBJS = ledhdl.o keyboard.o menudisplay.o queue.o menuctrl.o menufunction.o menulanguage.o menuconf.o redis_if.o

INCLUDES= -I./
LIBS = -L. -lledhdl -lpthread -L$(DEV_HOME)/basefs/usr/lib -lhiredis
LIB_NAME = libledhdl
DYNAMIC_LIB = $(LIB_NAME).a

$(LED_COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lib: $(LED_COBJS)
	$(AR) $(ARFLAGS) $(DYNAMIC_LIB) $(LED_COBJS)

led_server: led_server.c 
	$(CC) $(CFLAGS) $< -o $@ $(LIBS) $(INCLUDES)

clean: clean_lib clean_server

clean_lib:
	rm -f $(LED_COBJS) $(DYNAMIC_LIB)

clean_server:
	rm -f led_server
