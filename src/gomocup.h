#ifndef GOMOCUP
#define GOMOCUP

#include "misc.h"
#include "engine.h"

/*
Mandatory commands
START[size]
When the brain receives this command, it initializes itself and creates an empty board, but doesn't make any move yet. The parameter is size of the board. The brain must be able to play on board of size 20, because this size will be used in Gomocup tournaments. It is recommended but not required to support other board sizes. If the brain doesn't like the size, it responds ERROR.There can be a message after the ERROR word.The manager can try other sizes or it can display an error message to a user.The brain responds OK if it has been initialized successfully.
Example:
The manager sends :
START 20
The brain answers :
OK - everything is good
ERROR message - unsupported size or other error
TURN[X], [Y]
The parameters are coordinate of the opponent's move. All coordinates are numbered from zero.
Expected answer :
two comma - separated numbers - coordinates of the brain's move

Example :
    The manager sends :
TURN 10, 10
The brain answers :
11, 10
BEGIN
This command is send by the manager to one of the players(brains) at the beginning of a match.This means that the brain is expected to play(open the match) on the empty playing board.After that the other brain obtains the TURN command with the first opponent's move. The BEGIN command is not used when automatic openings are enabled, because in that case both brains receive BOARD commands instead.
Expected answer :
two numbers separated by comma - coordinates of the brain's move

Example :
    The manager sends :
BEGIN
The brain answers :
10, 10
BOARD
This command imposes entirely new playing field.It is suitable for continuation of an opened match or for undo / redo user commands.The BOARD command is usually send after START, RESTART or RECTSTART command when the board is empty.If there is any open match, the manager sends RESTART command before the BOARD command.
After this command the data forming the playing field are send.Every line is in the form :

[X] , [Y], [field]
where[X] and [Y] are coordinates and [field] is either number 1 (own stone) or number 2 (opponent's stone) or number 3 (only if continuous game is enabled, stone is part of winning line or is forbidden according to renju rules).
    If game rule is renju, then the manager must send these lines in the same order as moves were made.If game rule is Gomoku, then the manager may send moves in any order and the brain must somehow cope with it.Data are ended by DONE command.Then the brain is expected to answer such as to TURN or BEGIN command.

    Example:
The manager sends :
BOARD
10, 10, 1
10, 11, 2
11, 11, 1
9, 10, 2
DONE
The brain answers :
9, 9
INFO[key][value]
The manager sends information to the brain.The brain can ignore it.However, the brain will lose if it exceeds the limits.The brain must cope with situations when the manager doesn't send all information which is mentioned in this document. Most of this information is sent at the beginning of a match. The time limits will not be changed in the middle of a match during a tournament. It is recommended to react on commands at any time, because the human opponent can change these values even when the brain is thinking.
The key can be :
timeout_turn - time limit for each move(milliseconds, 0 = play as fast as possible)
timeout_match - time limit of a whole match(milliseconds, 0 = no limit)
max_memory - memory limit(bytes, 0 = no limit)
time_left - remaining time limit of a whole match(milliseconds)
game_type - 0 = opponent is human, 1 = opponent is brain, 2 = tournament, 3 = network tournament
rule - bitmask or sum of 1 = exactly five in a row win, 2 = continuous game, 4 = renju, 8 = caro
evaluate - coordinates X, Y representing current position of the mouse cursor
folder - folder for persistent files
Information about time and memory limits is sent before the first move(after or before START command).Info time_left is sent before every move(before commands TURN, BEGIN, BOARD and SWAP2BOARD).The remaining time can be negative when the brain runs out of time.Remaining time is equal to 2147483647 if the time for a whole match is unlimited.The manager is required to send info time_left if the time is limited, so that the brain can ignore info timeout_match and only rely on info time_left.
Time for a match is measured from creating a process to the end of a game(but not during opponent's turn). Time for a turn includes processing of all commands except initialization (commands START, RECTSTART, RESTART). Turn limit equal to zero means that the brain should play as fast as possible (eg count only a static evaluation and don't search possible moves).

INFO folder is used to determine a folder for files that are permanent.Because this folder is common for all brains and maybe other applications, the brain must create its own subfolder which name must be the same as the name of the brain.If the manager does not send INFO folder, then the brain cannot store permanent files.

Only debug versions should respond to INFO evaluate.For example, it can print evaluation of the square to some window.It cannot be written to the standard output.Release versions should just ignore INFO evaluate.

How should the brain behave when obtains unknown INFO command ?
-Ignore it, it is probably not important.If it was important, it is not in an INFO command form.

How should behave the brain obtaining the unachievable INFO command ?
(for example too small memory limit)
- The brain should wait with the output of the problem until the manager sends the first command not having an INFO form(TURN, BOARD or BEGIN).The manager does not read messages from the brain when sending INFO command.

Example :
    INFO timeout_match 300000
    INFO timeout_turn 10000
    INFO max_memory 83886080

    Expected answer : none
    END
    When the brain obtains this command, it must terminate as soon as possible.The manager waits until the brain is finished.If the time of termination is too long(e.g. 1 second), the brain will be terminated by the manager.The brain should not write anything to output after the END command.However, the manager should not close the pipe until the brain is ended.
    Expected answer : none
    The brain should delete its temporary files.
    ABOUT
    The brain is expected to send some information about itself on one line.Each info must be written as keyword, equals sign, text value in quotation marks.Recommended keywords are name, version, author, country, www, email.Values should be separated by commas that can be followed by spaces.The manager can use this info, but must cope with old brains that used to send only human - readable text.
    Example :
    The manager sends :
ABOUT
The brain answers :
name = "SomeBrain", version = "1.0", author = "Nymand", country = "USA"*/

class ProtocalEngine {

	Engine engine;
    int gameSize;
    std::vector<Square> seq;

public:
	ProtocalEngine() {
        gameSize = 15;
        init_search_update_listeners();
    }

    void init_search_update_listeners() {
        engine.set_on_iter([](const auto& i) {});
        engine.set_on_update_no_moves([](const auto& i) {});
        engine.set_on_update_full([](const auto& i) {});
        engine.set_on_bestmove([](Square bm) { 
            sync_cout << "bestmove " << bm.x() << "," << bm.y() << sync_endl;
            });
    }

    void setoption(std::istringstream& is) {

        auto& options = engine.get_options();
        std::string key, value;

        is >> key >> value;  

        if (options.count(key)) {
            options[key] = std::stoi(value);
        }
        else {
            sync_cout << "UNKNOWN No such option: " << key << sync_endl;
        }
    }

    void newgame(std::istringstream& is) {

        std::string size;
        is >> size;
        gameSize = std::stoi(size);
        if (15 <= gameSize && gameSize <= 20) {
            sync_cout << "OK" << sync_endl;
        }
        else {
            sync_cout << "ERROR unsupported size" << sync_endl;
        }
    }

    void board(std::istringstream& is) {

        std::string line;
        std::getline(is, line);
        while (line != "DONE") {
            std::stringstream ss(line);
            int x, y, piece;
            char comma;

            ss >> x >> comma >> y >> comma >> piece;
            seq.emplace_back(x, y);

            std::getline(is, line);
        }
    }

    void turn(std::istringstream& is) {
        char c;

        is >> c;
        int x = c - '0';
        is >> c;
        if (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            is >> c;
        }
       
        is >> c;
        int y = c - '0';
        if (is >> c) y = y * 10 + c - '0';

        seq.emplace_back(x, y);
    }

    void loop() {
        std::string token, cmd;

        do {

            if (!getline(std::cin, cmd)) {  // Wait for an input or an end-of-file (EOF) indication
                cmd = "quit";
            }  

            std::istringstream is(cmd);

            token.clear();  // Avoid a stale if getline() returns nothing or a blank line
            is >> std::skipws >> token;

            if (token == "quit" || token == "END") {
                engine.stop();
            }
            else if (token == "ABOUT") {
                sync_cout << "name=\"momoku\", version=\"dev\", author = \"\", country = \"cn\"" << sync_endl;
            }
            else if (token == "INFO") {
                setoption(is);
            }
            else if (token == "START") {
                newgame(is);
            }
            else if (token == "BEGIN") {
                engine.go(gameSize, seq);
            }
            else if (token == "TURN") {
                turn(is);
            }
            else if (token == "BOARD") {
                board(is);
            }
            else if (token == "RESTART") {
                seq.clear();
            }
            else if (token == "TAKEBACK") {
                seq.pop_back();
            }
            else if (!token.empty()) {
                sync_cout << "UNKNOWN unkown command" << sync_endl;
            }

        } while (token != "quit" || token == "END");  // The command-line arguments are one-shot
    }
};

#endif