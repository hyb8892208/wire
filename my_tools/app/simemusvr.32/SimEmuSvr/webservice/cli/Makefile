include $(shell pwd)/../../../../../../path
CXX      ?= g++
CXXFLAGS += -Wall -g


all: emu_cli

SOAPOBJS    = ./emu_cli.o ../gsoap/soapwebproxyProxy.o ../gsoap/stdsoap2.o ../gsoap/soapC.o

INCLUDES ?= -I$(DEV_HOME)/third/gsoap-2.8/gsoap

$(SOAPOBJS): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c $(INCLUDES) -o $@


emu_cli:$(SOAPOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(INCLUDES) -o $@

clean:
	rm -f emu_cli $(SOAPOBJS)
