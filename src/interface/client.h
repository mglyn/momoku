#ifndef CLIENT
#define CLIENT

#include<graphics.h>
#include<ctime>
#include <conio.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <windows.h>
#pragma comment(lib, "gdiplus.lib")

#include "../engine.h"

enum WidgetType :int {
	BlackStyle1,
	WhiteStyle1,
	BlackStyle2,
	WhiteStyle2,
	BlackStyle3,
	WhiteStyle3,
	SearchingMoveStyle1,
	SearchingMoveStyle2,
	EmptyCell,
	Cursor,
	ButtonStyle1,
	ButtonStyle2,
};

class Button {
	std::string text;
	bool isin = false, isclick = false, active = true;
	int x = 0, y = 0, w = 0, h = 0, type = 0;

	std::shared_ptr<Gdiplus::Graphics> graphics;
	void showText(char text[], int dx, int dy, int size, int thick);
	void showTextInMiddle(int size = 20, int thick = 2000);

public:
	Button(int _x, int _y, int _w, int _h, int _type,
		std::string _text, std::shared_ptr<Gdiplus::Graphics> _graphics) :
		x(_x), y(_y), w(_w), h(_h),
		type(_type), text(_text), graphics(_graphics) {
	}
	void set_type(int _type) { type = _type; }
	void set_text(std::string _text) { text = _text; }
	bool update(ExMessage msg);
	void show();
};

class Client {
	static constexpr int width = 720;
	static constexpr int height = 920;
	static constexpr int board_size = 560;
	static constexpr int board_stx = (width - board_size) / 2, board_sty = height * 0.11;
	static constexpr int btnw = 0.7 / 6. * width, btnh = 30;
	int gameSize;
	int grid_size;


	std::vector<Square> seq;
	std::vector<Square> PV;
	std::vector<Square> searchingMove;

	std::shared_ptr<Gdiplus::Graphics> graphics;

	std::map<std::string, int> settings;

	std::vector<Button> buttons;
	std::vector<Button> pieces;
	std::vector<Button> cells;
	std::vector<Button> info;

	ExMessage msg = {};

	void Draw();
	void set_pieces();
	std::string UpdateWidget();

	Engine engine;

	void init_search_update_listeners();

public:

	Client();

	void Loop();
};

#endif
