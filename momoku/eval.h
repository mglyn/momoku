#ifndef EVAL
#define EVAL

#include "common.h"

namespace Eval {
	constexpr int HALF_LINE_LEN = 4;
	constexpr int NUMCODE = 1 << 16;
	constexpr int MAXLINECOMB = 1 << 16;

	struct Line2 {
		uint8_t lineP1 : 4;
		uint8_t lineP2 : 4;
	};

	struct Flower {
		uint8_t value;
		FType type;
	};

	Line2 decode1(uint64_t code);
	Flower decode2(int l1, int l2, int l3, int l4);

	struct trie_node {
		int father;
		int fail;
		int edge[4];
		int lineID;
	};

	class ACautomation {      //AC自动机多串匹配
		trie_node nodes[256];
		int root;
		int cnt_node;
	public:
		ACautomation(std::vector< std::tuple<int, std::vector<int> > > patterns);
		int query(std::vector<int>& arr) const;
	};
}

#endif
