
#include "fft.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Spectrogram : public Fft {
	private:
		enum Mode { MOVE_RESIZE };
		Mode mode = MOVE_RESIZE;
		sf::RenderWindow win;
		sf::View view;
		sf::Vector2f mouse;
		sf::RectangleShape back;
		sf::Texture ball;
		float aspect;

		void draw_main();
		void draw_cursor();
		void draw_hud();

		void ev_handler(sf::Event);
		void ev_close(sf::Event);
		void ev_keypress(sf::Event);
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

