
#include "spectrogram.hpp"

int main(int argc, char *const *argv) {
	Spectrogram spec(argc, argv);
	spec.main_loop();
	return EXIT_SUCCESS;
}
