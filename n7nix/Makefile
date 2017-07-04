#
# Makefile for ows_init.c
#
#
VERSION = 0.1

CC	= gcc
CFLAGS	= -O2 -g -gstabs -Wall
LIBS	= -lc

SRC	= ows_init.c
OBJS	= ows_init.o
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

all:	ows_init

help:
	@echo "  SYSTYPE = $(SYSTYPE)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "  LIBS   = $(LIBS)"
	@echo ""
	@echo "  Pick one of the following targets:"
	@echo  "\tmake ows_init"
	@echo  "\tmake help"
	@echo " "


ows_init:	$(SRC) $(HDRS) $(OBJS) Makefile
		$(CC) $(OBJS) -o ows_init $(LIBS)


# Clean up the object files for distribution
clean:
		rm -f $(OBJS)
		rm -f core *.asc
		rm ows_init
