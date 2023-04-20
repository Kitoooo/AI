#include "game_state.hpp"
#include "game_searcher.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <algorithm>
#include <random>

#define ROW 6
#define COL 7
/*	
	  c c c c c c c
	  o o o o o o o 
	  l l l l l l l
	  _______________
row | 0 0 0 0 0 0 0 0
row | 0 0 0 0 0 0 0 1
row | 0 0 0 0 0 0 0 2
row | 0 0 0 0 0 X 0 3
row | 0 0 0 0 0 0 0 4
row | 0 0 0 0 0 0 0 5
      0 1 2 3 4 5 6
	board[row][col]

	board[3][5] = X
*/
class Connect4 : public game_state<int> {
public:
	Connect4() {
		current_player = 1;
		board.fill({0});
	}

	std::unique_ptr<game_state> clone() const override {
		return std::make_unique<Connect4>(*this);

	}
	
	std::vector<int> generate_moves() const override {
		std::vector<int> moves;
		for (int i = 0; i < 7; i++) {
			if (board[0][i] == 0) {
				moves.push_back(i);
			}
		}
		return moves;
	}
	
	std::unique_ptr<game_state> make_move(const int& move) const override {
		auto new_state = new Connect4(*this);

		for (int i = 5; i >= 0; i--) {
			if (new_state->board[i][move] == 0) {
				new_state->board[i][move] = current_player;
				break;
			}
		}
		new_state->current_player = 3 - current_player;
		return std::unique_ptr<game_state>(new_state);
	}
	
	size_t hash_code() const override {
		size_t h = 1;
		for (const auto &row : board) {
			for (const auto& cell : row) {
				h = h * 31 + cell;
			}
		}
		return h;
	}
	
	double get_h() const override {
		int values[4] = { 0,1,10,100 };
		int value = 0;
		int opponent = 3 - current_player;
		//horizontal
		for (int i = 0; i < ROW; i++) {
			for (int j = 0; j < COL-3; j++) {
				int myCount = 0;
				int oppCount = 0;
				for (int k = 0; k < 4; k++) {
					if (board[i][j + k] == current_player) {
						myCount++;
					}
					else if (board[i][j + k] == opponent) {
						oppCount++;
					}
				}
				if (oppCount == 0) {
					value += values[myCount];
				}
				else if (myCount == 0) {
					value -= values[myCount];
				}
			}
		}
		//vertical
		for (int i = 0; i < ROW-3; i++) {
			for (int j = 0; j < COL; j++) {
				int myCount = 0;
				int oppCount = 0;
				for (int k = 0; k < 4; k++) {
					if (board[i + k][j] == current_player) {
						myCount++;
					}
					else if (board[i + k][j] == opponent) {
						oppCount++;
					}
				}
				if (oppCount == 0) {
					value += values[myCount];
				}
				else if (myCount == 0) {
					value -= values[myCount];
				}
			}
		}
		//diagonal
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 4; j++) {
				int myCount = 0;
				int oppCount = 0;
				for (int k = 0; k < 4; k++) {
					if (board[i + k][j + k] == current_player) {
						myCount++;
					}
					else if (board[i + k][j + k] == opponent) {
						oppCount++;
					}
				}
				if (oppCount == 0) {
					value += values[myCount];
				}
				else if (myCount == 0) {
					value -= values[myCount];
				}
			}
		}
		for (int i = 0; i < 3; i++) {
			for (int j = 6; j > 2; j--) {
				int myCount = 0;
				int oppCount = 0;
				for (int k = 0; k < 4; k++) {
					if (board[i + k][j - k] == current_player) {
						myCount++;
					}
					else if (board[i + k][j - k] == opponent) {
						oppCount++;
					}
				}
				if (oppCount == 0) {
					value += values[myCount];
				}
				else if (myCount == 0) {
					value -= values[myCount];
				}
			}
		}
		return value;
	}

	std::optional<double> is_terminal() const override {
		// inf  = 1 wins
		// -inf = 2 wins
		// 0    = draw
		// {}   = nothing

		//int ROW = 6;
		//int COL = 7;

		// Check for a win
		
		//vertical
		for (int i = 0; i < ROW; i++) {
			for (int j = 0; j < COL - 3; j++) {
				if (board[i][j] != 0 && board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3]) {
					return board[i][j] == 1 ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
				}
			}
		}
		//horizontal
		for (int i = 0; i < ROW - 3; i++) {
			for (int j = 0; j < COL; j++) {
				if (board[i][j] != 0 && board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j]) {
					return board[i][j] == 1 ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
				}
			}
		}
		//diagonal
		for (int i = 0; i < ROW-3; i++) {
			for (int j = 0; j < COL - 3; j++) {
				if (board[i][j] != 0 && board[i][j] == board[i + 1][j + 1] && board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3]) {
					return board[i][j] == 1 ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
				}
			}
		}
		for (int i = 3; i < ROW; i++) {
			for (int j = 0; j < COL - 3; j++) {
				if (board[i][j] != 0 && board[i][j] == board[i - 1][j + 1] && board[i][j] == board[i - 2][j + 2] && board[i][j] == board[i - 3][j + 3]) {
					return board[i][j] == 1 ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
				}
			}
		}
		
		//Check for draw
		for (int j = 0; j < COL; j++) {
			if (board[0][j] == 0 ) {
				return {};
			}
		}
		return 0;
	}

	bool is_equal(const game_state& s) const override {
		const auto st = dynamic_cast<const Connect4*>(&s);
		return st != nullptr && st->board == this->board;
	}

	std::string to_string() const {
		std::ostringstream out;
		for (const auto& row : board) {
			for (const auto& cell : row) {
				out << cell << " ";
			}
			out << std::endl;
		}
		return out.str();
	}

	int current_player;
protected:
	std::array<std::array<int, 7>, 6> board;
};

int main() {
	Connect4 board;

	std::cout << board.to_string() << std::endl;
	bool pvp = false;
	bool pve = true;
	while (pvp) {
		int move;
		std::cin >> move;
		std::vector<int> moves = board.generate_moves();
		if (std::find(moves.begin(), moves.end(), move) == moves.end()) {
			std::cout << "Unavailable move! try again"<<std::endl;
			continue;
		}
		board = (Connect4&)*board.make_move(move);
		std::cout << board.to_string() << std::endl;
		if (board.is_terminal()) {
			std::cout << board.is_terminal().value() << std::endl;
		}
	}
	
	
	if (pve) {
		alpha_beta_searcher<int> searcher = alpha_beta_searcher<int>(11,false);
		while (1) {
			if (board.is_terminal().has_value() == false) {
				//player move
				int move;
				std::cin >> move;
				std::vector<int> moves = board.generate_moves();
				if (std::find(moves.begin(), moves.end(), move) == moves.end()) {
					std::cout << "Unavailable move! try again" << std::endl;
					continue;
				}
				board = (Connect4&)*board.make_move(move);
				std::cout << board.to_string() << std::endl;
			}
			else {
				break;
			}
			if (board.is_terminal().has_value() == false) {
				//computer move
				searcher.do_search(board);
				std::vector<std::pair<int, double>> move_scores = searcher.get_scores();
				std::pair<int, double> best_move = move_scores[0];
				for (auto moveScore : move_scores) {
					std::cout << moveScore.first << " " << moveScore.second << " | ";
					if (moveScore.second < best_move.second) {
						best_move = moveScore;
					}
				}
				std::cout << std::endl;
				std::cout << "Liczba odwiedzonych stan�w: " << searcher.get_number_of_visited_states() << std::endl;
				//std::cin >> move;
				board = (Connect4&)*board.make_move(best_move.first);
				std::cout << board.to_string() << std::endl;
			}
			else {
				break;
			}
		}
	}
	std::cout << "Game Over!";
	return 0;
}