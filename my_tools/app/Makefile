include $(shell pwd)/../../path

CC=gcc

all:ntpclient watchdog get_backport if_monitor net_tool\
    safe_append_to_file set_config set_default\
    cluster_slave rinetd_path process_slavesms remote_com_server\
    auto_update_path gsm_special_funs_path webservice_path\
	 probe_ttyUSBx probe_sndUSB gsm_control simemu\
    probe_mcu_module module_interface led_ctl_path mac++ init_gsm bsp_server_path\
    rri_path gen_extra_path module_upgrade syslog simquery network_path \
	call_limit_path async_sms_path async_ussd_path mcu_upgrade_path license_path \
	station_time_path handle_mms_path module_check_path call_monitor_path 

encrypt:encrypt.c
	${CC} $< -o $@ -lssl -lcrypto

shc:
	make -C ./shc-3.8.7

ntpclient:ntpclient.c
	${CC} $< -o $@

watchdog:watchdog.c
	${CC} $< -o $@

get_backport:get_backport.c
	${CC} $< -o $@

if_monitor:if_monitor.c
	${CC} $< -o $@
	
net_tool:net_tool.c
	${CC} $< -o $@

safe_append_to_file:safe_append_to_file.c
	${CC} $< -o $@
	
set_config:set_config.c
	${CC} $< -o $@

set_default:set_default.c
	${CC} $< -o $@

cluster_slave:cluster_slave.c
	${CC} $< -o $@ -lpthread

rinetd_path:
	make -C ./rinetd

process_slavesms:process_slavesms.c
	${CC} $< -o $@

remote_com_server:remote_com_server.c
	${CC} $< -o $@

probe_ttyUSBx:probe_ttyUSBx.c
	${CC} $< -o $@
	
probe_sndUSB:probe_sndUSB.c
	${CC} $< -o $@

probe_mcu_module:probe_mcu_module.c
	${CC} $< -o $@
	
module_interface:module_interface.c
	${CC} $< -o $@ -lpthread
	
mac++:mac++.c
	${CC} $< -o $@

init_gsm:init_gsm.c
	${CC} $< -o $@

call_monitor_path:
	make -C ./call_monitor

#mnp_get:mnp_get.c
#	${CC} $< -o $@ -lssl -lcrypto -I${SSL_PATH}/include -L${SSL_PATH}/lib

auto_update_path:
	make -C ./auto_update

gsm_special_funs_path:
	make -C ./gsm_special_funs

webservice_path:
	make -C ./webservice
	
bsp_server_path:
	make -C ./bsp_server

rri_path:
	make -C ./rri

gsm_control:gsm_control.c
	${CC} $< -o $@
	
module_check_path:
	make -C ./module_check

simemu:
	make -C ./simemusvr.32/SimEmuSvr
	make -C ./simemusvr.32/EmuUpdate
	make -C ./simemusvr.32/Emu_test
	./simemusvr.32/edge/make_edge.sh
	make -C ./simemusvr.32/edge/build

led_ctl_path:
	make -C ./led_controller

gen_extra_path:
	make -C ./gen_extra_channels

module_upgrade:
	make -C firwmare_upgrade

syslog:
	make -C sysklogd-1.5

simquery:
	make -C ./sim_query

network_path:
	make -C ./network

call_limit_path:
	make -C ./call_limit

async_sms_path:
	make -C ./async_sms

async_ussd_path:
	make -C ./async_ussd

mcu_upgrade_path:
	make -C ./mcu_upgrade

license_path:
	make -C ./license

station_time_path:
	make -C ./station_time

handle_mms_path:
	make -C ./handle_mms

.PHONY: clean

clean:
	\rm -f if_monitor net_tool safe_append_to_file set_config set_default\
    	ntpclient play_all_led cluster_slave get_backport watchdog\
		process_slavesms remote_com_server gsm_control mac++ init_gsm encrypt
#	\rm mnp_get
	make clean -C ./rinetd
	make clean -C ./gsm_special_funs
	make clean -C ./auto_update
	make clean -C ./webservice
	make clean -C ./bsp_server
	make clean -C ./rri
	make clean -C ./simemusvr.32/SimEmuSvr
	make clean -C ./simemusvr.32/Emu_test
	make clean -C ./simemusvr.32/mcuhdl
	make clean -C ./led_controller
	make clean -C sysklogd-1.5
	make clean -C sim_query
	make clean -C ./network
	make clean -C ./call_limit
	make clean -C ./async_sms
	make clean -C ./async_ussd
	make clean -C ./mcu_upgrade
	make clean -C ./license
	make clean -C ./station_time
	make clean -C ./handle_mms
	make clean -C ./module_check

