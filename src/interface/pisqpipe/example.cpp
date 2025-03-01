#include <windows.h>

#include "../../engine.h"
#include "pisqpipe.h"

namespace Piskvork {

    Engine engine;
    std::vector<Square> seq;

    const char* infotext = "name=\"momoku\", author=\"666\", version=\"1.1\", country=\"cn\", www=\"666\"";

    void brain_init() {
        engine.set_on_iter([](const auto& i) {});
        engine.set_on_update_no_moves([](const auto& i) {});
        engine.set_on_update_full([](const auto& i) {});
        engine.set_on_bestmove([](Square bm) {
            do_mymove(bm.x(), bm.y());
            });

        engine.set_options("timeout_turn", info_timeout_turn);
        engine.set_options("max_memory", info_max_memory);

        if (width > MAX_GAME_LENGTH || height > MAX_GAME_LENGTH) {
            width = height = 15;
            pipeOut("ERROR Maximal board size is %d", MAX_GAME_LENGTH);
            return;
        }
        seq.clear();
        pipeOut("OK");
    }

    void brain_restart() {
        seq.clear();
        pipeOut("OK");
    }

    int isFree(int x, int y) {
        bool valid = x < width && y < height && x >= 0 && y >= 0;
        for (auto& i : seq)
            valid &= i.x() != x || i.y() != y;
        return valid;
    }

    void brain_my(int x, int y) {
        if (isFree(x, y))
            seq.push_back({ x,y });
        else pipeOut("ERROR my move [%d,%d]", x, y);
    }

    void brain_opponents(int x, int y) {
        if (isFree(x, y))
            seq.push_back({ x,y });
        else pipeOut("ERROR opponents move [%d,%d]", x, y);
    }

    void brain_block(int x, int y) {
        if (isFree(x, y))
            seq.push_back({ x,y });
        else pipeOut("ERROR winning move [%d,%d]", x, y);
    }

    int brain_takeback(int x, int y) {
        if (seq.size()) {
            seq.pop_back();
            return 0;
        }
        return 2;
    }

    void brain_turn() {
        engine.go(width, seq);
    }

    void brain_end() {}

#ifdef DEBUG_EVAL
#include <windows.h>

    void brain_eval(int x, int y)
    {
        HDC dc;
        HWND wnd;
        RECT rc;
        char c;
        wnd = GetForegroundWindow();
        dc = GetDC(wnd);
        GetClientRect(wnd, &rc);
        c = (char)(board[x][y] + '0');
        TextOut(dc, rc.right - 15, 3, &c, 1);
        ReleaseDC(wnd, dc);
    }

#endif
}


