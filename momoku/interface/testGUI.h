#ifndef TESTGUI
#define TESTGUI

#include<graphics.h>
#include<ctime>
#include <conio.h>
#include <gdiplus.h>
#include <vector>
#include <iostream>
#include <map>
#pragma comment(lib, "gdiplus.lib")

#include "../position.h"
#include "../search.h"
#include "../test.h"

class Button {
	std::string text;
	bool isin = 0, isclick = 0;
	int x = 0, y = 0, w = 0, h = 0, type = 0;

	std::shared_ptr<Gdiplus::Graphics> graphics;
	void showText(char text[], int dx, int dy, int size, int thick) {
		LOGFONT f;
		gettextstyle(&f);
		f.lfHeight = size;
		f.lfWeight = thick;
		settextstyle(&f);
		outtextxy(dx + x, dy + y, text);
	}
	void showTextInMiddle(int size = 20, int thick = 2000) {
		settextcolor(BLACK);
		settextstyle(size, 0, "微软雅黑");
		int width1 = x + (w - textwidth(text.c_str())) / 2;
		int height1 = y + (h - textheight(text.c_str())) / 2;
		outtextxy(width1, height1, text.c_str());
	}

public:
	Button(int _x, int _y, int _w, int _h, int _type, std::string _text, std::shared_ptr<Gdiplus::Graphics> _graphics) :
		x(_x), y(_y), w(_w), h(_h), type(_type), text(_text), graphics(_graphics) {}

	void set_type(int _type) { type = _type; }
	void set_text(std::string _text) { text = _text; }
	bool update(ExMessage msg) {
		bool up = 0;
		isin = msg.x >= x && msg.x <= x + w && msg.y >= y && msg.y <= y + h;
		if (isin && msg.message == WM_LBUTTONDOWN) isclick = 1;
		if (!isin) isclick = 0;
		int oldone = isclick;
		if (isin && msg.message == WM_LBUTTONUP) {
			isclick = 0;
			up = oldone && !isclick;
		}
		return up;
	}
	bool show(ExMessage msg) {
		bool play = !isclick;
		bool up = update(msg);
		setlinecolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		if (type == 0) {				//黑棋
			Gdiplus::SolidBrush Pen_b(Gdiplus::Color(100, 100, 100));
			graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
			Gdiplus::Pen Pen_B(Gdiplus::Color(50, 50, 50), 2.f);
			graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == 1) {			//白棋
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(255, 255, 255));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
			Gdiplus::Pen Pen_W(Gdiplus::Color(165, 165, 165), 2.f);
			graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == 2) {			//空位
		}
		else if (type == 3) {	//黑棋 high_lighted
			Gdiplus::Pen Pen_R(Gdiplus::Color(200, 120, 120), 6.f);
			graphics->DrawEllipse(&Pen_R, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			Gdiplus::SolidBrush Pen_b(Gdiplus::Color(100, 100, 100));
			graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
			Gdiplus::Pen Pen_B(Gdiplus::Color(50, 50, 50), 2.f);
			graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == 4) {	//白棋 high_lighted
			Gdiplus::Pen Pen_R(Gdiplus::Color(200, 120, 120), 6.f);
			graphics->DrawEllipse(&Pen_R, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(255, 255, 255));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
			Gdiplus::Pen Pen_W(Gdiplus::Color(165, 165, 165), 2.f);
			graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == 5) {	//黑棋 pv
			Gdiplus::SolidBrush Pen_b(Gdiplus::Color(100, 100, 100));
			graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
			Gdiplus::Pen Pen_B(Gdiplus::Color(165, 200, 185), 4.f);
			graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == 6) {	//白棋 pv
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(255, 255, 255));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
			Gdiplus::Pen Pen_W(Gdiplus::Color(165, 200, 185), 4.f);
			graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == 7) {			//按键
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				Gdiplus::SolidBrush Pen_W(Gdiplus::Color(205, 205, 205));
				graphics->FillRectangle(&Pen_W, Gdiplus::Rect{ x, y, w, h });
				Gdiplus::SolidBrush Pen_w(Gdiplus::Color(180, 180, 180));
				graphics->FillRectangle(&Pen_w, Gdiplus::Rect{ x + 1, y + 1, w - 2, h - 2 });
			}
			else if (isclick) {
				Gdiplus::SolidBrush Pen_W(Gdiplus::Color(230, 230, 230));
				graphics->FillRectangle(&Pen_W, Gdiplus::Rect{ x, y, w, h });
				Gdiplus::SolidBrush Pen_w(Gdiplus::Color(200, 200, 200));
				graphics->FillRectangle(&Pen_w, Gdiplus::Rect{ x + 1, y + 1, w - 2, h - 2 });
			}
			else {
				Gdiplus::SolidBrush Pen_W(Gdiplus::Color(190, 190, 190));
				graphics->FillRectangle(&Pen_W, Gdiplus::Rect{ x, y, w, h });
				Gdiplus::SolidBrush Pen_w(Gdiplus::Color(170, 170, 170));
				graphics->FillRectangle(&Pen_w, Gdiplus::Rect{ x + 1, y + 1, w - 2, h - 2 });
			}
			showTextInMiddle(20, 300);
		}
		else if (type == 8) {			//透明信息栏
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				setfillcolor(RGB(255, 254, 215));
				fillroundrect(x, y, x + w + 1, y + h + 1, 10, 10);
			}
			else if (isclick) {
				setfillcolor(RGB(255, 255, 235));
				fillroundrect(x, y, x + w + 1, y + h + 1, 10, 10);
			}
		}
		if (type >= 0 && type <= 6) {			//光标
			if (isin && !isclick) {
				Gdiplus::Pen Pen_1(Gdiplus::Color(160, 180, 210), 2.f);
				graphics->DrawEllipse(&Pen_1, Gdiplus::Rect{ x + 2, y + 2, w - 4, w - 4 });
			}
			else if (isclick) {
				Gdiplus::Pen Pen_2(Gdiplus::Color(205, 225, 235), 2.f);
				graphics->DrawEllipse(&Pen_2, Gdiplus::Rect{ x + 2, y + 2, w - 4, w - 4 });
			}
		}
		showTextInMiddle(20, 300);

		return up;
	}
};

class GUI {
	static constexpr int width = 750;
	static constexpr int height = 900;
	static constexpr int board_lenth = 600;
	static constexpr int btnw = 90, btnh = 30;
	int gameSize;
	int grid_size = board_lenth / gameSize;
	int stx = (width - grid_size * (gameSize - 1)) / 2;
	int sty = -height * 0.03 + (height - grid_size * (gameSize - 1)) / 2;
	std::shared_ptr<Gdiplus::Graphics> graphics;

	std::vector<Button> buttons;
	std::vector<Button> pieces;

	ExMessage msg;

public:

	int Update() {

		::peekmessage(&msg, EM_MOUSE);
		int ops = -1;

		BeginBatchDraw();
		cleardevice();

		//1. 绘制棋盘
		//绘制棋盘位置标号
		setlinestyle(PS_SOLID, 1);
		settextcolor(BLACK);
		settextstyle(18, 0, "微软雅黑");
		for (int i = 0; i < gameSize; i++) {
			char c[4] = "";
			sprintf_s(c, 3, "%d", i);
			::outtextxy(stx + i * grid_size - 4, sty - grid_size, c);
			sprintf_s(c, 3, "%c", i + 'A');
			::outtextxy(stx - grid_size, sty + i * grid_size - 9, c);
		}
		//画线
		Gdiplus::Pen Pen_1(Gdiplus::Color(0, 0, 0), 1.f);
		for (int i = 0; i < gameSize; i++) {
			Gdiplus::Point st(stx, sty + i * grid_size), ed(stx + (gameSize - 1) * grid_size, sty + i * grid_size);
			graphics->DrawLine(&Pen_1, st, ed);
			st = Gdiplus::Point(stx + i * grid_size, sty), ed = Gdiplus::Point(stx + i * grid_size, sty + (gameSize - 1) * grid_size);
			graphics->DrawLine(&Pen_1, st, ed);
		}

		//2.绘制棋子
		for (int i = 0; i < pieces.size(); i++) {
			if (pieces[i].show(msg)) {
				ops = i;
			}
		}

		//3.绘制按钮
		for (int i = 0; i < buttons.size(); i++) {
			if (buttons[i].show(msg)) {
				ops = i + 666;
			}
		}

		EndBatchDraw();

		return ops;
	}

	void SetPiece(int x, int y, int type, std::string text) {
		pieces[x * gameSize + y].set_type(type);
		pieces[x * gameSize + y].set_text(text);
	}

	void SetButton(int x, int y, std::string text) {
		Button tmp(int(width * 0.15 + y * btnw), int(height * 0.82 + x * btnh), btnw, btnh, 7, text, graphics);
		buttons[x * 6 + y] = tmp;
	}

	GUI(int _gameSize) : gameSize(_gameSize) {

		initgraph(width, height, SHOWCONSOLE);
		graphics = std::make_shared<Gdiplus::Graphics>(GetImageHDC());
		//背景
		setbkcolor(RGB(220, 220, 210));

		//字体
		LOGFONT f;
		gettextstyle(&f);
		_tcscpy_s(f.lfFaceName, _T("微软雅黑"));
		f.lfQuality = ANTIALIASED_QUALITY;
		settextstyle(&f);
		setbkmode(TRANSPARENT);

		//初始化GDI
		Gdiplus::GdiplusStartupInput Input;
		ULONG_PTR Token;
		Gdiplus::GdiplusStartup(&Token, &Input, NULL);
		// 设置绘图质量为高质量
		graphics->SetSmoothingMode(Gdiplus::SmoothingMode::
			SmoothingModeHighQuality);

		//按键
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 3; j++) {
				buttons.emplace_back(int(width * 0.15 + i * btnw), int(height * 0.82 + j * btnh), btnw, btnh, 3, "", graphics);
			}
		}

		//棋子
		for (int i = 0; i < gameSize; i++) {
			for (int j = 0; j < gameSize; j++) {
				int pSize = grid_size - 4;
				pieces.emplace_back(stx + i * grid_size - pSize / 2,
					sty + j * grid_size - pSize / 2, pSize, pSize, 2, "", graphics);
			}
		}
	}
};

class Client {
	int gameSize;
	Position pos;
	std::vector<StateInfo> stList;
	std::vector<Sqare> PV;
	GUI gui;
	std::map<std::string, bool> mp;

	void renderCall() {	
		for (int i = 0; i < gameSize; i++) {
			for (int j = 0; j < gameSize; j++) {
				gui.SetPiece(j, i, Empty, "");
			}
		}
		for (int i = 1; i <= pos.cntMove(); i++) {
			int x = stList[i].move.x(), y = stList[i].move.y();
			if (i < pos.cntMove()) gui.SetPiece(y, x, (i + 1) % 2, std::to_string(i));
			else gui.SetPiece(y, x, (i + 1) % 2 + 3, std::to_string(i));
		}
		if (mp["PV"]) {
			for (int i = 1; i < PV.size(); i++) {
				int x = PV[i].x(), y = PV[i].y();
				gui.SetPiece(y, x, (i + pos.cntMove() - 1) % 2 + 5, std::to_string(i));
			}
		}
	}

	void consoleOutputInfo() {
		std::cout << "valueP1:" << stList[pos.cntMove()].valueP1 << '\n';
		std::cout << "staticEval:" << pos.staticEval() << '\n';
		std::cout << "P1 value:\n";
		for (int i = 0; i < 15; i++) {
			for (int j = 0; j < 15; j++) {
				if (pos[{i, j}] == Empty)
					std::cout << pos.value(P1, { i, j }) << "\t";
				else std::cout << "-\t";
			}
			std::cout << "\n";
		}
		std::cout << "P1 type:\n";
		for (int i = 0; i < 15; i++) {
			for (int j = 0; j < 15; j++) {
				if (pos[{i, j}] == Empty)
					std::cout << (int)pos.type(P1, { i, j }) << "\t";
				else std::cout << "-\t";
			}
			std::cout << "\n";
		}
		std::cout << "\n";

		std::cout << "P1 threats: \n";
		std::cout << "TH3:" << (int)stList[pos.cntMove()].cntT[TH3][P1] << " ";
		std::cout << "TDH3:" << (int)stList[pos.cntMove()].cntT[TDH3][P1] << " ";
		std::cout << "T4H3:" << (int)stList[pos.cntMove()].cntT[T4H3][P1] << " ";
		std::cout << "T4:" << (int)stList[pos.cntMove()].cntT[T4][P1] << " ";
		std::cout << "TH4:" << (int)stList[pos.cntMove()].cntT[TH4][P1] << " ";
		std::cout << "T5:" << (int)stList[pos.cntMove()].cntT[T5][P1] << " ";
		std::cout << "\n";

		std::cout << "P2 threats: \n";
		std::cout << "TH3:" << (int)stList[pos.cntMove()].cntT[TH3][P2] << " ";
		std::cout << "TDH3:" << (int)stList[pos.cntMove()].cntT[TDH3][P2] << " ";
		std::cout << "T4H3:" << (int)stList[pos.cntMove()].cntT[T4H3][P2] << " ";
		std::cout << "T4:" << (int)stList[pos.cntMove()].cntT[T4][P2] << " ";
		std::cout << "TH4:" << (int)stList[pos.cntMove()].cntT[TH4][P2] << " ";
		std::cout << "T5:" << (int)stList[pos.cntMove()].cntT[T5][P2] << " ";
		std::cout << "\n";
	}

public:
	Client(int gameSize) :gameSize(gameSize), gui(gameSize), pos(gameSize) {

		stList.resize(MAX_MOVE);
		pos.set(stList[pos.cntMove()]);

		char strplay[3][6][32] = { "PV","计算","撤销","重置","调试信息" };
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 6; j++) {
				gui.SetButton(i, j, strplay[i][j]);
			}
		}
	}

	void Run() {

		while (true) {
			renderCall();

			int op = gui.Update();
			int y = op / gameSize, x = op % gameSize;

			if (x < gameSize && y < gameSize && x >= 0 && y >= 0 && pos[Sqare(x, y)] == Empty) {

				pos.make_move(Sqare(x, y), stList[pos.cntMove() + 1]);
				PV.clear();
			}

			op -= 666;

			if (op == 0) {
				mp["PV"] = !mp["PV"];
			}
			else if (op == 1) {//AI落子
				PV = std::get<std::vector<Sqare>>(Search::search(pos));

				if (PV.size()) {
					pos.make_move(PV[0], stList[pos.cntMove() + 1]);
				}

				PrintTest();
			}
			else if (op == 2) {//撤销
				if (pos.cntMove()) {
					pos.undo();
					PV.clear();
				}
			}
			else if (op == 3) {//重置
				pos.set(stList[0]);
				PV.clear();
			}
			else if (op == 4) {//consoleInfo
				consoleOutputInfo();
			}
		}
	}
};

#endif
