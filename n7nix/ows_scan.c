/*
 * scan frequencies with Dorji DRA818V module
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
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
int gverbose_flag = 0;

static void usage(void);
int ms_sleep(int mswait);
const char *getprogname(void);

extern char *__progname;

int main(int argc, char *argv[])
{
	/* For command line parsing */
	int next_option;
	int option_index = 0; /* getopt_long stores the option index here. */

	int uart0fs;
	char readbuf[SIZE_READBUF];
	int len_readbuf = SIZE_READBUF;
	char atbuf[SIZE_ATBUF];
	char scan_freq[16];
	int retcode, scan_period = 1;

	time_t glStartTime, t;
	char *pTimeBuf;
	int timeBufLen;

	/* short options */
	static const char *short_options = "hVs:v:";
	/* long options */
	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"verbose",       no_argument,  &gverbose_flag, true},
		/* These options don't set a flag.
		We distinguish them by their indices. */
		{"help",          no_argument,       NULL, 'h'},
		{"period",        required_argument, NULL, 'p'},
		{NULL, no_argument, NULL, 0} /* array termination */
	};


		/*
	 * Get config from command line
	 */

	opterr = 0;
	option_index = 0;
	next_option = getopt_long (argc, argv, short_options,
				   long_options, &option_index);

	while( next_option != -1 ) {

		switch (next_option) {
			case 0:   /* long option without a short arg */
				/* If this option sets a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				fprintf (stderr, "Debug: option %s", long_options[option_index].name);
				if (optarg)
					fprintf (stderr," with arg %s", optarg);
				fprintf (stderr,"\n");
				break;
			case 'p': /* set period in msec */
				if(optarg != NULL) {
					scan_period = atoi(optarg);
				} else {
					usage();
				}

				printf("DEBUG: period: %d\n", scan_period);
				break;
			case 'V':   /* set verbose flag */
				gverbose_flag = true;
				break;
			case 'h':
				usage();  /* does not return */
				break;
			case '?':
				if (isprint (optopt)) {
					fprintf (stderr, "%s: Unknown option `-%c'.\n",
						getprogname(), optopt);
				} else {
					fprintf (stderr,"%s: Unknown option character `\\x%x'.\n",
						getprogname(), optopt);
				}
				/* fall through */
			default:
				usage();  /* does not return */
				break;
		}

		next_option = getopt_long (argc, argv, short_options,
					   long_options, &option_index);
	}

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
		ms_sleep(scan_period);
	}
	return(0);
}

int ms_sleep(int mswait)
{
	struct timeval tv;
	int retcode;

	tv.tv_sec = 1;
	tv.tv_usec = 1000 * mswait;

	/* select may get interrupted by sigalrm */
	do
	{
		retcode = select(1, NULL, NULL, NULL, &tv);
	}
	while((retcode == -1)&&(errno == EINTR));

	return(retcode);
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
	printf("Usage:  %s [options]\n", getprogname());
	printf("  Version: %s\n", PROG_VERSION);
	printf("  -p  --period     Set scan period in msec (500 = 1/2sec\n");
	printf("  -V  --verbose    Print verbose messages\n");
	printf("  -h  --help       Display this usage info\n");

	exit(EXIT_SUCCESS);
}
