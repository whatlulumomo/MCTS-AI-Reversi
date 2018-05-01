#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Qpainter"
#include "QPixmap"
#include "QLabel"
#include "QCursor"
#include "iostream"
#include "QMouseEvent"
#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>
#include <Windows.h>
#include <queue>
#include <stack>
#include <conio.h>


using namespace std;
double COMPUTER_PARA = 0.02;
double PLAYER_PARA = 0.01;

bool mousedown = false;
int mousex;
int mousey;

Board* mainBoard;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    mainBoard = new Board();
    resetBoard(mainBoard);
    ui->time->setText("5");
    turn = "computer";
    if (turn.compare("player") == 0) {
        playerTile = BLACK;
        computerTile = WHITE;
    }
    else {
        playerTile = WHITE;
        computerTile = BLACK;
    }
    btnGroup = new QButtonGroup(this);
    btnGroup->addButton(ui->AIchoice,0);
    btnGroup->addButton(ui->playerchoice,1);

    background.load(":/rsc/board.png");
    black.load(":/rsc/black.png");
    white.load(":/rsc/white.png");
    hintwhite.load(":/rsc/whitepotential.png");
    hintblack.load(":/rsc/blackpotential.png");
    hintred.load(":/rsc/redpotential.png");

    connect(ui->pushButton, SIGNAL(clicked()),this,SLOT(ClickButton()));
}

void MainWindow::start(){
    if(isGameStart == true && !gameOver){
        if(turn.compare("computer") == 0){
            while(turn.compare("computer") == 0)
            {
                cout << "Turn: " << turn << endl;
                if (turn.compare("computer") == 0 && isGameOver(mainBoard) == false&&!gameOver) {
                    Postion next = getComputerMove(mainBoard, computerTile,COMPUTER_PARA);
                    if (next.x != -1 && next.y != -1) {
                        makeMove(mainBoard, computerTile, next.x, next.y);
                        Last.x = next.x;
                        Last.y = next.y;
                        printReport();
                        cout << "AI GO: " << next.x << " " << next.y << endl;
                        ui->AIGO->setText(QString::number(next.y)+","+QString::number(next.x));
                    }
                    else {
                        if (getValidMove(mainBoard, playerTile).size() == 0) {
                            gameOver = true;
                        }
                    }
                    if (getValidMove(mainBoard, playerTile).size() != 0) {
                        turn = "player";
                    }
                    cout << "---------------------------------------" << endl;
                }
                printReport();
                //printBoard(mainBoard);
                repaint();
            }
        }
    }
}

void MainWindow::ClickButton(){
    if(isGameStart == true){
        Last.x = Last.y = -1;
        isGameStart = false;
        resetBoard(mainBoard);
        struct posnode originpos;
        originpos.x = originpos.y = 8;
        delete root;
        root = new MCTS(originpos, BLACK);
        ui->pushButton->setText("Start");
        Score score = getScoreofBoard(mainBoard);
        if (score.black == 0 || score.white == 0 || isGameOver(mainBoard)) {
            gameOver = true;
        }
        ui->wlabel->setText(QString::number(score.white,10));
        ui->blabel->setText(QString::number(score.black,10));
        repaint();
    }

    else if(isGameStart == false){
        ui->pushButton->setText("Reset");
        QString time = ui->time->text();
        this->TIME = time.toFloat();
        switch(btnGroup->checkedId())
        {
        case 0:
            turn = "computer";
            isGameStart = true;
            gameOver = false;
            break;
        case 1:
            turn = "player";
            isGameStart = true;
            gameOver = false;
            break;
        }
        if (turn.compare("player") == 0) {
            playerTile = BLACK;
            computerTile = WHITE;
        }
        else {
            playerTile = WHITE;
            computerTile = BLACK;
        }
        start();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    this->resize(600,400);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(0,0,400,400,background);

    int tile;
    if(turn.compare("computer") == 0){
        tile = computerTile;
    }
    else{
        tile = playerTile;
    }

    int markHaveDraw[8][8] = {0};
    vector <Postion> nodes = getValidMove(mainBoard,tile);

    for(int i=0;i<nodes.size();i++){
        int x = nodes[i].x;
        int y = nodes[i].y;
        markHaveDraw[x][y] = tile;
    }

    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(mainBoard->get(i,j) == WHITE){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, white);
            }else if(mainBoard->get(i,j) == BLACK){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, black);
            }

            if(markHaveDraw[i][j] == BLACK){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, hintblack);
            }
            if(markHaveDraw[i][j] == WHITE){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, hintwhite);
            }
        }
    }

    if(Last.x != -1){
        painter.drawPixmap(0+50*Last.x, 0+50*Last.y, 50, 50, hintred);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e){
    if(isGameStart == false || gameOver == true){
        return;
    }
    if (turn.compare("player") == 0 && isGameOver(mainBoard) == false && !gameOver ) {
        //cout << e->x() << " " << e->y() << endl;
        mousex = e->x()/50;
        mousey = e->y()/50;
        //cout << mousex << " " << mousey << endl;
        if (makeMove(mainBoard, playerTile, mousex, mousey) == true) {
            Last.x = mousex;
            Last.y = mousey;
            if (getValidMove(mainBoard, computerTile).size() != 0) {
                turn = "computer";
            }
        }
        else {
            cout << "Reinput: " << endl;
        }
    }
    repaint();
    //printBoard(mainBoard);

    Score score = getScoreofBoard(mainBoard);
    if (score.black == 0 || score.white == 0 || isGameOver(mainBoard)) {
        gameOver = true;
    }

    while(turn.compare("computer") == 0 && !gameOver)
    {
        cout << "Turn: " << turn << endl;
        if (turn.compare("computer") == 0 && isGameOver(mainBoard) == false&&!gameOver) {
            Postion next = getComputerMove(mainBoard, computerTile,COMPUTER_PARA);
            if (next.x != -1 && next.y != -1) {
                makeMove(mainBoard, computerTile, next.x, next.y);
                Last.x = next.x;
                Last.y = next.y;
                printReport();
                cout << "AI GO: " << next.x << " " << next.y << endl;
                ui->AIGO->setText(QString::number(next.y)+","+QString::number(next.x));
            }
            else {
                if (getValidMove(mainBoard, playerTile).size() == 0) {
                    gameOver = true;
                }
            }
            if (getValidMove(mainBoard, playerTile).size() != 0) {
                turn = "player";
            }
            cout << "---------------------------------------" << endl;
        }
        //printBoard(mainBoard);
        repaint();
    }
    score = getScoreofBoard(mainBoard);
    if (score.black == 0 || score.white == 0 || isGameOver(mainBoard)) {
        gameOver = true;
    }
    ui->wlabel->setText(QString::number(score.white,10));
    ui->blabel->setText(QString::number(score.black,10));

}

MainWindow::~MainWindow()
{
    delete ui;
}

Board* MainWindow::getNewBoard() {
    Board* board = new Board();
    return board;
}

void MainWindow::init() {
    struct posnode originpos;
    originpos.x = originpos.y = 8;
    root = new MCTS(originpos, BLACK);
}

void MainWindow::resetBoard(Board* board) {
    board->init();
    board->set(3, 3, WHITE);
    board->set(3, 4, BLACK);
    board->set(4, 3, BLACK);
    board->set(4, 4, WHITE);
}

void MainWindow::printReport(){

    for(int i=0;i<root->child.size();i++){
        cout << "[" << root->child[i]->pos.y << "," << root->child[i]->pos.x << "]  ";
        double winvalue = root->child[i]->score*1.0/root->child[i]->visit;
        double randomvalue = root->child[i]->C * sqrt(2 * log(root->child[i]->father->visit / root->child[i]->visit));
        printf("%.4f, %.4f, sum=%.4f, real=%.4f\n", winvalue,randomvalue, winvalue+randomvalue, root->child[i]->realscore);
    }
}

bool MainWindow::isOnBoard(int x, int y) {
    return x >= 0 && x <= 7 && y >= 0 && y <= 7;
}

vector<Postion> MainWindow::isValidMove(Board* board, int tile, int xstart, int ystart) {
    vector<Postion> tilesToFlip;
    if (isOnBoard(xstart, ystart) == false || board->get(xstart, ystart) != 0) {
        return tilesToFlip;
    }

    int othertile;
    board->set(xstart, ystart, tile);

    if (tile == BLACK) {
        othertile = WHITE;
    }
    else {
        othertile = BLACK;
    }


    for (int i = 0; i < 8; i++) {
        int x = xstart;
        int y = ystart;
        int xdirection = Direction[i][0];
        int ydirection = Direction[i][1];
        x += xdirection;
        y += ydirection;
        if (isOnBoard(x, y) && board->get(x, y) == othertile) {
            x += xdirection;
            y += ydirection;
            if (isOnBoard(x, y) == false) {
                continue;
            }

            while (board->get(x, y) == othertile) {
                x += xdirection;
                y += ydirection;
                if (isOnBoard(x, y) == false) {
                    break;
                }
            }

            if (isOnBoard(x, y) == false) {
                continue;
            }

            if (board->get(x, y) == tile) {
                while (true) {
                    x -= xdirection;
                    y -= ydirection;

                    if (x == xstart && y == ystart) {
                        break;
                    }
                    tilesToFlip.push_back(Postion(x, y));
                }
            }
        }
    }
    board->set(xstart, ystart, 0);
    return tilesToFlip;
}

bool MainWindow::makeMove(Board* board, int tile, int xstart, int ystart) {
    vector<Postion> tilesToFlip = isValidMove(board, tile, xstart, ystart);

    if (tilesToFlip.size()==0) {
        return false;
    }

    board->set(xstart, ystart, tile);

    while (tilesToFlip.size()) {
        Postion tilepos = tilesToFlip.back();
        tilesToFlip.pop_back();
        board->set(tilepos.x, tilepos.y, tile);
    }

    return true;
}

vector<Postion> MainWindow::getValidMove(Board* board, int tile, bool forpolicy) {
    vector<Postion> validMoves;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (isValidMove(board, tile, i, j).size() != 0) {
                if(forpolicy == true){
                    if ( i == 0 || i == 7 || j == 0 || j == 7){
                        validMoves.push_back(Postion(i, j));
                        validMoves.push_back(Postion(i, j));
                    }
                    if ((i == 0 && j == 0) || (i == 0 && j == 7) || (i == 7 && j == 0) || (i == 7 && j == 7)) {
                        validMoves.push_back(Postion(i, j));
                        validMoves.push_back(Postion(i, j));
                    }
                    if (!((i == 1 || i == 6) && (j == 1 || j == 6))) {
                        validMoves.push_back(Postion(i, j));

                    }
                }
                validMoves.push_back(Postion(i, j));
            }
        }
    }

    return validMoves;
}

void MainWindow::printBoard(Board* board) {
    static int id = 0;
    cout << id++ << ":" << endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            string mark;
            if (board->get(j, i) == BLACK) {
                mark = "b";
            }
            else if (board->get(j, i) == WHITE) {
                mark = "w";
            }
            else {
                mark = "_";
            }
            cout << mark << " ";
        }
        cout << endl;
    }
}

bool MainWindow::isGameOver(Board* board) {
    int black = 0;
    int white = 0;
    int flag = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board->get(i, j) == BLACK) {
                black++;
            }
            else if (board->get(i, j) == WHITE) {
                white++;
            }
            else {
                flag++;
            }
        }
    }

    if (black == 0 || white == 0) {
        return true;
    }

    if (flag != 0) {
        return false;
    }

    return true;
}

Board* MainWindow::getBoardCopy(Board* board) {
    Board* dupBoard = new Board();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            dupBoard->set(i, j, board->get(i, j));
        }
    }
    return dupBoard;
}

bool MainWindow::isOnCorner(int x, int y) {
    return (x == 0 && y == 0) || (x == 7 && y == 0) || (x == 0 && y == 7) || (x == 7 && y == 7);
}

int MainWindow::getEnemyTile(int tile) {
    if (tile == WHITE) {
        return BLACK;
    }
    else
        return WHITE;
}

Result MainWindow::treePolicy(Board* board, MCTS* state, int tile) {
    MCTS* expand = nullptr;
    queue<MCTS*> q;
    q.push(state);
    while (!q.empty()) {
        MCTS* n = q.front();
        q.pop();
        if (n->expandable == false) {
            for (int i = 0; i < n->child.size(); i++) {
                q.push(n->child[i]);
            }
        }
        else {
            expand = n;
            break;
        }
    }
    if (expand == nullptr) {
        expand = state->child[state->bestChild()];
        makeMove(board, tile, expand->pos.x, expand->pos.y);
        tile = getEnemyTile(tile);
    }
    else {
        expand->expandable = false;
        stack<MCTS*> s;
        MCTS* node = expand;
        while (!node->root) {
            s.push(node);
            node = node->father;
        }
        while (!s.empty()) {
            node = s.top();
            makeMove(board, tile, node->pos.x, node->pos.y);
            s.pop();
            tile = getEnemyTile(tile);
        }
        vector<Postion>nodes = getValidMove(board, tile);
        for (int i = 0; i < nodes.size(); i++) {
            MCTS* mnode = new MCTS(nodes[i], getEnemyTile(expand->mytile));
            mnode->father = expand;
            expand->child.push_back(mnode);
        }
    }

    return ResultNode(expand, tile);
}

Score MainWindow::getScoreofBoard(Board* board) {
    int white = 0;
    int black = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board->get(i, j) == WHITE) {
                white++;
            }
            if (board->get(i, j) == BLACK) {
                black++;
            }
        }
    }
    return Score(white, black);
}

Score MainWindow::defaultPolicy(Board* board, MCTS* state, int tile) {
    int flag = 0;
    srand((unsigned)time(NULL));
    while (!isGameOver(board)) {
        vector<Postion> nodes = getValidMove(board, tile, true);
        //cout << nodes.size() << endl;
        if (nodes.size() >= 1) {
            flag = 0;
            int random = rand() % (nodes.size());
            int x = nodes[random].x;
            int y = nodes[random].y;
            makeMove(board, tile, x, y);
        }
        else {
            flag += 1;
            if (flag == 2) {
                //cout << "oooooooooooooooooo" << endl;
                return getScoreofBoard(board);
            }
        }
        tile = getEnemyTile(tile);
    }
    return getScoreofBoard(board);
}

void MainWindow::backup(Board* board, MCTS* bottom, int tile, int computerTile, Score score) {
    tile = getEnemyTile(tile);
    int manTile = getEnemyTile(computerTile);
    int deta = score.get(computerTile) - score.get(manTile);

    if (deta >= 0) {
        deta = 1;
    }
    else if (deta < 0) {
        deta = 0;
    }

    MCTS* node = bottom;
    while (true) {
        node->score += deta;
        node->visit += 1;

        // test
        //if (node->child.size() > 0) {
        //	int tmp = -100000;
        //	for (int i = 0; i < node->child.size(); i++) {
        //		if (node->child[i]->score > tmp) {
        //			tmp = node->child[i]->score;
        //		}
        //	}
        //	node->score = tmp;
        //}

        if (node->root == true) {
            break;
        }

        node = node->father;
        tile = getEnemyTile(tile);
    }
}

Postion MainWindow::uctSearch(MCTS* root, Board* board, int computertile, int DEPTH, double TIME) {
    vector<Postion> nodes = getValidMove(board, computertile);
    if (nodes.size() == 0) {
        return Postion(-1, -1);;
    }
    else if (nodes.size() == 1) {
        return nodes[0];
    }
    for (int i = 0; i < nodes.size(); i++) {
        int x = nodes[i].x;
        int y = nodes[i].y;
        //if (isOnCorner(x, y) == true) {
        //	return Postion(x, y);
        //}
        MCTS* mnodes = new MCTS(nodes[i],computertile );
        mnodes->father = root;
        root->child.push_back(mnodes);
        root->visit += mnodes->visit;
    }

    double starttime = GetTickCount();
    int playtime = 0;
    while (true) {
        playtime++;
        Board* dupBoard = getBoardCopy(board);
        Result result = treePolicy(dupBoard, root,computertile);
        Score score = defaultPolicy(dupBoard, result.state, result.tile);
        backup(dupBoard, result.state, result.tile, computertile, score);
        delete dupBoard;
        double endtime = GetTickCount();
        if (endtime - starttime > TIME * 1000) {
            cout << playtime << endl;
            break;
        }
    }
    //cout << "Rrootvisit  " << root->visit << endl;
    //for (int i = 0; i < root->child.size(); i++) {
    //	cout << root->child[i]->pos.x << "," << root->child[i]->pos.y << ":" << root->child[i]->visit << " " << root->child[i]->score << endl;
    //}
    int index = root->bestChild();
//    cout << "final " << root->child[index]->score*1.0 / root->child[index]->visit +
//        root->child[index]->C * sqrt(2 * log(root->child[index]->father->visit / root->child[index]->visit))  << endl;
    return root->child[index]->pos;

}

Postion MainWindow::getComputerMove(Board* board, int computertile, int newc=0.02) {
    Board* dupBoard = getBoardCopy(board);
    root->C = newc;
    root->rootClear();
    return uctSearch(root, dupBoard, computertile, DEPTH, TIME);
}


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
    double sum = this->score*1.0 / this->visit + this->C * sqrt(2 * log(this->father->visit / this->visit));
    if (((pos.x == 1 || pos.x == 6) && (pos.y == 0 || pos.y == 7)) || (pos.x == 0 || pos.x == 7) && (pos.y == 1 || pos.y ==6)) {
        if(pos.x == 1 && pos.y == 0){
            if(this->mytile != mainBoard->get(0,0)){
                sum *= 0.95;
            }
        }
        if(pos.x == 6 && pos.y == 0){
            if(this->mytile != mainBoard->get(7,0)){
                sum *= 0.95;
            }
        }
        if(pos.x == 0 && pos.y == 1){
            if(this->mytile != mainBoard->get(0,0)){
                sum *= 0.95;
            }
        }
        if(pos.x == 7 && pos.y == 1){
            if(this->mytile != mainBoard->get(7,0)){
                sum *= 0.95;
            }
        }
        if(pos.x == 7 && pos.y == 6){
            if(this->mytile != mainBoard->get(7,7)){
                sum *= 0.95;
            }
        }
        if(pos.x == 0 && pos.y == 6){
            if(this->mytile != mainBoard->get(0,7)){
                sum *= 0.95;
            }
        }
        if(pos.x == 1 && pos.y == 7){
            if(this->mytile != mainBoard->get(0,7)){
                sum *= 0.95;
            }
        }
        if(pos.x == 6 && pos.y == 7){
            if(this->mytile != mainBoard->get(7,7)){
                sum *= 0.95;
            }
        }

    }

    else if (((pos.x == 1 ) && (pos.y == 1 || pos.y == 6)) || (pos.x == 6) && (pos.y == 1 || pos.y == 6)) {
        if(pos.x == 1 && pos.y == 1){
            if(this->mytile != mainBoard->get(0,0)){
                sum *= 0.87;
            }
        }
        if(pos.x == 6 && pos.y == 1){
            if(this->mytile != mainBoard->get(7,0)){
                sum *= 0.87;
            }
        }
        if(pos.x == 1 && pos.y == 6){
            if(this->mytile != mainBoard->get(0,7)){
                sum *= 0.87;
            }
        }
        if(pos.x == 6 && pos.y == 6){
            if(this->mytile != mainBoard->get(7,7)){
                sum *= 0.87;
            }
        }
    }
    else if((pos.x == 0&&pos.y == 0) ||(pos.x == 7&&pos.y == 0)||(pos.x == 0&&pos.y == 7)||(pos.x == 7&&pos.y == 7)){
        sum *= 1.15;
    }
    else if(pos.x == 0 || pos.x == 7 || pos.y == 0 || pos.y == 7){
        sum *= 1.1;
    }
    this->realscore = sum;
    return sum;
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
