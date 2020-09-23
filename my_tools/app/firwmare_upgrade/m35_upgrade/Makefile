include $(shell pwd)/../../../../path
PROCESS=module_m35_upgrade
UPGRADE_SH=./src/upgrade_module_m35.sh
EXIT_UPGRADE_SH=./src/exit_upgrade_module_m35.sh

CFLAGS   = -Wall -g -fPIC

OBJS=./src/m35_upgrade.o ./src/main.o ./src/debug.o

LIBS      = -L../../../../basefs/usr/lib -lczmq -lpthread -L../../rri/release -L../../bsp_server/release -lbsp_api -lrri_api
INCLUDES  = -I../../rri/release/ -I../../bsp_server/release/ -I./include/ -I$(DEV_HOME)/basefs/usr/include

CC    = gcc

module_m35_upgrade: $(OBJS)
	mkdir -p release
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -o ./release/$@ 
	cp $(UPGRADE_SH) ./release/
	cp $(EXIT_UPGRADE_SH) ./release/

$(OBJS): %.o:%.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(LIBS) $< -o $@


clean:
	rm -rf $(OBJS) release

.PHONY: clean
