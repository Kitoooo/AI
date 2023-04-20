#include "graph_search.hpp"
#include "graph_state.hpp"
#include <array>
#include <sstream>
#include <unordered_set>
#include <numeric>


template<int T_HEIGHT, int T_WIDTH, template <int, int> class Heuristic>
class sliding_puzzle : public f_state<uint8_t> {
public:
	sliding_puzzle() {
		int c = 0;
		for (auto& row : board) {
			for (auto& cell : row) {
				cell = c;
				c++;
			}
		}
		shuffleBoard(1000);
	}

	sliding_puzzle(std::string board_puzzle) {
		std::istringstream in(board_puzzle);
		int c;
		for (auto& row : board)
			for (auto& cell : row) {
				if (!(in >> c)) {
					throw std::exception("Invalid board");
				}
				cell = c;
			}

	}
	
	std::unique_ptr<graph_state> clone() const override {
		return std::unique_ptr<graph_state>(new sliding_puzzle(*this));
	}

	size_t hash_code() const {
		size_t h = 1;
		for (const auto& row : board)
			for (auto cell : row) {
				h = h * 31 + cell;
			}
		return h;
	}
	
	std::vector<std::unique_ptr<graph_state>> get_successors() const {
		std::vector<std::unique_ptr<graph_state>> successors;
		for (int i = 0; i < T_HEIGHT; i++)
			for (int j = 0; j < T_WIDTH; j++)
				if (board[i][j] == 0) {
					for (auto [di, dj] : possibilites(i, j)) {
						auto c = new sliding_puzzle(*this);

						std::swap(c->board[i][j], c->board[i + di][j + dj]);
						c->update_score(get_g() + 1);
						successors.push_back({});
						successors.back().reset(c);

						if (di < 0) {
							c->move = 'v';
						}
						else if (di > 0) {
							c->move = '^';
						}
						else if (dj < 0) {
							c->move = '>';
						}
						else if (dj > 0) {
							c->move = '<';
						}
						else c->move = '?';
					}
					return successors;
				}
		return{};
	}

	std::vector<std::pair<int, int>>possibilites(int row, int col) const {
		std::vector<std::pair<int, int>> poss;
		if (board[row][col] != 0) return {};

		if (row > 0) {
			poss.push_back({ -1,0 });
		}
		if (row < T_HEIGHT - 1) {
			poss.push_back({ 1,0 });
		}
		if (col > 0) {
			poss.push_back({ 0,-1 });
		}
		if (col < T_WIDTH - 1) {
			poss.push_back({ 0,1 });
		}

		return poss;
	}
	bool is_solution() const {
		int i = 0;
		for (const auto& row : board) {
			for (auto cell : row) {
				if (cell != i) return false;
				i++;
			}
		}
		return true;
	}
	std::string to_string() const {
		std::ostringstream out;
		for (const auto& row : board)
		{
			for (int cell : row)
				out << cell << ' ';
			out << std::endl;
		}
		return out.str();
	}
	std::string getBoardString() {
		std::ostringstream out;
		for (const auto& row : board)
		{
			for (int cell : row)
				out << cell << ' ';
		}
		return out.str();
	}
	void setBoard(std::array<std::array<int, T_WIDTH>, T_HEIGHT> board) {
		this->board = board;
	}

	static std::string get_path(const sliding_puzzle *s) {
		std::string p;
		while (s->get_parent()) {
			p += s->move;
			s = (const sliding_puzzle *)s->get_parent();
		}
		return p;
	}
	std::string move;
protected:
	void shuffleBoard(int moves) {
		for (int i = 0; i < moves; i++) {
			for (int i = 0; i < T_HEIGHT; i++) {
				for (int j = 0; j < T_WIDTH; j++) {
					if (board[i][j] == 0) {
						auto poss = possibilites(i, j);
						auto [di, dj] = poss[rand() % poss.size()];
						std::swap(board[i][j], board[i + di][j + dj]);
					}
				}
			}
		}
	}
	
	uint8_t get_heuristic_grade() const override {
		return heuristic(board);
	}
	std::array<std::array<int, T_WIDTH>, T_HEIGHT> board;
	static constexpr Heuristic<T_HEIGHT, T_WIDTH> heuristic{};
	
	bool is_equal(const graph_state& s) const {
		const auto st = dynamic_cast<const sliding_puzzle*>(&s);
		return st != nullptr && st->board == this->board;

	}
};

// Heuristic functions
template<int T_HEIGHT, int T_WIDTH>
struct None {
	int operator()(const auto& board) const {
		return 0;
	}
};

template<int T_HEIGHT, int T_WIDTH>
struct H_Manhattan {
	uint8_t operator()(const auto& board) const {
		int h = 0;
		for (int i = 0; i < T_HEIGHT; i++) {
			for (int j = 0; j < T_WIDTH; j++) {
				int cell = board[i][j];
				if (cell == 0) continue;
				int goalRow = cell / T_WIDTH;
				int goalCol = cell % T_WIDTH;
				h += abs(i - goalRow) + abs(j - goalCol);
			}
		}
		return h;
	}
};

template<int T_HEIGHT, int T_WIDTH>
struct H_Misplaced {
	uint8_t operator()(const auto& board) const {
		int h = 0;
		for (int i = 0; i < T_HEIGHT; i++) {
			for (int j = 0; j < T_WIDTH; j++) {
				int cell = board[i][j];
				if (cell == 0) continue;
				int goalRow = cell / T_WIDTH;
				int goalCol = cell % T_WIDTH;
				if (i != goalRow || j != goalCol) h++;
			}
		}
		return h;
	}
};

double average(std::vector<double> const& v) {
	if (v.empty()) {
		return 0;
	}

	auto const count = static_cast<float>(v.size());
	return std::reduce(v.begin(), v.end()) / count;
}

int main() {
	std::string test1 = "1 2 3 4 5 6 7 8 0";
	std::string test2 = "1 2 5 3 0 8 6 4 7";
	std::string test3 = "0 1 2 4 8 5 3 6 7";
	std::string test4 = "3 1 2 0 4 5 6 7 8";
	std::string test5 = "3 1 6 8 5 2 7 4 0";
	std::string test6 = "5 1 6 8 7 3 4 2 0";
	std::string test4x4 = "9 11 13 15 10 3 14 12 5 4 1 8 7 6 2 0";
	
	std::string test = test6;
	
	std::vector<double> avgSteps_noneAstar;
	std::vector<double> avgTime_noneAstar;
	std::vector<double> avgClosed_noneAstar;
		
	std::vector<double> avgSteps_manhattanAstar;
	std::vector<double> avgTime_manhattanAstar;
	std::vector<double> avgClosed_manhattanAstar;

	std::vector<double> avgSteps_misplacedAstar;
	std::vector<double> avgTime_misplacedAstar;
	std::vector<double> avgClosed_misplacedAstar;

	std::vector<double> avgSteps_noneBFS;
	std::vector<double> avgTime_noneBFS;
	std::vector<double> avgClosed_noneBFS;

	std::vector<double> avgSteps_manhattanBFS;
	std::vector<double> avgTime_manhattanBFS;
	std::vector<double> avgClosed_manhattanBFS;

	std::vector<double> avgSteps_misplacedBFS;
	std::vector<double> avgTime_misplacedBFS;
	std::vector<double> avgClosed_misplacedBFS;


	
	for (int i = 0; i < 100; i++) {
		sliding_puzzle<3, 3, None> puzel_none;
		sliding_puzzle<3, 3, H_Manhattan> puzel_manhatan(puzel_none.getBoardString());
		sliding_puzzle<3, 3, H_Misplaced> puzel_misplaced(puzel_none.getBoardString());
		
		informative_searcher<uint8_t> is_none(puzel_none, default_f_compare<uint8_t>);
		informative_searcher<uint8_t> is_manhatan(puzel_manhatan, default_f_compare<uint8_t>);
		informative_searcher<uint8_t> is_misplaced(puzel_misplaced, default_f_compare<uint8_t>);

		informative_searcher<uint8_t> is_noneBFS(puzel_none, default_h_compare<uint8_t>);
		informative_searcher<uint8_t> is_manhatanBFS(puzel_manhatan, default_h_compare<uint8_t>);
		informative_searcher<uint8_t> is_misplacedBFS(puzel_misplaced, default_h_compare<uint8_t>);
		
		avgSteps_noneAstar.push_back(is_none.get_solution_path(0).size() - 1);
		avgTime_noneAstar.push_back(is_none.get_elapsed_time());
		avgClosed_noneAstar.push_back(is_none.get_closed().size());

		std::cout << is_none.get_stats() << std::endl;
		//std::cout << is_none.get_solution(0)->to_string() << std::endl;
		std::cout << puzel_none.get_path((decltype(puzel_none)*)is_none.get_solution(0)) << std::endl;

		avgSteps_noneBFS.push_back(is_noneBFS.get_solution_path(0).size() - 1);
		avgTime_noneBFS.push_back(is_noneBFS.get_elapsed_time());
		avgClosed_noneBFS.push_back(is_noneBFS.get_closed().size());

		avgSteps_manhattanAstar.push_back(is_manhatan.get_solution_path(0).size() - 1);
		avgTime_manhattanAstar.push_back(is_manhatan.get_elapsed_time());
		avgClosed_manhattanAstar.push_back(is_manhatan.get_closed().size());
		std::cout << is_manhatan.get_stats() << std::endl;
		//std::cout << is_manhatan.get_solution(0)->to_string() << std::endl;

		avgSteps_manhattanBFS.push_back(is_manhatanBFS.get_solution_path(0).size() - 1);
		avgTime_manhattanBFS.push_back(is_manhatanBFS.get_elapsed_time());
		avgClosed_manhattanBFS.push_back(is_manhatanBFS.get_closed().size());
		
		avgSteps_misplacedAstar.push_back(is_misplaced.get_solution_path(0).size() - 1);
		avgTime_misplacedAstar.push_back(is_misplaced.get_elapsed_time());
		avgClosed_misplacedAstar.push_back(is_misplaced.get_closed().size());
		std::cout << is_misplaced.get_stats() << std::endl;
		//std::cout << is_misplaced.get_solution(0)->to_string() << std::endl;
		avgSteps_misplacedBFS.push_back(is_misplacedBFS.get_solution_path(0).size() - 1);
		avgTime_misplacedBFS.push_back(is_misplacedBFS.get_elapsed_time());
		avgClosed_misplacedBFS.push_back(is_misplacedBFS.get_closed().size());
	}	
	std::cout << "A*: " << std::endl;
	std::cout << "None: \n\tAvg steps: " << average(avgSteps_noneAstar) <<"\n\t Avg time: "<<average(avgTime_noneAstar)/ 1000 <<"s\n\t" <<"Avg Closed: "<< average(avgClosed_noneAstar)<< std::endl;
	std::cout << "Manhattan: \n\tAvg steps: " << average(avgSteps_manhattanAstar) << "\n\t Avg time: " << average(avgTime_manhattanAstar) / 1000 << "s\n\t" << "Avg Closed: " << average(avgClosed_manhattanAstar) << std::endl;
	std::cout << "Misplaced: \n\tAvg steps: " << average(avgSteps_misplacedAstar) << "\n\t Avg time: " << average(avgTime_misplacedAstar) / 1000 << "s\n\t" << "Avg Closed: " << average(avgClosed_misplacedAstar) << std::endl;
	std::cout << "\nBFS: " << std::endl;
	std::cout << "None: \n\tAvg steps: " << average(avgSteps_noneBFS) << "\n\t Avg time: " << average(avgTime_noneBFS) / 1000 << "s\n\t" << "Avg Closed: " << average(avgClosed_noneBFS) << std::endl;
	std::cout << "Manhattan: \n\tAvg steps: " << average(avgSteps_manhattanBFS) << "\n\t Avg time: " << average(avgTime_manhattanBFS) / 1000 << "s\n\t" << "Avg Closed: " << average(avgClosed_manhattanBFS) << std::endl;
	std::cout << "Misplaced: \n\tAvg steps: " << average(avgSteps_misplacedBFS) << "\n\t Avg time: " << average(avgTime_misplacedBFS) / 1000 << "s\n\t" << "Avg Closed: " << average(avgClosed_misplacedBFS) << std::endl;
	

	return 0;
}