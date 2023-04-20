#include "graph_state.hpp"
#include "graph_search.hpp"
#include "queue.hpp"
#include <array>
#include <sstream>
#include <unordered_set>


template<int M, int N, template<int, int> class Heuristic>
class generic_sudoku : public h_state<uint8_t>
{
public:
	generic_sudoku(std::string board_string) {
		std::istringstream in(board_string);
		int c;
		for (auto& row : board) {
			for (auto &cell : row) {
				if (!(in >> c) || c < 0 || c > M*N) {
					throw std::exception("Invalid board_string argument");
				}
				cell= c;
			}
		}
	}

	std::unique_ptr<graph_state> clone() const override {
		return std::unique_ptr<graph_state>(new generic_sudoku(*this));
	}
	
	std::size_t hash_code() const {
		size_t h = 1;
		for (const auto& row : board) {
			for (const auto cell : row) {
				h = h * 31 + cell;
			}
		}
		return h;
	}

	bool is_solution() const {
		for (const auto& row : board) {
			for (const auto cell : row) {
				if (cell == 0) return false;
			}
		}
		return true;
	}

	std::string to_string() const {
		std::ostringstream out;
		int i = 0;
		for (const auto& row : board) {
			for (const int cell : row) {
				out << cell << ' ';
			}
			out << std::endl;
		}
		return out.str();
	}

	std::unordered_set<int8_t> possibilities(int r, int c) const {
		std::unordered_set<int8_t> possibilities;
		for (int i = 1; i <= M * N; i++) {
			possibilities.insert(i);
		}
		for (int i = 0; i < M * N; i++) {
			possibilities.erase(board[r][i]);
			possibilities.erase(board[i][c]);
		}
		int a = r - r % M, b = c - c % N;
		for (int i = 0; i < M; i++) {
			for(int j = 0; j < N; j++){
				possibilities.erase(board[i + a][j + b]);
			}
		}

		return possibilities;
	}

	std::vector<std::unique_ptr<graph_state>> get_successors() const
	{

		std::vector<std::unique_ptr<graph_state>> successors;
		for (int i = 0; i < M*N; i++)
			for (int j = 0; j < M*N; j++)
				if (board[i][j] == 0)
				{
					for (int8_t p : possibilities(i, j))
					{
						auto c = clone();
						((generic_sudoku&)*c).board[i][j] = p;
						c->update_score(get_g() + 1);
						successors.push_back(move(c));
					}
					return successors;
				}
		return{};
	}

protected:
	uint8_t get_heuristic_grade() const override {
		return heuristic(board);
	}
	bool is_equal(const graph_state& s) const override {
		const auto st = dynamic_cast<const generic_sudoku*>(&s);
		return st != nullptr && st-> board == this-> board;
	}
	std::array<std::array<int, M* N>, M* N> board;
	static constexpr Heuristic <M, N> heuristic {};
};

template <int Rows, int Cols>
struct H_remaining {
	uint8_t operator()(const auto& board) const {
		int h = 0;
		for (const auto& row : board) {
			for (const int cell : row) {
				if (cell == 0) h += 1;
			}
		}
		return h;
	}
};

template<int M, int N, template<int, int> class Heuristic>
class sudoku2_21 : public generic_sudoku<M,N,Heuristic>  {
public:
	sudoku2_21(std::string board_string) : generic_sudoku<M, N, Heuristic>(board_string) {
		
	}

	std::unique_ptr<graph_state<uint8_t>> clone() const override {
		return std::unique_ptr<graph_state<uint8_t>>(new sudoku2_21(*this));
	}
	std::vector<std::unique_ptr<graph_state<uint8_t>>> get_successors() const
	{
		std::vector<std::unique_ptr<graph_state<uint8_t>>> successors;
		int lowest_i=0, lowest_j=0, current_size=100000;
		for (int i = 0; i < M * N; i++)
			for (int j = 0; j < M * N; j++)
				if (this->board[i][j] == 0)
				{
					size_t possibilities_size = this->possibilities(i, j).size();
					if (current_size > possibilities_size) {
						current_size = possibilities_size;
						lowest_i = i;
						lowest_j = j;
					}
					
				}
		for (int8_t p : this->possibilities(lowest_i, lowest_j))
		{
			auto c = clone();
			((sudoku2_21&)*c).board[lowest_i][lowest_j] = p;
			c->update_score(this->get_g() + 1);
			successors.push_back(move(c));
		}
		return successors;
		return{};
	}
};

int	main() {
	std::string test2x2 = "0 3 4 0 4 0 0 2 1 0 0 3 0 2 1 0";
	std::string more_solutions2x2 = "0 3 4 0 0 0 0 0 0 0 0 3 0 2 1 0";
	std::string test3x3 = "9 0 0 0 3 8 0 7 0 0 0 0 6 0 0 0 0 8 0 3 0 1 0 0 0 0 0 0 0 0 8 0 0 0 0 0 0 0 2 0 0 0 0 9 0 0 6 0 0 5 4 0 0 7 0 0 0 0 1 0 0 0 0 6 0 0 0 0 0 5 0 0 0 5 0 0 7 3 0 0 4";
	generic_sudoku<2,2, H_remaining> sudok2x2(more_solutions2x2);
	std::cout << sudok2x2.to_string()<<std::endl;
	for (const auto& successor : sudok2x2.get_successors()) {
		std::cout << successor->to_string() << std::endl;
	}
	informative_searcher<uint8_t> is(sudok2x2, default_h_compare<uint8_t>, std::numeric_limits<size_t>::max());
	std::cout << is.get_stats() <<std::endl;
	for (int i = 0; i < is.get_number_of_solutions(); i++) {
		std::cout << is.get_solution(i)->to_string() << std::endl;
	}

	//std::string test6x6 = "1 2 3 4 5 6 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
	//generic_sudoku<2, 3, H_remaining> sudok6x6(test6x6);
	//std::cout << sudok6x6.to_string() << std::endl;
	///*for (const auto& successor : sudok6x6.get_successors()) {
	//	std::cout << successor->to_string() << std::endl;
	//}*/
	//informative_searcher<uint8_t> is6x6(sudok6x6, default_h_compare<uint8_t>, std::numeric_limits<size_t>::max());
	//std::cout << is6x6.get_stats() << std::endl;
	//std::cout << "Liczba wszystkich rozwiazan:" << is6x6.get_number_of_solutions() * 6 * 5 * 4 * 3 * 2 << std::endl;
	
	generic_sudoku<3, 3, H_remaining> generic3x3(test3x3);
	//std::cout << generic3x3.to_string() << std::endl;
	sudoku2_21<3, 3, H_remaining> sudoku2_213x3(test3x3);
	std::cout << sudoku2_213x3.to_string() << std::endl;
	informative_searcher<uint8_t> is_generic(generic3x3, default_h_compare<uint8_t>);
	std::cout << is_generic.get_stats() << std::endl;
	std::cout << is_generic.get_solution(0)->to_string() << std::endl;
	informative_searcher<uint8_t> is_2_21(sudoku2_213x3, default_h_compare<uint8_t>);
	std::cout << is_2_21.get_stats() << std::endl;
	std::cout << is_2_21.get_solution(0)->to_string() << std::endl;
	return 0;
}
