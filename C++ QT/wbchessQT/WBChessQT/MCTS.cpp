#include "MCTS.h"
#include <math.h>
#include <conio.h>

void MCTS::rootClear() {
	while (child.empty() == false) {
		MCTS* ele = child.back();
		child.pop_back();
		delete ele;
	}
	child.clear();
	visit = 0;
	bestchild = nullptr;
	expandable = false;
	root = true;
}

MCTS::MCTS(Postion pos, int mytile) {
	//child = nullptr;
	bestchild = nullptr;
	visit = 0;
	score = 0;
	expandable = true;
	father = nullptr;
	root = false;
	this->pos = pos;
	this->mytile = mytile;
}


double MCTS::calculate() {
	//cout << "------------------------------" << endl;
	//visit = 1;
	//for (int i = 0; i < child.size(); i++) {
	//	//cout << "c-" << i << ": " << child[i]->visit << endl;
	//	visit += child[i]->visit;
	//}
	//cout << "visit: " << visit << endl;
	//cout << "------------------------------" << endl;
	cout << "C:" << this->C << endl;
	std::cout << "x:" << pos.x << ",  y:" << pos.y << ": " << this->score*1.0 / this->visit << "  " << sqrt(2 * log(this->father->visit / this->visit)) ;
	cout << "   sum : " << this->score*1.0 / this->visit + this->C * sqrt(2 * log(this->father->visit / this->visit)) << endl;
	return this->score*1.0 / this->visit + this->C * sqrt(2 * log(this->father->visit / this->visit));
}


int MCTS::bestChild() {
	double max = child[0]->calculate();
	int best = 0;
	for (int i = 1; i < child.size(); i++) {
		double tmp = child[i]->calculate();
		if (tmp > max) {
			max = tmp;
			best = i;
		}
	}
	return best;
}