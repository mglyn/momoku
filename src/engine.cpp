#include "engine.h"

std::size_t Options::count(const std::string& name) const { return optionsMap.count(name); }

int64_t Options::operator[](const std::string& name) const {
	auto it = optionsMap.find(name);
	return it != optionsMap.end() ? it->second : 0;
}

int64_t& Options::operator[](const std::string& name) {
	return optionsMap[name];
}

Engine::Engine() {

	options["timeout_turn"] = 1000;		//- time limit for each move(milliseconds, 0 = play as fast as possible)
	options["timeout_match"] = 0;		//- time limit of a whole match(milliseconds, 0 = no limit)
	options["max_memory"] = 300ULL * 1024ULL * 1024ULL;		//- memory limit(bytes, 0 = no limit)
	options["time_left"] = 0;			//- remaining time limit of a whole match(milliseconds)
	options["game_type"] = 0;			//- 0 = opponent is human, 1 = opponent is brain, 2 = tournament, 3 = network tournament
	options["rule"] = 0;				//- bitmask or sum of 1 = exactly five in a row win, 2 = continuous game, 4 = renju, 8 = caro
	//options["evaluate"]				//- coordinates X, Y representing current position of the mouse cursor
	//options["folder"]					//- folder for persistent files
	options["multipv"] = 1;
	options["nodes_limit"] = 0;

	options["Threads"] = 1;

	threads.set({ options, threads, tt }, updateContext);
	tt.resize(options["max_memory"] - 16LL * 1048576LL);
}

bool Engine::set_options(std::string option, size_t value) {

	if (options.count(option)) {
		options[option] = value;

		threads.wait_for_search_finished();

		if (option == "Threads")
			threads.set({ options, threads, tt }, updateContext);
		if (option == "max_memory") {
			tt.resize(std::max(options["max_memory"] - 16LL * 1048576LL, 128LL * 1048576LL));
		}
		return true;
	}
	else return false;
}

const Options& Engine::get_options() const {
	return options;
}

void Engine::go(int gameSize, std::vector<Square> seq) {

	Position pos;

	pos.set(gameSize);

	for (auto& sq : seq) {
		pos.make_move(sq);
	}

	LimitsType limits;
	limits.movetime = options["timeout_turn"];
	limits.nodes = options["nodes_limit"];
	limits.startTime = now();

	threads.start_thinking(pos, limits);
}

void Engine::stop() { threads.stop = true; }

void Engine::search_clear() {
	wait_for_search_finished();

	tt.clear();
	threads.clear();
}

void Engine::wait_for_search_finished() { threads.main_thread()->wait_for_search_finished(); }

void Engine::set_on_update_no_moves(std::function<void(const InfoShort&)>&& f) {
	updateContext.onUpdateNoMoves = std::move(f);
}

void Engine::set_on_update_full(std::function<void(const InfoFull&)>&& f) {
	updateContext.onUpdateFull = std::move(f);
}

void Engine::set_on_iter(std::function<void(const InfoIteration&)>&& f) {
	updateContext.onIter = std::move(f);
}

void Engine::set_on_bestmove(std::function<void(Square)>&& f) {
	updateContext.onBestmove = std::move(f);
}

int Engine::get_hashfull(int maxAge) const { return tt.hashfull(maxAge); }


void Engine::consoleDBG(int gameSize, std::vector<Square> seq) {

	//first center move

	Position pos;

	pos.set(gameSize);

	for (auto& sq : seq) {
		pos.make_move(sq);
	}

	const StateInfo& st = pos.st();

	std::cout << "valueP1:" << st.valueP1 << '\n';
	std::cout << "P1 value:\n";
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (pos[{i, j}] == Empty)
				std::cout << pos.value(P1, { i, j }) << "\t";
			else std::cout << "-\t";
		}
		std::cout << "\n";
	}
	std::cout << "P2 value:\n";
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (pos[{i, j}] == Empty)
				std::cout << pos.value(P2, { i, j }) << "\t";
			else std::cout << "-\t";
		}
		std::cout << "\n";
	}
	/*std::cout << "P1 type:\n";
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (pos[{i, j}] == Empty)
				std::cout << (int)pos.type(P1, { i, j }) << "\t";
			else std::cout << "-\t";
		}
		std::cout << "\n";
	}*/
	std::cout << "\n";

	std::cout << "P1 threats: \n";
	std::cout << "TH3:" << (int)st.cntT[TH3][P1] << " ";
	std::cout << "TDH3:" << (int)st.cntT[TDH3][P1] << " ";
	std::cout << "T4H3:" << (int)st.cntT[T4H3][P1] << " ";
	std::cout << "T4:" << (int)st.cntT[T4][P1] << " ";
	std::cout << "TH4:" << (int)st.cntT[TH4][P1] << " ";
	std::cout << "T5:" << (int)st.cntT[T5][P1] << " ";
	std::cout << "\n";

	std::cout << "P2 threats: \n";
	std::cout << "TH3:" << (int)st.cntT[TH3][P2] << " ";
	std::cout << "TDH3:" << (int)st.cntT[TDH3][P2] << " ";
	std::cout << "T4H3:" << (int)st.cntT[T4H3][P2] << " ";
	std::cout << "T4:" << (int)st.cntT[T4][P2] << " ";
	std::cout << "TH4:" << (int)st.cntT[TH4][P2] << " ";
	std::cout << "T5:" << (int)st.cntT[T5][P2] << " ";
	std::cout << "\n";
}
