
#include "config.hpp"
#include <math.h>
#include <fftw3.h>
#include <SFML/Audio.hpp>

class Fft : public Config {
	private:
		sf::Texture texSpec, texThresh;
		double *freq = NULL, *time = NULL, *amp = NULL;
		unsigned short int *erase = NULL;
		sf::VertexArray points, lines;

	protected:
		sf::Sprite spec, thresh;
		sf::String name;
		sf::SoundBuffer song;
		int ntime, nfreq;

		void makeSpectrogram();
		void makeThreshold();
		void makeOverlay();
		sf::VertexArray const &getPoints() const { return points; };
		sf::VertexArray const &getLines() const { return lines; };

		void eraseShift();
		void erasePoint(int, int);
		void eraseUndo();

	public:
		Fft(int, const char **);
		~Fft();
};

