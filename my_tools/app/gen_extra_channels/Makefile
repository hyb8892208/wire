include ../../../path

CXX=${CPP}
TMP_PREFIX_PATH=${PREFIX_PATH_LIB}
LD_FLAG=-lacfa -L${TMP_PREFIX_PATH}

all:script gen_extra_channels
gen_extra_channels: gen_extra_channels.cpp
	${CXX} $^ ${CXX_FLAG} ${LD_FLAG} -o $@

script:
	\cp -rf  lib/libacfa.a ${TMP_PREFIX_PATH}

.PHONY: clean install

clean:
	rm -f gen_extra_channels
install:
