#include "selfplay.h"

#include <windows.h>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <sstream>

std::string Writer::generateFileName() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(1, 1000000);

	std::ostringstream oss;
	oss << "data/states_" << std::setw(6) << std::setfill('0') << dis(gen) << ".bin";
	return oss.str();
}

Writer::Writer() : writingDone(false) {
	std::thread writerThread([this]() { this->writeThreadLoop(); });
	writerThread.detach();
}

void Writer::writeThreadLoop() {
	const size_t statesPerFile = 4096;
	size_t count = 0;
	std::ofstream outFile;

	while (!pipe_.done()) {
		for (State state; pipe_.read(state);) {
			if (count % 256 == 255) {
				if (count % 2048 == 2047)
					sync_cout << "writer writed 2048 states!!!!!!!!" << sync_endl;
				else
					sync_cout << "writer writed 256 states" << sync_endl;
			}

			if (count % statesPerFile == 0) {
				if (outFile.is_open())
					outFile.close();

				int tryNum = 0;
				do {
					outFile.open(generateFileName(), std::ios::binary);
					tryNum++;
				} while (!outFile.is_open() && tryNum < 16);
				assert(outFile.is_open());
				sync_cout << "writer opened a new file" << sync_endl;
			}
			outFile.write(reinterpret_cast<const char*>(&state), sizeof(State));
			count++;
		}
	}
	writingDone = true;
}

void Reader::readAndPrintFile(const fs::path& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return;
	}

	std::cout << "Content of " << filePath << ":" << std::endl;
	State state;
	while (file.read(reinterpret_cast<char*>(&state), sizeof(State))) {
		printState(state);
	}
	std::cout << "----------------------------------------" << std::endl;
	file.close();
}

void Reader::printState(const State& state) {
	std::cout << "Side to move: " << (state.side_to_move == 1 ? "@" : "#") << std::endl;
	std::cout << "Evaluation: " << state.eval << std::endl;
	std::cout << "BestMove: " << Square(state.bestMove).x() << "," << Square(state.bestMove).y() << std::endl;
	std::cout << "Board:" << std::endl;
	for (int i = 0; i < 15; ++i) {
		for (int j = 0; j < 15; ++j) {
			if (state.board[i][j] == 1) {
				std::cout << "@ ";
			}
			else if (state.board[i][j] == -1) {
				std::cout << "# ";
			}
			else {
				std::cout << "- ";
			}
		}
		std::cout << std::endl;
	}
}

void Reader::readFiles() {
	try {
		for (const auto& entry : fs::directory_iterator("data")) {
			if (entry.is_regular_file() &&
				entry.path().filename().string().find("states_") == 0 &&
				entry.path().filename().string().find(".bin") != std::string::npos) {
				readAndPrintFile(entry.path());
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << std::endl;
	}
}

void miniGUI::Draw(std::vector<Widget>& pieces, std::vector<Widget>& info) {

	BeginBatchDraw();
	cleardevice();

	//Gdiplus::Pen Pen_1(Gdiplus::Color(0, 0, 0), 1.f);
	//for (int bi = 0; bi < 8; bi++) {
	//    for (int bj = 0; bj < 4; bj++) {

	//        int board_stx = all_board_stx + bi * (board_size + 20);
	//        int board_sty = all_board_sty + bj * (board_size + 25);
	//        //绘制线
	//        for (int i = 0; i < gameSize; i++) {
	//            Gdiplus::Point st(board_stx, board_sty + i * grid_size),
	//                ed(board_stx + (gameSize - 1) * grid_size, board_sty + i * grid_size);
	//            graphics->DrawLine(&Pen_1, st, ed);
	//            st = Gdiplus::Point(board_stx + i * grid_size, board_sty),
	//                ed = Gdiplus::Point(board_stx + i * grid_size, board_sty + (gameSize - 1) * grid_size);
	//            graphics->DrawLine(&Pen_1, st, ed);
	//        }
	//    }
	//}

	for (auto& i : pieces)i.show();
	for (auto& i : info)i.show();

	EndBatchDraw();
}

void miniGUI::updateStats() {
	for (std::string str; pipe.read(str);) {
		std::stringstream ss(str);

		std::string token;

		while (ss >> token) {
			if (token == "thread") {
				ss >> token;
				int id = stoi(token);

				ss >> token;

				if (token == "score") {
					ss >> token;
					info[id].set_text("score: " + token);
				}
				else if (token == "bm") {
					ss >> token;
					std::stringstream t(token);
					std::string xx, yy;
					std::getline(t, xx, ',');
					std::getline(t, yy, ',');
					int x = stoi(xx), y = stoi(yy);

					int pn = id * gameSize * gameSize + x * gameSize + y;
					pieces[pn].set_type(side_to_move[id] == P1 ? Black : White);
					info[id].set_text("bm " + token);
					side_to_move[id] = ~side_to_move[id];
				}
				else if (token == "clear") {
					int pn = id * gameSize * gameSize;
					for (int i = pn; i < pn + gameSize * gameSize; i++) {
						assert(i < pieces.size() && i >= 0);
						pieces[i].set_type(EmptyCell);
					}
					info[id].set_text("");
					side_to_move[id] = P1;
				}
			}
		}
	}
}

miniGUI::miniGUI() {

	initgraph(width, height);
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

	for (int bi = 0; bi < 8; bi++) {
		for (int bj = 0; bj < 4; bj++) {

			int board_stx = all_board_stx + bi * (board_size + 20);
			int board_sty = all_board_sty + bj * (board_size + 20);

			for (int i = 0; i < gameSize; i++) {
				for (int j = 0; j < gameSize; j++) {

					pieces.emplace_back(board_stx + (i - 0.5) * grid_size,
						board_sty + (j - 0.5) * grid_size, grid_size, grid_size, EmptyCell, "", graphics);
				}
			}
			side_to_move.push_back(P1);
			info.emplace_back(all_board_stx + bi * (board_size + 20),
				all_board_sty + bj * (board_size + 25) + board_size + 1,
				board_size, 20, infoCell, "hellofsdfda", graphics);
		}
	}
}

void miniGUI::selfPlayParallel() {

	Writer writer; // 共享的写入器（需确保线程安全）
	std::vector<std::thread> threads;

	// 创建并启动所有线程
	for (int i = 0; i < 32; ++i) {
		threads.emplace_back(selfPlayThreadFunc, &writer, i, &pipe);
	}

	while (true) {

		static TimePoint lastDrawTime = now();
		constexpr int frameRate = 10;
		if (TimePoint elapsed = now() - lastDrawTime; elapsed < 1000 / frameRate) {
			Sleep(1000 / frameRate - elapsed);
		}
		Draw(pieces, info);
		lastDrawTime = now();

		updateStats();
	}

	// 等待所有线程结束（虽然当前selfplayThreadFunc是无限循环）
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

std::vector<std::vector<std::pair<int, int>>> opennings = {
	{														},
	{{0, 0},	{1, 0},		{1, 3}							},
	{{0, 0},	{1, 1}, 	{0, 3}							},
	{{0, 0},	{1, 0}, 	{0, 3}							},
	{{0, 0}, 	{1, 0}, 	{3, 0}							},
	{{0, 0}, 	{1, 1}, 	{1, 3}							},
	{{0, 0}, 	{1, 1},  	{-1, 3}							},
	{{0, 0}, 	{1, 1},  	{0, 1}							},
	{{0, 0}, 	{1, 0},  	{-1, -1}						},
	{{0, 0}, 	{0, 1}, 	{1, 2}							},
	{{0, 0}, 	{1, -1}, 	{0, 1}							},
	{{0, 0}, 	{0, 1},  	{0, 2}							},
	{{0, 0}, 	{1, -1}, 	{2, 0}							},
	{{0, 0}, 	{1, 1}, 	{2, 2}							},
	{{0, 0}, 	{-1, -1},	{-1, 2}							},
	{{0, 0}, 	{-1, 0}, 	{1, 2}							},
	{{0, 0},  	{0, 1},  	{0, -1}							},
	{{0, 0},  	{0, -1},	{0, 2}							},
	{{0, 0},  	{0, 1},		{2, -2}							},
	{{0, 0},  	{-1, 1},	{2, -2}							},
	{{0, 0},  	{-1, 1},	{1, -2}							},
	{{0, 0},  	{1, 0},		{2, 2}							},
	{{0, 0},  	{-1, 1},	{2, 0}							},
	{{0, 0},  	{1, 0},		{1, 1},		{0, 1},		{2, -1} },
	{{0, 0},  	{1, 1},		{-1, 1},	{1, -1},	{-1, 2} },
	{{0, 0}, 	{1, 0},		{1, 1},		{0, 1},		{2, 2}	},
	{{0, 0}, 	{1, 1},		{-1, 1},	{1, -1},	{ 0, 2} },
	{{0, 0}, 	{-1, 0},	{0, -2},	{-1, 1},	{ -2, 2}},
	{{0, 0},  	{-1, 0},	{1, -1},	{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, 0},	{2, -1},	{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, 0},	{2, 0},		{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, 0},	{2, 1},		{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, -1},	{0, -2},	{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, -1},	{1, -2},	{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, -1},	{1, -1},	{-1, 1},	{-2, 2} },
	{{0, 0},  	{-1, -1},	{2, -1},	{-1, 1},	{-2, 2} },
	{{0, 0},  	{0, -1},	{2, -1},	{-1, 1},	{-2, 2} },
	{{0, 0},	{0, -1},	{0, -2},	{-1, 1},	{-2, 2} },
	{{0, 0},	{0, -1},	{0, -2},	{-1, 1},	{-1, 2} },
	{{4, 4},	{5, 3},		{6, 2},		{5, 1},		{6, 0}, },
	{{-7, -5},	{-7, -6},	{-6, -5},	{-6, -6},	{-5, -4}},
	//{{-8, -6},	{-7, -6},	{-8, -5},	{-6, -4},	{-6, -5}}
};

void selfPlayThreadFunc(Writer* writer, int threadID, Pipe<std::string>* pipe) {

	std::random_device rd;
	PRNG prng(rd());

	Engine engine;
	engine.set_options("timeout_turn", 999999);
	engine.set_on_iter([](const auto& info) {});
	engine.set_on_update_no_moves([](const auto& info) {});
	engine.set_on_update_full([&](const auto& info) {});

	for (size_t counter = 0; true; counter++) {

		std::vector<State> states;
		State state = {};
		state.side_to_move = 1;
		std::vector<Square> seq;

		//openning
		std::vector<std::pair<int, int>>openning = opennings[prng.rand<size_t>() % opennings.size()];
		for (int i = 0; i < openning.size(); i++) {

			Square sq = { 7 + openning[i].first, 7 + openning[i].second };
			seq.push_back(sq);
			state.board[sq.x()][sq.y()] = state.side_to_move;
			state.side_to_move = -state.side_to_move;

			std::stringstream ss;
			ss << "thread " << threadID << " bm " << sq.gomocupMove();
			pipe->write(ss.str());
		}

		int endGame = -2;//unfinished
		for (int calcCnt = 0; endGame == -2; calcCnt++) {

			int moveCnt = calcCnt + openning.size();
			//const int multiTb[225] = { 3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
			//const int randP[225] = { 50,50,50,50,50,50,40,40,40,40,40,40,30,30,30,30,30,30,20,20,20,20 };
			const int multiTb[225] = { 3,3,3,3 };
			const int randP[225] = { 70,70,70,50 };
			const int multiPV = std::clamp(multiTb[calcCnt], 1, 4);
			const int nodesperpv = 2000000;
			engine.set_options("multipv", multiPV);
			engine.set_options("nodes_limit", multiPV * nodesperpv);

			std::vector<RootMove> moves;

			engine.set_on_bestmove([&](std::vector<RootMove> rootMoves) {

				moves = rootMoves;
				state.eval = rootMoves[0].score;
				state.bestMove = rootMoves[0].pv[0];
				});

			engine.go(15, seq);
			engine.wait_for_search_finished();

			Square choosed = moves[0].pv[0];
			auto a = prng.rand<size_t>();
			auto b = prng.rand<size_t>();

			if (moves.size() - 1 >= b % multiPV &&
				moves[b % multiPV].score > -200 &&
				moves[0].score - moves[b % multiPV].score < 150 &&
				a % 100 < randP[moveCnt]) {

				choosed = moves[b % multiPV].pv[0];
				//std::cout << "rand" << "\n";
			}

			if (state.eval >= 30000)
				endGame = state.side_to_move;
			else if (seq.size() >= 170)
				endGame = 0;

			seq.push_back(choosed);
			if (calcCnt >= 4)
				states.push_back(state);//record state

			state.board[choosed.x()][choosed.y()] = state.side_to_move;
			state.side_to_move = -state.side_to_move;

			std::stringstream ss;
			ss << "thread " << threadID << " bm " << choosed.gomocupMove();
			pipe->write(ss.str());
		}

		for (int i = 0; i < states.size(); i++) {
			states[i].gameResult = endGame;
			if (abs(states[i].eval) < 30000);
			writer->write(states[i]);
		}

		std::stringstream ss;
		ss << "thread " << threadID << " clear";
		pipe->write(ss.str());
	}
}

void readAll() {

	Reader reader;
	reader.readFiles();
}