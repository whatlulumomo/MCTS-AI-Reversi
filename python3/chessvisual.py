import pygame, sys, random
from pygame.locals import *
from MCTS import *

BACKGROUNDCOLOR = (255, 255, 255)
BLACK = (0, 0, 0)
YELLOW = (255, 255, 22)
CELLWIDTH = 50
CELLHEIGHT = 50
PIECEWIDTH = 44
PIECEHEIGHT = 44
BOARDX = 0
BOARDY = 0
FPS = 40


# 退出
def terminate():
    pygame.quit()
    sys.exit()


# 重置棋盘
def resetBoard(board):
    for x in range(8):
        for y in range(8):
            board[x][y] = 'none'

    # Starting pieces:
    board[3][3] = 'black'
    board[3][4] = 'white'
    board[4][3] = 'white'
    board[4][4] = 'black'


# 开局时建立新棋盘
def getNewBoard():
    board = []
    for i in range(8):
        board.append(['none'] * 8)

    return board


# 是否是合法走法
def isValidMove(board, tile, xstart, ystart):
    # 如果该位置已经有棋子或者出界了，返回False
    if not isOnBoard(xstart, ystart) or board[xstart][ystart] != 'none':
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


# 是否出界
def isOnBoard(x, y):
    return x >= 0 and x <= 7 and y >= 0 and y <= 7


# 获取可落子的位置
def getValidMoves(board, tile):
    validMoves = []

    for x in range(8):
        for y in range(8):
            if isValidMove(board, tile, x, y) != False:
                validMoves.append([x, y])
    return validMoves


# 获取棋盘上黑白双方的棋子数
def getScoreOfBoard(board):
    xscore = 0
    oscore = 0
    for x in range(8):
        for y in range(8):
            if board[x][y] == 'black':
                xscore += 1
            if board[x][y] == 'white':
                oscore += 1
    return {'black': xscore, 'white': oscore}


# 谁先走
def whoGoesFirst():
    if random.randint(0, 1) == 0:
        return 'computer'
    else:
        return 'player'


# 将一个tile棋子放到(xstart, ystart)
def makeMove(board, tile, xstart, ystart):
    tilesToFlip = isValidMove(board, tile, xstart, ystart)

    if tilesToFlip == False:
        return False

    board[xstart][ystart] = tile
    for x, y in tilesToFlip:
        board[x][y] = tile
    return True


# 复制棋盘
def getBoardCopy(board):
    dupeBoard = getNewBoard()

    for x in range(8):
        for y in range(8):
            dupeBoard[x][y] = board[x][y]

    return dupeBoard


# 是否在角上
def isOnCorner(x, y):
    return (x == 0 and y == 0) or (x == 7 and y == 0) or (x == 0 and y == 7) or (x == 7 and y == 7)


# 电脑走法，AI
def getComputerMove(board, computerTile):
    # 获取所以合法走法
    possibleMoves = getValidMoves(board, computerTile)

    # 打乱所有合法走法
    random.shuffle(possibleMoves)

    # [x, y]在角上，则优先走，因为角上的不会被再次翻转
    for x, y in possibleMoves:
        if isOnCorner(x, y):
            return [x, y]

    bestScore = -1
    for x, y in possibleMoves:
        dupeBoard = getBoardCopy(board)
        makeMove(dupeBoard, computerTile, x, y)
        # 按照分数选择走法，优先选择翻转后分数最多的走法
        score = getScoreOfBoard(dupeBoard)[computerTile]
        if score > bestScore:
            bestMove = [x, y]
            bestScore = score
    return bestMove

# # 电脑走法，AI, rewrite
# def getComputerMove(board, computerTile):
#
#     possibleMoves = getValidMoves(board, computerTile) # 获取所有合法走法
#     print(possibleMoves)  # for test
#     predict = []
#     for pos in possibleMoves: # 对所有走法进行尝试，用蒙特卡洛树算法进行搜索，最后选择模拟胜率最高的一个走法
#         MCTree = MCTS(board, pos, computerTile)  # MCTree 是蒙特卡洛树的一个对象， 输入当前棋盘和打算下棋位置
#         predict.append(MCTree.evaluate())  # 返回一个（位置，胜率）
#
#     # 从预测列表中选出一个胜率最大的位置
#     return predict[0]



# 是否游戏结束
def isGameOver(board):
    score = getScoreOfBoard(board)
    if score['black'] == 0 or score['white'] == 0:
        return True

    for x in range(8):
        for y in range(8):
            if board[x][y] == 'none':
                return False
    return True


if __name__ == '__main__':
    # 初始化
    pygame.init()
    mainClock = pygame.time.Clock()

    # 加载图片
    boardImage = pygame.image.load('board.png')
    boardRect = boardImage.get_rect()
    blackImage = pygame.image.load('black.png')
    blackRect = blackImage.get_rect()
    whiteImage = pygame.image.load('white.png')
    whiteRect = whiteImage.get_rect()

    basicFont = pygame.font.SysFont(None, 48)
    gameoverStr = 'Game Over Score '

    mainBoard = getNewBoard()
    resetBoard(mainBoard)

    turn = whoGoesFirst()
    if turn == 'player':
        playerTile = 'black'
        computerTile = 'white'
    else:
        playerTile = 'white'
        computerTile = 'black'

    print(turn)

    # 设置窗口
    windowSurface = pygame.display.set_mode((boardRect.width, boardRect.height))
    pygame.display.set_caption('黑白棋')

    gameOver = False

    # 游戏主循环
    while True:
        for event in pygame.event.get():
            if event.type == QUIT:
                terminate()
            if isGameOver(mainBoard) == False and turn == 'player' and event.type == MOUSEBUTTONDOWN and event.button == 1:
                x, y = pygame.mouse.get_pos()
                col = int((x - BOARDX) / CELLWIDTH)
                row = int((y - BOARDY) / CELLHEIGHT)
                if makeMove(mainBoard, playerTile, col, row) == True:
                    if getValidMoves(mainBoard, computerTile) != []:
                        turn = 'computer'

            if event.type == KEYUP:
                if event.key == K_q:
                    turn = 'computer'

        windowSurface.fill(BACKGROUNDCOLOR)
        windowSurface.blit(boardImage, boardRect, boardRect)


        if (isGameOver(mainBoard) == False and turn == 'computer'):
            x, y = getComputerMove(mainBoard, computerTile)
            makeMove(mainBoard, computerTile, x, y)
            savex, savey = x, y

            # 玩家没有可行的走法了
            if getValidMoves(mainBoard, playerTile) != []:
                turn = 'player'

        score = getScoreOfBoard(mainBoard)
        if score['black'] == 0 or score['white'] == 0 or isGameOver(mainBoard):
            gameOver = True


        windowSurface.fill(BACKGROUNDCOLOR)
        windowSurface.blit(boardImage, boardRect, boardRect)

        for x in range(8):
            for y in range(8):
                rectDst = pygame.Rect(BOARDX + x * CELLWIDTH + 2, BOARDY + y * CELLHEIGHT + 2, PIECEWIDTH, PIECEHEIGHT)
                if mainBoard[x][y] == 'black':
                    windowSurface.blit(blackImage, rectDst, blackRect)
                elif mainBoard[x][y] == 'white':
                    windowSurface.blit(whiteImage, rectDst, whiteRect)

        if isGameOver(mainBoard) == True:
            scorePlayer = getScoreOfBoard(mainBoard)[playerTile]
            scoreComputer = getScoreOfBoard(mainBoard)[computerTile]
            if scorePlayer > scoreComputer:
                outputStr = "Win! " + str(scorePlayer) + ":" + str(scoreComputer)
            elif scorePlayer == scoreComputer:
                outputStr = "Tie. " + str(scorePlayer) + ":" + str(scoreComputer)
            else:
                outputStr = "Die. " + str(scorePlayer) + ":" + str(scoreComputer)
            text = basicFont.render(outputStr, True, BLACK, YELLOW)
            textRect = text.get_rect()
            textRect.centerx = windowSurface.get_rect().centerx
            textRect.centery = windowSurface.get_rect().centery
            windowSurface.blit(text, textRect)

        pygame.display.update()
        mainClock.tick(FPS)