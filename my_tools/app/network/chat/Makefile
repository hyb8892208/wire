
all:	chat

chat:	chat.o
	$(CC) -o chat chat.o
	cp chat ../release/


chat.o:	chat.c
	$(CC) -c $(CFLAGS) -o chat.o chat.c

install: chat
	cp chat ../release/

clean:
	rm -f chat.o chat *~
