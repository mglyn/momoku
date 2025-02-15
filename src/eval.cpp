#include "eval.h"

#include <queue>
#include <tuple>
#include <map>
#include <algorithm>

namespace Eval {
	Line codeToLineID[NUMCODE];
	CombPattern code4ToCombPat[MATIRIAL_NB][MATIRIAL_NB][MATIRIAL_NB][MATIRIAL_NB];

	const std::vector<Pattern> patterns = {
			{1, 1, {1,0,0,0,1}},
			{1, 1, {0,1,0,0,1}},
			{1, 1, {1,0,0,1,0}},
			{1, 1, {1,0,1,0,0}},
			{1, 1, {0,1,0,1,0}},
			{1, 1, {0,0,1,0,1}},

			{2, 2, {0,0,0,1,1}},
			{2, 2, {0,0,1,1,0}},
			{2, 2, {0,1,1,0,0}},
			{2, 2, {1,1,0,0,0}},

			{3, 3, {0,1,0,0,1,0}},

			{4, 4, {0,0,1,0,1,0}},
			{4, 4, {0,1,0,1,0,0}},

			{5, 5, {0,1,1,0,0,0}},
			{5, 5, {0,0,1,1,0,0}},
			{5, 5, {0,0,0,1,1,0}},

			{6, 6, {1,0,1,0,1}},
			{6, 6, {1,0,0,1,1}},
			{6, 6, {1,1,0,0,1}},

			{7, 7, {0,1,0,1,1}},
			{7, 7, {1,1,0,1,0}},
			{7, 7, {0,1,1,0,1}},
			{7, 7, {1,0,1,1,0}},

			{8, 8, {0,0,1,1,1}},
			{8, 8, {0,1,1,1,0}},
			{8, 8, {1,1,1,0,0}},

			{9, 9, {0,1,0,1,1,0}},
			{9, 9, {0,1,1,0,1,0}},

			{10, 10, {0,0,1,1,1,0}},
			{10, 10, {0,1,1,1,0,0}},

			{11, 11, {1,1,1,1,0}},
			{11, 11, {1,1,1,0,1}},
			{11, 11, {1,1,0,1,1}},
			{11, 11, {1,0,1,1,1}},
			{11, 11, {0,1,1,1,1}},

			{12, 12, {0,1,1,1,1,0}},

			{13, 13, {1,1,1,1,1}},

			{14, 14, {1,1,1,0,1,0,1,1,1}},
			{14, 14, {1,1,0,1,1,1,0,1,1}},

			{15, 15, {1,1,1,1,1,1}},
	};

	const int value[MATIRIAL_NB] = { 0,3,4,5,6,7,14,16,19,32,38,60,100,100,100,100 };
	const FType type[MATIRIAL_NB] = {
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TH3,
		TH3,
		T4,
		TH4,
		T5,
		TH4,
		T5,
	};
	const FType typeRenjuP1[MATIRIAL_NB] = {
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TNone,
		TH3,
		TH3,
		T4,
		TH4,
		T5,
		Forbid,
		Forbid,
	};

	Line decodeLine(uint64_t code) {	
		return codeToLineID[(code >> 2) & 0b1111111100000000 | code & 0b11111111];
	}
	CombPattern decodeComb(int l1, int l2, int l3, int l4) {
		return code4ToCombPat[l1][l2][l3][l4];
	}

	void ACautomation::insert(Pattern pattern) {
		//insert
		int now = root;
		for (int c : pattern.pattern) {
			if (nodes[now].edge[c] != 0)
				now = nodes[now].edge[c];
			else {
				nodes[now].edge[c] = ++cnt_node;
				nodes[nodes[now].edge[c]].father = now;
				now = nodes[now].edge[c];
			}
		}
		if (pattern.priority > nodes[now].pattern.priority)
			nodes[now].pattern = pattern;
	}

	ACautomation::ACautomation() {
		//init
		nodes = new trie_node[256];
		for (int i = 0; i < 256; i++) {
			nodes[i].fail = nodes[i].father = 0;
			for (int c = 0; c < 4; c++) {
				nodes[i].edge[c] = 0;
			}
		}
		root = cnt_node = 1;

		//insert patterns
		for (auto pattern: patterns) {

			insert(pattern);
			//flip and insert agian
			for (int& c : pattern.pattern)
				if (c == C_P1)
					c = C_P2;
			insert(pattern);
		}

		//calulate fail pointer
		nodes[root].fail = 0;
		std::queue<int> q;
		for (int i = 0; i < 4; i++) {
			if (nodes[root].edge[i] != 0) {
				nodes[nodes[root].edge[i]].fail = root;
				q.push(nodes[root].edge[i]);
			}
		}
		while (!q.empty()) {
			int fa = q.front();
			int fafail = nodes[fa].fail;
			q.pop();
			for (int i = 0; i < 4; i++) {
				int now = nodes[fa].edge[i];
				if (now == 0) continue;
				if (nodes[fafail].edge[i] != 0) {
					nodes[now].fail = nodes[fafail].edge[i];
				}
				else nodes[now].fail = root;
				q.push(now);
			}
		}
	}

	Pattern ACautomation::query(std::vector<int>& arr) const {

		Pattern pattern;
		for (int i = 0, now = root; i < 9; i++) {
			int ch = arr[i];
			while (nodes[now].edge[ch] == 0 && nodes[now].fail != 0) //不能匹配时能跳则跳fail
				now = nodes[now].fail;
			if (nodes[now].edge[ch] != 0) {        //按边匹配
				now = nodes[now].edge[ch];

				if (nodes[now].pattern.priority > pattern.priority)
					pattern = nodes[now].pattern;
			}
		}
		return pattern;
	}

	const bool init = []() {

		ACautomation ac;

		//枚举code
		for (int code = 0; code < NUMCODE; code++) {

			auto simulate = [](int code) {
				std::vector<int> arr(9, 0);
				for (int i = 0; i <= 3; i++)
					arr[i] = (code >> 2 * i) & 0b11;
				for (int i = 4; i <= 7; i++)
					arr[i + 1] = (code >> 2 * i) & 0b11;
				return arr;
			};
			std::vector<int> arr = simulate(code);

			arr[4] = C_P1;
			codeToLineID[code].lineP1 = ac.query(arr).id;

			arr[4] = C_P2;
			codeToLineID[code].lineP2 = ac.query(arr).id;
		}

		//枚举line
		for (int a = 0; a < MATIRIAL_NB; a++) {
			for (int b = a; b < MATIRIAL_NB; b++) {
				for (int c = b; c < MATIRIAL_NB; c++) {
					for (int d = c; d < MATIRIAL_NB; d++) {

						CombPattern f;

						FType typeD = type[d];
						FType typeC = type[c];

						if (typeD == Forbid) f.type = Forbid;
						else if (typeD == T5) f.type = T5;
						else if (typeD == TH4) f.type = TH4;
						else if (typeD == T4 && typeC == T4) f.type = TH4;
						else if (typeD == T4 && typeC == TH3) f.type = T4H3;
						else if (typeD == T4) f.type = T4;
						else if (typeD == TH3 && typeC == TH3) f.type = TDH3;
						else if (typeD == TH3) f.type = TH3;
						else f.type = TNone;

						int val = value[a] + value[b] + value[c] + value[d];
						f.value = std::clamp(val, 0, 255);

						int p[4] = { a,b,c,d };
						do {
							code4ToCombPat[p[0]][p[1]][p[2]][p[3]] = f;
						} while (std::next_permutation(p, p + 4));
					}
				}
			}
		}

		return true;
	}();
}
