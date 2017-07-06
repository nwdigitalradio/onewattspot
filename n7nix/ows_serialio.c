/*
 * Fundamental serial io routines for Dorji DRA818V module
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>

extern int DebugFlag;

int ows_initserial(const char *pathname)
{
	int uart0fs;

	/* OPEN THE UART
	 * The flags (defined in fcntl.h):
	 *	Access modes (use 1 of these):
	 *		O_RDONLY - Open for reading only.
	 *		O_RDWR - Open for reading and writing.
	 *		O_WRONLY - Open for writing only.
	 *
	 *	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking
	 *	mode. When set read requests on the file can return immediately
	 *	with a failure status if there is no input immediately available
	 *	(instead of blocking). Likewise, write requests can also return
	 *	immediately with a failure status if the output can't be written
	 *	immediately.
	 *
	 *	O_NOCTTY - When set and path identifies a terminal
	 *	device, open() shall not cause the terminal device to become the
	 *	controlling terminal for the process.
	 */
	uart0fs = open(pathname, O_RDWR | O_NOCTTY | O_NDELAY);
	/* open in non blocking read/write mode */
	if (uart0fs == -1) {
		/* ERROR - CAN'T OPEN SERIAL PORT */
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		return(uart0fs);
	}

	/* CONFIGURE THE UART
	 * parameters differ depending on the serial device you are connecting to
	 *
	 * PARITY_NONE, STOPBITS_ONE, EIGHT_BITS
	 * The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	 *	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	 *	CSIZE:- CS5, CS6, CS7, CS8
	 *	CLOCAL - Ignore modem status lines
	 *	CREAD - Enable receiver
	 *	IGNPAR = Ignore characters with parity errors
	 *	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for binary comms!)
	 *	PARENB - Parity enable
	 *	PARODD - Odd parity (else even)
	 */
	struct termios options;
	if ((tcgetattr(uart0fs, &options)) == -1) {
		perror("tcgetattr()");
		close(uart0fs);
		return(-1);
	}
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0fs, TCIFLUSH);
	tcsetattr(uart0fs, TCSANOW, &options);
	return(uart0fs);
}

int ows_writeserbuf(int fs, char *outstring)
{
	int iocount;
	char outbuf[128];

	strcpy(outbuf, outstring);
	strcat(outbuf, "\r\n");

	iocount = write(fs, &outbuf[0], strlen(outbuf));
	if (iocount < 0) {
		printf("%s: UART TX error on buf: %s\n", __FUNCTION__, outbuf);
	} else {
		if(DebugFlag) {
			printf("%s: output(%d): %s", __FUNCTION__, iocount, outbuf);
		}
	}
	return(iocount);
}

int handle_serialread(int fs, char *rx_buffer, int len_rx_buffer)
{
	int rx_length;

	rx_length = read(fs, (void*)rx_buffer, len_rx_buffer);
	if (rx_length < 0) {
		/* An error occured (will occur if there are no bytes) */
	}
	else if (rx_length == 0) {
		/* No data waiting - this shouldn't happen */
		printf("%s: no data on read\n", __FUNCTION__);
	} else {
		/* byte(s) received */
		rx_buffer[rx_length] = '\0';
	}
	return(rx_length);
}

int ows_readserbuf(int serialfs, char *readbuf, int len_readbuf)
{
	fd_set set;
	int rv, iocnt, bufcnt=0;
	int retcode = 0;

	/* init read buffer for debug */
	memset(readbuf, 0, len_readbuf);

	while(1) {
		/* Set read timeout to 5 seconds */
		struct timeval timeout={5,0};

		FD_ZERO(&set);		/* clear the set */
		FD_SET(serialfs, &set); /* add our file descriptor to the set */

		rv = select(serialfs + 1, &set, NULL, NULL, &timeout);
		if(rv == -1) {
			perror("select"); /* a select error accured */
			if (errno == EBADF) {
				retcode = -1;
				break;
			}
			continue;
		} else if(rv == 0) {
			printf("%s: timeout\n", __FUNCTION__); /* a timeout occured */
			retcode = 0;
			break;
		} else {
			/* there was data to read, so read it */
			iocnt = handle_serialread( serialfs, &readbuf[bufcnt], len_readbuf-bufcnt );
			bufcnt+=iocnt;
			if (readbuf[bufcnt-2] == 0x0d && readbuf[bufcnt-1] == 0x0a) {
				if(DebugFlag) {
					printf("%s: Response(%d): %s", __FUNCTION__, bufcnt, readbuf);
				}
				retcode = bufcnt;
				break;
			}

		}
	}
	return(retcode);
}

