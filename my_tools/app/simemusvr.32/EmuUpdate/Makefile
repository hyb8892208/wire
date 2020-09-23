include $(shell pwd)/../../../../path
all: IapThread



CC       ?= gcc
CFLAGS   ?= -Wall -g

CXX      ?= g++
CXXFLAGS ?= -Wall -g -DRDR40

CPPOBJS   = IapThread.o ../common/serial.o  ../common/zprint.o
LIBS      = -lpthread libIapEmu.a  
INCLUDES  = -I../common


IapThread: $(CPPOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o emuUpdate


$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -f $(CPPOBJS) emuUpdate

.PHONY: clean
