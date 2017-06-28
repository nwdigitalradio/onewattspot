/*
 * Initialize the Dorji DRA818V module with a serial port
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#define RPI_SERIAL_DEVICE "/dev/ttyS0" /* /dev/ttyAMA0 */
#define SIZE_READBUF 128

/*
 * configure the serial connections (the parameters differs on the device you are connecting to)
 */

int writeserbuf(int fs, char *outstring)
{
	int iocount;
	char outbuf[128];

	strcpy(outbuf, outstring);
	strcat(outbuf, "\r\n");

	iocount = write(fs, &outbuf[0], strlen(outbuf));
	if (iocount < 0)
	{
		printf("UART TX error\n");
	}
	return(iocount);
}

int handle_serialread(int fs, char *rx_buffer, int len_rx_buffer)
{
	int rx_length;

	rx_length = read(fs, (void*)rx_buffer, len_rx_buffer);
	if (rx_length < 0)
	{
		//An error occured (will occur if there are no bytes)
	}
	else if (rx_length == 0)
	{
		//No data waiting
	}
	else
	{
		//Bytes received
		rx_buffer[rx_length] = '\0';
		printf("%i bytes read : %s\n", rx_length, rx_buffer);
	}
	return(rx_length);
}

int readserbuf(int serialfs, char *readbuf, int len_readbuf)
{
	fd_set set;
	struct timeval timeout;
	int rv;

	int retcode = 0;

	FD_ZERO(&set); /* clear the set */
	FD_SET(serialfs, &set); /* add our file descriptor to the set */

	/* Set read timeout to 5 seconds */
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	rv = select(serialfs + 1, &set, NULL, NULL, &timeout);
	if(rv == -1) {
		perror("select"); /* an error accured */
		retcode = -1;
	}
	else if(rv == 0) {
		printf("timeout"); /* a timeout occured */
		retcode = 0;
	}
	else {
		retcode = handle_serialread( serialfs, readbuf, len_readbuf ); /* there was data to read */
	}
	return(retcode);
}

int main(int argc, char *argv[])
{
	int uart0fs;
	char readbuf[SIZE_READBUF];
	int len_readbuf = SIZE_READBUF;

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
	 *	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	 */
	uart0fs = open(RPI_SERIAL_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
	/* open in non blocking read/write mode */
	if (uart0fs == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(EXIT_FAILURE);
	}

	/* CONFIGURE THE UART
	 * PARITY_NONE, STOPBITS_ONE, EIGHT_BITS
	 * The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	 *	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	 *	CSIZE:- CS5, CS6, CS7, CS8
	 *	CLOCAL - Ignore modem status lines
	 *	CREAD - Enable receiver
	 *	IGNPAR = Ignore characters with parity errors
	 *	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	 *	PARENB - Parity enable
	 *	PARODD - Odd parity (else even)
	 */
	struct termios options;
	if ((tcgetattr(uart0fs, &options)) == -1) {
		perror("tcgetattr()");
		close(uart0fs);
		exit(EXIT_FAILURE);
	}
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0fs, TCIFLUSH);
	tcsetattr(uart0fs, TCSANOW, &options);

	writeserbuf(uart0fs, "AT+DMOCONNECT");
	readserbuf(uart0fs, readbuf, len_readbuf);
	/*
	 * Group setting command
	 * Description: command used to configure a group of module parameters.
	 * Format: AT+DMOSETGROUP=GBW,TFV, RFV,Tx_CTCSS,SQ,Rx_CTCSS<CR><LF>
	 */
	writeserbuf(uart0fs, "AT+DMOSETGROUP=0,144.3900,144.3900,0000,4,0000");
	readserbuf(uart0fs, readbuf, len_readbuf);
	writeserbuf(uart0fs, "AT+SETFILTER=1,1,1");
	readserbuf(uart0fs, readbuf, len_readbuf);
	writeserbuf(uart0fs, "AT+DMOSETVOLUME=3");
	readserbuf(uart0fs, readbuf, len_readbuf);

	return 0;
}