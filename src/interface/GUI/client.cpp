#include "client.h"

#include <sstream>

Client::Client() {

	init_search_update_listeners();

	initgraph(width, height, SHOWCONSOLE);
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
	graphics = std::make_shared<Gdiplus::Graphics>(GetImageHDC());
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

			cursors.emplace_back(board_stx + (i - 0.5) * grid_size,
				board_sty + (j - 0.5) * grid_size, grid_size, grid_size, Cursor, "", graphics);
		}
	}

	char strplay[12][32] = { "PV","计算","撤销","重置","调试信息","","openning","read" };
	for (int i = 0; i < 12; i++) {
		int x = buttons.size() / 6, y = buttons.size() % 6;
		buttons.emplace_back(int(width * 0.15 + y * btnw), int(height * 0.87 + x * btnh),
			btnw, btnh, Button, strplay[i], graphics);
	}

	for (int i = 0; i < 12; i++) {
		int x = info.size() / 4, y = info.size() % 4;
		info.emplace_back(int(width * 0.15 + y * 1.5 * btnw), int(height * 0.76 + x * btnh),
			1.5 * btnw, btnh, infoCell, "", graphics);
	}
}

void Client::init_search_update_listeners() {

	engine.set_on_iter([this](const InfoIteration& info) {

		std::stringstream ss;
		ss << " depth " << info.depth
			<< " currmove " << (std::to_string(info.currmove.x()) + "," + std::to_string(info.currmove.y()))
			<< " currmovenumber " << info.currmovenumber;
		//sync_cout << ss.str() << sync_endl;
		pipe.write(ss.str());
		});

	engine.set_on_update_no_moves([this](const InfoShort& info) {

		//sync_cout << "depth " << info.depth << " score " << info.score << sync_endl;
		});

	engine.set_on_update_full([this](const InfoFull& info) {

		std::stringstream ss;

		ss  << " multipv " << info.multiPV
			<< " depth " << info.depth
			<< " seldepth " << info.selDepth
			<< " score " << info.score
			<< " nodes " << info.nodes
			<< " nps " << info.nps
			<< " time " << info.timeMs;

		ss << " pv ";
		for (auto& i : info.pv) {
			ss << i.gomocupMove() << " ";
		}
		pipe.write(ss.str());
		sync_cout << ss.str() << sync_endl;
		});

	engine.set_on_bestmove([this](const std::vector<RootMove>& rootMoves) {

		std::stringstream ss;

		ss << " bm " << rootMoves[0].pv[0].gomocupMove();
		ss << " pv ";
		for (auto& i : rootMoves[0].pv) {
			ss << i.gomocupMove() << " ";
		}

		sync_cout << ss.str() << sync_endl;
		pipe.write(ss.str());
		PrintTest();

		//for (int i = 0; i < rootMoves.size();i++) 
			//sync_cout << "pv" << i << ": " << rootMoves[i].pv[0].gomocupMove() << sync_endl;
		});
}

void Client::clearStats() {
	for (auto& i : info) i.set_text("");
	searchedMoves.clear();
	searchingMove.clear();
	PV.clear();
	pieces.clear();
	highLight.clear();
	seq.clear();
}

void Client::updateStats() {

	for (std::string str; pipe.read(str);) {
		std::stringstream ss(str);

		std::string token;

		while (ss >> token) {
			if (token == "multipv") {
				if (ss >> token; stoi(token) != 1)  //only display first pv
					std::getline(ss, token);
			}
			else if (token == "depth") {
				ss >> token;
				info[0].set_text("depth: " + token);
			}
			else if (token == "seldepth") {
				ss >> token;
				info[1].set_text("seldepth: " + token);
			}
			else if (token == "currmove") {
				ss >> token;
				info[2].set_text("currmove: " + token);

				std::stringstream t(token);
				std::string x, y;
				std::getline(t, x, ',');
				std::getline(t, y, ',');
				searchedMoves.emplace_back(board_stx + (stoi(y) - 0.5) * grid_size,
					board_sty + (stoi(x) - 0.5) * grid_size, grid_size, grid_size, SearchedMoves, "", graphics);
				searchingMove.clear();
				searchingMove.emplace_back(board_stx + (stoi(y) - 0.5) * grid_size,
					board_sty + (stoi(x) - 0.5) * grid_size, grid_size, grid_size, SearchingMove, "", graphics);
			}
			else if (token == "score") {
				ss >> token;
				info[3].set_text("score: " + token);
			}
			else if (token == "nodes") {
				ss >> token;
				info[4].set_text("nodes: " + token);
			}
			else if (token == "nps") {
				ss >> token;
				info[5].set_text("nps: " + token);
			}
			else if (token == "time") {
				ss >> token;
				info[6].set_text("time: " + token);
			}
			else if (token == "pv") {
				PV.clear();
				searchedMoves.clear();
				searchingMove.clear();
				for (int i = 0; ss >> token; i++) {
					std::stringstream t(token);
					std::string x, y;
					std::getline(t, x, ',');
					std::getline(t, y, ',');
					PV.emplace_back(board_stx + (stoi(y) - 0.5) * grid_size,
						board_sty + (stoi(x) - 0.5) * grid_size, grid_size, grid_size,
						(seq.size() + i) % 2 == 0 ? BlackPV : WhitePV,
						std::to_string(i + 1), graphics);
				}
			}
			else if (token == "bm") {
				searchedMoves.clear();
				searchingMove.clear();
				ss >> token;
				std::stringstream t(token);
				std::string x, y;
				std::getline(t, x, ',');
				std::getline(t, y, ',');

				pieces.emplace_back(board_stx + (stoi(y) - 0.5) * grid_size,
					board_sty + (stoi(x) - 0.5) * grid_size, grid_size, grid_size,
					(seq.size()) % 2 == 0 ? Black : White,
					std::to_string(seq.size() + 1), graphics);
				highLight.clear();
				highLight.emplace_back(board_stx + (stoi(y) - 0.5) * grid_size,
					board_sty + (stoi(x) - 0.5) * grid_size, grid_size, grid_size,
					HighLight, std::to_string(seq.size() + 1), graphics);

				seq.push_back({ stoi(x),stoi(y) });
				//engine.consoleDBG(gameSize, seq);
			}
			else if (token == "clear") {
				clearStats();
			}
		}
	}
}

std::string Client::UpdateWidget() {

	static ExMessage msg = {};
	while (::peekmessage(&msg, EM_MOUSE));
	std::string op = "NONE";

	for (int i = 0; i < cursors.size(); i++) {
		if (cursors[i].update(msg))
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

	for (auto& i : cursors) i.show();
	for (auto& i : buttons) i.show();
	for (auto& i : info) i.show();
	for (auto& i : searchedMoves) i.show();
	for (auto& i : searchingMove) i.show();
	if (settings["PV"])
	for (auto& i : PV) i.show();
	for (auto& i : pieces) i.show();
	for (auto& i : highLight) i.show();

	EndBatchDraw();
}

void Client::Loop() {

	while (true) {

		static TimePoint lastDrawTime = now();
		constexpr int frameRate = 120;
		if (TimePoint elapsed = now() - lastDrawTime; elapsed < 1000 / frameRate) {
			Sleep(1000 / frameRate - elapsed);
		}

		updateStats();
		std::string op = UpdateWidget();
		Draw();
		lastDrawTime = now();

		std::stringstream ss(op);

		std::string type;
		int square;
		ss >> type >> square;

		if (type == "USERMOVE") {

			int y = square / gameSize, x = square % gameSize;

			bool valid = x < gameSize && y < gameSize && x >= 0 && y >= 0;
			for (auto& i : seq) 
				valid &= i.x() != x || i.y() != y;
			if (valid) {
				Square sq{ x, y };
				pipe.write("bm " + sq.gomocupMove());
			}
		}
		else if (type == "BUTTON") {

			if (square == 0) {
				settings["PV"] = !settings["PV"];
			}
			else if (square == 1) {//AI落子
				engine.go(gameSize, seq);
			}
			else if (square == 2) {//撤销
				if (seq.size()) {
					seq.pop_back();
					PV.clear();
					pieces.pop_back();
					highLight.pop_back();
					if (pieces.size()) {
						highLight.push_back(pieces.back());
						highLight.back().set_type(HighLight);
					}
				}
			}
			else if (square == 3) {//重置
				clearStats();
			}
			else if (square == 4) {//DBG
				engine.consoleDBG(gameSize, seq);
			}
			else if (square == 5) {
			}
			else if (square == 6) {//beginning
				seq.clear();
				seq.push_back({ 7,7 });
				seq.push_back({ 8,8 });
				seq.push_back({ 7,6 });
				seq.push_back({ 6,6 });
				seq.push_back({ 5,5 });
				seq.push_back({ 0,0 });
				seq.push_back({ 14,14 });
			}
			else if (square == 7) {//read
				readAll();
			}
			else if (square == 8) {

			}
		}
	}
}
