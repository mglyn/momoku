//#include "selfplay.h"
//
//void storeGame(SelfPlay::State game, int game_result){
//
//
//}
//
//void SelfPlay::SelfPlay(int gameSize){
//
//
//	for (int finished = 0, ply = 0 ;finished < 100; finished++) {
//		// initialize new game
//		std::vector<State> game;
//		uint8_t game_result = 0;
//		bool stm = 0;
//		Board bd;
//
//		// start self-play
//		for (ply = 0; true; ply++) {
//			// run alpha-beta search for the best move and score
//			auto result = Search::search(bd);
//
//			// temporarily store the new sample to game
//			game[ply] = (position.PackedSfen(), score, move, ply);
//
//			// execute self-play move
//			position.makeMove(move);
//
//			if (bd.cntMove() == gameSize * gameSize || bd.cntFT(T5, bd.self())) { // game is over
//				game_result = position.gameResult();
//				break;
//			}
//			if (score >= WIN_CRITICAL) { // score has exceeded eval_limit
//				break;
//			}
//		}
//
//		// store last finished game
//		storeGame(game, game_result);
//		finished += ply;
//		
//	}
//}

