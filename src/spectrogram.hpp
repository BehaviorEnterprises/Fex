
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

		void draw_main();
		void draw_cursor(float, float);
		void draw_hud();
		void erase();

		void checkModKeys();
		void ev_handler(sf::Event);
		void ev_close(sf::Event);
		void ev_keypress(sf::Event);
		void ev_keyrelease(sf::Event);
		void ev_mousemove(sf::Event);
		void ev_resize(sf::Event);
		void ev_button(sf::Event);
		void ev_wheel(sf::Event);
	protected:
		void listen(float=1.0);
	public:
		int main_loop();
		Spectrogram(int, char *const *);
		~Spectrogram();
};

