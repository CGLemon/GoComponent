#include "game_state.h"
#include <random>

void GameState::clear_board(int board_size, float komi) {
    board.reset_board(board_size);

    m_game_history.clear();
    m_game_history.emplace_back(std::make_shared<Board>(board));

    m_komi = komi;
    m_movenum = 0;
}

bool GameState::play_move(int vtx, int color) {
    if (!board.legal_move(vtx, color)) {
        return false;
    }

    board.play_move_assume_legal(vtx, color);
    m_game_history.resize(++m_movenum);
    m_game_history.emplace_back(std::make_shared<Board>(board));

    return true;
}

void GameState::undo_move() {
    if (m_movenum == 0) return;

    m_game_history.resize(m_movenum--);
    board = *m_game_history[m_movenum];
}


int GameState::play_random_move(int color) {
    std::vector<int> legal_moves = { Board::PASS };
    int board_size = board.get_board_size();


    for (int y = 0; y < board_size; ++y) {
        for (int x = 0; x < board_size; ++x) {
            int vtx = board.get_vertex(x,y);
            if (board.legal_move(vtx, color)) {
                legal_moves.emplace_back(vtx);
            }
        }
    }

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<> dist(0, legal_moves.size()-1);

    int move = legal_moves[dist(mt)];
    play_move(move, color);

    return move;
}

bool GameState::legal_move(int vtx, int color) {
    return board.legal_move(vtx, color);
}

bool GameState::superko() {
    std::uint64_t hash = board.compute_hash();
    for (int i = 0; i < m_movenum-1; ++i) {
        if (hash == m_game_history[i]->compute_hash()) {
            return true;
        }
    }
    return false;
}

float GameState::final_score() {
    return board.compute_reach_color(Board::BLACK) -
               board.compute_reach_color(Board::WHITE) - m_komi;
}

int GameState::get_vertex(int x, int y) const {
    return board.get_vertex(x,y);
}

int GameState::get_index(int x, int y) const {
    return board.get_index(x,y);
}


void GameState::showboard() {
    std::string color_map[4] = {"black", "white", "empty", "invalid"};

    std::cerr << board.to_string();
    std::cerr << std::endl;
    std::cerr << "to move: " << color_map[get_tomove()] << std::endl;
    std::cerr << "komi: " << m_komi
                  << " | move number: " << m_movenum << std::endl;
    std::cerr << "hash: " << std::hex << board.compute_hash() << std::endl;
}

int GameState::get_state(int vtx) const {
    return board.get_state(vtx);
};

int GameState::get_x(int vtx) const {
    return board.get_x(vtx);
}

int GameState::get_y(int vtx) const {
    return board.get_y(vtx);
}

int GameState::get_tomove() const {
    return board.get_tomove();
}

int GameState::get_last_move() const {
    return board.get_last_move();
}

int GameState::get_komove() const {
    return board.get_komove();
}

int GameState::get_board_size() const {
    return board.get_board_size();
}

int GameState::get_passes() const {
    return board.get_passes();
}

float GameState::get_komi() const {
    return m_komi;
}

void GameState::set_komi(float komi) {
    m_komi = komi;
}

void GameState::set_to_move(int color) {
    board.set_to_move(color);
}
