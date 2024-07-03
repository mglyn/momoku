#include <windows.h>

#include "../game/board.h"
#include "../search/search.h"
#include "pisqpipe.h"

namespace Piskvork {
    const char* infotext = "name=\"Random\", author=\"Petr Lastovicka\", version=\"3.2\", country=\"Czech Republic\", www=\"https://plastovicka.github.io\"";

    Board bd(15);

    void brain_init() {
        if (width > MAX_GAME_LENGTH || height > MAX_GAME_LENGTH) {
            width = height = 0;
            pipeOut("ERROR Maximal board size is %d", MAX_GAME_LENGTH);
            return;
        }
        bd.reset(width);
        pipeOut("OK");
    }

    void brain_restart(){
        bd.reset(width);
        pipeOut("OK");
    }

    int isFree(int x, int y) {
        return !bd.notin(Pos(x, y)) && bd[Pos(x, y)] == Empty;
    }

    void brain_my(int x, int y) {
        if (isFree(x, y)) bd.update(Pos(x, y));
        else pipeOut("ERROR my move [%d,%d]", x, y);
    }

    void brain_opponents(int x, int y) {
        if (isFree(x, y)) bd.update(Pos(x, y));
        else pipeOut("ERROR opponents's move [%d,%d]", x, y);
    }

    void brain_block(int x, int y) {
        if (isFree(x, y)) bd.update(Pos(x, y));
        else  pipeOut("ERROR winning move [%d,%d]", x, y);
    }

    int brain_takeback(int x, int y){
        if (bd.cntMove()) {
            bd.undo();
            return 0;
        }
        return 2;
    }

    void brain_turn() {
        std::vector<Pos> answer;
        answer = Search::search(bd);
        do_mymove(answer[0].x(), answer[0].y());
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


