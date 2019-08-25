class AI:
    title = None # black or white
    board = None

    MyActionList = []
    EnemyActionList = []

    '''
    # Initialization
    '''
    def __init__(self,board,title):
        self.board = board
        self.title = title

    def Updateboard(self,x,y,title):
        self.board[x][y] = title
        if title != self.title:
            self.UpdateActionList()


    # def UpdateActionList(self):
    #     self.MyActionList = []
    #     self.EnemyActionList = []
    #
    #     for i in range(8):
    #         for j in range(8):
    #             if self.board[i][j] == None:  # empty place
    #                 pass
    #
    # def testVertical(self,x,y,title):
    #     uplist = {(x,i) for i in range(0,y-2)}
    #     downlist = {(x,i) for i in range(y+2,8)}
    #
    # def testHorizontal(self,x,y,title):
    #     leftlist = {(i,y) for i in range(0,x-2)}
    #     rightlist = {(i,y) for i in range(x+2,8)}
    #     print(leftlist)
    #     print(rightlist)
    #
    # def testDiagnal(self,x,y,title):
    #     pos = []
    #     for i in range(2,y):
    #         while x+i<8 and y+i<8 :
    # 是否是合法走法

    # 是否出界
    def isOnBoard(x, y):
        return x >= 0 and x <= 7 and y >= 0 and y <= 7

    def isValidMove(self, tile, xstart, ystart):
        # 如果该位置已经有棋子或者出界了，返回False
        if not self.isOnBoard(xstart, ystart) or self.board[xstart][ystart] != 'none':
            return False

        # 临时将tile 放到指定的位置
            self.board[xstart][ystart] = tile

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
            if isOnBoard(x, y) and board[x][y] == otherTile:
                x += xdirection
                y += ydirection
                if not isOnBoard(x, y):
                    continue
                # 一直走到出界或不是对方棋子的位置
                while board[x][y] == otherTile:
                    x += xdirection
                    y += ydirection
                    if not isOnBoard(x, y):
                        break
                # 出界了，则没有棋子要翻转OXXXXX
                if not isOnBoard(x, y):
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

    # 获取可落子的位置
    def getValidMoves(self, tile):
        validMoves = []

        for x in range(8):
            for y in range(8):
                if self.isValidMove(tile, x, y) != False:
                    validMoves.append([x, y])

        return validMoves







s = AI(1,2)
s.testHorizontal(4,2,3)

