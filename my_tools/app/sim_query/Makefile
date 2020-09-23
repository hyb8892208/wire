include $(shell pwd)/../../../path

all:sim_query

CFLAGS = -Wall -g

OBJS = sim_query.o 
LIBS = -L$(DEV_HOME)/basefs/usr/lib -lhiredis -lpthread  

sim_query:$(OBJS)
	$(CC) $^ $(CFLAGS)  $(LIBS) -o $@

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIBS) 

clean:
	rm -f $(OBJS) sim_query
