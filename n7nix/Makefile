#
# Makefile for ows_init.c
#
#
VERSION = 0.1

CC	= gcc
CFLAGS	= -O2 -g -gstabs -Wall
LIBS	= -lc

INIT_SRC  = ows_init.c ows_serialio.c
INIT_OBJS = ows_init.o ows_serialio.o
SCAN_SRC  = ows_scan.c ows_serialio.c
SCAN_OBJS = ows_scan.o ows_serialio.o

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

all:	ows_init ows_scan

help:
	@echo "  SYSTYPE = $(SYSTYPE)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "  LIBS   = $(LIBS)"
	@echo ""
	@echo "  Pick one of the following targets:"
	@echo  "\tmake ows_init"
	@echo  "\tmake help"
	@echo " "

#ows_serialio.o: ows_serialio.c

ows_init:	$(INIT_SRC) $(HDRS) $(INIT_OBJS) Makefile
		$(CC) $(INIT_OBJS) -o ows_init $(LIBS)

ows_scan:	$(SCAN_SRC) $(HDRS) $(SCAN_OBJS) Makefile
		$(CC) $(SCAN_OBJS) -o ows_scan $(LIBS)

# Clean up the object files for distribution
clean:
		rm -f $(INIT_OBJS) $(SCAN_OBJS)
		rm -f core *.asc
		rm -f ows_init ows_scan
