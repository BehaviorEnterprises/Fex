
#include "config.hpp"
#include <math.h>
#include <fftw3.h>
#include <SFML/Audio.hpp>

class Fft : public Config {
	private:
		sf::Texture texSpec, texThresh;
		double *freq = NULL, *time = NULL, *amp = NULL;
		unsigned short int *erase = NULL;
		int *peak;

		void checkPeaks();

	protected:
		sf::Sprite spec, thresh;
		sf::String name;
		sf::SoundBuffer song;
		int ntime, nfreq;

		sf::VertexArray getLines();
		sf::VertexArray getPoints();
		void make_spec();
		void make_thresh();
		void erase_shift();
		void erase_point(int, int);
		void erase_undo();

	public:
		Fft(int, const char **);
		~Fft();
};

