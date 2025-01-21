#ifndef DATA
#define DATA

#include "common.h"

#include <iostream>
#include <fstream>
#include <cassert>

namespace Data {
	std::vector<std::tuple<int, std::vector<int>>> LoadPattern(std::string path = "");
	std::vector<std::tuple<uint8_t, FType>> LoadEval(std::string path = "");
}

#endif
