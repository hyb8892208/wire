all: bsp_cli

CFLAGS   = -Wall -g
LDFLAGS	 = -shared -fpic
ARFLAGS  = -rc

RELEASE   = ./../release/
LIBS	  = ../../../../basefs/usr/lib
INCLUDES  = -I./ -I./../ -I./../gsoap/ -I./../release/ -I./../common/ -I../../../../basefs/usr/include

LD	  = ld
AR	  = ar
CC    = gcc

bsp_cli: 
	$(CC) $(CFLAGS)  bsp_cli.c cli.c -o $(RELEASE)bsp_cli $(INCLUDES) -L$(RELEASE) -L$(LIBS) -lbsp_api -lczmq
clean:
	rm -f $(RELEASE)bsp_cli
	rm -f ./*.o

.PHONY: clean
