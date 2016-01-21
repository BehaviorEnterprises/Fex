
#include "spectrogram.hpp"

int main(int argc, const char **argv) {
	Spectrogram spec(argc, argv);
	spec.main_loop();
	return EXIT_SUCCESS;
}
