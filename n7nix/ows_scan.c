/*
 * scan frequencies with Dorji DRA818V module
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define PROG_VERSION "1.0"
#define RPI_SERIAL_DEVICE "/dev/ttyS0" /* /dev/ttyAMA0 */
#define SIZE_READBUF 128
#define SIZE_ATBUF 128

#define TEST_FREQ "144.3900"
#define SLEEP_PACE .5 /* unsigned int */

int ows_initserial(const char *pathname);
int ows_writeserbuf(int fs, char *outstring);
int ows_readserbuf(int serialfs, char *readbuf, int len_readbuf);

int DebugFlag=0;

int main(int argc, char *argv[])
{
	int uart0fs;
	char readbuf[SIZE_READBUF];
	int len_readbuf = SIZE_READBUF;
	char atbuf[SIZE_ATBUF];
	char scan_freq[16];
	int retcode;

	time_t glStartTime, t;
	char *pTimeBuf;
	int timeBufLen;

	uart0fs = ows_initserial(RPI_SERIAL_DEVICE);
	if (uart0fs == -1) {
		exit(EXIT_FAILURE);
	}

	/* save start time */
	glStartTime = time(NULL);
	/* get rid of crlf line terminator */
	pTimeBuf = ctime(&glStartTime);
	timeBufLen = strlen(pTimeBuf);
	pTimeBuf[timeBufLen-1] = '\0';
	printf( "START time: %s ... running\n", pTimeBuf );

	while(1) {
		strcpy(scan_freq, TEST_FREQ);
		snprintf(atbuf, sizeof(atbuf), "S+%s", scan_freq);
		if(DebugFlag) {
			printf("DEBUG: scan freq: %s\n", atbuf);
		}

		ows_writeserbuf(uart0fs, atbuf);
		ows_readserbuf(uart0fs, readbuf, len_readbuf);
		retcode = atoi(&readbuf[2]);
		t = time(NULL);
		if(DebugFlag) {
			printf("sig: %d at %s", retcode, ctime(&t));
		}
		if(retcode != 1) {
			printf("packet[%d] at %s", retcode, ctime(&t));
		}
		sleep(0.5);
	}
	return(0);
}