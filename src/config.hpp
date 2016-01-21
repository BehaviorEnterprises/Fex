
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class Config {
	protected:
		struct {
			int winlen           = 256;
			int hop              = 64;
			double lopass        = 01.25;
			double hipass        = 10.00;
			double threshold     = 20.00;
			double floor         = 24.00;
			sf::Color specFG     = sf::Color::Black;
			sf::Color specBG     = sf::Color::White;
			sf::Color winBG      = sf::Color(200,200,210);
			sf::Color threshFG   = sf::Color(0,128,255,64);
		} conf;
		struct {
			bool cursor       = false;
			bool hud          = true;
			bool overlay      = true;
		} show;
		sf::String fname;

	public:
		Config(int, const char **);
};

