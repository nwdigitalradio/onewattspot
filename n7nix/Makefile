#
# Makefile for grntimer
#
#
# Please note that this Makefile *needs* GNU make. BSD make won't do.
#
#
VERSION = 0.1

CC	= gcc
CFLAGS	= -O2 -g -gstabs -Wall    #-I /usr/include/sys -O2 # -g -Wall
#LIBS	= -lm -lrt  #    POSIX Real-time Library (librt, -lrt)
LIBS	= -lc       # -lm

SRC	= ser_init.c
OBJS	= ser_init.o
HDRS	=

CFLAGS += -I/usr/local/include

SYSTYPE := $(shell uname -s)

CFLAGS += -DLINUX

# Set LOCK to yes for serial port locking support
LOCK = no
LIBS   += -L/usr/lib -lz

ifeq ($(LOCK), yes)
  CFLAGS += -DLOCKDEV
  LIBS   += -llockdev
endif

all:	ser_init

help:
	@echo "  SYSTYPE = $(SYSTYPE)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "  LIBS   = $(LIBS)"
	@echo ""
	@echo "Pick one of the following targets:"
	@echo -e "\tgmake grntimer"
	@echo " "


ser_init:	$(SRC) $(HDRS) $(OBJS) Makefile
		$(CC) $(OBJS) -o ser_init $(LIBS)


# Clean up the object files and the sub-directory for distributions
clean:
		rm -f $(OBJS)
		rm -f core *.asc
		rm ser_init
