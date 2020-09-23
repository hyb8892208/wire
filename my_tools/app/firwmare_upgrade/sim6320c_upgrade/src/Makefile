#############################################################
#
# Makefile for the tool QDL released by SIMCOM
# 
# Copyright (c) 2010 by SIMCOM, Incorporated. All Rights Reserved.
#
# Author: aaron
# 
# Date: 2010/08/26
#
############################################################

CC = g++
CFLAGS = -c -O2 -g

MAIN_DIR = .
TARGET_BIN_DIR = ../release/
TARGET_OBJ_DIR = ../obj/

BIN_NAME  = module_sim6320c_upgrade

QDL_DIR = $(MAIN_DIR)

SRC_DIR = $(QDL_DIR) 

INCLUDE_PREFIX = -I

ALL_PATHS= $(SRC_DIR)

ALL_INCLUDES = $(addprefix $(INCLUDE_PREFIX), $(ALL_PATHS))

OBJ_CMD = -o 

LD_CMD = -o

QDL_OBJS =$(TARGET_OBJ_DIR)/download.o \
		  $(TARGET_OBJ_DIR)/file.o \
		  $(TARGET_OBJ_DIR)/os_linux.o \
		  $(TARGET_OBJ_DIR)/serialif.o \
		  $(TARGET_OBJ_DIR)/qcn.o \
		  $(TARGET_OBJ_DIR)/openvox_version_record.o \
		  $(TARGET_OBJ_DIR)/openvox_process_bar.o \
		  $(TARGET_OBJ_DIR)/at_upgrade.o
			   
BIN_OBJS = $(QDL_OBJS) 

$(TARGET_OBJ_DIR)/%.o:$(QDL_DIR)/%.cpp 
	@echo ---------------------------------------------------------
	@echo Build OBJECT $(@) from SOURCE $<
	@$(CC) $(CFLAGS) $(ALL_INCLUDES) $(OBJ_CMD) $@ $<
	@echo ---------------------------------------------------------

.PHONY: all clean

all:prep bin

prep:
	@if test ! -d $(TARGET_BIN_DIR); then mkdir $(TARGET_BIN_DIR); fi
	@if test ! -d $(TARGET_OBJ_DIR); then mkdir $(TARGET_OBJ_DIR); fi

bin:$(BIN_OBJS)
	@echo ---------------------------------------------------------
	@echo Create bin file $(BIN_NAME)
	@$(CC) $(LD_CMD) $(TARGET_BIN_DIR)/$(BIN_NAME) $^
	cp upgrade_module_sim6320c.sh $(TARGET_BIN_DIR)
	cp exit_upgrade_module_sim6320c.sh $(TARGET_BIN_DIR)
	@echo ---------------------------------------------------------

	
clean:
	@rm -fr $(TARGET_OBJ_DIR) $(TARGET_BIN_DIR) 
	
