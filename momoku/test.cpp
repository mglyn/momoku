#include <string>
#include <iostream>
#include <algorithm>
#include <vector>

#include "test.h"
#include "tt.h"

int testData[128];

std::string testText;

void PrintTest() {

	testText += "--------- TEST INFO ---------\n";
	testText += "node: " + std::to_string(testData[node] / 1000.f) + " k nodes\n";
	testText += "vcfnode: " + std::to_string(testData[vcfnode] / 1000.f) + " k nodes\n";
	testText += "totnode: " + std::to_string((testData[vcfnode] + testData[node]) / 1000.f) + " k nodes\n\n";
	testData[node] = testData[vcfnode] = 0;
	testText += "TT cutoff: " + std::to_string(testData[TTcutoff] / 1000.f) + "\n";
	testData[TTcutoff] = 0;
	testText += "beta cutoff: " + std::to_string(testData[betacutoff] / 1000.f) + "\n";
	testData[betacutoff] = 0;
	testText += "vcf TT cutoff: " + std::to_string(testData[vcfTTcutoff] / 1000.f) + "\n";
	testData[vcfTTcutoff] = 0;
	testText += "vcf beta cutoff: " + std::to_string(testData[vcfbetacutoff] / 1000.f) + "\n";
	testData[vcfbetacutoff] = 0;
	testText += "moveCnt pruning: " + std::to_string(testData[moveCntpruning] / 1000.f) + "\n";
	testData[moveCntpruning] = 0;
	testText += "dispersed T: " + std::to_string(testData[dispersedT] / 1000.f) + "\n";
	testData[dispersedT] = 0;
	testText += "TDH3 T4H3 win check: " + std::to_string(testData[TDH3T4H3wincheck] / 1000.f) + "\n";
	testData[TDH3T4H3wincheck] = 0;
	testText += "tot hash error: " + std::to_string(testData[tothasherror]) + "\n";
	testData[tothasherror] = 0;
	testText += "numThreats: " + std::to_string(testData[numThreats]) + "\n";
	testData[numThreats] = 0;

	testText += "razor:\n";
	for (int i = 0; i < 32; i++) {
		testText += std::to_string(testData[razor + i]) + " ";
		testData[razor + i] = 0;
	}
	testText += "\n";

	std::cout << "futility:\n";
	for (int i = 0; i < 32; i++) {
		testText += std::to_string(testData[futility + i]) + " ";
		testData[futility + i] = 0;
	}
	testText += "\n";

	testText += "tt usage: " + std::to_string(tt.usage() / 10.f) + "%\n";
	testText += "store: " + std::to_string(testText[store] / 1000.f) + "k\n";

	std::cout << testText << "\n";
}
//std::cout << "up\n";
//std::cout << " now:" << pos.first << " " << pos.second << "\n";
//for (int ii = 0; ii < 15; ii++) {
//	for (int jj = 0; jj < 15; jj++) {
//		if (board[ii][jj] == P1)
//			std::cout << "* ";
//		else if (board[ii][jj] == P2)
//			std::cout << "# ";
//		else std::cout << "- ";
//	}
//	std::cout << "\n";
//}


	/*void get1(int x, int y) {                   //debug
		std::cout << (codeLR[x] >> (2 * y) & 3);
	}
	void get2(int x, int y) {
		std::cout << (codeUD[y] >> (2 * x) & 3);
	}
	void get3(int x, int y) {
		std::cout << (codeMain[x - y + BOARD_LENGTH - 1] >> (2 * y) & 3);
	}
	void get4(int x, int y) {
		std::cout << (codeVice[x + y] >> (2 * y) & 3);
	}*/