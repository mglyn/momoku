#include "eval.h"
#include "data.h"

#include <queue>
#include <tuple>
#include <map>
#include <algorithm>

namespace Eval {
	Line2	codeToLineID2[NUMCODE];
	Flower	lineToFlower[MAXLINECOMB];

	Line2 decode1(uint64_t code) {	
		return codeToLineID2[(code >> 2) & 0b1111111100000000 | code & 0b11111111];
	}
	Flower decode2(int l1, int l2, int l3, int l4) {
		return lineToFlower[(l1 << 12) + (l2 << 8) + (l3 << 4) + l4];
	}

	ACautomation::ACautomation(std::vector<std::tuple<int, std::vector<int>>> patterns) {
		memset(nodes, 0, sizeof(nodes));
		root = cnt_node = 1;

		//insert patterns
		for (int i = 0; i < patterns.size(); i++) {

			auto pattern = patterns[i];

			//insert
			int now = root;
			for (int c : std::get<std::vector<int>>(pattern)) {
				if (nodes[now].edge[c] != 0)
					now = nodes[now].edge[c];
				else {
					nodes[now].edge[c] = ++cnt_node;
					nodes[nodes[now].edge[c]].father = now;
					now = nodes[now].edge[c];
				}
			}
			nodes[now].lineID = std::max(std::get<int>(pattern), nodes[now].lineID);

			//flip and insert agian
			for (int& c : std::get<std::vector<int>>(pattern))
				if (c == C_P1)
					c = C_P2;

			now = root;
			for (int c : std::get<std::vector<int>>(pattern)) {
				if (nodes[now].edge[c] != 0)
					now = nodes[now].edge[c];
				else {
					nodes[now].edge[c] = ++cnt_node;
					nodes[nodes[now].edge[c]].father = now;
					now = nodes[now].edge[c];
				}
			}
			nodes[now].lineID = std::max(std::get<int>(pattern), nodes[now].lineID);
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

	int ACautomation::query(std::vector<int>& arr) const {

		int lineID = 0;

		for (int i = 0, now = root; i < 9; i++) {
			int ch = arr[i];
			while (nodes[now].edge[ch] == 0 && nodes[now].fail != 0) //不能匹配时能跳则跳fail
				now = nodes[now].fail;
			if (nodes[now].edge[ch] != 0) {        //按边匹配
				now = nodes[now].edge[ch];

				lineID = std::max(nodes[now].lineID, lineID); //大类型优先覆盖
			}
		}
		return lineID;
	}

	const bool init = []() {

		auto patterns = Data::LoadPattern();
		auto eval = Data::LoadEval();

		ACautomation ac(patterns);

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
			codeToLineID2[code].lineP1 = ac.query(arr);

			arr[4] = C_P2;
			codeToLineID2[code].lineP2 = ac.query(arr);
		}

		//枚举line
		for (int a = 0; a < eval.size(); a++) {
			for (int b = a; b < eval.size(); b++) {
				for (int c = b; c < eval.size(); c++) {
					for (int d = c; d < eval.size(); d++) {

						int p[4] = { a,b,c,d };
						Flower f;

						FType typeD = std::get<FType>(eval[d]);
						FType typeC = std::get<FType>(eval[c]);

						if (typeD == Forbid) f.type = Forbid;
						else if (typeD == T5) f.type = T5;
						else if (typeD == TH4) f.type = TH4;
						else if (typeD == T4 && typeC == T4) f.type = TH4;
						else if (typeD == T4 && typeC == TH3) f.type = T4H3;
						else if (typeD == T4) f.type = T4;
						else if (typeD == TH3 && typeC == TH3) f.type = TDH3;
						else if (typeD == TH3) f.type = TH3;
						else f.type = TNone;

						float val = std::get<uint8_t>(eval[a]) +
							std::get<uint8_t>(eval[b]) +
							std::get<uint8_t>(eval[c]) +
							std::get<uint8_t>(eval[d]);
						f.value = (int)std::clamp(pow(val, 1.15f) * 0.75f, 0.f, 254.f);

						do {
							lineToFlower[(p[0] << 12) + (p[1] << 8) + (p[2] << 4) + p[3]] = f;
						} while (std::next_permutation(p, p + 4));
					}
				}
			}
		}
		return true;
	}();
}
