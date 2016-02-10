
#include "config.hpp"

Config::Config(int argc, const char **argv) {
	// TODO read config file to override defaults
	if (argc != 2) exit(1);
	fname = strdup(argv[1]);
	char *dup = strdup(argv[1]);
	name = strdup(basename(dup));
	free(dup);
}

Config::~Config() {
	if (name) free(name);
	if (fname) free(fname);
}
