
######################################
#
#
######################################

include $(shell pwd)/../../../path

#target you can change test to what you want
#共享库文件名，lib*.so
TARGET  := async_sms
  
#=======================================================

#compile and lib parameter
#=======================================================
CC      := gcc
AR       = ar
RANLIB   = ranlib
LIBS    := -L$(DEV_HOME)/basefs/usr/lib -lhiredis -lpthread
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
RELEASE  =./release
#=======================================================

#source file path
SRCDIRS= ./src
LIBDIRS= ./src
 
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(SRCDIRS)/main.c $(SRCDIRS)/sms_redis_if.c $(SRCDIRS)/sms_send.c $(SRCDIRS)/utf8_simple.o $(SRCDIRS)/sms_ami_if.o $(SRCDIRS)/sms_list.o $(SRCDIRS)/log_debug.o
OBJS    := $(LIBDIRS)/main.o $(LIBDIRS)/sms_redis_if.o $(LIBDIRS)/sms_send.o $(SRCDIRS)/utf8_simple.o $(LIBDIRS)/sms_ami_if.o $(LIBDIRS)/sms_list.o $(LIBDIRS)/log_debug.o

.PHONY : clean

all : $(TARGET)
	cp -af $(TARGET) ${RELEASE}/
                
clean :
	rm -fr $(OBJS) $(TARGET) ${RELEASE}/$(TARGET)
  
$(TARGET) : $(OBJS)


	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)


