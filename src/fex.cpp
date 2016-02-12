
#include "spectrogram.hpp"

int main(int argc, char *const *argv) {
printf("ONE\n");
	Spectrogram spec(argc, argv);
	spec.main_loop();
printf("TWO\n");
	return EXIT_SUCCESS;
}
