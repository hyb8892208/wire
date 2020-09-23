include ../../../path

targent=station_time
objs=station_time.o
INCLUDE := -I./include -I${THIRD_LIB_PATH}/redis-3.0.6/deps/hiredis

$(targent):$(objs)
	$(CC) $(objs) -o $@ -lhiredis -L${THIRD_LIB_PATH}/redis-3.0.6/deps/hiredis

%.o:%.c
	$(CC) -c $< -o $@ $(INCLUDE)


clean:
	rm -rf *.o $(targent)

	

