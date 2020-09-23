
######################################
#
#
######################################

include $(shell pwd)/../../../path

#target you can change test to what you want
#共享库文件名，lib*.so
TARGET  := handle_mms
  
#=======================================================

#compile and lib parameter
#=======================================================
CC      := gcc
AR       = ar
RANLIB   = ranlib
LIBS    := -L$(DEV_HOME)/basefs/usr/lib -lhiredis -lpthread -lcurl
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include -I$(DEV_HOME)/basefs/usr/include
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
RELEASE  =./release
#=======================================================

#source file path
SRCDIRS= ./src
LIBDIRS= ./src
 
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(SRCDIRS)/main.c $(SRCDIRS)/mms_redis_if.c $(SRCDIRS)/mms_decode.c $(SRCDIRS)/mms_download.c $(SRCDIRS)/mms_queue.c $(SRCDIRS)/mms_file.c ${SRCDIRS}/mms_get_devport.c ${SRCDIRS}/mms_debug.c
OBJS    := $(LIBDIRS)/main.o $(LIBDIRS)/mms_redis_if.o $(SRCDIRS)/mms_decode.o $(SRCDIRS)/mms_download.o $(SRCDIRS)/mms_queue.o $(SRCDIRS)/mms_file.o $(SRCDIRS)/mms_get_devport.o ${SRCDIRS}/mms_debug.o

.PHONY : clean

all : $(TARGET)
	cp -af $(TARGET) ${RELEASE}/
                
clean :
	rm -fr $(OBJS) $(TARGET) ${RELEASE}/$(TARGET)
  
$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(LIBS) -o $@ $(OBJS) $(LDFLAGS)


