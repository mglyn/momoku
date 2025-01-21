#ifndef TEST
#define TEST
#include <unordered_map>

enum Test {
	node,
	vcfnode,
	TTcutoff,
	betacutoff,
	vcfTTcutoff,
	vcfbetacutoff,
	moveCntpruning,
	dispersedT,
	TDH3T4H3wincheck,
	tothasherror,
	store,
	numThreats,
	razor,
	futility = 60,
};

extern int testData[128];

extern std::string testText;

void PrintTest();

#endif

