all: simslot_tools

CFLAGS   = -Wall -g
LDFLAGS	 = -shared -fpic
ARFLAGS  = -rc

RELEASE   = ./../release/
INCLUDES  = -I./ -I./../ -I./../gsoap/ -I./../release/

LD	  = ld
AR	  = ar
CC    = gcc

simslot_tools:
	$(CC) $(CFLAGS) simslot_tools.c -o $(RELEASE)simslot_tools -lpthread

clean:
	rm -f $(RELEASE)simslot_tools
	rm -f ./*.o

.PHONY: clean
