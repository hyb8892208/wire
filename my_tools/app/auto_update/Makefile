CC=gcc
STRIP=strip
CFLAGS= -I.

..c.o:
	$(CC) -c $< $(CFLAGS)
	
UPDATE_OBJECTS =auto_update.o
	
all:auto_update
auto_update : $(UPDATE_OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) 
	$(STRIP) $@	
clean:
	rm *.o auto_update -f

