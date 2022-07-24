#include <memory>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "gtp.h"
#include "game_state.h"
#include "board.h"

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

void gtp_loop() {
    auto main_game = std::make_unique<GameState>();
    main_game->clear_board(9, 7.f);

    for (;;) {
        gtp_prcoess(main_game.get());
    }
}

void gtp_prcoess(GameState *main_game) {
    std::string input;
    if (!std::getline(std::cin, input)) {
        return;
    }

    std::istringstream ss{input};
    std::string buf;
    std::vector<std::string> args;
    while (ss >> buf) {
        args.emplace_back(buf);
    }
    const size_t argc = args.size();

    if (argc == 0) {
        return;
    }

    if (input.find("quit") == 0) {
        std::cout << gtp_success(std::string{});
        exit(-1);
    } else if (input.find("protocol_version") == 0) {
        std::cout << gtp_success("2");
    } else if (input.find("name") == 0) {
        std::cout << gtp_success("Go Bot");
    } else if (input.find("version") == 0) {
        std::cout << gtp_success("0.1");
    } else if (input.find("boardsize") == 0) {
        if (argc >= 2) {
            int bsize = std::stoi(args[1]);
            float komi = main_game->get_komi();
            main_game->clear_board(bsize, komi);

            std::cout << gtp_success(std::string{});
        } else {
            std::cout << gtp_fail(std::string{});
        }
    } else if (input.find("komi") == 0) {
        if (argc >= 2) {
            float komi = std::stof(args[1]);
            main_game->set_komi(komi);

            std::cout << gtp_success(std::string{});
        } else {
            std::cout << gtp_fail(std::string{});
        }
    } else if (input.find("clear_board") == 0) {
        int bsize = main_game->get_board_size();
        float komi = main_game->get_komi();
        main_game->clear_board(bsize, komi);

        std::cout << gtp_success(std::string{});
    }  else if (input.find("undo") == 0) {
        main_game->undo_move();
        std::cout << gtp_success(std::string{});
    } else if (input.find("play") == 0) {
        int color = Board::INVLD;
        int vtx = Board::NULL_VERTEX;

        if (argc >= 2) {
            auto color_str = args[1];
            if (color_str == "B" || color_str == "b" || color_str == "black") {
                color = Board::BLACK;
            } else if (color_str == "W" || color_str == "w" || color_str == "white") {
                color = Board::WHITE;
            }
        }

        if (argc >= 3) {
            auto vtx_str = args[2];
            if (vtx_str == "PASS" || vtx_str == "pass") {
                vtx = Board::PASS;
            } else if (vtx_str == "RESIGN" || vtx_str == "resign") {
                vtx = Board::RESIGN;
            } else if (vtx_str.size() <= 3) {
                int x = vtx_str[0] - 'a';
                int y = std::stoi(vtx_str.substr(1)) - 1;
                if (x >= 8) x--;

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
    } else if (input.find("genmove") == 0) {
        // TODO: You should implement your move generator.

        int color = main_game->get_tomove();
        if (argc >= 2) {
            auto color_str = args[1];
            if (color_str == "B" || color_str == "b" || color_str == "black") {
                color = Board::BLACK;
            } else if (color_str == "W" || color_str == "w" || color_str == "white") {
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
    } else if (input.find("showboard") == 0) {
        main_game->showboard();
        std::cout << gtp_success(std::string{});
    } else if (input.find("final_score") == 0) {
        float score = main_game->final_score();
        std::ostringstream result;

        if (score > 0.001f) {
            result << "b+" << score;
        } else if (score < -0.001f) {
            result << "w+" << -score;
        } else {
            result << "draw";
        } 

        std::cout << gtp_success(result.str());
    } else if (input.find("help") == 0 ||
                   input.find("list_commands") == 0) {
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
    auto prefix = std::string{"= "};
    auto suffix = std::string{"\n\n"};

    out << prefix << response << suffix;

    return out.str();
}

std::string gtp_fail(std::string response) {
    auto out = std::ostringstream{};
    auto prefix = std::string{"? "};
    auto suffix = std::string{"\n\n"};

    out << prefix << response << suffix;

    return out.str();
}
