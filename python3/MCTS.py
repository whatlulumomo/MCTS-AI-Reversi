import random, math

class MNode:
    def __init__(self, pos, mytile):
        self.child = []
        self.maxchild = 0
        self.win = 0
        self.total = 1
        self.pos = pos
        self.C = 2 # 选择公式的常数参数

        self.tile = mytile
        if self.tile == "black":
            self.enemytile = "white"
        if self.tile == "white":
            self.enemytile = "black"

    def Possibility_to_Win(self):
        return self.win/self.total

class MCTS(MNode):

    def __init__(self, board, pos, mytile):
        MNode.__init__(self, pos, mytile) # 父类初始化
        self.board = self.getBoardCopy(board)
        self.makeMove(self.board, mytile, pos[0], pos[1])
        optionalmove = self.getValidMoves(self.board, mytile)
        self.maxchild = len(optionalmove)
        self.child = []

    def evaluate(self):
        for time in range(10):
            testboard = self.getBoardCopy(self.board)
            while self.isGameOver(testboard) == False:
                pass





    def select(self):
        '''
        这个过程的第一步叫选择（Selection）。从根节点往下走，每次都选一个
        “最值得看的子节点”（具体规则稍后说），直到来到一个“存在未扩展的子节
        点”的节点，如图中的 3/3 节点。什么叫做“存在未扩展的子节点”，其实就
        是指这个局面存在未走过的后续着法。
        :return: 
        '''

        root = self
        # 寻找下一个节点，这个节点拥有尚未走过的走法
        while( len(root.child) == root.maxchild ):
            nextnode = None
            maxscore = -1
            for node in self.child:
                # 根据选择公式进行计算，最后选择分数最高的节点作为子节点
                score = node.Possibility_to_Win() + self.C * math.sqrt( math.log(self.total) / node.total )
                if score > maxscore:
                    score = maxscore
                    nextnode = node
            root = nextnode

        # 找到节点并返回
        return root




    def expansion(self):
        '''
        第二步叫扩展（Expansion），我们给这个节点加上一个 0/0 子节点，对应之前
        所说的“未扩展的子节点”，就是还没有试过的一个着法。
        :return: 
        '''

        pass

    def simulation(self):
        '''
        第三步是模拟（Simluation）。从上面这个没有试过的着法开始，用快速走子策略（Rollout policy）
        走到底，得到一个胜负结果。按照普遍的观点，快速走子策略适合选择一个棋力很弱但走子很快的策略。
        因为如果这个策略走得慢（比如用 AlphaGo 的策略网络走棋），虽然棋力会更强，结果会更准确，但由
        于耗时多了，在单位时间内的模拟次数就少了，所以不一定会棋力更强，有可能会更弱。这也是为什么我
        们一般只模拟一次，因为如果模拟多次，虽然更准确，但更慢。
        :return: 
        '''

        pass

    def backpropagation(self):
        '''
        把模拟的结果加到它的所有父节点上。例如第三步模拟的结果是 0/1（代表黑棋失败），那么就把这个节点的
        所有父节点加上 0/1。
        :return: 
        '''
        pass

    def rollout_policy(self, child):
        return child[random.randint(0,len(child)-1)]

    # 获取可落子的位置
    def getValidMoves(self, board, tile):
        validMoves = []

        for x in range(8):
            for y in range(8):
                if self.isValidMove(board, tile, x, y) != False:
                    validMoves.append([x, y])
        return validMoves


    # 开局时建立新棋盘
    def getNewBoard(self):
        board = []
        for i in range(8):
            board.append(['none'] * 8)

        return board

    # 复制棋盘
    def getBoardCopy(self, board):
        dupeBoard = self.getNewBoard()

        for x in range(8):
            for y in range(8):
                dupeBoard[x][y] = board[x][y]

        return dupeBoard

    # 将一个tile棋子放到(xstart, ystart)
    def makeMove(self, board, tile, xstart, ystart):
        tilesToFlip = self.isValidMove(board, tile, xstart, ystart)

        if tilesToFlip == False:
            return False

        board[xstart][ystart] = tile
        for x, y in tilesToFlip:
            board[x][y] = tile
        return True

    # 是否出界
    def isOnBoard(self, x, y):
        return x >= 0 and x <= 7 and y >= 0 and y <=7

    # 是否游戏结束
    def isGameOver(self, board):
        for x in range(8):
            for y in range(8):
                if board[x][y] == 'none':
                    return False
        return True



            # 将一个tile棋子放到(xstart, ystart)
    def makeMove(self, board, tile, xstart, ystart):
        tilesToFlip = self.isValidMove(board, tile, xstart, ystart)

        if tilesToFlip == False:
            return False

        board[xstart][ystart] = tile
        for x, y in tilesToFlip:
            board[x][y] = tile
        return True

    # 是否是合法走法
    def isValidMove(self, board, tile, xstart, ystart):
        # 如果该位置已经有棋子或者出界了，返回False
        if not self.isOnBoard(xstart, ystart) or board[xstart][ystart] != 'none':
            return False

        # 临时将tile 放到指定的位置
        board[xstart][ystart] = tile

        if tile == 'black':
            otherTile = 'white'
        else:
            otherTile = 'black'

        # 要被翻转的棋子
        tilesToFlip = []
        for xdirection, ydirection in [[0, 1], [1, 1], [1, 0], [1, -1], [0, -1], [-1, -1], [-1, 0], [-1, 1]]:
            x, y = xstart, ystart
            x += xdirection
            y += ydirection
            if self.isOnBoard(x, y) and board[x][y] == otherTile:
                x += xdirection
                y += ydirection
                if not self.isOnBoard(x, y):
                    continue
                # 一直走到出界或不是对方棋子的位置
                while board[x][y] == otherTile:
                    x += xdirection
                    y += ydirection
                    if not self.isOnBoard(x, y):
                        break
                # 出界了，则没有棋子要翻转OXXXXX
                if not self.isOnBoard(x, y):
                    continue
                # 是自己的棋子OXXXXXXO
                if board[x][y] == tile:
                    while True:
                        x -= xdirection
                        y -= ydirection
                        # 回到了起点则结束
                        if x == xstart and y == ystart:
                            break
                        # 需要翻转的棋子
                        tilesToFlip.append([x, y])

        # 将前面临时放上的棋子去掉，即还原棋盘
        board[xstart][ystart] = 'none'  # restore the empty space

        # 没有要被翻转的棋子，则走法非法。翻转棋的规则。
        if len(tilesToFlip) == 0:  # If no tiles were flipped, this is not a valid move.
            return False
        return tilesToFlip

