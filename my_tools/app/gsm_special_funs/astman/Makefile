##################################################################
# libastman: An implementation of asterisk ami connection
#
# Parts taken from https://code.google.com/p/ast-api/
#
# This program is free software, distributed under the terms of
# the GNU General Public License Version 2 as published by the
# Free Software Foundation. See the LICENSE file included with
# this program for more details.
##################################################################

STATIC_LIBRARY=libastman.a
DYNAMIC_LIBRARY:=libastman.so
#STATIC_OBJS=astman.o connection.o action.o astevent.o update.o
#DYNAMIC_OBJS=astman.lo connection.lo action.lo astevent.lo update.lo
#CFLAGS=-Wall -Werror -Wstrict-prototypes -Wmissing-prototypes -g3 -O0 -fPIC $(ALERTING) $(LIBEXTEND_COUNTERS) 
STATIC_OBJS=astman.o action.o astevent.o update.o
DYNAMIC_OBJS=astman.lo action.lo astevent.lo update.lo
CFLAGS=-Wall -Werror -g3 -O0 -fPIC $(ALERTING) $(LIBEXTEND_COUNTERS) -lpthread
LD_FLAGS=-lm -lrt
SOFLAGS:=-Wl,-h$(DYNAMIC_LIBRARY)



all: $(STATIC_LIBRARY) $(DYNAMIC_LIBRARY)

install: $(STATIC_LIBRARY) $(DYNAMIC_LIBRARY)

uninstall:
	@echo ""


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.lo: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(STATIC_LIBRARY): $(STATIC_OBJS)
	ar rcs $(STATIC_LIBRARY) $(STATIC_OBJS)
	ranlib $(STATIC_LIBRARY)

$(DYNAMIC_LIBRARY): $(DYNAMIC_OBJS)
	$(CC) -shared $(SOFLAGS) -o $@ $(DYNAMIC_OBJS) $(LD_FLAGS)

clean:
	rm -f *.o *.so *.lo
	rm -f $(STATIC_LIBRARY) $(DYNAMIC_LIBRARY)

.PHONY:
