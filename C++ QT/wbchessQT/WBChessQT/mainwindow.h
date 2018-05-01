#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<stdio.h>
#include<string>
#include<iostream>
#include<vector>
#include"QMutex"
#include"QButtonGroup"

using namespace std;
#define WHITE  1
#define BLACK  2

typedef struct posnode {
    int x;
    int y;
    posnode(int x, int y) {
        this->x = x;
        this->y = y;
    }
    posnode() {
        x = y = -1;
    }
}Postion;

class MCTS {
public:

    vector<MCTS*> child;
    MCTS* bestchild = nullptr;
    int visit;
    double score;
    bool expandable = true;
    MCTS* father = nullptr;
    bool root = false;
    Postion pos;
    int mytile;
    double C = 0.02;
    float realscore;
    MCTS(Postion pos, int mytile);
    void rootClear();
    double calculate();
    int bestChild();
};





class Board {
    int ele[8][8] = { 0 };
public:
    int get(int i, int j) {
        return ele[i][j];
    }

    void set(int i, int j, int val) {
        ele[i][j] = val;
    }

    void init() {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                ele[i][j] = 0;
            }
        }
    }
};



typedef struct ResultNode {
    MCTS* state;
    int tile;
    ResultNode(MCTS* state, int tile) {
        this->state = state;
        this->tile = tile;
    }
}Result;


typedef struct ScoreNode {
    int white;
    int black;
    ScoreNode(int white, int black) {
        this->white = white;
        this->black = black;
    }
    int get(int tile) {
        if (tile == WHITE) {
            return white;
        }
        else {
            return black;
        }
    }
}Score ;




namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    MCTS* root;
    double TIME = 1;
    int DEPTH = 5;
    bool gameOver = false;
    int playerTile, computerTile;
    QMutex mutex;
    string turn;
    Postion Last;


    int Direction[8][2] = {
        {0,1},{ 1,1 },{ 1,0 },{ 1,-1 },{ 0,-1 },{ -1,-1 },{ -1,0 },{-1,1}
    };
    QButtonGroup *btnGroup;
    bool isGameStart = false;

    QPixmap background;
    QPixmap white,black,hintwhite,hintblack,hintred;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void DrawBoard(int i);
    void init();
    Board* getNewBoard();
    Board* getBoardCopy(Board* board);
    void resetBoard(Board* board);
    bool makeMove(Board* mainboard, int playerTile, int col, int row);
    vector<Postion> isValidMove(Board* board, int tile, int xstart, int ystart);
    vector<Postion> getValidMove(Board* board, int tile,  bool forpolicy=false);
    bool isOnBoard(int x, int y);
    void printBoard(Board* board);
    bool isGameOver(Board* board);
    struct posnode getComputerMove(Board* board, int computertile);
    Postion getComputerMove(Board* board, int computertile, int newc);
    Postion uctSearch(MCTS* root, Board* board, int computertile, int DEPTH, double TIME);
    Score defaultPolicy(Board* board, MCTS* state, int tile);
    Score getScoreofBoard(Board* board);
    Result treePolicy(Board* board, MCTS* state, int tile);
    int getEnemyTile(int tile);
    bool isOnCorner(int x, int y);
    //Board* getBoardCopy(Board* board);
    void backup(Board* board, MCTS* bottom, int tile, int computerTile, Score score);
    void start();
    void printReport();


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *e);       //--鼠标按下事件

private slots:
    void ClickButton();



};

#endif // MAINWINDOW_H
