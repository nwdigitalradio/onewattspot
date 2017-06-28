#
# Makefile for ser_init.c
#
#
VERSION = 0.1

CC	= gcc
CFLAGS	= -O2 -g -gstabs -Wall
LIBS	= -lc

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
	@echo "  Pick one of the following targets:"
	@echo  "\tmake ser_init"
	@echo  "\tmake help"
	@echo " "


ser_init:	$(SRC) $(HDRS) $(OBJS) Makefile
		$(CC) $(OBJS) -o ser_init $(LIBS)


# Clean up the object files for distribution
clean:
		rm -f $(OBJS)
		rm -f core *.asc
		rm ser_init
