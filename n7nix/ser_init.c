/*
 * Initialize the Dorji DRA818V module with a serial port
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define RPI_SERIAL_DEVICE "/dev/ttyS0" /* /dev/ttyAMA0 */
#define SIZE_READBUF 128
#define SIZE_GSCBUF 128
#define DEFAULT_FREQ 1443900
#define DORJI_SIG_DIG 7

static void usage(void);
const char *getprogname(void);
int writeserbuf(int fs, char *outstring);
int readserbuf(int serialfs, char *readbuf, int len_readbuf);
bool check_freq( int freq );
int parse_freq(char *strfreq);
int padrightzeros(char *str_in, char *str_out);
int add_decimal( char *str);

/* Structure of DRA818V Group Setting Command */
typedef struct gsc {
	int gbw;
	char tfv[DORJI_SIG_DIG + 2];
	char rfv[DORJI_SIG_DIG + 2];
	int tx_ctcss;
	int sq;
	int rx_ctcss;
} gsc_t;

extern char *__progname;

int main(int argc, char *argv[])
{
	int uart0fs, i;
	char readbuf[SIZE_READBUF];
	int len_readbuf = SIZE_READBUF;
	char gscbuf[SIZE_GSCBUF];

	int bytecnt;
	char *ptx_freq, *prx_freq;
	long int itx_freq, irx_freq;
	gsc_t gsc;

	/* init group set command struct */
	memset(&gsc, 0, sizeof(gsc));
	gsc.sq = 4;
	itx_freq = irx_freq = DEFAULT_FREQ;

	/* parse any command line args */

	/* get  transmit/receive frequency:
	 * range: 134.000 - 174.000 Mhz */
	if(argc == 1) {
		snprintf(gsc.tfv, DORJI_SIG_DIG+1, "%ld", itx_freq);
		snprintf(gsc.rfv, DORJI_SIG_DIG+1, "%ld", irx_freq);
		add_decimal(gsc.tfv);
		add_decimal(gsc.rfv);
	}
	if(argc > 1) {
		ptx_freq = argv[1];

		/* Don't allow decimal points on command line*/
		if( memchr(ptx_freq, '.', strlen(ptx_freq)) != NULL) {
			usage(); /* does not return */
		}
		padrightzeros(ptx_freq, gsc.tfv);
		itx_freq = strtol(gsc.tfv, NULL, 0);
		add_decimal(gsc.tfv);

		printf(" transmit frequency: %ld, %zd\n",
		       itx_freq, strlen(ptx_freq));

		/* make receive frequency the same */
		strcpy(gsc.rfv, gsc.tfv);
		irx_freq = itx_freq;
	}
	if(argc > 2) {
		prx_freq = argv[2];

		/* Don't allow decimal points */
		if( memchr(ptx_freq, '.', strlen(ptx_freq)) != NULL) {
			usage(); /* does not return */
		}
		padrightzeros(prx_freq, gsc.rfv);
		irx_freq = strtol(gsc.rfv, NULL, 0);
		add_decimal(gsc.rfv);

		printf(" receive frequency: %ld, %zd\n",
		       irx_freq, strlen(prx_freq));

	}
	/* Only handle tx freq & rx freq on command line */
	if(argc > 3) {
		usage();  /* does not return */
	}

	if( !check_freq(itx_freq) ) {
		printf("%s: Transmit frequency out of range: %ld\n", getprogname(), itx_freq);
		usage(); /* does not return */
	}
	if( !check_freq(irx_freq) ) {
		printf("%s: Receive frequency out of range: %ld\n", getprogname(), irx_freq);
		usage(); /* does not return */
	}

	snprintf(gscbuf, sizeof(gscbuf), "AT+DMOSETGROUP=%d,%s,%s,%04d,%d,%04d",
		 gsc.gbw,gsc.tfv, gsc.rfv, gsc.tx_ctcss, gsc.sq, gsc.rx_ctcss);

	printf("DEBUG: gscbuf: %s\n", gscbuf);

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
	uart0fs = open(RPI_SERIAL_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
	/* open in non blocking read/write mode */
	if (uart0fs == -1) {
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
	 *	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for binary comms!)
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

	/*
	 * Handshake command
	 * Description: Used to check if the module works normally.
	 * DRA818V module will send back response information when it
	 * receives this command from the host. If the host doesn't
	 * receive any response from module after three times of
	 * continuously sending this command, it will restart the
	 * module.
	 */

	for (i=0 ; i < 3; i++) {
		writeserbuf(uart0fs, "AT+DMOCONNECT");
		bytecnt=readserbuf(uart0fs, readbuf, len_readbuf);
		if(bytecnt !=0) {
			printf("%s: Handshake successful (%d) at index %d\n",
			       __FUNCTION__, bytecnt, i);
			break;
		}
	}
	if (bytecnt == 0) {
		printf("%s: Failed to initialize DRA818V, exiting\n", __FUNCTION__);

	} else {
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
	}

	close(uart0fs);

	return 0;
}

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
	if (iocount < 0) {
		printf("%s: UART TX error on buf: %s\n", __FUNCTION__, outbuf);
	} else {
		printf("%s: output(%d): %s", __FUNCTION__, iocount, outbuf);

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

int readserbuf(int serialfs, char *readbuf, int len_readbuf)
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
#if 0
			printf("%s: Debug last 2 chars 0x%02x, 0x%02x\n", __FUNCTION__, readbuf[bufcnt-2], readbuf[bufcnt-1]);
#endif
			if (readbuf[bufcnt-2] == 0x0d && readbuf[bufcnt-1] == 0x0a) {
				printf("%s: Response(%d): %s", __FUNCTION__, bufcnt, readbuf);
				retcode = bufcnt;
				break;
			}

		}
	}
	return(retcode);
}

int add_decimal( char *str)
{
	char copy_str[16];

	if( strlen(str) > 16 ) {
		printf("%s: %s: string too long\n", getprogname(), __FUNCTION__);
		return 0;
	}
	memset(copy_str, 0, 16);
	strncpy(copy_str, str, 16);

	*(str+3) = '.';
	*(str+4) = '\0';

	strncat(str, copy_str+3, 4);
	printf("str2: %s\n", str);
	return 1;
}

int padrightzeros(char *str_in, char *str_out)
{
	char zerostr[DORJI_SIG_DIG];

	int numstrsize = strlen(str_in);

	if( numstrsize > DORJI_SIG_DIG ) {
		strncpy(str_out, str_in, DORJI_SIG_DIG);
	} else if (numstrsize == DORJI_SIG_DIG ) {
		strncpy(str_out, str_in, DORJI_SIG_DIG);
	} else {
		strcpy(str_out, str_in);
		memset(zerostr, 0x30, DORJI_SIG_DIG - strlen(str_in));
		strncat(str_out, zerostr, DORJI_SIG_DIG - strlen(str_in));
	}

	return 0;
}

bool check_freq( int freq )
{
	bool retcode = false;

	if (freq >= 1340000 && freq <= 1740000) {
		retcode = true;
	}
	return (retcode);
}

int parse_freq(char *strfreq)
{
	bool retcode = false;
	return (retcode);
}

const char *getprogname(void)
{
	return __progname;
}

/*
 * Print usage information and exit
 *  - does not return
 */
static void usage(void)
{
	printf("Usage:  %s [tx freq] [rx freq]\n", getprogname());
	printf("  frequency range: 1340000 to 1740000\n");
	printf("  No decimal points used in freq\n");
	exit(EXIT_SUCCESS);
}
