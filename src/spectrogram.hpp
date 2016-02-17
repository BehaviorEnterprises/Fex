
#include "fft.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Spectrogram : public Fft {
	private:
		sf::RenderWindow win;
		sf::View view;
		sf::Vector2f mouse, crop1;
		sf::RectangleShape back;
		sf::Texture ball;
		sf::RectangleShape eraser;
		bool mod_ctrl, mod_shift, mod_alt;
		float aspect;

		void drawMain();
		void drawCursor(float, float);
		void drawHud();
		void erase();

		void checkModKeys();
		void evHandler(sf::Event);
		void evClose(sf::Event);
		void evKeyPress(sf::Event);
		void evKeyRelease(sf::Event);
		void evMouseButton(sf::Event);
		void evMouseMove(sf::Event);
		void evMouseWheel(sf::Event);
		void evResize(sf::Event);
	protected:
		void listen(float=1.0);
	public:
		int mainLoop();
		Spectrogram(int, char *const *);
		~Spectrogram();
};

