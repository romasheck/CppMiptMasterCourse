#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 10.04
// Description: Conway's Game of Life on a 10x10 board stored in std::vector<std::vector<int>>.

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using Board = std::vector<std::vector<int>>;

constexpr int kSize = 10;

Board make_board() {
    return Board(kSize, std::vector<int>(kSize, 0));
}

Board make_initial_board() {
    Board board = make_board();

    // A glider in the upper-left area and a stable block in the lower-right area.
    board[1][2] = 1;
    board[2][3] = 1;
    board[3][1] = 1;
    board[3][2] = 1;
    board[3][3] = 1;

    board[6][6] = 1;
    board[6][7] = 1;
    board[7][6] = 1;
    board[7][7] = 1;

    return board;
}

int count_neighbors(const Board& board, int row, int col) {
    int neighbors = 0;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) {
                continue;
            }

            const int nr = row + dr;
            const int nc = col + dc;

            if (0 <= nr && nr < kSize && 0 <= nc && nc < kSize) {
                neighbors += board[nr][nc];
            }
        }
    }

    return neighbors;
}

Board next_generation(const Board& board) {
    Board next = make_board();

    for (int row = 0; row < kSize; ++row) {
        for (int col = 0; col < kSize; ++col) {
            const int neighbors = count_neighbors(board, row, col);

            if (board[row][col] == 1) {
                next[row][col] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                next[row][col] = (neighbors == 3) ? 1 : 0;
            }
        }
    }

    return next;
}

void print_board(const Board& board, int generation) {
    std::cout << "Generation " << generation << ":\n";
    for (const auto& row : board) {
        for (int cell : row) {
            std::cout << (cell ? '#' : '.');
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

namespace tests {

struct Logger {
    int passed = 0;
    int failed = 0;

    void require(bool condition, const char* message) {
        if (condition) {
            ++passed;
            std::cout << "[OK]   " << message << '\n';
        } else {
            ++failed;
            std::cout << "[FAIL] " << message << '\n';
        }
    }
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_block_is_stable(Logger& log) {
    Board board = make_board();
    board[4][4] = 1;
    board[4][5] = 1;
    board[5][4] = 1;
    board[5][5] = 1;

    const Board next = next_generation(board);
    require(next == board, "2x2 block should be stable");
    log.require(true, "stable block remains unchanged");
}

void test_blinker_oscillates(Logger& log) {
    Board board = make_board();
    board[4][3] = 1;
    board[4][4] = 1;
    board[4][5] = 1;

    Board expected = make_board();
    expected[3][4] = 1;
    expected[4][4] = 1;
    expected[5][4] = 1;

    require(next_generation(board) == expected, "blinker should rotate");
    require(next_generation(expected) == board, "blinker should return after two steps");
    log.require(true, "blinker oscillates");
}

void test_glider_stays_alive(Logger& log) {
    Board board = make_initial_board();

    for (int i = 0; i < 4; ++i) {
        board = next_generation(board);
    }

    int alive = 0;
    for (const auto& row : board) {
        for (int cell : row) {
            alive += cell;
        }
    }

    require(alive >= 4, "expected surviving cells after several steps");
    log.require(true, "initial pattern evolves non-trivially");
}

void run_all() {
    Logger log;

    try {
        test_block_is_stable(log);
        test_blinker_oscillates(log);
        test_glider_stays_alive(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();

    Board board = make_initial_board();
    constexpr int generations = 6;

    std::cout << '\n';
    for (int generation = 0; generation < generations; ++generation) {
        print_board(board, generation);
        board = next_generation(board);
    }

    return 0;
}
