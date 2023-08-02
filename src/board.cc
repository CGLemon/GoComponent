#include <string>
#include <sstream>
#include <queue>
#include <functional>

#include "board.h"

constexpr int Board::BOARD_SIZE;
constexpr int Board::NUM_VERTICES;
constexpr int Board::NUM_INTESECTIONS;

constexpr int Board::PASS;
constexpr int Board::RESIGN;
constexpr int Board::NULL_VERTEX;

void Board::reset_board(int board_size) {
    m_board_size = std::min(board_size, BOARD_SIZE);

    const int x_shift = m_board_size+2;
    m_directions[0] = (-x_shift);
    m_directions[1] = (-1);
    m_directions[2] = (+1);
    m_directions[3] = (+x_shift);

    for (int vtx = 0; vtx < NUM_VERTICES + 1; ++vtx) {
        m_parent[vtx] = NUM_VERTICES;
        m_next[vtx] = NUM_VERTICES;
        m_stones[vtx] = 0;
        m_liberties[vtx] = 0;
        if (vtx != NUM_VERTICES)
            m_state[vtx] = INVLD;
    }
    m_liberties[NUM_VERTICES] = 16384;

    for (int y = 0; y < m_board_size; ++y) {
        for (int x = 0; x < m_board_size; ++x) {
            m_state[get_vertex(x,y)] = EMPTY;
        }
    }

    m_passes = 0;
}

bool Board::legal_move(int vtx, int color) const {
    if (vtx == PASS || vtx == RESIGN) {
        return true;
    }

    if (m_state[vtx] != EMPTY) {
        return false;
    }

    if (is_suicide(vtx, color)) {
        return false;
    }

    if (vtx == m_komove) {
        return false;
    }

    return true;
}

bool Board::is_suicide(int vtx, int color) const {
    for (auto k = 0; k < 4; ++k) {
        const int avtx = vtx + m_directions[k];
        const int libs =  m_liberties[m_parent[avtx]];
        const int state = m_state[avtx];

        if (state == EMPTY) {
            return false;
        } else if (state == color && libs > 1) {
            // Be sure that the string at least is one liberty.
            return false;
        } else if (state == (!color) && libs <= 1) {
            // We can capture opponent's stone.
            return false;
        }
    }

    return true;
}

void Board::play_move_assume_legal(int vtx, int color) {
    m_tomove = color;

    if (vtx == PASS) {
        m_passes++;
        m_komove = NULL_VERTEX;
    } else {
        m_passes = 0;
        m_komove = update_board(vtx, m_tomove);
    }

    m_last_move = vtx;
    m_tomove = !m_tomove ;
}

int Board::update_board(int vtx, int color) {
    add_stone(vtx, color);

    int captured_stones = 0;
    int captured_vtx = NULL_VERTEX;
    bool is_eyeplay = true;

    for (int k = 0; k < 4; ++k) {
        const int avtx = vtx + m_directions[k];
        const int aip = m_parent[avtx];
        const int state = m_state[avtx];

        if (state == !color) {
            if (m_liberties[aip] <= 0) {
                const int this_captured = remove_string(avtx);
                captured_vtx = avtx;
                captured_stones += this_captured;
            }
        } else if (state == color) {
            const int ip = m_parent[vtx];
            if (ip != aip) {
                merge_strings(ip, aip);
            }
            is_eyeplay = false;
        }
    }

    if (m_liberties[m_parent[vtx]] == 0) {
        // Suicide move, this move is illegal in general rule.
        const int sucide_stones = remove_string(vtx);
    }

    if (captured_stones == 1 && is_eyeplay) {
        // Make a ko.
        return captured_vtx;
    }

    return NULL_VERTEX;
}

void Board::merge_strings(int ip, int aip) {
    if (m_stones[ip] < m_stones[aip]) {
        std::swap(aip, ip);
    }
    m_stones[ip] += m_stones[aip];
    int next_pos = aip;

    do {
        for (int k = 0; k < 4; k++) {
            const int apos = next_pos + m_directions[k];
            if (m_state[apos] == EMPTY) {
                bool found = false;
                for (int kk = 0; kk < 4; kk++) {
                    const int aapos = apos + m_directions[kk];
                    if (m_parent[aapos] == ip) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    m_liberties[ip]++;
                }
            }
        }

        m_parent[next_pos] = ip;
        next_pos = m_next[next_pos];
    } while (next_pos != aip);

    std::swap(m_next[aip], m_next[ip]);
}

int Board::remove_string(int ip) {
    int pos = ip;
    int removed = 0;
    int color = m_state[ip];

    do {
        remove_stone(pos, color);
        m_parent[pos] = NUM_VERTICES;

        removed++;

        pos = m_next[pos];
    } while (pos != ip);

    return removed;
}

void Board::add_stone(int vtx, int color) {
    m_next[vtx] = vtx;
    m_parent[vtx] = vtx;
    m_liberties[vtx] = 0;
    m_stones[vtx] = 1;

    int nbr_pars[4];
    int nbr_par_cnt = 0;

    // Set board content.
    m_state[vtx] = static_cast<vertex_t>(color);

    for (int k = 0; k < 4; ++k) {
        const auto avtx = vtx + m_directions[k];

        if (m_state[avtx] == EMPTY) {
            m_liberties[vtx]++;
        }


        bool found = false;
        const auto ip = m_parent[avtx];
        for (int i = 0; i < nbr_par_cnt; ++i) {
            if (nbr_pars[i] == ip) {
                found = true;
                break;
            }
        }
        if (!found) {
            m_liberties[ip]--;
            nbr_pars[nbr_par_cnt++] = ip;
        }
    }
}

void Board::remove_stone(int vtx, int color) {
    int nbr_pars[4];
    int nbr_par_cnt = 0;

    // Set board content.
    m_state[vtx] = EMPTY;

    for (int k = 0; k < 4; ++k) {
        const int avtx = vtx + m_directions[k];

        bool found = false;
        const int ip = m_parent[avtx];
        for (int i = 0; i < nbr_par_cnt; i++) {
            if (nbr_pars[i] == ip) {
                found = true;
                break;
            }
        }
        if (!found) {
            m_liberties[ip]++;
            nbr_pars[nbr_par_cnt++] = ip;
        }
    }
}

int Board::compute_reach_color(int color) const {
    bool marked[NUM_VERTICES];
    for (int vtx = 0; vtx < NUM_VERTICES; ++vtx) {
        marked[vtx] = false;
    }

    int reachable = 0;
    std::queue<int> open;

    for (int y = 0; y < m_board_size; ++y) {
        for (int x = 0; x < m_board_size; ++x) {
            const int vtx = get_vertex(x, y);
            const int state = m_state[vtx];

            if (state == color) {
                ++reachable;
                marked[vtx] = true;
                open.emplace(vtx);
            } else {
                marked[vtx] = false;
            }
        }
    }
    while (!open.empty()) {
        const int vtx = open.front();
        open.pop();

        for (int k = 0; k < 4; ++k) {
            const int neighbor = vtx + m_directions[k];
            const int state = m_state[neighbor];

            if (!marked[neighbor] && state == color) {
                ++reachable;
                marked[neighbor] = true;
                open.emplace(neighbor);
            }
        }
    }

    return reachable;
}

std::string Board::to_string() const {
    std::ostringstream ss;

    for (int y = m_board_size-1; y >= 0; --y) {
        if (y == m_board_size-1) {
            const char *x_lable_map = "ABCDEFGHJKLMNOPQRST";
            ss << "  ";
            for (int x = 0; x < m_board_size; ++x) {
                ss << x_lable_map[x] << ' ';
            }
            ss << '\n';
        }

        ss << y+1 << ' ';

        for (int x = 0; x < m_board_size; ++x) {

            int state = m_state[get_vertex(x,y)];
            if (state == BLACK) ss << 'x';
            else if (state == WHITE) ss << 'o';
            else if (state == EMPTY) ss << '.';

            ss << ' ';
        }
        ss << '\n';
    }
    return ss.str();
}

int Board::get_tomove() const {
    return m_tomove;
}

int Board::get_last_move() const {
    return m_last_move;
}

int Board::get_komove() const {
    return m_komove;
}

int Board::get_board_size() const {
    return m_board_size;
}

int Board::get_passes() const {
    return m_passes;
}

int Board::get_state(int vtx) const {
    return m_state[vtx];
}

std::uint64_t Board::compute_hash() const {
    // TODO: Use Zobrist hash instead of it.
    char state_map[4] = {'x','o','.', '-'};
    std::string str;
    str.resize(NUM_VERTICES, state_map[INVLD]);

    for (int vtx = 0; vtx < NUM_VERTICES; ++vtx) {
        str[vtx] = state_map[m_state[vtx]];
    }

    return std::hash<std::string>{}(str);
}

int Board::get_index(int x, int y) const {
    return y * m_board_size + x;
}

int Board::get_vertex(int x, int y) const {
    return (y+1) * (m_board_size + 2) + (x+1);
}

int Board::get_x(int vtx) const {
    return vtx % (m_board_size + 2) - 1;
}

int Board::get_y(int vtx) const {
    return vtx / (m_board_size + 2) - 1;
}

void Board::set_to_move(int color) {
    m_tomove = color;
}
