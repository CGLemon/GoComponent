#include <memory>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>

#include "gtp.h"
#include "game_state.h"
#include "board.h"

static int command_id;

std::vector<std::string> GTP_COMMANDS_LIST = {
    // Part of GTP version 2 standard command
    "protocol_version",

    // Part of GTP version 2 standard command
    "name",

    // Part of GTP version 2 standard command
    "version",

    // Part of GTP version 2 standard command
    "quit",

    // Part of GTP version 2 standard command
    "list_commands",

    // Part of GTP version 2 standard command
    "boardsize",

    // Part of GTP version 2 standard command
    "clear_board",

    // Part of GTP version 2 standard command
    "komi",

    // Part of GTP version 2 standard command
    "play",

    // Part of GTP version 2 standard command
    "genmove",

    // Part of GTP version 2 standard command
    "showboard",

    // Part of GTP version 2 standard command
    "undo",

    // Part of GTP version 2 standard command
    "final_score"
};

void gtp_prcoess(GameState *main_game);
std::string gtp_success(std::string response);
std::string gtp_fail(std::string response);
void gtp_hint();

void gtp_loop(bool hint) {
    if (hint) gtp_hint();

    auto main_game = std::make_shared<GameState>();
    main_game->clear_board(9, 7.f);

    for (;;) {
        gtp_prcoess(main_game.get());
    }
}

void gtp_prcoess(GameState *main_game) {
    std::string inputs;
    if (!std::getline(std::cin, inputs)) {
        return;
    }

    std::istringstream ss{inputs};
    std::string buf;
    std::vector<std::string> args;

    while (ss >> buf) {
        args.emplace_back(buf);
    }

    if (args.empty()) {
        return;
    }

    // check the command id here
    command_id = -1;
    auto toke_str = args[0];
    bool is_digit = true;

    for (char c : toke_str) {
        is_digit &= isdigit(c);
    }
    if (is_digit) {
        command_id = std::stoi(toke_str);
        args.erase(std::begin(args)); // remove command id
    }

    const size_t argc = args.size();
    const auto main_cmd = args[0];

    if (argc == 0) {
        return;
    }

    if (main_cmd == "quit") {
        std::cout << gtp_success(std::string{});
        exit(EXIT_SUCCESS);
    } else if (main_cmd == "protocol_version") {
        std::cout << gtp_success("2");
    } else if (main_cmd == "name") {
        std::cout << gtp_success("Go Bot");
    } else if (main_cmd == "version") {
        std::cout << gtp_success("0.1");
    } else if (main_cmd == "boardsize") {
        if (argc >= 2) {
            int bsize = std::stoi(args[1]);
            float komi = main_game->get_komi();
            main_game->clear_board(bsize, komi);

            std::cout << gtp_success(std::string{});
        } else {
            std::cout << gtp_fail(std::string{});
        }
    } else if (main_cmd == "komi") {
        if (argc >= 2) {
            float komi = std::stof(args[1]);
            main_game->set_komi(komi);

            std::cout << gtp_success(std::string{});
        } else {
            std::cout << gtp_fail(std::string{});
        }
    } else if (main_cmd == "clear_board") {
        int bsize = main_game->get_board_size();
        float komi = main_game->get_komi();
        main_game->clear_board(bsize, komi);

        std::cout << gtp_success(std::string{});
    }  else if (main_cmd == "undo") {
        main_game->undo_move();
        std::cout << gtp_success(std::string{});
    } else if (main_cmd == "play") {
        int color = Board::INVLD;
        int vtx = Board::NULL_VERTEX;

        if (argc >= 2) {
            auto color_str = args[1];
            if (std::tolower(color_str[0]) == 'b') {
                color = Board::BLACK;
            } else if (std::tolower(color_str[0]) == 'w') {
                color = Board::WHITE;
            }
        }

        if (argc >= 3) {
            auto vtx_str = args[2];
            for (char &v : vtx_str) {
                v = std::tolower(v);
            }
            if (vtx_str == "pass") {
                vtx = Board::PASS;
            } else if (vtx_str == "resign") {
                vtx = Board::RESIGN;
            } else if (vtx_str.size() <= 3) {
                char start = 'A'; // 65
                if (vtx_str[0] >= 'a') {
                    start = 'a'; // 97
                }

                int x = vtx_str[0] - start;
                int y = std::stoi(vtx_str.substr(1)) - 1;
                if (x >= 8) x--; // skip I

                vtx = main_game->get_vertex(x,y);
            }
        }

        if (color != Board::INVLD &&
                vtx != Board::NULL_VERTEX &&
                main_game->play_move(vtx, color)) {
            std::cout << gtp_success(std::string{});
        } else {
            std::cout << gtp_fail(std::string{});
        }
    } else if (main_cmd == "genmove") {
        // TODO: You should implement your move generator here.

        int color = main_game->get_tomove();
        if (argc >= 2) {
            auto color_str = args[1];
            if (std::tolower(color_str[0]) == 'b') {
                color = Board::BLACK;
            } else if (std::tolower(color_str[0]) == 'w') {
                color = Board::WHITE;
            }
        }
        int vtx = main_game->play_random_move(color);
        std::string out;

        if (vtx == Board::PASS) {
            out = "pass";
        } else if (vtx == Board::RESIGN) {
            out = "resign";
        } else if (vtx == Board::NULL_VERTEX) {
            out = "null";
        } else {
            const char *x_lable_map = "ABCDEFGHJKLMNOPQRST";
            int x = main_game->get_x(vtx);
            int y = main_game->get_y(vtx);
            out += x_lable_map[x];
            out += std::to_string(y+1);
        }

        std::cout << gtp_success(out);
    } else if (main_cmd == "showboard") {
        main_game->showboard();
        std::cout << gtp_success(std::string{});
    } else if (main_cmd == "final_score") {
        float score = main_game->final_score();
        std::ostringstream result;

        if (std::abs(score) < 0.001f) {
            result << "draw";
        } else if (score > 0.f) {
            result << "b+" << score;
        } else if (score < 0.f) {
            result << "w+" << -score;
        }
        std::cout << gtp_success(result.str());
    } else if (main_cmd == "help" ||
                   main_cmd == "list_commands") {
        auto list_commands = std::ostringstream{};
        auto idx = size_t{0};

        std::sort(std::begin(GTP_COMMANDS_LIST), std::end(GTP_COMMANDS_LIST));

        for (const auto &cmd : GTP_COMMANDS_LIST) {
            list_commands << cmd;
            if (++idx != GTP_COMMANDS_LIST.size()) list_commands << std::endl;
        }
        std::cout << gtp_success(list_commands.str());
    } else {
        std::cout << gtp_fail("unknown command");
    }
}

std::string gtp_success(std::string response) {
    auto out = std::ostringstream{};

    if (command_id >= 0) {
        out << '=' << command_id << ' ' << response << "\n\n";
    } else {
        out << '='               << ' ' << response << "\n\n";
    }

    return out.str();
}

std::string gtp_fail(std::string response) {
    auto out = std::ostringstream{};

    out << "? " << response << "\n\n";

    return out.str();
}

void gtp_hint() {
    std::cerr 
        << "Start to running the main GTP loop. GTP is not for human. But you\n"
        << "could still try to play the bot with it. Here are some tips.\n"
        << "\n"
        << "Enter \"list_commands\" to show whole supported commands.\n"
        << "Enter \"showboard\"     to show the current board state.\n"
        << "Enter \"play b d6\"     to place the black stone on the board at the E6.\n"
        << "Enter \"play w f4\"     to place the white stone on the board at the F4.\n"
        << "Enter \"genmove b\"     to genrate a random move and play it.\n"
        << "Enter \"clear_board\"   to create a new game.\n"
        << "Enter \"komi 7.5\"      to set the komi as 7.5.\n"
        << "Enter \"boardsize 13\"  to set the board size as 13 and create a new game.\n"
        << "Enter \"help\"          to show all commands.\n"
        << "Enter \"quit\"          to end the program.\n"
        << "\n"
        << "If you want to know the more details, see here: https://www.gnu.org/software/gnugo/gnugo_19.html.\n"
        << "\n"
    ;
}
