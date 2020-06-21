#ifndef REVERSIMODERNCPP_H
#define REVERSIMODERNCPP_H

#include<iostream>
#include<string>
#include<tuple>
#include<stack>
#include<queue>
#include<random>
#include<thread>
#include<atomic>
#include<shared_mutex>
#include<ostream>
#include<fstream>
#include <optional>
#include <chrono>
#include <ctime>
#include <tuple>
#include<boost/thread.hpp>
// #include<vector> // all vector are replaced by rvector
#include"RecyclableVectorContainer.h"
#include"IndentStreamBuf.h"
#include"ctre.hpp"

#define underline "\033[4m"
#define reset   "\033[0m"
inline void clear(){
	std::cout << "\033[2J\033[1;1H";
}

enum class Status { UNOCCUPIED, PLAYER, AI };

namespace mpcs51045 {
	/*
	* Monte Carlo tree search algorithm (MCTS)
	* https://en.wikipedia.org/wiki/Monte_Carlo_tree_search
	*/
	class MCTS {
		rvector<std::shared_ptr<MCTS>> children;
	public:
		std::weak_ptr<MCTS> parent;
		bool expandable = true;
		bool isRoot;
		std::tuple<int, int> position;
		Status status;
		std::atomic<int> score;
		std::atomic<int> visit;
		std::shared_mutex children_mutex;
		/*int score = 0;
		int visit  = 0;*/
		MCTS() : isRoot{ true } {}
		MCTS(std::tuple<int, int> position, Status status, std::shared_ptr<MCTS>& parent)
			:position{ position }, status{ status }, parent{ parent }, isRoot{ false }{}

		void pushchild(const std::shared_ptr<MCTS>& child){
			std::unique_lock<std::shared_mutex> lock(children_mutex);
			children.data.push_back(child);
		}

		std::shared_ptr<MCTS>& getChild(int index){
			std::shared_lock<std::shared_mutex> lock(children_mutex);
			return children.data[index];
		}

		int childrenNum(){
			std::shared_lock<std::shared_mutex> lock(children_mutex);
			return children.data.size();
		}

		int indexOfBestChild() {
			std::shared_lock<std::shared_mutex> lock(children_mutex);
			double score = 0;
			int index = 0;
			for (size_t i = 0; i < children.data.size(); i++) {
				double calculate = children.data[i]->score * 1.0 / children.data[i]->visit + 0.1 * sqrt(2 * std::log(children.data[i]->parent.lock()->visit * 1.0 / children.data[i]->visit));
				if (calculate > score) {
					calculate = score;
					index = i;
				}
			}
			return index;
		}
	};

	struct info { int x; int y; std::string player;};
	std::optional<info> extract_data(std::string_view s) noexcept {
		constexpr static auto pattern = ctll::fixed_string{ "x=(\\d+), y=(\\d+), turn=(\\w+).*" };
		if (auto [whole, sx, sy, splayer] = ctre::match<pattern>(s); whole) {
			return info{std::stoi(static_cast<std::string>(sx)), std::stoi(static_cast<std::string>(sy)), static_cast<std::string>(splayer)};
		} else {
			return std::nullopt;
		}
	}

	class ReversiChess {
	private:
		int const ROW = 8;
		int const COL = 8;
		int coreNum = boost::thread::physical_concurrency();
		// int coreNum = sysconf(_SC_NPROCESSORS_ONLN);;
		int unOccupiedPositionNum;
		rvector<Status> board;
		Status currentPlayer;
		std::string history;
		friend std::ostream& operator<<(std::ostream& os, const ReversiChess& chess);
	public:

		void initGame() {
			clear();
			unOccupiedPositionNum = 0;
			currentPlayer = Status::PLAYER;
			board.data.resize(ROW * COL);
			std::fill(board.data.begin(), board.data.end(), Status::UNOCCUPIED);
			currentPlayer = Status::PLAYER;
			setChessPiece(board, 3, 3, Status::PLAYER);
			setChessPiece(board, 4, 4, Status::PLAYER);
			setChessPiece(board, 3, 4, Status::AI);
			setChessPiece(board, 4, 3, Status::AI);
			std::cout << "Welcome to play ReversiChess!\nThe current number of cores: " << coreNum << std::endl;
		}

		ReversiChess() {
			initGame();
		}

		ReversiChess(const ReversiChess& other) : currentPlayer{ other.currentPlayer }, unOccupiedPositionNum{ other.unOccupiedPositionNum }{
			board.data.assign(other.board.data.begin(), other.board.data.end());
		}

		ReversiChess& operator=(const ReversiChess& other) {
			if (this != &other) {
				currentPlayer = other.currentPlayer;
				unOccupiedPositionNum = other.unOccupiedPositionNum;
				board.data.assign(other.board.data.begin(), other.board.data.end());
			}
			return *this;
		}

		std::string unix_timestamp_string(){
			time_t t = std::time(0);
			long int now = static_cast<long int> (t);
			return std::to_string(now);
		}

		void appendHistory(int x, int y, Status player){
			history.append("x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", turn=" + (player == Status::AI ? "AI" : "PLAYER") + "\n" );
		}

		void saveHistory(){
			std::string fname = unix_timestamp_string();
			std::ofstream file(fname);
			file << history;
			history.clear();
			std::cout << "This chess record has been saved to " + fname << std::endl;
		}

		~ReversiChess() noexcept {}

		bool isOnBoard(int x, int y) {
			return x >= 0 && x < ROW && y >= 0 && y < COL;
		}

		bool isValidMoveDecision(rvector<Status>& board, int x, int y, Status player) {
			bool isValid = false;
			if (!isOnBoard(x, y) || board.data[x * COL + y] != Status::UNOCCUPIED) {
				return isValid;
			}
			Status otherPlayer = player == Status::PLAYER ? Status::AI : Status::PLAYER;
			setChessPiece(board, x, y, player);
			int optionalDirections[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
			for (auto [dx, dy] : optionalDirections) {
				int nx = x + dx, ny = y + dy;
				if (isOnBoard(nx, ny) && board.data[nx * COL + ny] == otherPlayer) {
					while (board.data[nx * COL + ny] == otherPlayer) {
						nx += dx;
						ny += dy;
						if (!isOnBoard(nx, ny)) break;
					}
					if (!isOnBoard(nx, ny) || board.data[nx * COL + ny] != player) continue;
					nx -= dx;
					ny -= dy;
					if (board.data[nx * COL + ny] == otherPlayer) {
						isValid = true;
						break;
					}
				}
			}
			setChessPiece(board, x, y, Status::UNOCCUPIED);
			return isValid;
		}

		void reverseChessPieces(rvector<Status>& board, int x, int y, Status player) {
			int optionalDirections[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
			Status otherPlayer = player == Status::PLAYER ? Status::AI : Status::PLAYER;

			for (auto [dx, dy] : optionalDirections) {
				int nx = x + dx, ny = y + dy;
				if (isOnBoard(nx, ny) && board.data[nx * COL + ny] == otherPlayer) {
					while (board.data[nx * COL + ny] == otherPlayer) {
						nx += dx;
						ny += dy;
						if (!isOnBoard(nx, ny)) break;
					}
					if (!isOnBoard(nx, ny) || board.data[nx * COL + ny] != player) continue;
					nx -= dx;
					ny -= dy;
					while (!(nx == x && ny == y)) {
						board.data[nx * COL + ny] = player;
						nx -= dx;
						ny -= dy;
					}
				}
			}
		}

		rvector<std::tuple<int, int>> getAllValidMoveDecisions(rvector<Status> copyBoard, Status player) {
			rvector<std::tuple<int, int>> decisions;
			for (int i = 0; i < ROW; i++) {
				for (int j = 0; j < COL; j++) {
					if (isValidMoveDecision(copyBoard, i, j, player)) {
						decisions.data.push_back(std::tuple<int, int>(i, j));
					}
				}
			}
			return decisions;
		}


		std::tuple<int, int> getUserInput() {
			int x, y;
			char row;
			std::cout << "Enter row[A-H]:";
			std::cin >> row;
			x = row - 'A';
			std::cout << "Enter col[0-7]:";
			std::cin >> y;
			return std::make_tuple(x, y);
		}

		void setChessPiece(rvector<Status>& board, int x, int y, Status player) {
			board.data[x * COL + y] = player;
			unOccupiedPositionNum++;
		}

		void nextPlayer() {
			currentPlayer = currentPlayer == Status::PLAYER ? Status::AI : Status::PLAYER;
		}

		bool showOptionalMoveDecisions(Status player) {
			std::cout << "\nOptional Move Decisions: ";
			auto options = getAllValidMoveDecisions(this->board, player);
			for (auto option : options.data) {
				std::cout << "(" << static_cast<char>('A' + std::get<0>(option)) << ", " << std::get<1>(option) << "), ";
			}
			std::cout << std::endl;
			return !options.data.empty();
		}

		void playerMoveChessPiece() {
			bool hasOptionalPlace = showOptionalMoveDecisions(currentPlayer);
			if(!hasOptionalPlace) {
				std::cout << "Player has no place to set piece." << std::endl;
				return;
			}
			int x, y;
			while (hasOptionalPlace) {
				auto [tx, ty] = getUserInput();
				if (isValidMoveDecision(board, tx, ty, Status::PLAYER)) {
					x = tx;
					y = ty;
					break;
				}
				std::cin.clear();
				std::cout.clear();
				std::cout << "Illegal Move Decision! Try again." << std::endl;
			}
			clear();
			std::cout << "Player place piece X at (" << static_cast<char>('A' + x) << ", " << y << ")" << std::endl;
			appendHistory(x, y, currentPlayer);
			setChessPiece(board, x, y, currentPlayer);
			reverseChessPieces(board, x, y, currentPlayer);
		}

		std::tuple<int, int> getScoreOfBoard(const rvector<Status>& board) {
			int playerPieceNum = 0, AIPieceNum = 0;
			for (int i = 0; i < ROW; i++) {
				for (int j = 0; j < COL; j++) {
					if (board.data[i * ROW + j] == Status::AI) AIPieceNum++;
					if (board.data[i * ROW + j] == Status::PLAYER) playerPieceNum++;
				}
			}
			return std::tuple<int, int>(playerPieceNum, AIPieceNum);
		}

		bool isGameOver(const rvector<Status>& board) {
			int playerPieceNum = 0, AIPieceNum = 0;
			int unOccupied = 0;
			for (int i = 0; i < ROW; i++) {
				for (int j = 0; j < COL; j++) {
					if (board.data[i * ROW + j] == Status::AI) AIPieceNum++;
					else if (board.data[i * ROW + j] == Status::PLAYER) playerPieceNum++;
					else unOccupied++;
				}
			}
			if (playerPieceNum == 0 || AIPieceNum == 0) {
				return false;
			}
			return unOccupied == 0;
		}

		std::tuple<std::shared_ptr<MCTS>, Status> 
		treePolicy(rvector<Status>& copiedBoard, std::shared_ptr<MCTS> node, Status stepPlayer) {
			std::shared_ptr<MCTS> expand = nullptr;
			std::queue<std::shared_ptr<MCTS>> nodeQueen;
			nodeQueen.push(node);
			while (!nodeQueen.empty()) {
				auto next = nodeQueen.front();
				nodeQueen.pop();
				if (!next->expandable) {
					for (size_t i = 0; i < next->childrenNum(); i++) {
						nodeQueen.push(next->getChild(i));
					}
				}
				else {
					expand = next;
					break;
				}
			}

			if (expand == nullptr) {
				expand = node->getChild(0); // should be best child; later change
				int x = std::get<0>(expand->position), y = std::get<1>(expand->position);
				setChessPiece(copiedBoard, x, y, stepPlayer);
				reverseChessPieces(copiedBoard, x, y, stepPlayer);
				stepPlayer = stepPlayer == Status::AI ? Status::PLAYER : Status::AI;
			}
			else {
				expand->expandable = false;
				std::stack<std::shared_ptr<MCTS>> nodeStack;
				std::shared_ptr<MCTS> tp = expand;
				while (!tp && !tp->isRoot) {
					nodeStack.push(tp);
					tp = tp->parent.lock();
				}
				while (!nodeStack.empty()) {
					tp = nodeStack.top();
					int x = std::get<0>(tp->position), y = std::get<1>(tp->position);
					setChessPiece(copiedBoard, x, y, stepPlayer);
					reverseChessPieces(copiedBoard, x, y, stepPlayer);
					nodeStack.pop();
					stepPlayer = stepPlayer == Status::AI ? Status::PLAYER : Status::AI;
				}
				rvector<std::tuple<int, int>> positions = getAllValidMoveDecisions(copiedBoard, stepPlayer);
				for (std::tuple<int, int> position : positions.data) {
					expand->pushchild(std::make_shared<MCTS>(position, stepPlayer, std::ref(expand)));
					// expand->children.push_back(make_shared<MCTS>(position, stepPlayer, std::ref(expand)));
				}
			}
			// std::cout << "33" << std::endl;
			return std::tuple<std::shared_ptr<MCTS>, Status>(expand, stepPlayer);
		}

		std::tuple<int, int> defaultPolicy(rvector<Status>& board, std::shared_ptr<MCTS> game, Status stepPlayer) {
			int flag = 0;
			std::random_device r;
			std::default_random_engine e(r());
			std::uniform_int_distribution<int> uniform_dist(0, ROW * COL);
			while (isGameOver(board)) {
				rvector<std::tuple<int, int>> positions = getAllValidMoveDecisions(board, stepPlayer);
				if (!positions.data.empty()) {
					flag = 0;
					int random = uniform_dist(e) % positions.data.size();
					int x = std::get<0>(positions.data[random]), y = std::get<1>(positions.data[random]);
					setChessPiece(board, x, y, stepPlayer);
					reverseChessPieces(board, x, y, stepPlayer);
				}
				else {
					flag += 1;
					if (flag == 2) {
						break;
					}
				}
				stepPlayer = stepPlayer == Status::AI ? Status::PLAYER : Status::AI;
			}
			return getScoreOfBoard(board);
		}

		void backup(std::shared_ptr<MCTS> bottom, Status stepPlayer, std::tuple<int, int> score) {
			int deta = std::get<0>(score) - std::get<1>(score) > 0 ? 1 : 0;
			while (true) {
				bottom->score += deta;
				bottom->visit += 1;
				if (bottom->isRoot) {
					break;
				}
				bottom = bottom->parent.lock();
				stepPlayer = stepPlayer == Status::AI ? Status::PLAYER : Status::AI;
			}
		}

		void simulate(std::shared_ptr<MCTS>& root, rvector<Status> board) {
			rvector<Status> copyBoard;
			for (int i = 0; i < 800; i++) {
				copyBoard.data.assign(board.data.begin(), board.data.end());
				std::tuple<std::shared_ptr<MCTS>, Status> expandDecision = treePolicy(copyBoard, std::ref(root), Status::AI);
				std::tuple<int, int> score = defaultPolicy(copyBoard, std::get<0>(expandDecision), std::get<1>(expandDecision));
				backup(std::get<0>(expandDecision), std::get<1>(expandDecision), score);
			}
		}

		void AIMoveChessPiece() {
			bool hasOptionalPlace = showOptionalMoveDecisions(currentPlayer);
			if(!hasOptionalPlace) {
				std::cout << "AI has no place to set piece." << std::endl;
				return;
			}
			int x, y;
			rvector<Status> copyBoard;
			copyBoard.data.assign(board.data.begin(), board.data.end());
			rvector<std::tuple<int, int>> options = getAllValidMoveDecisions(copyBoard, Status::AI);
			int threadNum = coreNum;
			if (options.data.empty()) {
				return;
			}
			else if (options.data.size() == 1) {
				x = std::get<0>(options.data[0]);
				y = std::get<1>(options.data[0]);
			}
			else {
				std::shared_ptr<MCTS> root = std::make_shared<MCTS>();
				for (std::tuple<int, int> option : options.data) {
					std::shared_ptr<MCTS> node = std::make_shared<MCTS>(option, Status::AI, std::ref(root));
					root->pushchild(std::make_shared<MCTS>(option, Status::AI, std::ref(root)));
				}
				std::thread threadPool[threadNum];
				for (int i = 0; i < threadNum; i++) {
					threadPool[i] = std::thread(&ReversiChess::simulate, this, std::ref(root), copyBoard);
				}
				for (int i = 0; i < threadNum; i++) {
					threadPool[i].join();
				}

				int index = root->indexOfBestChild();
				std::tuple<int, int> position = root->getChild(index)->position;
				x = std::get<0>(position);
				y = std::get<1>(position);
			}
			clear();
			std::cout << "AI places piece O at (" << static_cast<char>('A' + x) << ", " << y << ")" << std::endl;
			appendHistory(x, y, currentPlayer);
			setChessPiece(board, x, y, currentPlayer);
			reverseChessPieces(board, x, y, currentPlayer);
		}

		void startPlay() {
			while (!isGameOver(board)) {
				std::cout << *this;
				if (currentPlayer == Status::PLAYER) {
					playerMoveChessPiece();
				}
				else {
					std::cout << "\nWait for seconds. AI is thinking~" << std::endl;
					AIMoveChessPiece();
				}
				nextPlayer();
			}
			std::cout << "------------------------- GAME OVER -------------------------" << std::endl;
			std::cout << *this;
			std::tuple<int, int> score = getScoreOfBoard(board);
			int playerNum = std::get<0>(score), AINum = std::get<1>(score);
			if (playerNum > AINum) {
				std::cout << "Congrats! Player wins!" << std::endl;
			}
			else if (playerNum < AINum) {
				std::cout << "Good game! AI wins!" << std::endl;
			}
			else {
				std::cout << "Tie!" << std::endl;
			}
			saveHistory();
		}

		void replayByHistory(std::string fname){
			std::ifstream in(fname);
			if(in.fail()){
				std::cout << "\nInvalid Argument: " << fname << " doesn't exist!" << std::endl;
				exit(1);
			}
			for(std::string line; getline(in, line); ){
				clear();
				auto res = extract_data(line);
				if(res != std::nullopt){
					int x = res->x, y = res->y;
					if(res->player == "AI"){
						std::cout << "AI places piece O at (" << static_cast<char>('A' + x) << ", " << y << ")" << std::endl;
						setChessPiece(board, x, y, Status::AI);
						reverseChessPieces(board, x, y, Status::AI);
						std::cout << *this;

					}else{
						std::cout << "Player places piece X at (" << static_cast<char>('A' + x) << ", " << y << ")" << std::endl;
						setChessPiece(board, x, y, Status::PLAYER);
						reverseChessPieces(board, x, y, Status::PLAYER);
						std::cout << *this;
					}
					sleep(2);
				}
			}
			std::tuple<int, int> score = getScoreOfBoard(board);
			int playerNum = std::get<0>(score), AINum = std::get<1>(score);
			if (playerNum > AINum) {
				std::cout << "Congrats! Player wins!" << std::endl;
			}
			else if (playerNum < AINum) {
				std::cout << "Good game! AI wins!" << std::endl;
			}
			else {
				std::cout << "Tie!" << std::endl;
			}
		}
	};

	inline std::string boradHead(int col){
		std::string res = "";
		for (int i = 0; i < col; i++) {
			res += std::to_string(i) + " ";
		}
		return res;
	}

	// overload operator<< to output board
	std::ostream& operator<<(std::ostream& os, const ReversiChess& chess) {
		IndentStream ins(os);
		ins << indent << "  "  << underline << " " << boradHead(chess.COL) << reset << std::endl;
		rvector<char> row(chess.COL);
		for (int i = 0; i < chess.ROW; i++) {
			for (int j = 0; j < chess.COL; j++) {
				row.data[j] = (chess.board.data[i * chess.COL + j] == Status::UNOCCUPIED ? ' ' : 
					(chess.board.data[i * chess.COL + j] == Status::PLAYER ? 'X' : 'O'));
			}
			ins << static_cast<char>('A' + i) << " " << row << std::endl;
		}
		ins << unindent;
		return os;
	}
}

#endif