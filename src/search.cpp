#include <iostream>

#include "threads.h"
#include "search.h"
#include "tt.h"
#include "engine.h"
#include "gomocup.h"

static constexpr int checkMate(const Stack* ss, const Position& pos) {
	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& st = pos.st();

	if ((ss - 1)->moveFT[op] == T5) 
		return mated_in(ss->ply - 1);
		
	if (st.cntT[T5][us])
		return mate_in(ss->ply);

	int op5 = st.cntT[T5][op];
	if (op5)
		return op5 > 1 ? mated_in(ss->ply + 1) : 0;

	if (st.cntT[TH4][us])
		return mate_in(ss->ply + 2);

	if (st.cntT[TDH3][us] + st.cntT[T4H3][us] && !st.cntT[T4][op]) {
		testData[TDH3T4H3wincheck]++;
		return mate_in(ss->ply + 4);
	}

	return VALUE_ZERO;
}

static int staticEval(const Position& pos) {
	Piece us = pos.side_to_move();
	const StateInfo& st = pos.st();
	const StateInfo& prevst = pos.prevst();

	int evalP1 = (st.valueP1 + prevst.valueP1) / 2;

	if (evalP1 == 0)return 1;
	return us == P1 ? evalP1 : -evalP1;
}

// Adjusts a mate from "plies to mate from the root" to
// "plies to mate from the current position". Standard scores are unchanged.
// The function is called before storing a value in the transposition table.
Value value_to_tt(Value v, int ply) {
	return is_win(v) ? v + ply : is_loss(v) ? v - ply : v;
}

// Inverse of value_to_tt(): it adjusts a mate score from the transposition
// table (which refers to the plies to mate/be mated from current position) to
// "plies to mate/be mated from the root". 
Value value_from_tt(Value v, int ply) {
	return is_valid(v) ? is_win(v) ? v - ply : is_loss(v) ? v + ply : v : VALUE_NONE;
}

void updatePV(Square* pv, Square sq, Square* childPV) {
	for (*pv++ = sq; childPV && *childPV != NULLSQUARE;)
		*pv++ = *childPV++;
	*pv = NULLSQUARE;
}

Worker::Worker(SharedState& sharedState,
	std::unique_ptr<ISearchManager> sm,
	size_t                          threadId) :
	// Unpack the SharedState struct into member variables
	threadIdx(threadId),
	manager(std::move(sm)),
	options(sharedState.options),
	threads(sharedState.threads),
	tt(sharedState.tt) {
	clear();
}

void Worker::start_searching() {

	// Non-main threads go directly to iterative_deepening()
	if (!is_mainthread()) {
		iterative_deepening();
		return;
	}

	main_manager()->tm.init(limits, rootPos.side_to_move(), rootPos.cntMove());
	tt.new_search();

	if (rootMoves.empty()) {
		rootMoves.emplace_back(NULLSQUARE);
		main_manager()->updates.onUpdateNoMoves({ 0, -VALUE_MATE });
	}
	else {
		threads.start_searching();  // start non-main threads
		iterative_deepening();      // main thread start searching
	}

	// When we reach the maximum depth
	// Stop the threads if not already stopped 
	threads.stop = true;

	// Wait until all threads have finished
	threads.wait_for_search_finished();
	PrintTest();

	Worker* bestThread = this;

	if (int(options["MultiPV"]) == 1 && rootMoves[0].pv[0] != NULLSQUARE)
		bestThread = threads.get_best_thread()->worker.get();

	main_manager()->bestPreviousScore = bestThread->rootMoves[0].score;
	main_manager()->bestPreviousAverageScore = bestThread->rootMoves[0].averageScore;

	// Send again PV info if we have a new best thread
	if (bestThread != this)
		main_manager()->pv(*bestThread, threads, tt, bestThread->completedDepth);

	main_manager()->updates.onBestmove(bestThread->rootMoves[0].pv[0]);
}

Depth Worker::pick_next_depth(int lastDepth) {

	if (is_mainthread() || threads.size() < 3)
		return lastDepth + 1;

	for (int nextDepth = lastDepth + 1;; nextDepth++) {
		size_t numThreadsAboveNextDepth = 0;

		for (const auto& th : threads) {
			if (th->id() != threadIdx && th->worker->completedDepth + 1 >= nextDepth)
				numThreadsAboveNextDepth++;
		}

		// If more than half of all threads are already searching depth above
		// the next depth, skip and find another next depth.
		if (numThreadsAboveNextDepth > threads.size() / 2)
			continue;
		else
			return nextDepth;
	}
}

// Main iterative deepening loop. It calls search()
// repeatedly with increasing depth until the allocated thinking time has been
// consumed, the user stops the search, or the maximum search depth is reached.
void Worker::iterative_deepening() {

	SearchManager* mainThread = (is_mainthread() ? main_manager() : nullptr);

	Square pv[MAX_PLY + 1];

	int lastBestMoveDepth = 0;
	int lastBestScore = -VALUE_INFINITE;
	std::vector<Square> lastBestPV = std::vector{ NULLSQUARE };

	int  alpha, beta;
	int  bestValue = -VALUE_INFINITE;
	Piece  us = rootPos.side_to_move(), op = ~us;
	double timeReduction = 1, totBestMoveChanges = 0;
	int    delta, iterIdx = 0;

	// Allocate stack with extra size to allow access from (ss - 7) to (ss + 2):
	// (ss - 7) is needed for update_continuation_histories(ss - 1) which accesses (ss - 6),
	// (ss + 2) is needed for initialization of cutOffCnt.
	Stack  stack[MAX_PLY + 1] = {};
	Stack* ss = stack + 1;

	for (int i = 1; i < 2; i++) {
		(ss - i)->staticEval = staticEval(rootPos);
	}

	for (int i = 0; i < MAX_PLY; i++)
		(ss + i)->ply = i;

	ss->pv = pv;

	size_t multiPV = size_t(options["MultiPV"]);
	multiPV = std::min(multiPV, rootMoves.size());

	// Iterative deepening loop until requested to stop or the target depth is reached
	for (; rootDepth < 200 && !threads.stop; rootDepth = pick_next_depth(rootDepth)) {
		// Age out PV variability metric
		if (mainThread)
			totBestMoveChanges /= 2;///////////////////////////////////////////////////////////////time

		// Save the last iteration's scores before the first PV line is searched and
		// all the move scores except the (new) PV are set to -VAL_INF.
		for (RootMove& rm : rootMoves)
			rm.previousScore = rm.score;

		size_t pvFirst = 0;
		pvLast = rootMoves.size();

		// MultiPV loop. We perform a full root search for each PV line
		for (pvIdx = 0; pvIdx < multiPV; ++pvIdx) {
			// Reset UCI info selDepth for each depth and each PV line
			selDepth = 0;

			// Reset aspiration window starting size
			delta = 10 + std::abs(rootMoves[pvIdx].meanSquaredScore) / 44000;
			int avg = rootMoves[pvIdx].averageScore;
			alpha = std::max(avg - delta, -VALUE_INFINITE);
			beta = std::min(avg + delta, VALUE_INFINITE);

			// Start with a small aspiration window and, in the case of a fail
			// high/low, re-search with a bigger window until we don't fail
			// high/low anymore.
			int failedHighCnt = 0;
			while (true) {
				// Adjust the effective depth searched, but ensure at least one
				// effective increment for every four searchAgain steps (see issue #2717).
				int adjustedDepth = std::max(1, rootDepth - failedHighCnt / 4);
				rootDelta = beta - alpha;

				if (mainThread) {
					sync_cout << alpha << " " << beta << sync_endl;
				}
				bestValue = search(Root, rootPos, ss, alpha, beta, adjustedDepth, false);
				// Bring the best move to the front. It is critical that sorting
				// is done with a stable algorithm because all the values but the
				// first and eventually the new best one is set to -VAL_INF
				// and we want to keep the same order for all the moves except the
				// new PV that goes to the front. Note that in the case of MultiPV
				// search the already searched PV lines are preserved.
				std::stable_sort(rootMoves.begin() + pvIdx, rootMoves.begin() + pvLast);

				// If search has been stopped, we break immediately. Sorting is
				// safe because RootMoves is still valid, although it refers to
				// the previous iteration.
				if (threads.stop)
					break;

				// When failing high/low give some update before a re-search. To avoid
				// excessive output that could hang GUIs, only start at nodes > 10M
				// (rather than depth N, which can be reached quickly)
				if (mainThread && multiPV == 1 && (bestValue <= alpha || bestValue >= beta))
					main_manager()->pv(*this, threads, tt, rootDepth);

				// In case of failing low/high increase aspiration window and re-search,
				// otherwise exit the loop.
				if (bestValue <= alpha) {
					beta = (alpha + beta) / 2;
					alpha = std::max(bestValue - delta, -VALUE_INFINITE);

					failedHighCnt = 0;
				}
				else if (bestValue >= beta) {
					beta = std::min(bestValue + delta, VALUE_INFINITE);
					++failedHighCnt;
				}
				else {
					break;
				}

				delta += delta / 3;

				assert(alpha >= -VALUE_INFINITE && beta <= VALUE_INFINITE);
			}

			// Sort the PV lines searched so far and update the GUI
			std::stable_sort(rootMoves.begin() + pvFirst, rootMoves.begin() + pvIdx + 1);

			if (mainThread
				&& (threads.stop || pvIdx + 1 == multiPV)
				// A thread that aborted search can have mated-in PV and
				// score that cannot be trusted, i.e. it can be delayed or refuted
				// if we would have had time to fully search other root-moves. Thus
				// we suppress this output and below pick a proven score/PV for this
				// thread (from the previous iteration).
				&& !(threads.abortedSearch && is_loss(rootMoves[0].uciScore)))
					main_manager()->pv(*this, threads, tt, rootDepth);

			if (threads.stop)
				break;
		}

		if (!threads.stop)
			completedDepth = rootDepth;

		// We make sure not to pick an unproven mated-in score,
		// in case this thread prematurely stopped search (aborted-search).
		if (threads.abortedSearch && rootMoves[0].score != -VALUE_INFINITE
			&& is_loss(rootMoves[0].score)) {

			// Bring the last best move to the front for best thread selection.
			auto it = std::find_if(rootMoves.begin(), rootMoves.end(), [pv = lastBestPV[0]](
				const auto& rm) { return rm == pv; });
			if (it != rootMoves.end()) {
				std::rotate(rootMoves.begin(), it, it + 1);
			}

			rootMoves[0].pv = lastBestPV;
			rootMoves[0].score = rootMoves[0].uciScore = lastBestScore;
		}
		else if (rootMoves[0].pv[0] != lastBestPV[0]) {
			lastBestPV = rootMoves[0].pv;
			lastBestScore = rootMoves[0].score;
			lastBestMoveDepth = rootDepth;
		}

		if (!mainThread)
			continue;

		// Use part of the gained time from a previous stable move for the current move
		for (auto&& th : threads) {
			totBestMoveChanges += th->worker->bestMoveChanges;
			th->worker->bestMoveChanges = 0;
		}

		// Do we have time for the next iteration? Can we stop searching now?
		if (limits.use_time_management() && !threads.stop) {
			// Stop the search if we have exceeded the totalTime
			if (elapsed_time() > mainThread->tm.optimum())
				threads.stop = true;
		}
	}
}

// Reset histories, usually before a new game
void Worker::clear() {

}

//template............
int Worker::search(NType NT, Position& pos, Stack* ss, Value alpha, Value beta, Depth depth, bool cutNode) {

 	bool PvNode = NT != NonPV;
	bool rootNode = NT == Root;

	//深度小于零搜索VCF
	if (depth <= 0) {
		return qsearch(NT, pos, ss, alpha, beta);
	}

	assert(-VALUE_INFINITE <= alpha && alpha < beta && beta <= VALUE_INFINITE);
	assert(PvNode || (alpha == beta - 1));
	assert(0 < depth && depth < MAX_PLY);
	assert(!(PvNode && cutNode));

	Square pv[MAX_PLY + 1];
	const StateInfo& st = pos.st();;

	Key posKey;
	Square move, bestMove;// excludedMove;
	Depth extension, newDepth;
	Value bestValue, value, eval; //maxValue, probCutBeta;
	bool opT5, opT;

	// Step 1. Initialize node
	Worker* thisThread = this;
	Piece us = pos.side_to_move(), op = ~us;
	opT5 = st.cntT[T5][op];
	opT = st.cntT[TH4][op] | st.cntT[T4H3][op] | st.cntT[TDH3][op];
	bestValue = -VALUE_INFINITE;

	// Check for the available remaining time
	if (is_mainthread())
		main_manager()->check_time(*thisThread);

	// Used to send selDepth info to GUI (selDepth counts from 1, ply from 0)
	if (PvNode && thisThread->selDepth < ss->ply + 1)
		thisThread->selDepth = ss->ply + 1;

	// Step 2. Check for end of game and aborted search
	if (!rootNode) {

		//检查赢/输棋
		if (value = checkMate(ss, pos))
			return value;

		//检查平局
		if (pos.full())
			return VALUE_DRAW;

		if (threads.stop.load(std::memory_order_relaxed) || ss->ply >= MAX_PLY)
			return VALUE_DRAW;
		
		// Step 3. Mate distance pruning. Even if we mate at the next move our score
		// would be at best mate_in(ss->ply + 1), but if alpha is already bigger because
		// a shorter mate was found upward in the tree then there is no need to search
		// because we will never beat the current alpha. Same logic but with reversed
		// signs apply also in the opsite condition of being mated instead of giving
		// mate. In this case, return a fail-high score.
		alpha = std::max(mated_in(ss->ply), alpha);
		beta = std::min(mate_in(ss->ply + 1), beta);
		if (alpha >= beta) {
			return alpha;
		}

		//T5跳过节点 tobe further optimized
		if (opT5) {
			move = st.T5Square;

			pos.make_move(move);

			if (PvNode) {
				(ss + 1)->pv = pv;
				(ss + 1)->pv[0] = NULLSQUARE;

				value = -search(PV, pos, ss + 1, -beta, -alpha, depth, false);
			}
			else 
				value = -search(NonPV, pos, ss + 1, -beta, -alpha, depth, !cutNode);

			pos.undo();

			if (PvNode && !rootNode)
				updatePV(ss->pv, move, (ss + 1)->pv);

			return value;
		}
	}

	assert(0 <= ss->ply && ss->ply < MAX_PLY);

	bestMove = NULLSQUARE;

	// Step 4. Transposition table lookup
	posKey = pos.key();
	auto [ttHit, ttData, ttWriter] = tt.probe(posKey);
	// Need further processing of the saved data
	ttData.move = rootNode ? thisThread->rootMoves[thisThread->pvIdx].pv[0]
		: ttHit ? ttData.move
		: NULLSQUARE;
	ttData.value = ttHit ? value_from_tt(ttData.value, ss->ply) : VALUE_NONE;
	ss->ttPv = PvNode || (ttHit && ttData.is_pv);

	// At this point, if excluded, skip straight to step 5, static eval. However,
	// to save indentation, we list the condition in all code between here and there.

	// At non-PV nodes we check for an early TT cutoff
	if (!PvNode && ttData.depth > depth - (ttData.value <= beta)
		&& is_valid(ttData.value)  // Can happen when !ttHit or when access race in probe()
		&& (ttData.bound & (ttData.value >= beta ? BOUND_LOWER : BOUND_UPPER))
		&& (cutNode == (ttData.value >= beta) || depth > 9)) 
		return ttData.value;
	

	// Step 5. Static evaluation of the position
	if (opT) {
		// Skip early pruning when in check
		ss->staticEval = eval = -(ss - 1)->staticEval;
	}
	else if (ttHit) {
		// Never assume anything about values stored in TT
		ss->staticEval = eval = is_valid(ttData.eval) ? ttData.eval : staticEval(pos);
		// ttValue can be used as a better position evaluation
		if (is_valid(ttData.value)
			&& (ttData.bound & (ttData.value > eval ? BOUND_LOWER : BOUND_UPPER)))
			eval = ttData.value;
	}
	else {
		ss->staticEval = eval = staticEval(pos);
		// Static evaluation is saved as it was before adjustment by correction history
		ttWriter.write(posKey, VALUE_NONE, ss->ttPv, BOUND_NONE, DEPTH_UNSEARCHED, NULLSQUARE,
			eval, tt.generation());
	}

	// Step 6. Razoring
	// If eval is really low, skip search entirely and return the qsearch value.
	// For PvNodes, we must have a guard against mates being returned.
	if (!PvNode && eval + 10 * depth * depth + 48 < alpha) {
		testData[razor + (int)depth]++;
		value = qsearch(NonPV, pos, ss, alpha, alpha + 1);
		return value;
	}

	// Step 7. Futility pruning: child node
	// The depth condition is important for mate finding.
	if (!ss->ttPv && depth < 16 && 
		!is_loss(beta) && !is_win(eval) &&
		eval - (12 * depth * depth + 80) >= beta
		&& (!ttData.move)) {
		testData[futility + (int)depth]++;
		return eval;
	}

	// Step 9. Internal iterative reductions
	// For PV nodes without a ttMove as well as for deep enough cutNodes, we decrease depth.
	// This heuristic is known to scale non-linearly, current version was tested at VVLTC.
	// Further improvements need to be tested at similar time control if they make IIR
	// more aggressive.
	if ((PvNode || (cutNode && depth >= 7)) && !ttData.move) {
		depth -= 2;
		// Use qsearch if depth <= 0
		if (depth <= 0)
			return qsearch(PV, pos, ss, alpha, beta);
	}


	MovePicker mp(P_main, pos, ttData.move);

	value = bestValue;

	int moveCount = 0;

	// Step 12. Loop through all pseudo-legal moves until no moves remain
	// or a beta cutoff occurs.
	while ((move = mp.nextMove()) != NULLSQUARE) {

		// At root obey the "searchmoves" option and skip moves not listed in Root Move List.
		// In MultiPV mode we also skip PV moves that have been already searched.
		if (rootNode
			&& !std::count(thisThread->rootMoves.begin() + thisThread->pvIdx,
						   thisThread->rootMoves.begin() + thisThread->pvLast, move))
			continue;

		moveCount++;

		if (rootNode && is_mainthread()) {
			main_manager()->updates.onIter({ (int)depth, move, moveCount + thisThread->pvIdx });
		}

		if (PvNode)
			(ss + 1)->pv = nullptr;

		ss->moveFT[P1] = pos.type(P1, move);
		ss->moveFT[P2] = pos.type(P2, move);

		bool trivialMove = ss->moveFT[P1] == TNone && ss->moveFT[P2] == TNone;

		extension = 0;
		int disop = Square::dis1(st.move, move);
		int disus = Square::dis1(pos.prevst().move, move);
		bool dispersed = disus > 4 && disop > 4;
		bool improving = ss->staticEval - (ss - 2)->staticEval > 0;

		//pruning at shallow depth
		if (!rootNode && !is_loss(bestValue)) {
			
			// Prun distract defence move which is likely to delay a winning
			if (dispersed && opT && ss->moveFT[op] < T4 && depth <= 4) {
				testData[dispersedT]++;
				continue;
			}

			// Skip trivial moves at lower depth
			if (trivialMove && depth <= 4) break;

			// Move count pruning: skip move if movecount is above threshold
			if (moveCount > (1.6 * depth + 3) / (2 - improving)) break;
		}

		newDepth = depth - (st.cntT[TH4][op] ? 1 : 2);

		// Step 15. Make the move
		pos.make_move(move);
		thisThread->nodes.fetch_add(1, std::memory_order_relaxed);
		uint64_t nodeCount = rootNode ? uint64_t(nodes) : 0;

		//if (ss->moveFT[us] != T4)
			//tt.prefetch(newSt.key);

		if (!PvNode || moveCount > 1) {
			value = -search(NonPV, pos, ss + 1, -alpha - 1, -alpha, newDepth, true);
		}

		if (PvNode && (moveCount == 1 || value > alpha && (value < beta || rootNode))) {

			(ss + 1)->pv = pv;
			(ss + 1)->pv[0] = NULLSQUARE;

			// Extend move from transposition table if we are about to dive into qsearch.
			if (move == ttData.move && ss->ply <= thisThread->rootDepth * 2)
				newDepth = std::max(newDepth, 1);

			value = -search(PV, pos, ss + 1, -beta, -alpha, newDepth, false);
		}

		// Step 18. Undo move
		pos.undo();

		// Step 19. Check for a new best move
		// Finished searching the move. If a stop occurred, the return value of
		// the search cannot be trusted, and we return immediately without updating
		// best move, principal variation nor transposition table.
		if (threads.stop.load(std::memory_order_relaxed))
			return VALUE_ZERO;
		
		if (rootNode) {

			RootMove& rm =
				*std::find(thisThread->rootMoves.begin(), thisThread->rootMoves.end(), move);

			rm.effort += nodes - nodeCount;

			rm.averageScore =
				rm.averageScore != -VALUE_INFINITE ? (2 * value + rm.averageScore) / 3 : value;

			rm.meanSquaredScore = rm.meanSquaredScore != -VALUE_INFINITE * VALUE_INFINITE
				? (value * std::abs(value) + rm.meanSquaredScore) / 2
				: value * std::abs(value);

			if (moveCount == 1 || value > alpha) {

				rm.score = rm.uciScore = value;
				rm.selDepth = thisThread->selDepth;
				rm.scoreLowerbound = rm.scoreUpperbound = false;

				if (value >= beta){
					rm.scoreLowerbound = true;
					rm.uciScore = beta;
				}
				else if (value <= alpha){
					rm.scoreUpperbound = true;
					rm.uciScore = alpha;
				}

				rm.pv.resize(1);

				assert((ss + 1)->pv);

				for (Square* m = (ss + 1)->pv; *m != NULLSQUARE; ++m)
					rm.pv.push_back(*m);

				// We record how often the best move has been changed in each iteration.
				// This information is used for time management. In MultiPV mode,
				// we must take care to only do this for the first PV line.
				if (moveCount > 1 && !thisThread->pvIdx)
					++thisThread->bestMoveChanges;
			}
			else {
				// All other moves but the PV, are set to the lowest value: this
				// is not a problem when sorting because the sort is stable and the
				// move position in the list is preserved - just the PV is pushed up.
				rm.score = -VALUE_INFINITE;
			}
		}

		if (value > bestValue) {

			bestValue = value;

			if (value > alpha) {

				bestMove = move;

				if (PvNode && !rootNode)  // Update pv even in fail-high case
					updatePV(ss->pv, move, (ss + 1)->pv);

				if (value >= beta) {
					testData[betacutoff]++;
					break;
				}
				else {
					alpha = value;  // Update alpha! Always alpha < beta
				}
			}
		}
	}

	// Step 20. Check for mate
	// All legal moves have been searched and if there are no legal moves,
	// it must be a mate. If we are in a singular extension search then
    // return a fail low score.
	if (!moveCount)
		bestValue = mated_in(ss->ply);

	// If no good move is found and the previous position was ttPv, then the previous
   // opponent move is probably good and the new position is added to the search tree. (~7 Elo)
	if (bestValue <= alpha)
		ss->ttPv = ss->ttPv || ((ss - 1)->ttPv && depth > 3);

	// Write gathered information in transposition table. Note that the
	// static evaluation is saved as it was before correction history.
	if (!(rootNode && thisThread->pvIdx))
		ttWriter.write(posKey, value_to_tt(bestValue, ss->ply), ss->ttPv,
			bestValue >= beta ? BOUND_LOWER
			: PvNode && bestMove ? BOUND_EXACT
			: BOUND_UPPER,
			depth, bestMove, ss->staticEval, tt.generation());

	assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);

	return bestValue;
}

int Worker::qsearch(NType NT, Position& pos, Stack* ss, Value alpha, Value beta) {

	bool PvNode = NT != NonPV;

	assert(alpha >= -VALUE_INFINITE && alpha < beta && beta <= VALUE_INFINITE);
	assert(PvNode || (alpha == beta - 1));

	Square pv[MAX_PLY + 1];
	const StateInfo& st = pos.st();

	Key   posKey;
	Square move, bestMove;// excludedMove;
	Value bestValue, value; //maxValue, probCutBeta;
	bool pvHit, opT5, opT;
	int moveCount;

	// Step 1. Initialize node
	if (PvNode) {
		(ss + 1)->pv = pv;
		ss->pv[0] = NULLSQUARE;
	}

	Worker* thisThread = this;
	Piece us = pos.side_to_move(), op = ~us;
	opT5 = st.cntT[T5][op];
	opT = st.cntT[TH4][op] | st.cntT[T4H3][op] | st.cntT[TDH3][op];
	bestMove = NULLSQUARE;
	moveCount = 0;

	// Used to send selDepth info to GUI (selDepth counts from 1, ply from 0)
	if (PvNode && thisThread->selDepth < ss->ply + 1)
		thisThread->selDepth = ss->ply + 1;

	//检查赢/输棋
	if (value = checkMate(ss, pos)) 
		return value;
	
	//检查平局
	if (pos.full()) 
		return VALUE_DRAW;

	//max ply
	if (ss->ply >= MAX_PLY) 
		return staticEval(pos);
	
	//mate distance pruning
	alpha = std::max(mated_in(ss->ply), alpha);
	beta = std::min(mate_in(ss->ply + 1), beta);
	if (alpha >= beta) 
		return alpha;
	
	//T5跳过节点
	if (opT5) {

		move = st.T5Square;
		pos.make_move(move);
		
		value = -qsearch(NT, pos, ss + 1, -beta, -alpha);
		
		pos.undo();

		if (PvNode) 
			updatePV(ss->pv, move, (ss + 1)->pv);

		return value;
	}

	// Step 3. Transposition table lookup
	posKey = pos.key();
	auto [ttHit, ttData, ttWriter] = tt.probe(posKey);
	// Need further processing of the saved data
	ttData.move = ttHit ? ttData.move : NULLSQUARE;
	ttData.value = ttHit ? value_from_tt(ttData.value, ss->ply) : VALUE_NONE;
	pvHit = ttHit && ttData.is_pv;

	// At non-PV nodes we check for an early TT cutoff
	if (!PvNode && ttData.depth >= DEPTH_QS
		&& is_valid(ttData.value)  // Can happen when !ttHit or when access race in probe()
		&& (ttData.bound & (ttData.value >= beta ? BOUND_LOWER : BOUND_UPPER))) 
		return ttData.value;


	// Step 4. Static evaluation of the position
	if (ttHit) {
		// Never assume anything about values stored in TT
		ss->staticEval = bestValue = is_valid(ttData.eval) ? ttData.eval : staticEval(pos);

		// ttValue can be used as a better position evaluation (~7 Elo)
		if (is_valid(ttData.value)
			&& (ttData.bound & (ttData.value > bestValue ? BOUND_LOWER : BOUND_UPPER)))
			bestValue = ttData.value;
	}
	else {
		ss->staticEval = bestValue = staticEval(pos);
	}

	// Stand pat. Return immediately if static value is at least beta
	if (bestValue >= beta) {
		if (!ttHit)
			ttWriter.write(posKey, value_to_tt(bestValue, ss->ply), false, BOUND_LOWER,
				DEPTH_UNSEARCHED, NULLSQUARE, ss->staticEval, tt.generation());
		return bestValue;
	}

	if (bestValue > alpha)
		alpha = bestValue;

	MovePicker mp(P_VCF, pos, ttData.move);
	while ((move = mp.nextMove()) != NULLSQUARE) {

		moveCount++;

		pos.make_move(move);
		thisThread->nodes.fetch_add(1, std::memory_order_relaxed);

		value = -qsearch(NT, pos, ss + 1, -beta, -alpha);

		pos.undo();

		if (value > bestValue) {

			bestValue = value;

			if (value > alpha) {

				bestMove = move;

				if (PvNode)  // Update pv even in fail-high case
					updatePV(ss->pv, move, (ss + 1)->pv);

				if (value < beta)  // Update alpha
					alpha = value;
				else {
					testData[vcfbetacutoff]++;
					break;
				}
			}
		}
	}

	// Save gathered info in transposition table. The static evaluation
	// is saved as it was before adjustment by correction history.
	ttWriter.write(posKey, value_to_tt(bestValue, ss->ply), pvHit,
		bestValue >= beta ? BOUND_LOWER : BOUND_UPPER, DEPTH_QS, bestMove,
		ss->staticEval, tt.generation());

	assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);

	return bestValue;
}

TimePoint Worker::elapsed_time() const { return main_manager()->tm.elapsed_time(); }

// Used to print debug info and, more importantly, to detect
// when we are out of available time and thus stop the search.
void SearchManager::check_time(Worker& worker) {

	if (--callsCnt > 0)
		return;

	callsCnt = 512;

	static TimePoint lastInfoTime = now();

	TimePoint elapsed = tm.elapsed_time();
	TimePoint tick = worker.limits.startTime + elapsed;

	if (tick - lastInfoTime >= 500) {
		lastInfoTime = tick;
		//PrintTest();//////////////////////////////////////////////////////////////////////
	}

	if (
		// Later we rely on the fact that we can at least use the mainthread previous
		// root-search score and PV in a multithreaded environment to prove mated-in scores.
		worker.completedDepth >= 1
		&& ((worker.limits.use_time_management() && (elapsed > tm.maximum()))
			|| (worker.limits.movetime && elapsed >= worker.limits.movetime))) {
		worker.threads.stop = worker.threads.abortedSearch = true;
	}
}

void SearchManager::pv(const Worker& worker,
	const ThreadPool& threads,
	const TranspositionTable& tt,
	Depth                     depth) const {

	const auto  nodes = threads.nodes_searched();
	const auto& rootMoves = worker.rootMoves;
	const auto& pos = worker.rootPos;
	size_t      pvIdx = worker.pvIdx;
	size_t      multiPV = std::min(size_t(worker.options["MultiPV"]), rootMoves.size());

	for (size_t i = 0; i < multiPV; ++i) {

		bool updated = rootMoves[i].score != -VALUE_INFINITE;

		if (depth == 1 && !updated && i > 0)
			continue;

		Depth d = updated ? depth : std::max(1, depth - 1);
		Value v = updated ? rootMoves[i].uciScore : rootMoves[i].previousScore;

		if (v == -VALUE_INFINITE)
			v = VALUE_ZERO;

		auto bound = rootMoves[i].scoreLowerbound
			? "lowerbound"
			: (rootMoves[i].scoreUpperbound ? "upperbound" : "");

		InfoFull info;

		info.depth = d;
		info.selDepth = rootMoves[i].selDepth;
		info.multiPV = i + 1;
		info.score = v;

		if (i == pvIdx && updated)  // previous-scores are exact
			info.bound = bound;

		TimePoint time = std::max(TimePoint(1), tm.elapsed_time());
		info.timeMs = time;
		info.nodes = nodes;
		info.nps = nodes * 1000 / time;
		info.pv = rootMoves[i].pv;

		updates.onUpdateFull(info);
	}
}