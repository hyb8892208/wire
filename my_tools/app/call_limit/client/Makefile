
######################################
#
#
######################################
  
#target you can change test to what you want
#共享库文件名，lib*.so
TARGET  := calllimit_cli
LIBOUT  := libcalllimit.a
  
#=======================================================
include $(shell pwd)/../../../../path
CC      := gcc
AR       = ar
LD	     = ld
RANLIB   = ranlib
LIBS    := -lpthread -L${PREFIX_PATH_LIB} -ljson-c
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include -I${PREFIX_PATH_INC}/json-c 
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE) -Wl,-rpath=${PREFIX_PATH_LIB}
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
#=======================================================

#sys path
#=======================================================
# sys include path
SYS_HEADFILE= /usr/include/

# sys lib path
LIB_INSTALL= /usr/lib/debug/usr/lib/ 
#=======================================================

#source file path
SRCDIRS= ./src/

GSOAPOUTPUTDIR= ../gsoap/xml_client/

#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(GSOAPOUTPUTDIR)soapClient.c $(GSOAPOUTPUTDIR)soapC.c $(GSOAPOUTPUTDIR)stdsoap2.c $(SRCDIRS)calllimit_cli.c $(SRCDIRS)cli.c $(SRCDIRS)gsoap_cli.c
OBJS    := $(GSOAPOUTPUTDIR)soapClient.o $(GSOAPOUTPUTDIR)soapC.o $(GSOAPOUTPUTDIR)stdsoap2.o $(SRCDIRS)calllimit_cli.o $(SRCDIRS)cli.o $(SRCDIRS)gsoap_cli.o
CLIENT_COBJS := $(GSOAPOUTPUTDIR)soapClient.o $(GSOAPOUTPUTDIR)soapC.o $(GSOAPOUTPUTDIR)stdsoap2.o  $(SRCDIRS)gsoap_cli.o


.PHONY : clean

all : $(TARGET) ${LIBOUT}

                
clean :
	rm -fr $(OBJS) $(TARGET) $(LIBOUT)
  
$(TARGET) : $(OBJS)

	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)


$(LIBOUT) : $(CLIENT_COBJS)

	$(AR) cru $(LIBOUT) $(CLIENT_COBJS)
	$(RANLIB) $(LIBOUT)


