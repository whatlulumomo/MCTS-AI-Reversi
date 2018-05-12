# MCTS-AI-Chess
基于蒙特卡洛的黑白棋AI（C++ QT 和 python）
I provide 2 version of AI of reverse chess of python and QT(C++), basing on MCTS algorithm. Actually, the version of C++ performs better than python by virtue of high efficiency.

### 1.	Project Introduction
##### 1.1开发环境
我们写了python和c++两个版本的黑白棋。
	Python: 3.6
	C++: g++, QT
##### 1.2工作分配
	康自蓉：MCTS算法实现
	葛帅琦：python，QT图像显示，MCTS算法改进。C++重写黑白棋加速。
### 2.	Technical Details
内容包括：
*(1)	工程实践当中所用到的理论知识阐述
*(2)	具体的算法，请用文字、示意图或者是伪代码等形式进行描述（不要贴大段的代码）
*(3)	程序开发中重要的技术细节，比如用到了哪些重要的函数？这些函数来自于哪些基本库？功能是什么？自己编写了哪些重要的功能函数？等等

##### 2.1理论知识
黑白棋AI使用的主体思想是蒙特卡洛搜索树。蒙特卡洛树是基于统计和随机的广度搜索的思想。算法主要分为四个部分。
* 首先是选择，从蒙特卡洛树的根节点开始，寻找可扩展的节点。一个节点是可扩展的需要满足两个条件，即该节点代表非终结状态，且该节点有未被访问过的子节点。
* 然后是扩展，扩展即是根据当前状态下可采取的动作，将多个子节点加入MCT（Monte-Carlo Tree，蒙特卡洛树）中。
* 然后是模拟，模拟就是从第一步中选择的节点开始，随机的采取行动，直至完成整场决策。
* 最后是回溯，即将第三步模拟的结果的信息添加到MCT中从根节点到第一步选择节点的path上的所有节点信息中，作为之后的统计信息和判断依据。



##### 2.2具体算法
具体算法用伪代码加文字描述如下；
##### 2.2.1数据结构
工程采用面向对象的思想，给MCT的每个节点构造了一个对象，结构如下；
```c++
class MCTS {
public:

    vector<MCTS*> child;         //child nodes of this node
    MCTS* bestchild = nullptr;  //best child of this node
    int visit;                     //visit times of this node
    double score;                 //score of this node
    bool expandable = true;     //whether this node is expandable, initialize to be true
    MCTS* father = nullptr;     //father node of this node in MCT
    bool root = false;           //whether this node is dummy root
    Postion pos;                  //the corresponding position on the board
    int mytile;                   //the corresponding chess tile on the board
    double C = 0.02;            //arguments used when determine the best child, which can be adjusted
float realscore;

    MCTS(Postion pos, int mytile);  //constructor
    void rootClear();  //function to initialize the dummy root
    double calculate(); //function to calculate for determining best child
    int bestChild();    //function to get the best child
};
```

##### 2.2.2 选择+扩展：treepolicy函数
在treepolicy函数中完成了对当前MCT的遍历，此处选择做了细微的代码实现上的改变。对节点可扩展性的判定是基于MCT节点中的expandable变量，除根节点外其他节点的该成员变量初始化为true，在代码实现中在treepolicy函数中被初次访问的节点expandable变量会设置为false。
>伪代码如下；
```C++
treepolicy{
采用层次遍历法，利用队列数据结构，寻找MCT中可扩展节点；
if can’t find expandable tree:
选择当前状态的best child node作为选定节点
Else:
    选择找到的第一个可扩展节点作为选定节点
利用堆栈数据结构，完成MCT上从根节点到选定节点的path上所有节点对应的activity;
将选定节点的所有valid子节点加入到MCT中完成扩展；
将选定节点的expandable设置为false；
返回选定节点以及下一手的执棋方； }
```

##### 2.2.3模拟：defaultpolicy
>伪代码如下；
此处做了一个优化，最原始的defaultpolicy中，所有选定子之后的落子都是纯随机的。优化过后，选定子之后的落子在随机的基础上加了权重，使得选择角上的子的概率选择边上的子的概率>选择中间的子的概率。优化的方法在getValidMove函数中，通过改变加入同一个位置的子的个数来改变该落子被随机到的权重。
```C++
defaultpolicy{
while(游戏未结束){
    得到当前状态下valid走子方法；
    if 走子方法个数 >= 1：
        随机选择一种可行的走子方法并执行；
   else:
        更换执棋方；
        如果更换执棋方的次数超过1次：
            说明当前状态下双方都无法落子，返回当前棋面的信息；
}
返回当前棋面的信息； }
```
##### 2.2.4回溯：backup函数
>伪代码如下；
```c++
backup{
(根据defaultpolicy返回的结果)
if 棋面上AI方的子不少于对手方的子:
score = 1;
else :
score = 0;
根据MCT node的father成员变量，自底向下访问选定子到root的path上的所有节点 node：
node->score += score;
Node->visit += 1; }
```

##### 2.2.5 蒙特卡洛搜索树：uctSearch函数
>伪代码如下；
```c++
uctSearch{
获得当前状态所有可能走法；
处理特殊情况，即可能走法数为0或1的情况；
构造MCT：
    获得dummy root并设置成员变量；
将所有可能走法作为dummy root的子节点；
while(未超过人为规定时间限制)：
获得duplicate board，接下来的操作在duplicate board上进行；
调用treepolicy获得选定子，并在选定子可扩展的情况下扩展MCT；
调用defaultpolicy随机完成整盘棋局，并返回棋面信息；
调用backup根据棋面信息更新部分棋子的统计信息；
    根据棋子的统计信息，返回root的best child
}
```

##### 2.2.6 蒙特卡洛快速出子随机函数：getValidMove函数
>伪代码如下：
```c++
getValidMove{
获得当前状态所有可能走法；
将可能的走法按一定权重存入vector
如果该点是四个边角，则重复存入vector 3次
如果该点是棋盘上非边角，但为边的位置，往vector中存入两次
其余位置只存入一次
}
```

##### 2.2.7 bestChild的估值函数
    怎么选择bestchild节点？和从前一样：如果轮到黑棋走，就选对于黑棋有利的；如果轮到白棋走，就选对于黑棋最不利的。但不能太贪心，不能每次都只选择“最有利的/最不利的”，因为这会意味着搜索树的广度不够，容易忽略实际更好的选择。
因此，最简单有效的选择公式是这样的：

C可以经过多次测试选取一个合适的值。

2.3重要函数
程序主要可以分为三部分；
第一部分为Qt图形界面，以及监听回调函数；
```C++
void DrawBoard(int i); //绘制棋盘
void paintEvent(QPaintEvent *event); //更新棋盘内容
void mousePressEvent(QMouseEvent *e);       //鼠标按下事件
```
第二部分为黑白棋的基本逻辑，例如判断棋局是否结束的函数；统计棋面上双方棋子个数的函数；决定或更换当前执棋方的函数等。
//创建新的棋盘
```c++
Board* getNewBoard();
//拷贝棋盘
Board* getBoardCopy(Board* board);
//重置棋盘
void resetBoard(Board* board);
//下一步棋
bool makeMove(Board* mainboard, int playerTile, int col, int row);
//判定该步是否有效
vector<Postion> isValidMove(Board* board, int tile, int xstart, int ystart);
//获取有效的行为集合
vector<Postion> getValidMove(Board* board, int tile,  bool forpolicy=false);
//判定子是否在棋盘上
bool isOnBoard(int x, int y);
//命令行打印棋盘
void printBoard(Board* board);
//判定游戏是否结束
bool isGameOver(Board* board);
//获取AI动作
Postion getComputerMove(Board* board, int computertile, int newc);
//获取当前比分
Score getScoreofBoard(Board* board);
```

第三部分为MCTS的有关函数，是程序的核心部分，主要包括2.2节中给出了伪代码的四个函数，即uctSearch函数及其调用的三个函数。
//进行蒙特卡洛搜索
```c++
Postion uctSearch(MCTS* root, Board* board, int computertile, int DEPTH, double TIME);
//快速出子
Score defaultPolicy(Board* board, MCTS* state, int tile);
//拓展
Result treePolicy(Board* board, MCTS* state, int tile);
//回溯
void backup(Board* board, MCTS* bottom, int tile, int computerTile, Score score);
```

第四部分是MCTS是蒙特卡洛算法的改进，一是尽量减小MCTS快速出子阶段的纯随机性，但是又不要过重增加计算负担。二是对于一些边角特定位置的估值函数进行略微调整。
```c++
// 判定该步是否有效
vector<Postion> isValidMove(Board* board, int tile, int xstart, int ystart);
// 获取有效动作的集合，根据位置按不同的权重
vector<Postion> getValidMove(Board* board, int tile,  bool forpolicy=false);
int bestChild();                //function to get the best child
```

### 3.	Experiment Results
用图文并茂的形式给出实验结果，如系统界面、操作说明、运行结果等，并对实验结果进行总结和说明。
启动界面：


先手问题：


AI 计算时间上限：


开始与重启按钮，以及比分对比


行动力提示：


最近一步提示：


正常对局：

### References:
>* 28 天自制你的 AlphaGo（五）：蒙特卡洛树搜索（MCTS）基础。
* 详解AlphaGo背后的力量：蒙特卡洛树搜索入门指南 —— 机器之心
*《双人博弈问题中的蒙特卡洛树搜索算法的改进》 季辉 ， 丁泽军

