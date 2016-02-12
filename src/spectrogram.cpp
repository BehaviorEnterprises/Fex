
#include "spectrogram.hpp"

Spectrogram::Spectrogram(int argc, char *const *argv) : Fft(argc, argv) {
//	if (getenv("FEX_FULLSCREEN"))
		win.create(sf::VideoMode::getDesktopMode(), "Fex", sf::Style::Fullscreen);
//	else
//		win.create(sf::VideoMode(640,480), "Fex");
	aspect = (ntime * win.getSize().y * conf.hop) / (float) (nfreq * win.getSize().x * conf.winlen);
	view.reset(sf::FloatRect(0, -nfreq * (1 + aspect)/2.0, ntime, nfreq * aspect));
	win.setView(view);
	spec.setColor(conf.specFG);
	sf::RenderTexture render_ball;
	render_ball.setSmooth(true);
	render_ball.create(64,64);
	sf::CircleShape dot(32);
	dot.setFillColor(conf.pointBG);
	dot.setOutlineColor(conf.pointFG);
	dot.setOutlineThickness(-8);
	render_ball.draw(dot);
	ball = render_ball.getTexture();
	back.setSize(sf::Vector2f(ntime,-nfreq));
	back.setFillColor(conf.specBG);
}

Spectrogram::~Spectrogram() { }

int Spectrogram::main_loop() {
	sf::Event ev;
	while (win.isOpen() && win.waitEvent(ev)) {
		ev_handler(ev);
		while (win.pollEvent(ev)) ev_handler(ev);
		draw_main();
		if (show.cursor) draw_cursor();
		win.display();
		char out[256];
		snprintf(out,255,"%s - (%0.3fs, %0.3fKHz) FE: %lf ",
			name,
			song.getDuration().asSeconds() * mouse.x / ntime,
			conf.lopass - (conf.hipass - conf.lopass) * mouse.y / nfreq,
			1234.5
		);
		win.setTitle(sf::String(out));
	}
	printf("%lf\t%lf\t%lf\n", pathLength, timeLength, pathLength / timeLength);
	return 0;
}

void Spectrogram::ev_handler(sf::Event ev) {
	switch (ev.type) {
		case sf::Event::Closed: ev_close(ev); break;
		case sf::Event::KeyPressed: ev_keypress(ev); break;
		case sf::Event::MouseMoved: ev_mousemove(ev); break;
		case sf::Event::MouseButtonPressed: ev_button(ev); break;
		case sf::Event::MouseWheelScrolled: ev_wheel(ev); break;
		case sf::Event::Resized: ev_resize(ev); break;
	}
}

void Spectrogram::draw_main() {
	win.clear(conf.winBG);
	win.draw(back);
	win.draw(spec);
	thresh.setColor(conf.threshFG);
	if (show.overlay) {
		win.draw(thresh);
		win.draw(getPoints(), &ball);
		win.draw(getLines());
	}
}

void Spectrogram::draw_cursor() {
	sf::RectangleShape lineV(sf::Vector2f(1, nfreq));
	lineV.setFillColor(sf::Color(255,0,0,120));
	lineV.setPosition(view.getViewport().left + mouse.x, -nfreq);
	win.draw(lineV);
	sf::RectangleShape lineH(sf::Vector2f(ntime, 1));
	lineH.setFillColor(sf::Color(255,0,0,120));
	lineH.setPosition(0,view.getViewport().top + mouse.y);
	win.draw(lineH);
}

void Spectrogram::listen(float speed) {
	sf::Sound snd(song);
	snd.play();
	snd.setPitch(speed);
	while(snd.getStatus() == 2) {
		sf::RectangleShape line(sf::Vector2f(10, nfreq));
		line.setFillColor(sf::Color(0,255,0,120));
		//line.setPosition((float) view.getSize().x * snd.getPlayingOffset().asSeconds() / song.getDuration().asSeconds(), -nfreq);
		line.setPosition(ntime * (snd.getPlayingOffset() / song.getDuration()), -nfreq);
		draw_main();
		win.draw(line);
	//	if (show.hud) draw_hud();
		win.display();
	}
}

void Spectrogram::ev_close(sf::Event ev) {
}

void Spectrogram::ev_keypress(sf::Event ev) {
	if (ev.key.control) {
		if (ev.key.code == sf::Keyboard::Q) win.close();
		else if (ev.key.code == sf::Keyboard::Right) { conf.floor -= 0.25; makeSpectrogram(); }
		else if (ev.key.code == sf::Keyboard::Left) { conf.floor += 0.25; makeSpectrogram(); }
		else if (ev.key.code == sf::Keyboard::Up) { conf.threshold -= 0.25;  /* redraw overlay */ }
		else if (ev.key.code == sf::Keyboard::Down) { conf.threshold += 0.25;  /* redraw overlay */ }
	}
	else if (ev.key.alt) {
		if (ev.key.code == sf::Keyboard::C) win.setMouseCursorVisible(!(show.cursor=!show.cursor));
		if (ev.key.code == sf::Keyboard::H) show.hud = !show.hud;
		if (ev.key.code == sf::Keyboard::O) show.overlay = !show.overlay;
		if (ev.key.code == sf::Keyboard::Num1) listen(1.0);
		if (ev.key.code == sf::Keyboard::Num2) listen(0.5);
		if (ev.key.code == sf::Keyboard::Num3) listen(0.333);
		if (ev.key.code == sf::Keyboard::Num4) listen(0.25);
		if (ev.key.code == sf::Keyboard::Num5) listen(0.2);
	}
}

void Spectrogram::ev_resize(sf::Event ev) {
	aspect = (ntime * win.getSize().y * conf.hop) / (float) (nfreq * win.getSize().x * conf.winlen);
}

void Spectrogram::ev_mousemove(sf::Event ev) {
	sf::Vector2f prev = mouse;
	mouse = win.mapPixelToCoords(sf::Vector2i(ev.mouseMove.x,ev.mouseMove.y));
	if (mode == MOVE_RESIZE) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
			view.setSize(view.getSize().x + prev.x - mouse.x, view.getSize().y + prev.y - mouse.y);
			view.move((prev.x - mouse.x)/2.0, (prev.y - mouse.y)/2.0);
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			view.move(prev.x - mouse.x, prev.y - mouse.y);
		}
		win.setView(view);
		mouse = win.mapPixelToCoords(sf::Vector2i(ev.mouseMove.x,ev.mouseMove.y));
	}
	if (mouse.x < 0) mouse.x = 0.0;
	else if (mouse.x > ntime) mouse.x = ntime;
	if (mouse.y > 0) mouse.y = 0.0;
	else if (mouse.y < - nfreq) mouse.y = - nfreq;
}

void Spectrogram::ev_button(sf::Event ev) {
	switch (ev.mouseButton.button) {
		case sf::Mouse::Button::Middle:
			view.reset(sf::FloatRect(0, -nfreq * (1 + aspect)/2.0, ntime, nfreq * aspect));
			break;
	}
}

void Spectrogram::ev_wheel(sf::Event ev) {
	if (ev.mouseWheelScroll.wheel == 0) { /* vertical */
		// TODO, make 0.0075 step size customizable?
		float vx = view.getSize().x / ntime, vy = view.getSize().y / nfreq;
		float dx = ev.mouseWheelScroll.delta;
		if (dx < 0 && vx > 1.20 && vy > 1.20) return;
		if (dx > 0 && vx < 0.01 && vy > 0.01) return;
		view.zoom(1.0 - 0.0075 * ev.mouseWheelScroll.delta);
		win.setView(view);
	}
}

/*
void Spectrogram::draw_hud() {
	float w = view.getSize().x, h = view.getSize().y;
	float x = view.getCenter().x - w / 2.0, y = view.getCenter().y - h / 2.0;
	sf::RectangleShape shape(sf::Vector2f(w, h * 0.036));
	shape.setPosition(x, y);
	shape.setFillColor(sf::Color(0, 0, 128,100));
	win.draw(shape);
	char out[256];
	sf::Text text;
	text.setFont(font); text.setCharacterSize(28); text.setColor(sf::Color(255,255,255,200)); text.setScale(w/1000,h/1000);
	snprintf(out,255," FE: %0.3f", 123.456789);
	text.setString(out);
	text.setPosition(x, y);
	win.draw(text);
	snprintf(out,255,"(%0.3fs, %0.3fKHz) ",
		song.getDuration().asSeconds() * mouse.x / ntime,
		conf.lopass - (conf.hipass - conf.lopass) * mouse.y / nfreq
	);
	text.setString(out);
	text.setPosition(x + w - text.getGlobalBounds().width, y);
	win.draw(text);
}
*/

