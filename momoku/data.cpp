#include "data.h"

namespace Data{

	std::vector<std::tuple<int, std::vector<int>>> LoadPattern(std::string path) {

		std::vector<std::tuple<int, std::vector<int>>> ret;

		std::ifstream inputFile(path + "eval.txt");
		if (!inputFile) {
			std::cerr << "无法打开文件" << std::endl;

			assert(0);
			return ret;
		}

		std::string line;
		// 逐行读取文件内容
		while (std::getline(inputFile, line)) {
			std::istringstream word_stream(line);
			std::string word;
			std::vector<std::string> words;

			// 使用istringstream按空格分割字符串
			while (word_stream >> word) {
				words.push_back(word);
			}

			if (words.empty() || words[0] != "p") continue;

			// 输出分割后的单词
			/*for (const auto& w : words) {
				std::cout << w << std::endl;
			}*/
			
			std::istringstream num_stream(words[2]);
			std::string num;
			std::vector<int> nums;

			while (std::getline(num_stream, num, ',')) {
				nums.push_back(std::stoi(num));
			}

			ret.push_back(std::tuple<int, std::vector<int>>{std::stoi(words[1]), nums});
		}

		inputFile.close();

		return ret;
	}

	std::vector<std::tuple<uint8_t, FType>> LoadEval(std::string path){

		std::vector<std::tuple<uint8_t, FType>> ret;

		std::ifstream inputFile(path + "eval.txt");
		if (!inputFile) {
			std::cerr << "无法打开文件" << std::endl;

			assert(0);
			return ret;
		}

		std::string line;
		// 逐行读取文件内容
		while (std::getline(inputFile, line)) {
			std::istringstream word_stream(line);
			std::string word;
			std::vector<std::string> words;

			// 使用istringstream按空格分割字符串
			while (word_stream >> word) {
				words.push_back(word);
			}

			if (words.empty() || words[0] != "v") continue;

			// 输出分割后的单词
			/*for (const auto& w : words) {
				std::cout << w << std::endl;
			}*/

			ret.push_back(std::tuple<uint8_t, FType>{(uint8_t)std::stoi(words[2]), (FType)std::stoi(words[3])});
		}

		inputFile.close();

		return ret;
	}
	
}

