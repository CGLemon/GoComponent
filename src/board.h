#ifndef BOARD_H_INCLUDE
#define BOARD_H_INCLUDE

#include <array>
#include <cstdint>

class Board {
public:
    static constexpr int BOARD_SIZE = 19;
    static constexpr int NUM_VERTICES = (BOARD_SIZE+2) * (BOARD_SIZE+2);
    static constexpr int NUM_INTESECTIONS = BOARD_SIZE * BOARD_SIZE;

    static constexpr int PASS = -1;
    static constexpr int RESIGN = -2;
    static constexpr int NULL_VERTEX = NUM_VERTICES+1;

    enum vertex_t : std::uint8_t {
        BLACK = 0,
        WHITE = 1,
        EMPTY = 2,
        INVLD = 3
    };

    int get_vertex(int x, int y) const;

    int get_index(int x, int y) const;

    void reset_board(int board_size);

    void play_move_assume_legal(int vtx, int color);

    bool legal_move(int vtx, int color) const;

    int compute_reach_color(int color) const;

    std::string to_string() const;

    int get_x(int vtx) const;
    int get_y(int vtx) const;
    int get_state(int vtx) const;
    int get_tomove() const;
    int get_last_move() const;
    int get_komove() const;
    int get_board_size() const;
    int get_passes() const;

    void set_to_move(int color);

    std::uint64_t compute_hash() const;

private:
    // Return true if it is suicide move.
    bool is_suicide(int vtx, int color) const;

    // Update whole board.
    int update_board(int vtx, int color);

    // Merge two same color strings.
    void merge_strings(int ip, int aip);

    // Capture a string and remove it.
    int remove_string(int ip);

    void remove_stone(int vtx, int color);

    void add_stone(int vtx, int color);

    // Return true if surround colors are mine.
    bool is_eyeshape(int vtx, int color);

    std::array<int, 4> m_directions;

    // The board state.
    std::array<vertex_t, NUM_VERTICES> m_state;
    
    // The next stone in string.
    std::array<std::uint16_t, NUM_VERTICES+1> m_next;

    // The parent node of string.
    std::array<std::uint16_t, NUM_VERTICES+1> m_parent;

    // The liberties per string parent.
    std::array<std::uint16_t, NUM_VERTICES+1> m_liberties;

    // The stones per string parent.
    std::array<std::uint16_t, NUM_VERTICES+1> m_stones;

    int m_board_size;

    int m_last_move;

    int m_tomove;

    int m_komove;

    int m_passes;
};

#endif
