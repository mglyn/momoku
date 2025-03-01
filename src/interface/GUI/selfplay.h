#ifndef SELFPLAY
#define SELFPLAY

#include "../../engine.h"
#include "pipe.h"

#include <iostream>
#include <random>
#include <fstream>
#include <filesystem>

constexpr int statesPerFile = 4096;

struct State {
	int8_t side_to_move;
	int16_t eval;
	int8_t board[15][15];
};

class Writer {
    Pipe<State> pipe_;
    std::atomic<bool> writingDone;
    std::string generateFileName() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1, 1000000);
 
        std::ostringstream oss;
        oss << "data/states_" << std::setw(6) << std::setfill('0') << dis(gen) << ".bin";
        return oss.str();
    }
 
public:
    Writer() : writingDone(false) {
        std::thread writerThread([this]() { this->writeLoop(); });
        writerThread.detach();
    }
 
    void writeLoop() {
        const size_t statesPerFile = 4096;
        size_t count = 0;
        std::ofstream outFile;
 
        while (!pipe_.done()) {
			for (State state; pipe_.read(state);) {
                if (count % statesPerFile == 0) {
                    if (outFile.is_open()) 
						outFile.close();

					int tryNum = 0;
					do {
						outFile.open(generateFileName(), std::ios::binary);
						tryNum++;
					} while (!outFile.is_open() && tryNum < 16);
					assert(outFile.is_open());
                }
                outFile.write(reinterpret_cast<const char*>(&state), sizeof(State));
                count++;
            }
        }
		writingDone = true;
    }

    void write(const State& state) {
        pipe_.write(state);
    }
 
    void finish() {
        pipe_.terminate();
    }
 
    bool isDone() const {
        return writingDone;
    }
};

namespace fs = std::filesystem;
class Reader {
public:
    // 读取并打印所有符合条件的文件
    void readFiles() {
        try {
            for (const auto& entry : fs::directory_iterator("data")) {
                if (entry.is_regular_file() &&
                    entry.path().filename().string().find("state_") == 0 &&
                    entry.path().filename().string().find(".bin") != std::string::npos) {
                    readAndPrintFile(entry.path());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

private:
    // 读取并打印单个文件的内容
    void readAndPrintFile(const fs::path& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return;
        }

        std::cout << "Content of " << filePath << ":" << std::endl;
        State state;
        while (file.read(reinterpret_cast<char*>(&state), sizeof(State))) {
            printState(state);
        }
        std::cout << "----------------------------------------" << std::endl;
        file.close();
    }

    // 打印单个State的内容
    void printState(const State& state) {
        std::cout << "Side to move: " << static_cast<int>(state.side_to_move) << std::endl;
        std::cout << "Evaluation: " << state.eval << std::endl;
        std::cout << "Board:" << std::endl;
        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j) {
                std::cout << std::setw(2) << static_cast<int>(state.board[i][j]) << " ";
            }
            std::cout << std::endl;
        }
    }
};

void selfplay(Pipe<std::string>& pipe, int gameCnt);

void readAll();
#endif
