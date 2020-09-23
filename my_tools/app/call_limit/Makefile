include $(shell pwd)/../../../path

all_programs= calllimit_server calllimit_cli  old2new_path
all: clean soap ${all_programs} libcalllimit

RELEASE=./release
GSOAP=soapcpp2
GSOAP_ROOT = ${THIRD_LIB_PATH}/gsoap-2.8/gsoap
GSOAPOUTPUTDIR=./gsoap
SEROUTPUTDIR = ./gsoap/xml_server
CLIOUTPUTDIR=  ./gsoap/xml_client
SERGSOAPFLAGS= -cS -d$(SEROUTPUTDIR)
CLIGSOAPFLAGS= -cC -d$(CLIOUTPUTDIR)

calllimit_server:
	make -C ./server
	cp -af ./server/calllimit_server ${RELEASE}

calllimit_cli:
	make -C ./client
	cp -af ./client/calllimit_cli ${RELEASE}

libcalllimit:
	cp -af ./client/libcalllimit.a ${RELEASE}
	cp -af ./client/include/calllimit_api.h ${RELEASE}

.PHONY: clean soap


soap:
	mkdir -p ${SEROUTPUTDIR} ${CLIOUTPUTDIR}
	@cp -v $(GSOAP_ROOT)/stdsoap2.* ${SEROUTPUTDIR}
	@$(GSOAP_ROOT)/src/$(GSOAP) ${SERGSOAPFLAGS} ${GSOAPOUTPUTDIR}/gsoap_header.h
	@sed -i 's/namespaces/namespaces1/g' ${SEROUTPUTDIR}/calllimit.nsmap
	@cp -v $(GSOAP_ROOT)/stdsoap2.* ${CLIOUTPUTDIR}
	@$(GSOAP_ROOT)/src/$(GSOAP) ${CLIGSOAPFLAGS} ${GSOAPOUTPUTDIR}/gsoap_header.h

old2new_path:
	make -C old2new

clean:
	cd ${RELEASE};\
	rm -rf ${all_programs} ../${CLIOUTPUTDIR} ../${SEROUTPUTDIR} libcalllimit.a calllimit_api.h
	make clean -C ./server
	make clean -C ./client
	make clean -C old2new
