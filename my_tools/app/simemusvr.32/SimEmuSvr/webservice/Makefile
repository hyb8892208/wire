include $(shell pwd)/../../../path
CXX      ?= g++
CXXFLAGS += -Wall -g


all: client 

SOAPOBJS	= ./src/client.o gsoap/soapwebproxyProxy.o gsoap/stdsoap2.o gsoap/soapC.o

INCLUDES ?= -I$(SDK_INCLUDE_DIR)

$(SOAPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@


client:$(SOAPOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(INCLUDES) -o client

clean:
	rm -f client $(SOAPOBJS)
	
