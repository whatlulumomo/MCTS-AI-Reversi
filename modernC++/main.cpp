#include "ReversiChess.h"
#include <string>

using namespace mpcs51045;

int main(int argc, char *argv[]) {
	auto chess = std::make_unique<ReversiChess>();
	if(argc == 1){
		chess->startPlay();
	}else{
		std::string fname(argv[1]);
		chess->replayByHistory(fname);
	}
	return 0;
}