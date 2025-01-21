#include <iostream>

#include "search.h"
#include "tt.h"
#include "test.h"

namespace Search {
	std::vector<Worker> workers;

	void updatePV(Sqare* pv, Sqare sq, Sqare* childPV) {
		for (*pv++ = sq; childPV && *childPV;)
			*pv++ = *childPV++;
		*pv = NULLSQARE;
	}

	static constexpr int checkMate(const Stack* ss, const Position& pos) {
		Piece us = pos.side_to_move(), op = ~us; 
		const StateInfo& st = pos.stateInfo();

		if (st.cntT[T5][us]) return mate_in(ss->ply);

		int op5 = st.cntT[T5][op];
		if (op5) {
			return op5 > 1 ? mated_in(ss->ply + 1) : 0;
		}

		if (st.cntT[TH4][us])return mate_in(ss->ply + 2);

		if (st.cntT[TDH3][us] + st.cntT[T4H3][us] && !st.cntT[T4][op]) {
			testData[TDH3T4H3wincheck]++;
			return mate_in(ss->ply + 4);
		}

		return VAL_ZERO;
	}

	std::tuple<std::vector<Sqare>, int> search(Position& pos) {

		tt.newGen();

		workers.clear();
		for (int i = 0; i < threadNum; i++)
			workers.emplace_back(pos);

		workers[0].iterative_deepening();

		std::vector<Sqare> answer;

		answer.push_back(workers[0].rootMoves[0].sq);
		for (auto& i : workers[0].rootMoves[0].pv)	//pv line 
			answer.push_back(i);

		return { answer, workers[0].rootMoves[0].val };
	}

	void Worker::iterative_deepening() {

		Sqare pv[MAX_PLY + 1];

		RootMove lastBestMove(NULLSQARE, -VAL_INF, -VAL_INF);

		int bestVal = -VAL_INF;
		std::vector<Sqare> moves = genRootMove(rootPos);

		Piece us = rootPos.side_to_move();
		if (rootPos.cntMove() == 0) {  // beginning move
			rootMoves.push_back(RootMove({ 7,7 }, 0, 0));
			return;
		}
		if (rootSt.cntT[T5][us]) { // winning move
			Sqare sq = *find_if(moves.begin(), moves.end(), 
				[this, &us](Sqare sq) {return rootPos.type(us, sq) == T5; });
			rootMoves.push_back(RootMove(sq, 0, 0));
			return;
		}

		int multiPV = 1;                  //num of multiPV
		bool stop = false;

		// Allocate stack with extra size to allow access from (ss - 7) to (ss + 2):
		// (ss - 7) is needed for update_continuation_histories(ss - 1) which accesses (ss - 6),
		// (ss + 2) is needed for initialization of cutOffCnt and killers.
		Stack stack[MAX_PLY + 10] = {};
		Stack* ss = stack + 7;

		for (int i = 7; i > 0; i--)
			(ss - i)->eval = -VAL_INF;

		for (int i = 0; i < MAX_PLY + 2; i++)
			(ss + i)->ply = i;

		ss->pv = pv;

		//init rootMoves
		for (auto& move : moves)
			rootMoves.emplace_back(move, -VAL_INF, -VAL_INF);

		multiPV = std::min(multiPV, (int)rootMoves.size());

		for (int dep = 2; !stop && dep <= END_DEP; dep++) {   //iterative deepening loop

			for (auto& rm : rootMoves)
				rm.lastVal = rm.val;

			for (pvIdx = 0; !stop && pvIdx < multiPV; pvIdx++) { // multiPVloop

				//aspiration search
				auto& rm = rootMoves[pvIdx];
				int delta = std::abs(rootMoves[pvIdx].meanSquared) / 38000 + 2200;
				int alpha = std::max(rm.avg - delta, -WIN_MAX);
				int beta = std::min(rm.avg + delta, WIN_MAX);
				int failHighCnt = 0;

				while (true) { // aspiration search loop

					bestVal = search(Root, rootPos, ss, alpha, beta, dep - failHighCnt / 4, false);

					testText += "d: " + std::to_string(dep) + "  ab: " + std::to_string(alpha) + " " + std::to_string(beta) + "  v: " + std::to_string(bestVal) + "\n";

					auto cmpRootMove = [](const RootMove& a, const RootMove& b) {
						return a.val == b.val ? a.lastVal > b.lastVal : a.val > b.val;
						};
					std::stable_sort(rootMoves.begin() + pvIdx, rootMoves.end(), cmpRootMove);

					if (timer.time_out()) {
						stop = true;
						break;
					}

					if (bestVal <= alpha) {
						beta = (alpha + beta) / 2;
						alpha = std::max(bestVal - delta, -WIN_MAX);
						failHighCnt = 0;
					}
					else if (bestVal >= beta) {
						beta = std::min(bestVal + delta, WIN_MAX);
						failHighCnt++;
					}
					else break;

					delta = std::min(delta * 4 / 3, VAL_INF);

					assert(-WIN_MAX <= alpha && alpha < beta && beta <= WIN_MAX);
				}

				// Sort the PV lines searched so far
				auto cmpRootMove = [](const RootMove& a, const RootMove& b) {
					return a.val == b.val ? a.lastVal > b.lastVal : a.val > b.val;
					};
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + pvIdx + 1, cmpRootMove);
			}

			if (rootMoves[0].val >= WIN_CRITICAL)break;

			// We make sure not to pick an unproven mated-in score,
			// in case this thread prematurely stopped search (aborted-search).
			if (stop && rootMoves[0].val <= -WIN_CRITICAL) {

				auto cmp = [&lastBestMove](const RootMove& rma, const RootMove& rmb) {
					return rma.sq.x() == lastBestMove.sq.x() && rma.sq.y() == lastBestMove.sq.y()
					> rmb.sq.x() == lastBestMove.sq.x() && rmb.sq.y() == lastBestMove.sq.y();
				};

				std::stable_sort(rootMoves.begin(), rootMoves.end(), cmp);
				rootMoves[0] = lastBestMove;
			}

			lastBestMove = rootMoves[0];
		}

		/*for (int i = 0; i < rootMoves.size(); i++) {
			std::cout << rootMoves[i].sq.x() << " " << rootMoves[i].sq.y() << "\n";
			std::cout << "val: " << rootMoves[i].val << "lastval: " << rootMoves[i].lastVal << "\n";
		}*/
	}

	//template............
	int Worker::search(NType NT, Position& pos, Stack* ss, int alpha, int beta, float dep, bool cutNode) {

		bool pvNode = NT != NonPV;
		bool rootNode = NT == Root;

		//深度小于零搜索VCF
		if (dep <= 0)
			return VCFSearch(NT, pos, ss, alpha, beta, 0);

		assert(-VAL_INF <= alpha && alpha < beta && beta <= VAL_INF);
		assert(pvNode || (alpha == beta - 1));
		assert(!(pvNode && cutNode));

		Sqare pv[MAX_PLY + 1];
		const StateInfo& st = pos.stateInfo();
		StateInfo newSt;

		Sqare move, bestMove;// excludedMove;
		float extension, newDep;
		int bestVal, val, eval; //maxValue, probCutBeta;
		//bool givesCheck, improving, priorCapture, opnentWorsening;
		//bool capture, ttCapture;

		// Step 1. Initialize node
		Piece us = pos.side_to_move(), op = ~us;
		bool opT5 = st.cntT[T5][op];
		bool opT = st.cntT[TH4][op] | st.cntT[T4H3][op] | st.cntT[TDH3][op];
		int moveCnt = 0;
		bestVal = -VAL_INF;
		bestMove = NULLSQARE;

		// Check for the available remaining time
		/*if (is_mainthread())
			main_manager()->check_time(*thisThread);*/

		// Used to send selDepth info to GUI (selDepth counts from 1, ply from 0)
		/*if (PvNode && thisThread->selDepth < ss->ply + 1)
			thisThread->selDepth = ss->ply + 1;*/

		testData[node]++;

		// Step 2. Check for end of game and aborted search
		if (!rootNode) {

			//检查赢/输棋
			if (val = checkMate(ss, pos))
				return val;

			//检查平局
			if (pos.draw())
				return VAL_DRAW;

			/*if (threads.stop.load(std::memory_order_relaxed))
				return (!ss->inCheck) ? evaluate(pos) : VAL_DRAW;*/

			//max ply
			if (ss->ply > MAX_PLY)
				return pos.staticEval();

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

			//T5跳过节点
			if (opT5) {
				move = st.T4cost;

				pos.make_move(move, newSt);
				tt.prefetch(newSt.key);

				if (pvNode) {
					pv[0] = NULLSQARE;
					(ss + 1)->pv = pv;

					val = -search(PV, pos, ss + 1, -beta, -alpha, dep, false);
				}
				else val = -search(NonPV, pos, ss + 1, -beta, -alpha, dep, !cutNode);

				pos.undo();

				if (pvNode && !rootNode)
					updatePV(ss->pv, move, (ss + 1)->pv);

				return val;
			}
		}
		
		(ss + 2)->killer[0] = (ss + 2)->killer[1] = NULLSQARE;
		
		int ttVal = -VAL_INF;
		Sqare ttMove = NULLSQARE;
		int ttDep = -VAL_INF;
		HashType ttBound = B_Initial;
		HashEntry* tte = nullptr;

		//估值
		eval = pos.staticEval();//????
		//bool improving = eval - pos.lastEval(2) > 0;

		//访问置换表
		bool ttHit = tt.probe(st.key, tte);
		if (ttHit) {
			ttVal = tte->value(ss->ply);
			ttMove = tte->movePos();
			ttDep = tte->depth();
			ttBound = tte->type();

			if (!ss->excludedMove)
				ss->ttpv = pvNode || tte->isPV();
		}

		if (!pvNode && !ss->excludedMove && ttDep > dep && //???HIt??
			std::abs(ttVal) < WIN_CRITICAL &&
			(ttVal >= beta ? (ttBound & B_Lower) : (ttBound & B_Upper))) {

			if (ttMove != NULLSQARE && pos[tte->movePos()] != Empty) {
				testData[tothasherror]++;
			}
			testData[TTcutoff]++;
			return ttVal;
		}

		//razoring
		if (dep <= 4 && eval + 17 * dep * dep + 39 < alpha) {
			testData[razor + (int)dep]++;
			return VCFSearch(NonPV, pos, ss, alpha, alpha + 1, 0);
		}

		//futility pruning
		if (!ss->ttpv && !opT && beta > -WIN_CRITICAL &&
			eval - (17 * dep * dep + 58) > beta) {
			testData[futility + (int)dep]++;
			return eval;
		}


		//内部迭代加深  
		if (!rootNode && pvNode && !ttMove)
			dep -= 2.f;

		if (dep <= 0)
			return VCFSearch(NT, pos, ss, alpha, beta, 0);

		if (dep >= 9 && cutNode && !ttMove)
			dep -= 3.f;

		//尝试所有着法直到产生beta截断
		MovePicker mp(P_main, pos, ttMove, ss->killer[0], ss->killer[1]);
		bool skipQuietMove = false;

		for (; move = mp.nextMove(skipQuietMove); ) {

			if (rootNode && 
				!std::count_if(rootMoves.begin() + pvIdx, rootMoves.end(), [&move](auto& a) {return move == a.sq; }))
				continue;

			if (move == ss->excludedMove)continue;

			if (pos[move] != Empty)continue;

			moveCnt++;
			ss->moveFT[P1] = pos.type(P1, move);
			ss->moveFT[P2] = pos.type(P2, move);

			int disop = Sqare::dis1(st.move, move);
			int disus = Sqare::dis1(st.prev->move, move);
			bool dispersed = disus > 4 && disop > 4;

			//pruning at shallow depth
			if (!rootNode && bestVal > -WIN_CRITICAL) {
				//moveCount pruning
				if (!skipQuietMove && moveCnt > .5f * dep * dep + 3) {
					testData[moveCntpruning]++;
					skipQuietMove = true;
				}

				//prune the dispersed move
				if (dispersed && opT && ss->moveFT[op] < T4 && dep <= 4 && moveCnt > .6f * dep * dep) {
					testData[dispersedT]++;
					continue;
				}

				//maxExpand
				if (moveCnt > std::max(12.f, 36.f - 2 * ss->ply)) break;
			}

			newDep = dep  - (st.cntT[TH4][op] ?
				logf(std::max(st.cntT[TH4][op] + st.cntT[TH4][op] + st.cntT[T4][us], 1)) / 1.3 :
				logf(std::max(st.range.area() - pos.cntMove(), 1)) / 1.3);

			pos.make_move(move, newSt);

			if (ss->moveFT[us] != T4)
				tt.prefetch(newSt.key);

			if (!pvNode || moveCnt > 1) {
				val = -search(NonPV, pos, ss + 1, -alpha - 1, -alpha, newDep, true);
			}

			if (pvNode && (moveCnt == 1 || val > alpha && (val < beta || rootNode))) {

				pv[0] = NULLSQARE;
				(ss + 1)->pv = pv;

				val = -search(PV, pos, ss + 1, -beta, -alpha, newDep, false);
			}

			pos.undo();

			if (timer.time_out()) return 0;

			if (rootNode) {
				RootMove& rm = *std::find_if(rootMoves.begin(), rootMoves.end(), [&move](auto& a) {return a.sq == move; });

				rm.avg = rm.avg == -VAL_INF ? val : (2 * val + rm.avg) / 3;
				rm.meanSquared = rm.meanSquared == -VAL_INF * VAL_INF ? 
					val * std::abs(val) : (val * std::abs(val) + rm.meanSquared) / 2;

				if (moveCnt == 1 || val > alpha) {

					rm.val = val;

					rm.pv.clear();

					assert((ss + 1)->pv);
					for (Sqare* p = (ss + 1)->pv; *p; p++)
						rm.pv.push_back(*p);
				}
				else rm.val = -VAL_INF;
			}

			if (val > bestVal) {
				bestVal = val;

				if (val > alpha) {
					bestMove = move;

					// Update pv even in fail-high case
					if (pvNode && !rootNode) updatePV(ss->pv, move, (ss + 1)->pv);

					if (pvNode && val < beta)  // Update alpha
						alpha = val;
					else {
						testData[betacutoff]++;
						break;
					}
				}
			}
		}

		// Step 20. Check for mate (in renju)
		// All legal moves have been searched and if there are no legal moves,
		// it must be a mate. 


		// Write gathered information in transposition table.
		HashType bound = bestVal >= beta ? B_Lower :
			pvNode && bestMove ? B_Exact : B_Upper;
		tte->store(st.key, ss->ttpv, bestMove, bestVal, dep, ss->ply, bound);

		return bestVal;
	}

	int Worker::VCFSearch(NType NT, Position& pos, Stack* ss, int alpha, int beta, float dep) {

		testData[vcfnode]++;

		bool pvNode = NT != NonPV;
		bool rootNode = NT == Root;

		assert(-VAL_INF <= alpha && alpha < beta && beta <= VAL_INF);
		assert(pvNode || (alpha == beta - 1));

		Sqare pv[MAX_PLY + 1];
		const StateInfo& st = pos.stateInfo();
		StateInfo newSt;

		Sqare move, bestMove;// excludedMove;
		float extension, newDep;
		int bestVal, val, eval; //maxValue, probCutBeta;
		//bool givesCheck, improving, priorCapture, opnentWorsening;
		//bool capture, ttCapture;

		// Step 1. Initialize node
		Piece us = pos.side_to_move(), op = ~us;
		bool opT5 = st.cntT[T5][op];
		bool opT = st.cntT[TH4][op] | st.cntT[T4H3][op] | st.cntT[TDH3][op];
		int moveCnt = 0;
		bestVal = -VAL_INF;
		bestMove = NULLSQARE;

		//检查赢/输棋
		if (val = checkMate(ss, pos)) return val;

		//检查平局
		if (pos.draw()) return VAL_DRAW;

		//max ply
		if (ss->ply > MAX_PLY)return pos.staticEval();

		//mate distance pruning
		alpha = std::max(mated_in(ss->ply), alpha);
		beta = std::min(mate_in(ss->ply + 1), beta);
		if (alpha >= beta) return alpha;

		//T5跳过节点
		if (opT5) {
			Sqare move = st.T4cost;

			pos.make_move(move, newSt);

			if (pvNode) {
				pv[0] = NULLSQARE;
				(ss + 1)->pv = pv;

				val = -VCFSearch(PV, pos, ss + 1, -beta, -alpha, dep);
			}
			else val = -VCFSearch(NonPV, pos, ss + 1, -beta, -alpha, dep);

			pos.undo();

			if (pvNode) updatePV(ss->pv, move, (ss + 1)->pv);

			return val;
		}

		HashEntry* tte;
		int ttVal = -VAL_INF;
		Sqare ttMove = NULLSQARE;
		int ttDep = -VAL_INF;
		bool pvHit = false;
		HashType ttBound = B_Initial;

		//访问置换表
		bool ttHit = tt.probe(st.key, tte);
		if (ttHit) {
			ttVal = tte->value(ss->ply);

			ttMove = tte->movePos();
			if (ttMove != NULLSQARE && pos[tte->movePos()] != Empty) {
				ttMove = NULLSQARE;
				testData[tothasherror]++;
			}

			ttDep = tte->depth();
			pvHit = tte->isPV();
			ttBound = tte->type();
		}

		//使用置换表中的值截断
		if (ttDep >= dep && ttVal != -VAL_INF) {
			if (ttBound & B_Lower)
				alpha = std::max(alpha, ttVal);
			if (ttBound & B_Upper)
				beta = std::min(beta, ttVal);
			if (alpha >= beta) {
				testData[vcfTTcutoff]++;
				return ttVal;
			}
		}

		bestVal = pos.staticEval();
		if (bestVal >= beta) {
			return bestVal;
		}

		if (pvNode) alpha = std::max(alpha, bestVal);

		MovePicker mp(P_VCF, pos, NULLSQARE);
		for (Sqare move; move = mp.nextMove(); ) {

			pos.make_move(move, newSt);
			tt.prefetch(newSt.key);

			if (pvNode) {
				pv[0] = NULLSQARE;
				(ss + 1)->pv = pv;
			}

			val = -VCFSearch(NT, pos, ss + 1, -beta, -alpha, dep - 2);

			pos.undo();

			if (timer.time_out())return 0;

			if (val > bestVal) {
				bestVal = val;

				if (val > alpha) {
					bestMove = move;

					// Update pv even in fail-high case
					if (pvNode) updatePV(ss->pv, move, (ss + 1)->pv);

					if (pvNode && val < beta)  // Update alpha
						alpha = val;
					else {
						testData[vcfbetacutoff]++;
						break;
					}
				}
			}
		}

		HashType bound = bestVal >= beta ? B_Lower : B_Upper;
		tte->store(st.key, pvHit, bestMove, bestVal, dep, ss->ply, bound);

		return bestVal;
	}
}

