
#include "config.hpp"
#include <math.h>
#include <fftw3.h>
#include <SFML/Audio.hpp>

class Fft : public Config {
	private:
		sf::Texture texSpec, texThresh;
		sf::VertexArray points, lines;
		double *freq = NULL, *time = NULL, *amp = NULL;
		unsigned short int *erase = NULL;
		int t1, t2, f1, f2;

	protected:
		sf::Sprite spec, thresh;
		sf::SoundBuffer song;
		int ntime, nfreq;
		double pathLength, timeLength;

		void makeSpectrogram();
		void makeThreshold();
		void makeOverlay();
		void setCrop(sf::Vector2f, sf::Vector2f);
		sf::FloatRect getCrop() { return sf::FloatRect(t1, f1, t2 - t1, f2 - f1); }
		sf::VertexArray const &getPoints() const { return points; };
		sf::VertexArray const &getLines() const { return lines; };

		void eraseShift();
		void erasePoint(int, int);
		void eraseUndo();

	public:
		Fft(int, char *const *);
		~Fft();
};

