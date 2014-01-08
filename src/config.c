
#include "fex.h"

#define CONF_SET	0x01
#define CONF_COL	0x02

#define C_TYPE__	0x00
#define C_TYPE_D	0x01
#define C_TYPE_F	0x02
#define C_TYPE_LF	0x03
#define C_TYPE_S	0x04

#define STRING(s)		STRINGIFY(s)
#define STRINGIFY(s)	#s
#define LINE_LEN	256

static FT_Library library;
static FT_Face face, bface;
static WindowFunction custom;
static const WindowFunction windows[] = {
	{ "hanning",         {0.5,       0.5,       0.0,       0.0}       },
	{ "hamming",         {0.54,      0.46,      0.0,       0.0}       },
	{ "blackman",        {0.42659,   0.49656,   0.076849,  0.0}       },
	{ "nuttall",         {0.355768,  0.487396,  0.144232,  0.012604}  },
	{ "blackman-nutall", {0.3635819, 0.4891775, 0.1365995, 0.0106411} },
	{ "blackman-harris", {0.35875,   0.48829,   0.14128,   0.01168}   },
	{ "rectangular",     {1.0,       0.0,       0.0,       0.0}       },
};

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
		"\n" );
	exit(0);
}

const char *configure(int argc, const char **argv) {
	int i;
	const char *arg, *fname = NULL, *rcname = NULL;
	strcpy(conf.help_cmd,"xterm -e man fex-help");
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
	else
		rc = fopen("/usr/share/fex/config","r");
	chdir(pwd);
	if (!rc) die("unable to open configuration file");
	char line[LINE_LEN], prefix[32], option[32], fmt[LINE_LEN];
	char window[32], font_path1[LINE_LEN], font_path2[LINE_LEN];
	const char *fspec[] = { "", "%d ","%f ", "%lf ", "%s" };
	int j, mode;
	conf.thresh = 14.0;
	conf.spect_floor = 40.0;
	conf.hipass = 12.0;
	conf.lopass = 800.0;
	conf.winlen = 256;
	conf.hop = 0;
	conf.win = (WindowFunction *) windows;
	struct {
		const char *name;
		int mode;
		int type[5];
		void *var[5];
	} cf[] = {
		#include "config.h"
	};
	while (fgets(line,LINE_LEN,rc)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		sscanf(line,"%s %s",prefix,option);
		if (strncasecmp(prefix,"set",3) == 0) mode = CONF_SET;
		else if (strncasecmp(prefix,"col",3) == 0) mode = CONF_COL;
		else fprintf(stderr,"bad config entry: %s",line);
		sprintf(fmt,"%s %s ",prefix,option);
		if (mode == CONF_SET) strcat(fmt,"= ");
		for (i = 0; i < sizeof(cf) / sizeof(cf[0]); i++) {
			if ( !strncasecmp(option,cf[i].name,strlen(cf[i].name)) &&
					cf[i].mode == mode ) {
				for (j = 0; j < 5; j++) strcat(fmt,fspec[cf[i].type[j]]);
				sscanf(line,fmt,cf[i].var[0],cf[i].var[1],cf[i].var[2],
						cf[i].var[3],cf[i].var[4]);
			}
		}
	}
	if (!conf.hop) conf.hop = conf.winlen / 4;
	conf.thresh *= -1;
	conf.spect_floor *= -1;
	if (strlen(window))
		for (i = 0; i < sizeof(windows)/sizeof(windows[0]); i++)
			if (!strncasecmp(window,windows[i].type,strlen(window)))
				conf.win = (WindowFunction *) &windows[i];
	if (FT_Init_FreeType(&library)) die("unable to init freetype");
	if ( FT_New_Face(library, font_path1, 0, &face) |
			FT_Set_Pixel_Sizes(face, 0, conf.font_size) )
		fprintf(stderr,"unable to load freetype font: %s\n",font_path1);
	if ( FT_New_Face(library, font_path2, 0, &bface) |
			FT_Set_Pixel_Sizes(bface, 0, conf.font_size) )
		fprintf(stderr,"unable to load freetype font: %s\n",font_path2);
	conf.font = cairo_ft_font_face_create_for_ft_face(face,0);
	conf.bfont = cairo_ft_font_face_create_for_ft_face(bface,0);
	return fname;
}

int deconfig() {
	cairo_font_face_destroy(conf.font);
	cairo_font_face_destroy(conf.bfont);
	FT_Done_Face(face);
	FT_Done_Face(bface);
}
