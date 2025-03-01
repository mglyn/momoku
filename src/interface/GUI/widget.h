#ifndef WIDGET
#define WIDGET

#include <memory>
#include <string>
#include <graphics.h>
#include <conio.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

enum WidgetType :int {
	Black,
	White,
	HighLight,
	BlackPV,
	WhitePV,
	SearchedMoves,
	SearchingMove,
	EmptyCell,
	Cursor,
	Button,
	infoCell,
};


class Widget {

	std::string text;
	bool isin = false, isclick = false, active = true;
	int x = 0, y = 0, w = 0, h = 0, type = 0;
	std::shared_ptr<Gdiplus::Graphics> graphics;

	void showTextInMiddle(int size, int thick) {
		settextcolor(BLACK);
		settextstyle(size, 0, "微软雅黑");
		int width1 = x + (w - textwidth(text.c_str())) / 2;
		int height1 = y + (h - textheight(text.c_str())) / 2;
		outtextxy(width1, height1, text.c_str());
	}

public:

	Widget(int _x, int _y, int _w, int _h, int _type,
		std::string _text, std::shared_ptr<Gdiplus::Graphics> _graphics) :
		x(_x), y(_y), w(_w), h(_h),
		type(_type), text(_text), graphics(_graphics) {
	}
	void set_type(int _type) { type = _type; }

	void set_text(std::string _text) { text = _text; }

	bool update(ExMessage msg) {

		isin = msg.x >= x && msg.x <= x + w && msg.y >= y && msg.y <= y + h;
		if (!isin || msg.message == WM_LBUTTONUP) active = true;
		isclick = isin && msg.message == WM_LBUTTONDOWN;
		if (active && isclick) {
			active = false;
			return true;
		}
		return false;
	}

	void show() {

		setlinecolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		if (type == Black) {				//黑棋
			Gdiplus::SolidBrush Pen_b(Gdiplus::Color(100, 100, 100));
			graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子内圈
			Gdiplus::Pen Pen_B(Gdiplus::Color(50, 50, 50), 2.f);
			graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == White) {			//白棋
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(255, 255, 255));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子内圈
			Gdiplus::Pen Pen_W(Gdiplus::Color(165, 165, 165), 2.f);
			graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == HighLight) {	    //high_light
			Gdiplus::Pen Pen_R(Gdiplus::Color(200, 120, 120), 2.f);
			graphics->DrawEllipse(&Pen_R, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		}
		else if (type == BlackPV) {	//黑棋 pv
			Gdiplus::SolidBrush Pen_b(Gdiplus::Color(150, 150, 150));
			graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子内圈
			Gdiplus::Pen Pen_B(Gdiplus::Color(165, 200, 185), 7.f);
			graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == WhitePV) {	//白棋 pv
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(235, 235, 235));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子内圈
			Gdiplus::Pen Pen_W(Gdiplus::Color(165, 200, 185), 7.f);
			graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x + 1, y + 1, w - 2, w - 2 });//绘制棋子外圈
			showTextInMiddle(20, 300);
		}
		else if (type == SearchedMoves) {
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(50, 100, 30));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x + w / 2 - 3, y + w / 2 - 3, 6, 6 });//绘制棋子内圈
		}
		else if (type == SearchingMove) {
			Gdiplus::SolidBrush Pen_w(Gdiplus::Color(100, 60, 50));
			graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x + w / 2 - 7, y + w / 2 - 7, 14, 14 });//绘制棋子内圈
		}
		else if (type == EmptyCell) {

		}
		else if (type == Cursor) {	//游标
			if (isin && !isclick) {
				Gdiplus::Pen Pen_1(Gdiplus::Color(160, 180, 210), 3.f);
				graphics->DrawEllipse(&Pen_1, Gdiplus::Rect{ x + 3, y + 3, w - 6, w - 6 });
			}
			else if (isclick) {
				Gdiplus::Pen Pen_2(Gdiplus::Color(205, 225, 235), 3.f);
				graphics->DrawEllipse(&Pen_2, Gdiplus::Rect{ x + 3, y + 3, w - 6, w - 6 });
			}
		}
		else if (type == Button) {			//按键
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
		else if (type == infoCell) {			//透明信息栏
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				setfillcolor(RGB(230, 230, 230));
				fillroundrect(x, y, x + w + 1, y + h + 1, 10, 10);
			}
			else if (isclick) {
				setfillcolor(RGB(237, 237, 237));
				fillroundrect(x, y, x + w + 1, y + h + 1, 10, 10);
			}
			showTextInMiddle(20, 300);
		}
	}
};
#endif
