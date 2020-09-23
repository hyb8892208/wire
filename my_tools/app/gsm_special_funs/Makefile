CC=gcc

all:auto_lock_cell auto_intercall auto_intersms astman_path
astman_path:
	make -C ./astman CC=${CC}

auto_lock_cell:auto_lock_cell.c astman_path
	${CC} -I./astman -L./astman -lastman -lpthread $< -o $@

auto_intercall:auto_intercall.c astman_path
	${CC} -I./astman -L./astman -lastman -lpthread $< -o $@

auto_intersms:auto_intersms.c astman_path
	${CC} -I./astman -L./astman -lastman -lpthread $< -o $@


.PHONY: clean

clean:
	\rm -f auto_lock_cell auto_intercall auto_intersms
	make clean -C ./astman
