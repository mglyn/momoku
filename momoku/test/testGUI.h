#ifndef TESTGUI
#define TESTGUI

#include<graphics.h>
#include<ctime>
#include <conio.h>
#include <gdiplus.h>
#include <vector>
#include <iostream>
#pragma comment(lib, "gdiplus.lib")

#include "../common/common.h"
#include "../game/board.h"
#include "../search/search.h"
#include "test.h"

class button {
	char text[30] = {};
	bool isin = 0, isclick = 0;
	int mx = 0, my = 0, mw = 0, mh = 0, mtype = 0;

public:
	void set_text(char s[], int size) {
		memcpy(text, s, size);
	}
	void set(int x, int y, int w, int h, int type) {
		mx = x, my = y, mw = w, mh = h, mtype = type;
	}
	bool update(ExMessage& msg) {
		bool up = 0;
		isin = msg.x >= mx && msg.x <= mx + mw && msg.y >= my && msg.y <= my + mh;
		if (isin && msg.message == WM_LBUTTONDOWN) isclick = 1;
		if (!isin) isclick = 0;
		int oldone = isclick;
		if (isin && msg.message == WM_LBUTTONUP) {
			isclick = 0;
			up = oldone && !isclick;
		}
		return up;
	}
	bool show(ExMessage& msg) {
		bool play = !isclick;
		bool up = update(msg);
		setlinecolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		if (mtype == 1) {
			Gdiplus::Graphics Graphics(GetImageHDC());
			Gdiplus::Pen Pen_1(Gdiplus::Color(160, 180, 210), 2.f);
			Gdiplus::Pen Pen_2(Gdiplus::Color(205, 225, 235), 2.f);
			Graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
			if (isin && !isclick) {
				Graphics.DrawEllipse(&Pen_1, Gdiplus::Rect{ mx - 1, my - 1, mw + 2,mw + 2 });
			}
			else if (isclick) {
				Graphics.DrawEllipse(&Pen_2, Gdiplus::Rect{ mx - 1, my - 1, mw + 2,mw + 2 });
			}
		}
		else if (mtype == 2) {
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				setfillcolor(RGB(255, 254, 215));
				fillroundrect(mx, my, mx + mw + 1, my + mh + 1, 10, 10);
			}
			else if (isclick) {
				setfillcolor(RGB(255, 255, 235));
				fillroundrect(mx, my, mx + mw + 1, my + mh + 1, 10, 10);
			}
		}
		else {
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				setfillcolor(RGB(148, 110, 20));
				fillroundrect(mx, my, mx + mw + 1, my + mh + 1, 10, 10);
				setfillcolor(RGB(200, 180, 120));
				fillroundrect(mx + 1, my + 1, mx + mw - 1, my + mh - 1, 10, 10);
			}
			else if (isclick) {
				int shift = 3;
				setfillcolor(RGB(148, 110, 20));
				fillroundrect(mx + 2 * shift, my + shift, (mx + mw) - 2 * shift, (my + mh) - shift, 10, 10);
				setfillcolor(RGB(170, 150, 100));
				fillroundrect(mx + 2 * shift + 1, my + shift + 1, mx + mw - 2 * shift - 1, my + mh - shift - 1, 10, 10);
			}
			else {
				setfillcolor(RGB(240, 220, 170));
				fillroundrect(mx + 1, my + 1, mx + mw - 1, my + mh - 1, 10, 10);
			}
			showTextInMiddle(20, 300);
		}
		return up;
	}
	void showText(char text[], int dx, int dy, int size, int thick) {
		LOGFONT f;
		gettextstyle(&f);
		f.lfHeight = size;
		f.lfWeight = thick;
		settextstyle(&f);
		outtextxy(dx + mx, dy + my, text);
	}
	void showTextInMiddle(int size = 20, int thick = 2000) {
		settextcolor(BLACK);
		settextstyle(size, 0, "微软雅黑");
		int width1 = mx + (mw - textwidth(text)) / 2;
		int height1 = my + (mh - textheight(text)) / 2;
		outtextxy(width1, height1, text);
	}
};

class GUI {
	static constexpr int width = 750;
	static constexpr int height = 800;
	static constexpr int row = 15;
	static constexpr int col = 15;
	static constexpr int grid_size = 36;
	static constexpr int start_x = (width - grid_size * (col - 1)) / 2;
	static constexpr int start_y = -height * 0.03 + (height - grid_size * (row - 1)) / 2;

	Board bd = Board(15);
	bool PV = false;

	button btnplay[5];
	button infoo;
	button btnGrid[15][15];

	std::vector<Pos> answer;

	void draw_board() {
		Gdiplus::Graphics Graphics(GetImageHDC());
		Gdiplus::Pen Pen_1(Gdiplus::Color(0, 0, 0), 1.f);
		Gdiplus::Pen Pen_2(Gdiplus::Color(0, 0, 0), 2.f);
		// 设置绘图质量为高质量
		Graphics.SetSmoothingMode(Gdiplus::SmoothingMode::
			SmoothingModeHighQuality);
		//绘制棋盘位置标号
		setlinestyle(PS_SOLID, 1);
		settextcolor(BLACK);
		settextstyle(20, 0, "微软雅黑");
		char c[3] = "";
		for (int i = 1; i <= 9; i++) {
			sprintf_s(c, 3, "%d", i);
			::outtextxy(start_x + (i - 1) * grid_size - 4, start_y - grid_size, c);
		}
		for (int i = 10; i <= 15; i++) {
			sprintf_s(c, 3, "%d", i);
			::outtextxy(start_x + (i - 1) * grid_size - 8, start_y - grid_size, c);
		}
		for (int i = 1; i <= 9; i++) {
			sprintf_s(c, 3, "%c", i + 'A' - 1);
			::outtextxy(start_x - grid_size + 5, start_y + (i - 1) * grid_size - 12, c);
		}
		for (int i = 10; i <= 15; i++) {
			sprintf_s(c, 3, "%c", i + 'A' - 1);
			::outtextxy(start_x - grid_size + 5, start_y + (i - 1) * grid_size - 12, c);
		}
		//画线
		for (int i = 0; i < row; i++) {
			Graphics.DrawLine(&Pen_1, start_x, start_y + i * grid_size, start_x + (col - 1) * grid_size, start_y + i * grid_size);
		}
		for (int i = 0; i < col; i++) {
			Graphics.DrawLine(&Pen_1, start_x + i * grid_size, start_y, start_x + i * grid_size, start_y + (row - 1) * grid_size);
		}
		//外圈线加粗
		Graphics.DrawLine(&Pen_2, start_x, start_y, start_x + (col - 1) * grid_size, start_y);
		Graphics.DrawLine(&Pen_2, start_x, start_y + 14 * grid_size, start_x + (col - 1) * grid_size, start_y + 14 * grid_size);
		Graphics.DrawLine(&Pen_2, start_x, start_y, start_x, start_y + (row - 1) * grid_size);
		Graphics.DrawLine(&Pen_2, start_x + 14 * grid_size, start_y, start_x + 14 * grid_size, start_y + (row - 1) * grid_size);
		//绘制黑点
		setfillcolor(BLACK);
		solidcircle(start_x + 7 * grid_size, start_y + 7 * grid_size, 2);
		solidcircle(start_x + 3 * grid_size, start_y + 3 * grid_size, 2);
		solidcircle(start_x + 11 * grid_size, start_y + 11 * grid_size, 2);
		solidcircle(start_x + 3 * grid_size, start_y + 11 * grid_size, 2);
		solidcircle(start_x + 11 * grid_size, start_y + 3 * grid_size, 2);
	}

	void draw_piece(Piece p, Pos pos, bool vir, int label, int labelColor) {
		Gdiplus::Graphics Graphics(GetImageHDC());
		int t = 255 - vir * 30;
		Gdiplus::Pen Pen_W(Gdiplus::Color(t - 20, t - 20, t - 20), 17.f);
		Gdiplus::Pen Pen_w(Gdiplus::Color(t - vir * 20, t - vir * 20, t), 16.f);
		t = 0 + vir * 70;
		Gdiplus::Pen Pen_B(Gdiplus::Color(t + 80, t + 80, t + 80), 17.f);
		Gdiplus::Pen Pen_b(Gdiplus::Color(t, t, t + vir * 20), 16.f);
		Graphics.SetSmoothingMode(Gdiplus::SmoothingMode::
			SmoothingModeHighQuality);
		settextstyle(20, 0, "微软雅黑");
		int y = pos.x(), x = pos.y();

		if (p == P1) {
			int r = grid_size - 18, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_B, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子外圈
			r = grid_size - 20, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_b, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子内圈
		}
		else if (p == P2) {
			int r = grid_size - 18, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_W, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子外圈
			r = grid_size - 20, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_w, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子内圈
		}

		TCHAR s[8];
		_stprintf_s(s, _T("%d"), label);
		//标号居中
		int width = start_x + x * grid_size - grid_size / 2 - 1 + (grid_size - textwidth(s)) / 2;
		int height = start_y + y * grid_size - grid_size / 2 - 1 + (grid_size - textheight(s)) / 2;
		settextcolor(labelColor);
		outtextxy(width, height, s);
	}

	bool Isin(int x, int y) { return x < 15 && y < 15 && x >= 0 && y >= 0; }
public:
	GUI() {
		initgraph(width, height, SHOWCONSOLE);
		//initgraph(width, height);
		setbkcolor(RGB(255, 248, 196));
		LOGFONT f;
		gettextstyle(&f);
		_tcscpy_s(f.lfFaceName, _T("微软雅黑"));
		f.lfQuality = ANTIALIASED_QUALITY;
		settextstyle(&f);
		setbkmode(TRANSPARENT);
		cleardevice();
		Gdiplus::GdiplusStartupInput Input;
		ULONG_PTR Token;
		Gdiplus::GdiplusStartup(&Token, &Input, NULL);

		char strplay[5][30] = { "PV","计算","撤销","重置","" };
		int btnw = 90, btnh = 30;
		for (int i = 0; i < 5; i++) {
			btnplay[i].set(width / 2 - 5 * (btnw + 4) / 2 + i * (btnw + 4), height * 0.88, btnw, btnh, 3);
			btnplay[i].set_text(strplay[i], 30);
		}
		infoo.set(width / 2 - 14 * grid_size / 2, start_y + 14.5 * grid_size, 14 * grid_size, btnh, 2);
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				btnGrid[i][j].set(start_x + i * grid_size - grid_size / 2, start_y + j * grid_size - grid_size / 2, grid_size - 2, grid_size - 2, 1);
			}
		}
		/*for (int i = 0; i < 7; i++) {
			for (int k = 0; k < 3; k++) {
				makeMove(2 * i, k * 5);
				makeMove(2 * i + 1, k * 5);
				makeMove(2 * i, 1 + k * 5);
				makeMove(2 * i + 1, 1 + k * 5);
				makeMove(2 * i, 2 + k * 5);
				makeMove(2 * i + 1, 2 + k * 5);
				makeMove(2 * i, 3 + k * 5);
				makeMove(2 * i + 1, 3 + k * 5);
			}
			makeMove(2 * i + 1, 4);
			makeMove(2 * i, 4);
			makeMove(2 * i + 1, 9);
			makeMove(2 * i, 9);
			makeMove(2 * i + 1, 14);
			makeMove(2 * i, 14);
		}*/
	}

	void Run() {

		while (1) {
			Sleep(5);
			BeginBatchDraw();
			cleardevice();
			draw_board();
			int op = -1;
			ExMessage msg;
			while(::peekmessage(&msg, EM_MOUSE));
			int y = (msg.x - start_x + grid_size / 2) / grid_size;
			int x = (msg.y - start_y + grid_size / 2) / grid_size;
			if (Isin(x, y))btnGrid[y][x].show(msg);

			for (int i = bd.cntMove(); i >= 1; i--) {
				Pos pos = bd.lastMove(i);
				Piece self = i & 1 ? ~bd.self() : bd.self();
				draw_piece(self, pos, false, bd.cntMove() - i + 1, i == 1? RGB(250, 100, 100) : self == P1 ? WHITE : BLACK);
			}

			if (PV) {
				for (int i = 1; i < answer.size(); i++) {
					Piece self = i & 1 ? bd.self() : ~bd.self();
					draw_piece(self, answer[i], true, bd.cntMove() + i, GREEN);
				}
			}
			

			if (msg.message == WM_LBUTTONDOWN) {
				if (Isin(x, y) && bd[Pos(x, y)] == Empty) {
					bd.update(Pos(x, y));
					answer.clear();

					/*std::cout << bd.bdnf().valueP1 << '\n';
					std::cout << bd.staticEval() << '\n';
					for (int i = 0; i < 15; i++) {
						for (int j = 0; j < 15; j++) {
							if (bd[{i, j}] == Empty)
								std::cout << bd.value(P1, { i, j }) << "\t";
							else std::cout << "-\t";
						}
						std::cout << "\n";
					}
					std::cout << "\n";*/
					/*for (int i = 0; i < 15; i++) {
						for (int j = 0; j < 15; j++) {
							if (bd[{i, j}] == Empty)
								std::cout << bd.value(P2, { i, j }) << "\t";
							else std::cout << "-\t";
						}
						std::cout << "\n";
					}*/


					std::cout << "TH3:" << bd.cntFT(TH3, P1) << " ";
					std::cout << "TDH3:" << bd.cntFT(TDH3, P1) << " ";
					std::cout << "T4H3:" << bd.cntFT(T4H3, P1) << " ";
					std::cout << "T4:" << bd.cntFT(T4, P1) << " ";
					std::cout << "TH4:" << bd.cntFT(TH4, P1) << " ";
					std::cout << "T5:" << bd.cntFT(T5, P1) << " ";
					std::cout << "\n";

					std::cout << "TH3:" << bd.cntFT(TH3, P2) << " ";
					std::cout << "TDH3:" << bd.cntFT(TDH3, P2) << " ";
					std::cout << "T4H3:" << bd.cntFT(T4H3, P2) << " ";
					std::cout << "T4:" << bd.cntFT(T4, P2) << " ";
					std::cout << "TH4:" << bd.cntFT(TH4, P2) << " ";
					std::cout << "T5:" << bd.cntFT(T5, P2) << " ";
					std::cout << "\n";

					/*for (int j = 0; j < 7; j++)
						std::cout << bd.cntFT((FType)j, P2) << " ";
					std::cout << "\n";*/
				}
			}

			infoo.show(msg);
			for (int i = 0; i < 5; i++) {
				if (btnplay[i].show(msg)) {
					op = i;
				}
			}

			if (op == 0) {
				PV = !PV;
				msg.message = 0;
			}
			else if (op == 1) {//AI落子
				answer = Search::search(bd);
				
				if (answer.size())
					bd.update(answer[0]);

				PrintTest();

				msg.message = 0;
			}
			else if (op == 2) {//撤销
				if (bd.cntMove())bd.undo();
				for (int i = 0; i < 15; i++) {
					for (int j = 0; j < 15; j++) {
						if (bd[{i, j}] == Empty)
							std::cout << bd.value(P1, { i, j }) << "\t";
						else std::cout << "-\t";
					}
					std::cout << "\n";
				}
				msg.message = 0;
			}
			else if (op == 3) {//重置
				bd.reset();
				msg.message = 0;
			}
			else if (op == 4) {
				//bd.sssw(P2);
				msg.message = 0;
			}
			EndBatchDraw();
		}
	}
};

#endif
