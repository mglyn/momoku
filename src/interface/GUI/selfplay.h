#ifndef SELFPLAY
#define SELFPLAY

#include "../../engine.h"
#include "client.h"
#include "pipe.h"

#include <iostream>
#include <random>
#include <fstream>
#include <filesystem>

constexpr int statesPerFile = 4096;

struct State {
	int8_t side_to_move;
	int16_t eval;
    int16_t bestMove;
    int8_t gameResult;
	int8_t board[15][15];
};

class Writer {
    Pipe<State> pipe_;
    std::atomic<bool> writingDone;
    std::string generateFileName();

public:
    Writer();
    void writeThreadLoop();
    void write(const State& state) { pipe_.write(state); }
    void finish() { pipe_.terminate(); }
    bool isDone() const { return writingDone; }
};

namespace fs = std::filesystem;
class Reader {
    // 读取并打印单个文件的内容
    void readAndPrintFile(const fs::path& filePath);
    // 打印单个State的内容
    void printState(const State& state);
public:
    // 读取并打印所有符合条件的文件
    void readFiles();
};

void selfPlayThreadFunc(Writer* writer, int threadID, Pipe<std::string>* pipe);

class miniGUI {
    static constexpr int width = 1600;
    static constexpr int height = 900;
    static constexpr int board_size = 170;
    static constexpr int all_board_stx = (width - (board_size + 20) * 8) / 2;
    static constexpr int all_board_sty = height * 0.05;
    static constexpr int gameSize = 15;
    static constexpr int grid_size = board_size / (gameSize - 1);

    std::shared_ptr<Gdiplus::Graphics> graphics;
    std::vector<Widget> pieces;
    std::vector<Piece> side_to_move;
    std::vector<Widget> info;
    Pipe<std::string> pipe;

    void Draw(std::vector<Widget>& pieces, std::vector<Widget>& info);
    void updateStats();

public:
    miniGUI();
    void selfPlayParallel();
};

void readAll();
#endif
