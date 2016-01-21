
#include "config.hpp"

Config::Config(int argc, const char **argv) {
	// TODO read config file to override defaults
	if (argc != 2) exit(1);
	fname = argv[1];
}
