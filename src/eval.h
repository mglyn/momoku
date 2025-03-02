#ifndef EVAL
#define EVAL

#include "common.h"

#include <vector>

namespace Eval {
	constexpr int HALF_LINE_LEN = 4;
	constexpr int NUMCODE = 1 << (4 * HALF_LINE_LEN);
	constexpr int MATIRIAL_NB = 16;

	struct Line {
		uint8_t lineP1 : 4;
		uint8_t lineP2 : 4;
	};

	struct CombPattern {
		FType type;
		uint8_t value;
	};

	Line decodeLine(uint64_t code);
	CombPattern decodeComb(int l1, int l2, int l3, int l4);

	struct Pattern {
		int id = 0;
		int priority = 0;
		std::vector<int> pattern;
	};

	class ACautomation {      //AC自动机多串匹配

		struct trie_node {
			int father;
			int fail;
			int edge[4];
			Pattern pattern;
		};
		trie_node* nodes;
		int root;
		int cnt_node;

		void insert(Pattern pattern);

	public:
		ACautomation();
		~ACautomation() { delete[] nodes; }
		Pattern query(std::vector<int>& arr) const;
	};
}

#endif
