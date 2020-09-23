include ../../../path
targent=module_check

all:$(targent)

module_check_objs=module_check.o
LIBS=-lpthread

module_check:$(module_check_objs)
	$(CC) -o $@ $^ $(LIBS)

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	rm -rf $(targent) $(module_check_objs)
