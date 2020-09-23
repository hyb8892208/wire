
######################################
#
#
######################################
  
#target you can change test to what you want
#共享库文件名，lib*.so
TARGET  := calllimit_server
  
#=======================================================

#compile and lib parameter
#=======================================================
include $(shell pwd)/../../../../path
CC      := gcc
#CC      := g++
AR       = ar
RANLIB   = ranlib
LIBS    := -lpthread -L${THIRD_LIB_PATH}/redis-3.0.6/deps/hiredis -lhiredis  -L${DEV_HOME}/my_tools/app/bsp_server/release -lbsp_api
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include -I${DEV_HOME}/my_tools/app/bsp_server/release -I${THIRD_LIB_PATH}/redis-3.0.6/deps/hiredis
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
#=======================================================

#sys path
#=======================================================
# sys include path
SYS_HEADFILE= /usr/include/

GSOAPOUTPUTDIR=../gsoap/xml_server/

# sys lib path
LIB_INSTALL= /usr/lib/debug/usr/lib/ 
#=======================================================

#source file path
SRCDIRS= ./src/
 
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(GSOAPOUTPUTDIR)soapServer.c $(GSOAPOUTPUTDIR)soapC.c $(GSOAPOUTPUTDIR)stdsoap2.c $(SRCDIRS)callevent.c $(SRCDIRS)calllimit.c $(SRCDIRS)calllimit_cfg.c $(SRCDIRS)gsoap_server.c $(SRCDIRS)queue.c $(SRCDIRS)calllimit_log.c
OBJS    := $(GSOAPOUTPUTDIR)soapServer.o $(GSOAPOUTPUTDIR)soapC.o $(GSOAPOUTPUTDIR)stdsoap2.o $(SRCDIRS)callevent.o $(SRCDIRS)calllimit.o $(SRCDIRS)calllimit_cfg.o $(SRCDIRS)gsoap_server.o $(SRCDIRS)queue.o $(SRCDIRS)calllimit_log.o

.PHONY : clean

all : $(TARGET)

                
clean :
	rm -fr $(OBJS) $(TARGET)
  
$(TARGET) : $(OBJS)


	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS) 


