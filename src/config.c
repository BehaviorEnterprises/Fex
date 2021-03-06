/**********************************************************************\
* FEX - The Frequency Excursion Calculator
*
* Author: Jesse McClure, copyright 2013-2014
* License: GPL3
*
*    This program is free software: you can redistribute it and/or
*    modify it under the terms of the GNU General Public License as
*    published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see
*    <http://www.gnu.org/licenses/>.
*
\**********************************************************************/

#include "fex.h"

#define CONF_SET	0x01
#define CONF_COL	0x02

#define C_TYPE__	0x00
#define C_TYPE_D	0x01
#define C_TYPE_F	0x02
#define C_TYPE_LF	0x03
#define C_TYPE_S	0x04
#define C_TYPE_LN	0x05

#define STRING(s)		STRINGIFY(s)
#define STRINGIFY(s)	#s
#define LINE_LEN		256

static WindowFunction custom;
static WindowFunction windows[] = {
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
", Copyright © 2013-2014 Jesse McClure <www.behaviorenterprises.com>\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
	 exit(0);
}

static inline void help() {
	printf("\n"
		"USAGE\n  " STRING(PROGRAM_NAME) " <options> <wavefile>\n\n"
		"OPTIONS\n"
		"  -l|--long                long output (path, duration, + FE)\n"
		"  -v|--version             show version information then exit\n"
		"  -h|--help                show help information then exit\n\n"
		"SEE ALSO\n"
		"  fex(1) and fex-help(1)\n"
		"\n" );
	exit(0);
}

const char *configure(int argc, const char **argv) {
	int i;
	/* set defaults */
	const char *arg, *fname = NULL, *rcname = NULL;
	char help_cmd[256] = "xterm -e man man";
	conf.help_cmd = NULL;
	conf.long_out = False;
	conf.layers = True;
	/* process command line */
	for (i = 1; i < argc; i++) {
		arg = argv[i];
		if (strncmp(arg,"--h",3) == 0 || strncmp(arg,"-h",2) == 0)
			help();
		else if (strncmp(arg,"--v",3) == 0 || strncmp(arg,"-v",2) == 0)
			version();
		else if (strncmp(arg,"--l",3) == 0 || strncmp(arg,"-l",2) == 0)
			conf.long_out = True;
		else if (strncmp(arg,"-c",2) == 0 && (++i) < argc)
			rcname = argv[i];
		else
			fname = argv[i];
	}
	if (!fname) die("no audio file provided");
	/* find configuration file */
	FILE *rc = NULL;
	char *pwd = getenv("PWD");
	if (rcname) rc = fopen(rcname,"r");
	else if ( (!chdir(getenv("XDG_CONFIG_HOME")) && !chdir("fex")) ||
			(!chdir(getenv("HOME")) && !chdir(".config/fex")) )
		rc = fopen("config","r");
	if (!rc && !chdir(getenv("HOME"))) rc = fopen(".fexrc","r");
	chdir(pwd);
	if (!rc) rc = fopen("/usr/share/fex/config","r");
	if (!rc) die("unable to open configuration file");
	/* initialize conf structure and config reading variables */
	char line[LINE_LEN], prefix[32], option[32], fmt[LINE_LEN];
	char window[32], font_fam[LINE_LEN], logFreq[32];
	const char *fspec[] = { "", "%d ","%f ", "%lf ", "%s", "%[^\n]" };
	int j, mode;
	conf.thresh = 14.0;
	conf.spect_floor = 40.0;
	conf.hipass = 12.0;
	conf.lopass = 800.0;
	conf.winlen = 256;
	conf.hop = 0;
	conf.win = (WindowFunction *) windows;
	conf.log10 = False;
	struct {
		const char *name;
		int mode;
		int type[5];
		void *var[5];
	} cf[] = {
		#include "config.h"
	};
	/* read config file */
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
	/* set hop, threshold, floor, and windowing function */
	if (!conf.hop) conf.hop = conf.winlen / 4;
	if (logFreq[0] == 't' || logFreq[0] == 'T') conf.log10 = True;
	conf.thresh *= -1;
	conf.spect_floor *= -1;
	if (strncasecmp(window,"custom",6) == 0)
		conf.win = (WindowFunction *) &custom;
	else if (strlen(window))
		for (i = 0; i < sizeof(windows)/sizeof(windows[0]); i++)
			if (!strncasecmp(window,windows[i].type,strlen(window)))
				conf.win = (WindowFunction *) &windows[i];
	/* set fonts */
	conf.font = cairo_toy_font_face_create(font_fam, CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);
	conf.bfont = cairo_toy_font_face_create(font_fam, CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);
	/* prep 'help' function */
	char *help_arg = strtok(help_cmd," ");
	for (i = 0; help_arg; i++) {
		conf.help_cmd = realloc(conf.help_cmd, (i+2) * sizeof(char *));
		conf.help_cmd[i] = strdup(help_arg);
		conf.help_cmd[i+1] = NULL;
		help_arg = strtok(NULL," ");
	}
	/* return audio file name */
	return fname;
}

int deconfigure() {
	/* clean up, free data */
	int i;
	for (i = 0; conf.help_cmd[i]; i++) free(conf.help_cmd[i]);
	free(conf.help_cmd);
	cairo_font_face_destroy(conf.font);
	cairo_font_face_destroy(conf.bfont);
	return 0;
}
