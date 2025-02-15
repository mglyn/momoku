#ifndef ENGINE
#define ENGINE

#include "position.h"
#include "search.h"
#include "tt.h"
#include "threads.h"

#include <vector>
#include <map>

class Options {
	struct CaseInsensitiveCompare;
	std::map <std::string, int64_t,
		decltype([](const std::string& s1, const std::string& s2) {
		return std::lexicographical_compare(
			s1.begin(), s1.end(), s2.begin(), s2.end(),
			[](char c1, char c2) { return std::tolower(c1) < std::tolower(c2); });
		})> optionsMap;
public:
	int64_t  operator[](const std::string&) const;
	int64_t& operator[](const std::string&);
	std::size_t count(const std::string& name) const;
};

class Engine {
	TranspositionTable tt;
	
	Options options;
	ThreadPool threads;

	SearchManager::UpdateContext updateContext;

public:

	Engine();

	// Cannot be movable due to components holding backreferences to fields
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;

	~Engine() { wait_for_search_finished(); }

	//std::uint64_t perft(const std::string& fen, Depth depth);

	// non blocking call to start searching
	void go(int, std::vector<Square>);
	// non blocking call to stop searching
	void stop();

	// blocking call to wait for search to finish
	void wait_for_search_finished();

	// modifiers
	void resize_threads();
	void set_tt_size(size_t mb);
	void search_clear();

	void set_on_update_no_moves(std::function<void(const InfoShort&)>&&);
	void set_on_update_full(std::function<void(const InfoFull&)>&&);
	void set_on_iter(std::function<void(const InfoIteration&)>&&);
	void set_on_bestmove(std::function<void(Square)>&&);

	// utility functions
	int get_hashfull(int maxAge = 0) const;

	Options& get_options();
	const Options& get_options() const;

	void consoleDBG(int gameSize, std::vector<Square> seq);
};

#endif