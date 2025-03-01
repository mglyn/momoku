#ifndef CLIENT
#define CLIENT

#include "../../engine.h"
#include "widget.h"
#include "pipe.h"
#include "selfplay.h"

#include <ctime>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <windows.h>
#include<graphics.h>
#include <conio.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

class Client {

	static constexpr int width = 720;
	static constexpr int height = 920;
	static constexpr int board_size = 560;
	static constexpr int board_stx = (width - board_size) / 2, board_sty = height * 0.11;
	static constexpr int btnw = 0.7 / 6. * width, btnh = 30;
	int gameSize;
	int grid_size;

	Engine engine;
	Pipe<std::string> pipe;
	std::shared_ptr<Gdiplus::Graphics> graphics;
	std::map<std::string, int> settings;
	
	std::vector<Square> seq;

	//display widgets
	std::vector<Widget> buttons;
	std::vector<Widget> info;
	std::vector<Widget> cursors;
	std::vector<Widget> pieces;
	std::vector<Widget> highLight;
	std::vector<Widget> PV;
	std::vector<Widget> searchedMoves;
	std::vector<Widget> searchingMove;

	void Draw();
	void clearStats();
	void updateStats();
	std::string UpdateWidget();
	void init_search_update_listeners();

public:

	Client();
	void Loop();
};
#endif
