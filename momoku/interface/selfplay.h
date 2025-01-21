#ifndef SELFPLAY
#define SELFPLAY

#include "../search.h"

namespace SelfPlay{

	struct State{
		uint8_t stm : 1;			// 0, 1
		uint8_t result : 2;			// 0, 1, 2
		uint8_t gameSize : 5;	
		Pos move;				
		int data[25];  //800 bits
	};

	void SelfPlay(int gameSize);
}





#endif