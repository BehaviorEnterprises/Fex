
#include "spectrogram.hpp"

int main(int argc, char *const *argv) {
	Spectrogram spec(argc, argv);
	spec.mainLoop();
	return EXIT_SUCCESS;
}
