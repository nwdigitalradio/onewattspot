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
/* Links to: /dev/ttyAMA0 on RPi 2, /dev/ttyS0 on RPi 3 */
#define RPI_SERIAL_DEVICE "/dev/serial0"
#define SIZE_READBUF 128
#define SIZE_ATBUF 128

/* Testing these frequencies:
 *  144.390
 *  144.350
 *  144.990
 *  144.950
 *  145.630
 *  145.690
 *  144.970
 *  145.050
 *  145.070
 */
#define DEFAULT_FREQ "1443900"
#define DORJI_SIG_DIG 7  /* number of significant digits for frequency */
#define DORJI_FREQ_SIZE (DORJI_SIG_DIG + 2)

#define SLEEP_PACE .5 /* unsigned int */
#define MAX_FREQ_COUNT 15

int ows_initserial(const char *pathname);
int ows_writeserbuf(int fs, char *outstring);
int ows_readserbuf(int serialfs, char *readbuf, int len_readbuf);

static void usage(void);
int ms_sleep(int mswait);
const char *getprogname(void);
bool check_freq( int freq );
char *parse_freq(char *pScanFreq);
int padrightzeros(char *str_in, char *str_out);
int add_decimal( char *str);

int DebugFlag = false;
int gverbose_flag = false;

extern char *__progname;

int main(int argc, char *argv[])
{
	/* For command line parsing */
	int next_option, i;
	int option_index = 0; /* getopt_long stores the option index here. */

	int uart0fs;
	char readbuf[SIZE_READBUF];
	int len_readbuf = SIZE_READBUF;
	char atbuf[SIZE_ATBUF];
	char *freqlist[MAX_FREQ_COUNT+1]; /* store frequencines from command line */
	int freqlist_index = 0;
	int retcode;
	/* set default scan wait period to 100 ms */
	int scanwait_period = 100;
	/* set default scan check period to 5 s */
	int scancheck_period = 5;

	time_t glStartTime;
	time_t start_time, current_time;
	char *pTimeBuf;
	int timeBufLen;

	/* initialize frequency list */
	freqlist[0] = NULL;

	/* short options */
	static const char *short_options = "hVdw:s:";
	/* long options */
	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"verbose",     no_argument,  &gverbose_flag, true},
		{"debug",       no_argument,  &DebugFlag, true},
		/* These options don't set a flag.
		We distinguish them by their indices. */
		{"help",          no_argument,       NULL, 'h'},
		{"wait",        required_argument, NULL, 'w'},
		{"scan",        required_argument, NULL, 's'},
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
			case 'w': /* set wait period in msec */
				if(optarg != NULL) {
					scanwait_period = atoi(optarg);
				} else {
					usage();
				}
				if(DebugFlag) {
					printf("DEBUG: scan wait period: %d\n", scanwait_period);
				}
				break;
			case 's': /* set scan period in sec */
				if(optarg != NULL) {
					scancheck_period = atoi(optarg);
				} else {
					usage();
				}
				if(DebugFlag) {
					printf("DEBUG: scan check period: %d\n", scancheck_period);
				}
				break;
			case 'V':   /* set verbose flag */
				gverbose_flag = true;
				break;
			case 'd':
				DebugFlag = true;
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

	/* Check for no frequencies listed on command line
	 *  - use default frequency
	 */
	if (optind == argc) {
		char *sfreq;

		sfreq = parse_freq(DEFAULT_FREQ);
		if (sfreq != NULL) {
			freqlist[0] = sfreq;
			freqlist_index = 1;

			printf(" Default frequency index: %d, %s, %zd\n",
			       freqlist_index-1, sfreq, strlen(sfreq));
		} else {
			printf("Parse error for default frequency: %s\n", DEFAULT_FREQ);
		}
	}

	/* Everything else on command line is considered a frequency
	 * to scan */
	while(optind < argc) {
		char *prx_freq, *prxm_freq;

		prx_freq = argv[optind];
		if(DebugFlag) {
			printf("DEBUG: arg chk tx: %s\n", prx_freq);
		}

		prxm_freq = parse_freq(prx_freq);
		if (prxm_freq != NULL) {
			freqlist[freqlist_index] = prxm_freq;
			freqlist_index++;

			if(gverbose_flag) {
				printf(" frequency index: %d, %s, %zd\n",
				       freqlist_index-1, prxm_freq, strlen(prxm_freq));
			}
		} else {
			printf("Parse error for frequency: %s\n", prx_freq);
		}

		optind++;
	}

	uart0fs = ows_initserial(RPI_SERIAL_DEVICE);
	if (uart0fs == -1) {
		exit(EXIT_FAILURE);
	}

	printf("Scanning these frequencies:\n");
	for (i = 0; i < freqlist_index; i++) {
		printf ("  %s ", freqlist[i]);
	}
	printf("\n");

	/* save start time */
	glStartTime = time(NULL);
	/* get rid of crlf line terminator */
	pTimeBuf = ctime(&glStartTime);
	timeBufLen = strlen(pTimeBuf);
	pTimeBuf[timeBufLen-1] = '\0';
	printf( "START time: %s with scan: wait %d ms, check %d sec... running\n",
		  pTimeBuf, scanwait_period, scancheck_period );

	while(1) {

		for (i = 0; i < freqlist_index; i++) {
			snprintf(atbuf, sizeof(atbuf), "S+%s", freqlist[i]);

			start_time = current_time = time(NULL);

			while(difftime(current_time, start_time) < scancheck_period) {
				ows_writeserbuf(uart0fs, atbuf);
				ows_readserbuf(uart0fs, readbuf, len_readbuf);
				retcode = atoi(&readbuf[2]);

				if(DebugFlag) {
					printf("DEBUG: freq: %s, sig: %d at %s",
					       atbuf, retcode, ctime(&current_time));
				}
				current_time = time(NULL);

				if(retcode != 1) {
					printf("packet[%d] on freq: %s at %s",
					       retcode, freqlist[i], ctime(&current_time));
				}
				ms_sleep(scanwait_period);
			}
		}
	}
	return(0);
}

int ms_sleep(int mswait)
{
	struct timeval tv;
	int retcode;

	if (mswait == 0) {
		return(0);
	}
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

char *parse_freq(char *pScanFreq)
{
	char *prxm_freq;

	/* Don't allow decimal points for frequency input */
	if( memchr(pScanFreq, '.', strlen(pScanFreq)) != NULL) {
		usage(); /* does not return */
	}

	prxm_freq=(char *)malloc(DORJI_FREQ_SIZE);
	if(DebugFlag) {
		printf("malloc size %d\n", DORJI_FREQ_SIZE);
	}
	if(prxm_freq != NULL) {
		if(DebugFlag) {
			printf("copy scanfreq %zd to malloc mem %d\n",
			       strlen(pScanFreq), DORJI_FREQ_SIZE);
		}
		padrightzeros(pScanFreq, prxm_freq);

		if(gverbose_flag) {
			long int irx_freq;

			irx_freq = strtol(prxm_freq, NULL, 0);
			printf(" receive frequency: %ld, %zd\n",
			       irx_freq, strlen(pScanFreq));
		}

		add_decimal(prxm_freq);
	}
	return(prxm_freq);
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

	if(DebugFlag) {
		printf("str2: %s\n", str);
	}
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
	printf("  -w  --wait	   Set scan period in msec (500 = 1/2sec)\n");
	printf("  -s  --scan       Set scan period in sec\n");
	printf("  -V  --verbose    Print verbose messages\n");
	printf("  -d  --debug      Turn on debug messages\n");
	printf("  -h  --help       Display this usage info\n");

	exit(EXIT_SUCCESS);
}
