
#include "fex.h"

static WindowFunction custom;
static const WindowFunction windows[] = {
	{ "hanning", 		{0.5,			0.5,			0.0,			0.0}			},
	{ "hamming",	 	{0.54,		0.46,			0.0,			0.0}	 		},
	{ "blackman",		{0.42659,	0.49656,		0.076849,	0.0}			},
	{ "nuttall",		{0.355768,	0.487396,	0.144232,	0.012604}	},
	{ "blacknut",		{0.3635819,	0.4891775,	0.1365995,	0.0106411}	},
	{ "blackharris",	{0.35875,	0.48829,		0.14128,		0.01168}		},
	{ "rectangular",	{1.0,			0.0,			0.0,			0.0}			},
};

#define STRING(s)		STRINGIFY(s)
#define STRINGIFY(s)	#s

static inline void version() {
	printf(STRING(PROGRAM_NAME) " v" STRING(PROGRAM_VER)
", Copyright © 2013 Jesse McClure <www.mccluresk9.com>\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
	 exit(0);
}

static inline void help() {
	printf("\n"
"USAGE\n  " STRING(PROGRAM_NAME) " <options> <wavefile>\n\n"
"OPTIONS\n"
"  -v                show version information then exit\n"
"  -h                show help information then exit\n"
"  -i                inhibit interactive plotting mode\n"
"  -w <name>         select a windowing function (default=hanning)\n"
"  -l <length>       dft window length in number of samples (default=256)\n"
"  -o <offset>       offset between windows (default=window length)\n"
"  -t <threshold>    minimum amplitude, in dB below signal max, to accept for peaks (default=24.0)\n\n"
"WINDOW FUNCTIONS\n"
"  Any function that can be represented as a 4-term cosine function can be\n"
"  used for windowing.  Choose from any of the preprogrammed options below,\n"
"  or use \"custom=a0,a1,a2,a3\" to define your own.\n\n"
"  rectangular       rectangular (1.0,0.0,0.0,0.0)\n"
"  hamming           Hamming (0.54,0.46,0,0).\n" 
"  hanning           Hann (0.5,0.5,0.0,0.0) [default]\n"
"  blackman          Blackman (0.42659,0.49656,0.076849,0.0)\n"
"  nuttall           Nutall (0.355768,0.487396,0.144232,0.012604)\n"
"  blacknut          Blackman-Nuttall (0.3635819,0.4891775,0.1365995,0.0106411)\n"
"  blackharris       Blackman-Harris (0.35875,0.48829,0.14128,0.01168)\n"
"  custom=...        select alpha values for a custom function\n\n"
	); exit(0);
}

#define LINE_LEN	256
const char *configure(int argc, const char **argv) {
	int i;
	const char *arg, *fname = NULL, *rcname = NULL;
	for (i = 1; i < argc; i++) {
		arg = argv[i];
		if (strncmp(arg,"--h",3) == 0 || strncmp(arg,"-h",2) == 0)
			help();
		else if (strncmp(arg,"--v",3) == 0 || strncmp(arg,"-v",2) == 0)
			version();
		else if (strncmp(arg,"-c",2) == 0 && (++i) < argc)
			rcname = argv[i];
		else
			fname = argv[i];
	}
	if (!fname) die("no audio file provided");
	FILE *rc = NULL;
	char *pwd = getenv("PWD");
	if (rcname) rc = fopen(rcname,"r");
	else if ( (!chdir(getenv("XDG_CONFIG_HOME")) && !chdir("fex")) ||
			(!chdir(getenv("HOME")) && !chdir(".config/fex")) )
		rc = fopen("config","r");
	else if ( !chdir(getenv("HOME")) )
		rc = fopen(".fexrc","r");
	chdir(pwd);
	if (!rc) die("unable to open configuration file");
	char line[LINE_LEN], *lptr;
	int col;
	conf.thresh = 14.0;
	conf.spect_floor = 40.0;
	conf.hipass = 12.0;
	conf.lopass = 800.0;
	conf.winlen = 256;
	conf.hop = 0;
	conf.win = (WindowFunction *) windows;
	while (fgets(line,LINE_LEN,rc)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		line[strlen(line)-1] = '\0';
		if ( strncmp(line,"set",3) == 0 && !(
sscanf(line+3," threshold = %lf",&conf.thresh) == 1 ||
sscanf(line+3," floor = %lf",&conf.spect_floor) == 1 ||
sscanf(line+3," bandpass = %lf - %lf",&conf.hipass, &conf.lopass) == 2 ||
sscanf(line+3," samples = %d",&conf.winlen) == 1 ||
sscanf(line+3," scale = %d",&conf.scale) == 1 ||
sscanf(line+3," hop = %d",&conf.hop) == 1
				) ) {
			fprintf(stderr,"Unrecognized config entry \"%s\"\n",line);
		}
		else if (strncmp(line,"option",6) == 0) {
			if (strstr(line+6,"window")) {
				// TODO!
				conf.win = (WindowFunction *) windows;
			}
		}
		else if (strncmp(line,"color",5) == 0) {
			if (strstr(line+5,"spectro")) col = RGBA_SPECT;
			else if (strstr(line+5,"thresh")) col = RGBA_THRESH;
			else if (strstr(line+5,"point")) col = RGBA_POINTS;
			else if (strstr(line+5,"line")) col = RGBA_LINES;
			else if (strstr(line+5,"erase")) col = RGBA_ERASE;
			else if (strstr(line+5,"zoom")) col = RGBA_ZOOMER;
			else col = -1;
			if (col < 0) 
				fprintf(stderr,"Unrecognized config entry \"%s\"\n",line);
			else {
				lptr = line + 5;
				while (*lptr == ' ' || *lptr == '\t') lptr++;
				while (*lptr != ' ' && *lptr != '\t') lptr++;
				if (sscanf(lptr," %lf %lf %lf %lf %lf",
						&conf.col[col].r, &conf.col[col].g,
						&conf.col[col].b, &conf.col[col].a,
						&conf.col[col].w) < 4)
					fprintf(stderr,"Unrecognized config entry \"%s\"\n",line);
			}
		}
	}
	if (!conf.hop) conf.hop = conf.winlen / 4;
	conf.thresh *= -1;
	conf.spect_floor *= -1;

	return fname;
}
