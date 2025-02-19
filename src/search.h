#ifndef SEARCH
#define SEARCH

#include "common.h"
#include "position.h"
#include "move.h"
#include "time.h"
#include "history.h"

#include <map>
#include <array>
#include <functional>

enum NType {
	NonPV,
	PV,
	Root
};

class TranspositionTable;
class ThreadPool;
class Options;

constexpr int threadNum = 1;

struct Stack {
	int ply;
	Square* pv;
	Value staticEval;
	bool ttPv;
	FType moveFT[2];
};

// RootMove struct is used for moves at the root of the tree. For each root move
// we store a score and a PV (really a refutation in the case of moves which
// fail low). Score is normally set at -VALUE_INFINITE for all non-pv moves.
struct RootMove {
	uint64_t          effort = 0;
	Value             score = -VALUE_INFINITE;
	Value             previousScore = -VALUE_INFINITE;
	Value             averageScore = -VALUE_INFINITE;
	Value             meanSquaredScore = -VALUE_INFINITE * VALUE_INFINITE;
	Value             uciScore = -VALUE_INFINITE;
	bool              scoreLowerbound = false;
	bool              scoreUpperbound = false;
	int               selDepth = 0;
	std::vector<Square> pv;

	explicit RootMove(Square sq) : pv(1, sq) {}
	bool operator==(const Square& m) const { return pv[0] == m; }
	// Sort in descending order
	bool operator<(const RootMove& m) const {
		return m.score != score ? m.score < score : m.previousScore < previousScore;
	}
};

// LimitsType struct stores information sent by the caller about the analysis required.
struct LimitsType {
	std::vector<Square>		 searchmoves;
	TimePoint                time[2], startTime, movetime;

	// Init explicitly due to broken value-initialization of non POD in MSVC
	LimitsType() {
		startTime = time[P1] = time[P2] = movetime = TimePoint(0);
	}

	bool use_time_management() const { return time[P1] || time[P2]; }
};

struct SharedState {
	const Options& options;
	ThreadPool& threads;
	TranspositionTable& tt;

	SharedState(const Options& options,
		ThreadPool& threads,
		TranspositionTable& tt) :
		options(options),
		threads(threads),
		tt(tt) {
	}
};

struct InfoShort {
	int		depth;
	Value	score;
};

struct InfoFull : InfoShort {
	int              selDepth;
	size_t           multiPV;
	std::string_view bound;
	size_t           timeMs;
	size_t           nodes;
	size_t           nps;
	std::vector<Square> pv;
};

struct InfoIteration {
	int    depth;
	Square currmove;
	size_t currmovenumber;
};

class Worker;

class ISearchManager {
public:
	virtual ~ISearchManager() {}
	virtual void check_time(Worker&) = 0;
};

// SearchManager manages the search from the main thread. It is responsible for
// keeping track of the time, and storing data strictly related to the main thread.
class SearchManager : public ISearchManager {
public:
	using UpdateShort = std::function<void(const InfoShort&)>;
	using UpdateFull = std::function<void(const InfoFull&)>;
	using UpdateIter = std::function<void(const InfoIteration&)>;
	using UpdateBestmove = std::function<void(Square)>;

	struct UpdateContext {
		UpdateShort    onUpdateNoMoves;
		UpdateFull     onUpdateFull;
		UpdateIter     onIter;
		UpdateBestmove onBestmove;
	};

	TimeManagement tm;
	int	callsCnt;

	Value bestPreviousScore;
	Value bestPreviousAverageScore;

	size_t id;

	const UpdateContext& updates;

	SearchManager(const UpdateContext& updateContext) :
		updates(updateContext) {
	}

	void check_time(Worker& worker) override;

	void pv(const Worker& worker,
		const ThreadPool& threads,
		const TranspositionTable& tt,
		Depth depth) const;
};

class NullSearchManager : public ISearchManager {
public:
	void check_time(Worker&) override {}
};

class Worker {

	Depth pick_next_depth(int lastDepth);

	void iterative_deepening();

	// This is the main search function, for both PV and non-PV nodes
	Value search(NType, Position&, Stack*, Value, Value, Depth, bool cutNode);

	// This is the qsearch function, which is called by the main search
	Value qsearch(NType, Position&, Stack*, Value, Value);

	// Pointer to the search manager, only allowed to be called by the main thread
	SearchManager* main_manager() const {
		assert(threadIdx == 0);
		return static_cast<SearchManager*>(manager.get());
	}

	TimePoint elapsed_time() const;

	LimitsType limits;

	size_t                pvIdx, pvLast;
	std::atomic<uint64_t> nodes, bestMoveChanges;
	int                   selDepth;

	Position  rootPos;
	std::vector<RootMove>  rootMoves;
	Depth   rootDepth, completedDepth;
	Value   rootDelta;

	size_t threadIdx;

	// Reductions lookup table initialized at startup
	//std::array<int, MAX_PLY + 10> reductions;  // [depth or moveNumber]

	// The main thread has a SearchManager, the others have a NullSearchManager
	std::unique_ptr<ISearchManager> manager;

	const Options& options;
	ThreadPool& threads;
	TranspositionTable& tt;

	friend class ThreadPool;
	friend class SearchManager;
public:
	Worker(SharedState&, std::unique_ptr<ISearchManager>, size_t);

	// Called at instantiation to initialize reductions tables.
	// Reset histories, usually before a new game.
	void clear();

	// Called when the program receives the UCI 'go' command.
	// It searches from the root position and outputs the "bestmove".
	void start_searching();

	bool is_mainthread() const { return threadIdx == 0; }

	MainHistory mainHistory;
	CounterMoveHistory counterMoveHistory;
};

#endif


