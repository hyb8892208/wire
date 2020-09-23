all:module_uc15_upgrade

objs=main.o quectel_crc.o load_nprg.o detect_port.o openvox_version_record.o openvox_process_bar.o openvox_status_api.o
module_uc15_upgrade:$(objs)
	
	g++ -g $^ -o $@
	cp $@ ../release/
	cp upgrade_module_uc15.sh ../release

%.o:%.c
	gcc -g -c $^ -o $@

%.o:%.cpp
	g++ -g -c $^ -o $@

clean:
	rm -rf *.o
	rm module_uc15_upgrade
	
