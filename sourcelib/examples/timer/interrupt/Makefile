# Add additional project sources like this:
# SRCS += X.c
#
# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

# Name of project
PROJ_NAME=main

# main.c file
SRCS = $(PROJ_NAME).c
###################################################
ROOT=$(shell pwd)

OBJS=$(SRCS:.c=.o)

###################################################

.PHONY: $(shell mkdir -p obj) lib proj all depend
all: proj

#include other files
include ./filelist.mk

#include nucelo-f429zi makefile - NOTE MUST BE LAST TO INCLUDE
include $(SOURCELIB_ROOT)/components/boards/nucleo-f429zi/board.mk
include $(SOURCELIB_ROOT)/components/mk/m_launch.mk
include $(SOURCELIB_ROOT)/components/mk/m_common.mk

lib:
	$(MAKE) -C lib

proj: 	$(PROJ_NAME).elf

include .depend
