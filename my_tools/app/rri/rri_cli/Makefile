all: rri_cli

CFLAGS   = -Wall -g
LDFLAGS	 = -shared -fpic
ARFLAGS  = -rc

RELEASE   = ./../release/
INCLUDES  = -I./ -I./../ -I./../gsoap/ -I./../release/

LD	  = ld
AR	  = ar
CC    = gcc

rri_cli: 
	$(CC) $(CFLAGS)  rri_cli.c cli.c -o $(RELEASE)rri_cli $(INCLUDES) -L$(RELEASE) -lrri_api
clean:
	rm -f $(RELEASE)rri_cli
	rm -f ./*.o

.PHONY: clean
