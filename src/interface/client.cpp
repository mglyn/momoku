#include "client.h"

#include <sstream>

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

void Button::showText(char text[], int dx, int dy, int size, int thick) {
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = size;
	f.lfWeight = thick;
	settextstyle(&f);
	outtextxy(dx + x, dy + y, text);
}

void Button::showTextInMiddle(int size, int thick) {
	settextcolor(BLACK);
	settextstyle(size, 0, "微软雅黑");
	int width1 = x + (w - textwidth(text.c_str())) / 2;
	int height1 = y + (h - textheight(text.c_str())) / 2;
	outtextxy(width1, height1, text.c_str());
}

bool Button::update(ExMessage msg) {
	isin = msg.x >= x && msg.x <= x + w && msg.y >= y && msg.y <= y + h;
	if(!isin || msg.message == WM_LBUTTONUP) active = true;
	isclick = isin && msg.message == WM_LBUTTONDOWN;
	if (active && isclick) {
		active = false;
		return true;
	}
	return false;
}

void Button::show() {
	setlinecolor(BLACK);
	setlinestyle(PS_SOLID, 1);
	if (type == BlackStyle1) {				//黑棋
		Gdiplus::SolidBrush Pen_b(Gdiplus::Color(100, 100, 100));
		graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
		Gdiplus::Pen Pen_B(Gdiplus::Color(50, 50, 50), 2.f);
		graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		showTextInMiddle(20, 300);
	}
	else if (type == WhiteStyle1) {			//白棋
		Gdiplus::SolidBrush Pen_w(Gdiplus::Color(255, 255, 255));
		graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
		Gdiplus::Pen Pen_W(Gdiplus::Color(165, 165, 165), 2.f);
		graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		showTextInMiddle(20, 300);
	}
	else if (type == BlackStyle2) {			//黑棋 high_lighted
		Gdiplus::Pen Pen_R(Gdiplus::Color(200, 120, 120), 6.f);
		graphics->DrawEllipse(&Pen_R, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		Gdiplus::SolidBrush Pen_b(Gdiplus::Color(100, 100, 100));
		graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
		Gdiplus::Pen Pen_B(Gdiplus::Color(50, 50, 50), 2.f);
		graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		showTextInMiddle(20, 300);
	}
	else if (type == WhiteStyle2) {			//白棋 high_lighted
		Gdiplus::Pen Pen_R(Gdiplus::Color(200, 120, 120), 6.f);
		graphics->DrawEllipse(&Pen_R, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		Gdiplus::SolidBrush Pen_w(Gdiplus::Color(255, 255, 255));
		graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
		Gdiplus::Pen Pen_W(Gdiplus::Color(165, 165, 165), 2.f);
		graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		showTextInMiddle(20, 300);
	}
	else if (type == BlackStyle3) {	//黑棋 pv
		Gdiplus::SolidBrush Pen_b(Gdiplus::Color(150, 150, 150));
		graphics->FillEllipse(&Pen_b, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
		Gdiplus::Pen Pen_B(Gdiplus::Color(165, 200, 185), 7.f);
		graphics->DrawEllipse(&Pen_B, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		showTextInMiddle(20, 300);
	}
	else if (type == WhiteStyle3) {	//白棋 pv
		Gdiplus::SolidBrush Pen_w(Gdiplus::Color(235, 235, 235));
		graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x, y, w, w });//绘制棋子内圈
		Gdiplus::Pen Pen_W(Gdiplus::Color(165, 200, 185), 7.f);
		graphics->DrawEllipse(&Pen_W, Gdiplus::Rect{ x, y, w, w });//绘制棋子外圈
		showTextInMiddle(20, 300);
	}
	else if (type == SearchingMoveStyle1) {
		Gdiplus::SolidBrush Pen_w(Gdiplus::Color(50, 100, 30));
		graphics->FillEllipse(&Pen_w, Gdiplus::Rect{ x + w / 2 - 3, y + w / 2 - 3, 6, 6 });//绘制棋子内圈
	}
	else if (type == SearchingMoveStyle2) {
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
	else if (type == ButtonStyle1) {			//按键
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
	else if (type == ButtonStyle2) {			//透明信息栏
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

Client::Client() {

	init_search_update_listeners();

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

	gameSize = 15;
	grid_size = board_size / (gameSize - 1);

	for (int i = 0; i < gameSize; i++) {
		for (int j = 0; j < gameSize; j++) {
			int pSize = grid_size;
			cells.emplace_back(board_stx + i * grid_size - pSize / 2,
				board_sty + j * grid_size - pSize / 2, pSize, pSize, Cursor, "", graphics);
		}
	}

	for (int i = 0; i < gameSize; i++) {
		for (int j = 0; j < gameSize; j++) {
			int pSize = grid_size - 4;
			pieces.emplace_back(board_stx + i * grid_size - pSize / 2,
				board_sty + j * grid_size - pSize / 2, pSize, pSize, EmptyCell, "", graphics);
		}
	}

	char strplay[12][32] = { "PV","计算","撤销","重置","调试信息" };
	for (int i = 0; i < 12; i++) {
		int x = buttons.size() / 6, y = buttons.size() % 6;
		buttons.emplace_back(int(width * 0.15 + y * btnw), int(height * 0.87 + x * btnh),
			btnw, btnh, ButtonStyle1, strplay[i], graphics);
	}

	for (int i = 0; i < 3; i++) {
		info.emplace_back(int(width * 0.05), int(height * 0.76) + i * btnh, 
			width * 0.9, btnh, ButtonStyle2, "", graphics);
	}
}

void Client::set_pieces() {

	for (int i = 0; i < gameSize; i++) {
		for (int j = 0; j < gameSize; j++) {
			pieces[i * gameSize + j].set_type(EmptyCell);
		}
	}
	
	for (int i = 0; i < seq.size(); i++) {
		int x = seq[i].x(), y = seq[i].y();
		if (i < seq.size() - 1)
			pieces[y * gameSize + x].set_type((i + 1) % 2 ? BlackStyle1 : WhiteStyle1);
		else
			pieces[y * gameSize + x].set_type((i + 1) % 2 ? BlackStyle2 : WhiteStyle2);

		pieces[y * gameSize + x].set_text(std::to_string(i + 1));
	}

	for (int i = 0; i < searchingMove.size(); i++) {
		int x = searchingMove[i].x(), y = searchingMove[i].y();
		pieces[y * gameSize + x].set_type(i != searchingMove.size() - 1 ? SearchingMoveStyle1 : SearchingMoveStyle2);
	}

	if (settings["PV"]) {
		for (int i = 0; i < PV.size(); i++) {
			int x = PV[i].x(), y = PV[i].y();
			pieces[y * gameSize + x].set_type((i + seq.size()) % 2 ? BlackStyle3 : WhiteStyle3);
			pieces[y * gameSize + x].set_text(std::to_string(i));
		}
	}
}

std::string Client::UpdateWidget() {

	::peekmessage(&msg, EM_MOUSE);
	std::string op = "NONE";

	for (int i = 0; i < cells.size(); i++) {
		if (cells[i].update(msg)) 
			op = "USERMOVE " + std::to_string(i);
	}

	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].update(msg)) 
			op = "BUTTON " + std::to_string(i);
	}

	for (int i = 0; i < info.size(); i++) {
		info[i].update(msg);
	}

	return op;
}

void Client::Draw() {
	BeginBatchDraw();
	cleardevice();

	//1. 绘制棋盘
		//绘制标号
	setlinestyle(PS_SOLID, 1);
	settextcolor(BLACK);
	settextstyle(18, 0, "微软雅黑");
	for (int i = 0; i < gameSize; i++) {
		char c[4] = "";
		sprintf_s(c, 3, "%c", i + 'A');
		::outtextxy(board_stx + i * grid_size - textwidth(c) / 2, 
			board_sty + (gameSize - 1 + 0.5) * grid_size - textheight(c) / 2, c);
		sprintf_s(c, 3, "%d", i + 1);
		::outtextxy(board_stx - grid_size * 0.5 - textwidth(c) / 2, 
			board_sty + (gameSize - i - 1) * grid_size - textheight(c) / 2, c);
	}
	//绘制线
	Gdiplus::Pen Pen_1(Gdiplus::Color(0, 0, 0), 1.f);
	for (int i = 0; i < gameSize; i++) {
		Gdiplus::Point st(board_stx, board_sty + i * grid_size),
			ed(board_stx + (gameSize - 1) * grid_size, board_sty + i * grid_size);
		graphics->DrawLine(&Pen_1, st, ed);
		st = Gdiplus::Point(board_stx + i * grid_size, board_sty), 
			ed = Gdiplus::Point(board_stx + i * grid_size, board_sty + (gameSize - 1) * grid_size);
		graphics->DrawLine(&Pen_1, st, ed);
	}

	//2.绘制光标
	for (int i = 0; i < cells.size(); i++) {
		cells[i].show();
	}

	//3.绘制棋子
	for (int i = 0; i < pieces.size(); i++) {
		pieces[i].show();
	}

	//4.绘制按钮
	for (int i = 0; i < buttons.size(); i++) {
		buttons[i].show();
	}

	//5.绘制信息
	for (int i = 0; i < info.size(); i++) {
		info[i].show();
	}

	EndBatchDraw();
}

void Client::init_search_update_listeners() {
	engine.set_on_iter([this](const InfoIteration& info) {

		searchingMove.push_back(info.currmove);
		//std::stringstream ss;
		/*ss << "info";
		ss << " depth " << info.depth
			<< " currmove " << info.currmove.x() << "," << info.currmove.y()
			<< " currmovenumber " << info.currmovenumber;*/
		//sync_cout << ss.str() << sync_endl;
		});

	engine.set_on_update_no_moves([](const InfoShort& info) {

		sync_cout << "info depth " << info.depth << " score " << info.score << sync_endl;
		});

	engine.set_on_update_full([this](const InfoFull& info) {

		std::stringstream ss1, ss2;

		ss1 << " depth " << info.depth                 //
			<< " seldepth " << info.selDepth           //
			<< " multipv " << info.multiPV             //
			<< " score " << info.score;  //

		if (!info.bound.empty())
			ss1 << " " << info.bound;

		this->info[0].set_text(ss1.str());

		ss2 << " nodes " << info.nodes        //
			<< " nps " << info.nps            //
			<< " time " << info.timeMs;       //

		this->info[1].set_text(ss2.str());
		
		PV = info.pv;
		searchingMove.clear();

		sync_cout << ss1.str() + ss2.str() << sync_endl;
		});

	engine.set_on_bestmove([this](Square bm) { 

		sync_cout << "bestmove " << char('A' + bm.y()) << gameSize - bm.x() << sync_endl;

		bool valid = true;
		for (auto& i : seq) 
			valid &= i.x() != bm.x() || i.y() != bm.y();
		if (valid) 
			seq.push_back(bm);

		searchingMove.clear();
		PrintTest();
		});
}

void Client::Loop() {
	
	while (true) {
		set_pieces();

		Draw();

		std::string op = UpdateWidget();
		std::stringstream ss(op);

		std::string type;
		int sqare;
		ss >> type >> sqare;

		if (type == "USERMOVE") {

			int y = sqare / gameSize, x = sqare % gameSize;

			bool valid = x < gameSize && y < gameSize && x >= 0 && y >= 0;
			for (auto& i : seq) 
				valid &= i.x() != x || i.y() != y;
			if (valid) {
				seq.push_back(Square{ x, y });
				engine.consoleDBG(gameSize, seq);
			}
		}
		else if (type == "BUTTON") {

			if (sqare == 0) {
				settings["PV"] = !settings["PV"];
			}
			else if (sqare == 1) {//AI落子
				engine.go(gameSize, seq);
			}
			else if (sqare == 2) {//撤销
				if (seq.size())
					seq.pop_back();
				PV.clear();
			}
			else if (sqare == 3) {//重置
				seq.clear();
				PV.clear();
			}
			else if (sqare == 4) {//DBG
				engine.consoleDBG(gameSize, seq);
			}
		}
	}
}
