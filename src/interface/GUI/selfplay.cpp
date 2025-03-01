#include "selfplay.h"

#include <windows.h>
#include <iostream>
#include <fstream>
#include <random>
#include <string>

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

static int getRandomInt(int a) {
	// random_device 用于获得一个真正随机的种子
	std::random_device rd;
	// mt19937 是一个高质量的伪随机数生成器
	std::mt19937 gen(rd());
	// uniform_int_distribution 用于生成均匀分布的整数
	std::uniform_int_distribution<> distr(0, a - 1);
	return distr(gen);
}

void selfplay(Pipe<std::string>& pipe, int gameCnt) {
	
	Engine engine;
	engine.set_options("nodes_limit", 600000);
	engine.set_options("timeout_turn", 1000000);
	engine.set_on_iter([](const auto& info) {});
	engine.set_on_update_no_moves([](const auto& info) {});
	

	Writer writer;

	for (int i = 0; i < gameCnt; i++) {

		State state = {};
		state.side_to_move = 1;
		std::vector<Square> seq;
		std::vector<std::pair<int, int>>openning = opennings[getRandomInt(opennings.size())];

		for (auto& i : openning) {

			Square sq = { 7 + i.first, 7 + i.second };

			seq.push_back(sq);
			state.board[sq.x()][sq.y()] = state.side_to_move;
			state.side_to_move = -state.side_to_move;

			pipe.write(" bm " + sq.gomocupMove());
			Sleep(10);
		}

		int randomMove = 0;
		const int maxRandom = 8;
		for (int moveCnt = 1, endGame = false; !endGame; moveCnt++) {

			int multipv = 1;
			engine.set_options("multipv", multipv);
			std::vector<std::pair<int, Square>> moves;

			if (randomMove < maxRandom) {
				multipv = 2;
				engine.set_options("multipv", multipv);
			}

			engine.set_on_update_full([&](const InfoFull& info) {

				std::stringstream ss;

				ss << " depth " << info.depth
					<< " seldepth " << info.selDepth
					<< " multipv " << info.multiPV
					<< " score " << info.score
					<< " nodes " << info.nodes
					<< " nps " << info.nps
					<< " time " << info.timeMs;

				sync_cout << ss.str() << sync_endl;
				pipe.write(ss.str());

				if (info.score != -VALUE_INFINITE) {
					int i = 0;
					for (; i < moves.size(); i++) {
						if (moves[i].second == info.pv[0]) {
							moves[i] = { info.score,info.pv[0] };
							break;
						}
					}
					if (i == moves.size()) {
						moves.push_back({ info.score,info.pv[0] });
					}
				}
				});

			engine.set_on_bestmove([&](Square bm) {
				std::stringstream ss;
				ss << " bm " << bm.gomocupMove();
				sync_cout << ss.str() << sync_endl;
				});

			engine.go(15, seq);
			engine.wait_for_search_finished();


			std::sort(moves.begin(), moves.end(),
				[](std::pair<int, Square> a, std::pair<int, Square> b) {return a.first > b.first; });

			state.eval = moves[0].first;
			writer.write(state);

			Square choosed = moves[0].second;
			if (randomMove < maxRandom && getRandomInt(100) < std::clamp(30 - moveCnt * 2, 0, 100)) {
				int r = getRandomInt((std::min)(multipv, (int)moves.size()) - 1) + 1;
				if (moves[0].first - moves[r].first < 150) {
					std::cout << "PICKED WEAKER " << ++randomMove << "/" << maxRandom << "\n";
					std::cout << "ORIGINAL " << moves[0].first << " " << moves[0].second.x() << "," << moves[0].second.y() << "\n";
					std::cout << "NEW      " << moves[r].first << " " << moves[r].second.x() << "," << moves[r].second.y() << "\n";
					choosed = moves[r].second;
				}
			}
			
			seq.push_back(choosed);
			state.board[choosed.x()][choosed.y()] = state.side_to_move;
			pipe.write("bm " + choosed.gomocupMove());

			state.side_to_move = -state.side_to_move;
			if (state.eval == VALUE_MATE)
				endGame = true;

			Sleep(2000);
		}

		pipe.write("clear");
	}
}

void readAll() {

	Reader reader;
	reader.readFiles();
}