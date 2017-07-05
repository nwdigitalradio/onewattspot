/*
 * Initialize the Dorji DRA818V module with a serial port
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>

#define PROG_VERSION "1.0"
#define RPI_SERIAL_DEVICE "/dev/ttyS0" /* /dev/ttyAMA0 */
#define SIZE_READBUF 128
#define SIZE_ATBUF 128
#define DEFAULT_FREQ 1443900 /* APRS 2M 1200 baud */
#define DORJI_SIG_DIG 7  /* number of significant digits for frequency */

int DebugFlag=0;

int ows_initserial(const char *pathname);
int ows_writeserbuf(int fs, char *outstring);
int ows_readserbuf(int serialfs, char *readbuf, int len_readbuf);

static void usage(void);
const char *getprogname(void);
bool check_freq( int freq );
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

int gverbose_flag = true;

extern char *__progname;

int main(int argc, char *argv[])
{
	/* For command line parsing */

	int next_option;
	int option_index = 0; /* getopt_long stores the option index here. */

	int uart0fs, i;
	char readbuf[SIZE_READBUF];
	int len_readbuf = SIZE_READBUF;
	char atbuf[SIZE_ATBUF];

	int bytecnt;
	char *ptx_freq, *prx_freq;
	long int itx_freq, irx_freq;
	gsc_t gsc; /* instance of group setting command */
	int dra_volume = 0;

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
		{"volume",        required_argument, NULL, 'v'},
		{"squelch",       required_argument, NULL, 's'},
		{NULL, no_argument, NULL, 0} /* array termination */
	};

	/* init some variables */
	memset(&gsc, 0, sizeof(gsc));
	gsc.sq = 4;
	dra_volume = 3;
	itx_freq = irx_freq = DEFAULT_FREQ;
	snprintf(gsc.tfv, DORJI_SIG_DIG+1, "%ld", itx_freq);
	snprintf(gsc.rfv, DORJI_SIG_DIG+1, "%ld", irx_freq);
	add_decimal(gsc.tfv);
	add_decimal(gsc.rfv);


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
			case 'v': /* set volume */
				if(optarg != NULL) {
					dra_volume = atoi(optarg);
				} else {
					usage();
				}

				printf("DEBUG: volume: %d\n", dra_volume);
				break;
			case 'V':   /* set verbose flag */
				gverbose_flag = true;
				break;
			case 's':   /* set squelch */
				if(optarg != NULL) {
					gsc.sq = atoi(optarg);
				} else {
					usage();
				}
				printf("DEBUG: squelch: %d\n", gsc.sq);
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


	/* if there are any more args left on command line
	 *  parse tx_freq, rx_freq */

	printf("DEBUG: optind %d, argc %d, arg: %s\n", optind, argc, argv[optind]);

	if(optind < argc) {
		/* get  transmit/receive frequency:
		 * range: 134.000 - 174.000 Mhz */

		ptx_freq = argv[optind];
		printf("DEBUG: arg chk tx: %s\n", ptx_freq);

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
		optind++;
	}

	if(optind < argc ) {
		prx_freq = argv[optind];
		printf("DEBUG: arg chk rx: %s\n", prx_freq);

		/* Don't allow decimal points */
		if( memchr(ptx_freq, '.', strlen(ptx_freq)) != NULL) {
			usage(); /* does not return */
		}
		padrightzeros(prx_freq, gsc.rfv);
		irx_freq = strtol(gsc.rfv, NULL, 0);
		add_decimal(gsc.rfv);

		printf(" receive frequency: %ld, %zd\n",
		       irx_freq, strlen(prx_freq));
		optind++;
	}

	/* If anything else is on command line it's an error */
	if( optind < argc) {
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

	uart0fs = ows_initserial(RPI_SERIAL_DEVICE);
	if (uart0fs == -1) {
		exit(EXIT_FAILURE);
	}

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
		ows_writeserbuf(uart0fs, "AT+DMOCONNECT");
		bytecnt=ows_readserbuf(uart0fs, readbuf, len_readbuf);
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
		snprintf(atbuf, sizeof(atbuf), "AT+DMOSETGROUP=%d,%s,%s,%04d,%d,%04d",
			 gsc.gbw,gsc.tfv, gsc.rfv, gsc.tx_ctcss, gsc.sq, gsc.rx_ctcss);

		printf("DEBUG: set group: %s\n", atbuf);

		ows_writeserbuf(uart0fs, atbuf);
		ows_readserbuf(uart0fs, readbuf, len_readbuf);
		ows_writeserbuf(uart0fs, "AT+SETFILTER=1,1,1");
		ows_readserbuf(uart0fs, readbuf, len_readbuf);

		snprintf(atbuf, sizeof(atbuf), "AT+DMOSETVOLUME=%d", dra_volume);
		printf("DEBUG: set volume: %s\n", atbuf);

		/* ows_writeserbuf(uart0fs, "AT+DMOSETVOLUME=3"); */
		ows_writeserbuf(uart0fs, atbuf);
		ows_readserbuf(uart0fs, readbuf, len_readbuf);
	}

	close(uart0fs);

	return 0;
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

	if(gverbose_flag) {
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
	printf("Usage:  %s [options] [tx freq] [rx freq]\n", getprogname());
	printf("  Version: %s\n", PROG_VERSION);
	printf("  frequency range: 1340000 to 1740000\n");
	printf("  No decimal points used in freq\n");
	printf("  -v  --volume     Set volume of module (1-8)\n");
	printf("  -s  --squelch    Set squelch level (0-8)\n");
	printf("  -V  --verbose    Print verbose messages\n");
	printf("  -h  --help       Display this usage info\n");

	exit(EXIT_SUCCESS);
}
