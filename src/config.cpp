
#include "config.hpp"
#include <getopt.h>

sf::Color toColor(char *str) {
	if (str[0] == '#') ++str;
	unsigned long hex = strtoul(str,NULL,16);
	unsigned short int r,g,b,a;
	r = (hex & 0xFF000000) >> 24;
	g = (hex & 0x00FF0000) >> 16;
	b = (hex & 0x0000FF00) >> 8;
	a = (hex & 0x000000FF);
	return sf::Color(r,g,b,a);
}

bool toBool(char *str) {
	switch (str[0]) {
		case 'T': case 't': case '1': case 'Y': case 'y': return true; break;
		case 'F': case 'f': case '0': case 'N': case 'n': return false; break;
		default: return false;
	}
}

Config::Config(int argc, char *const *argv) {
	struct option opts[] = {
		{ "winSize",     optional_argument, 0, 0 },
		{ "binSize",     optional_argument, 0, 0 },
		{ "loPass",      optional_argument, 0, 0 },
		{ "hiPass",      optional_argument, 0, 0 },
		{ "threshold",   optional_argument, 0, 0 },
		{ "floor",       optional_argument, 0, 0 },
		{ "log10",       optional_argument, 0, 0 },
		{ "winBG",       optional_argument, 0, 0 },
		{ "specBG",      optional_argument, 0, 0 },
		{ "specFG",      optional_argument, 0, 0 },
		{ "threshFG",    optional_argument, 0, 0 },
		{ "pointBG",     optional_argument, 0, 0 },
		{ "pointFG",     optional_argument, 0, 0 },
		{ "linesFG",     optional_argument, 0, 0 },
		{ "cursorFG",    optional_argument, 0, 0 },
		{0,              0,                 0, 0 }
	};
	int i, c, index;
	char noarg[] = "";
	for (i = 1; (c=getopt_long_only(argc, argv, "", opts, &index)) != -1; ++i) {
		if (c != 0 || !optarg) continue;
		else if (index == 0)  conf.winlen     = atoi(optarg);
		else if (index == 1)  conf.hop        = atoi(optarg);
		else if (index == 2)  conf.lopass     = atof(optarg);
		else if (index == 3)  conf.hipass     = atof(optarg);
		else if (index == 4)  conf.threshold  = atof(optarg);
		else if (index == 5)  conf.floor      = atof(optarg);
		else if (index == 6)  conf.log10      = toBool(optarg);
		else if (index == 7)  conf.winBG      = toColor(optarg);
		else if (index == 8)  conf.specBG     = toColor(optarg);
		else if (index == 9)  conf.specFG     = toColor(optarg);
		else if (index == 10) conf.threshFG   = toColor(optarg);
		else if (index == 11) conf.pointBG    = toColor(optarg);
		else if (index == 12) conf.pointFG    = toColor(optarg);
		else if (index == 13) conf.linesFG    = toColor(optarg);
		else if (index == 14) conf.cursorFG   = toColor(optarg);
	}
	if (i > argc) exit(1);
	fname = strdup(argv[i]);
	char *dup = strdup(argv[i]);
	name = strdup(basename(dup));
	free(dup);
}

Config::~Config() {
	if (name) free(name);
	if (fname) free(fname);
}

