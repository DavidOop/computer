
#ifdef _DEBUG
#pragma comment(lib, "sfml-main-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-audio-d.lib")
#elif defined(NDEBUG)
#pragma comment(lib, "sfml-main.lib")
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-audio-d.lib")
#else
#error "Unrecognized configuration!"
#endif

#include "Controller.h"
#include <Windows.h>//Sleep

#define GAME_OVER -1
#define START_GAME 0
#define SETTINGS_SCREEN 1




//============================================================================
template<typename T>//this function draws the board for all vectors
void draw(sf::RenderWindow& window, const T & vec) {
	for (auto& tile : vec)
		tile->display(window);
}

//====================== main function =====================================================
int main() {

	srand(unsigned(time(NULL)));
	auto controller = std::make_unique<Controller>();
	controller->run();

	return 0;

}
/*****************************************************************************************
				basic Controller functions
**********************************************************************************************/
//===================== constructor ===================================
Controller::Controller() :m_fonts(), m_Menus(m_fonts) {
}
/************************************************************************
					Controller running functions

************************************************************************/
//=======================================================================
void menuWindow(sf::RenderWindow& window) {
	window.create(sf::VideoMode{ unsigned(SCREEN_WIDTH), unsigned(SCREEN_HEIGHT) }, "Agar.io");// , sf::Style::None);
}
//========================= run =====================================
void Controller::run() {

	sf::RenderWindow window;
	menuWindow(window);

	//for the drawMouse
	window.setJoystickThreshold(10000);

	// Menu window
	MenuEvents(window); //draw menu window and get events

}
//======================== while window is open =================================
void Controller::MenuEvents(sf::RenderWindow& window) {
	while(true)
	play(window);//if game is over (no levels left)

}
//========================= start playing ====================================
//if user pressed "Start"
void Controller::play(sf::RenderWindow& window) {
	sf::View view(sf::FloatRect{ 0, 0, float(SCREEN_WIDTH),float(SCREEN_HEIGHT) });
	Game game{ m_images, m_fonts,sf::Uint32(rand()%11)  ,view,"computer"  };
	auto score = game.play(window, m_images, m_fonts); //run current level
	Sleep(1000);
}
