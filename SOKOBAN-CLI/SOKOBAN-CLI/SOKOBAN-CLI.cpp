#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <fstream>
#include <iostream>

#define ESC                 27
#define FUNCTIONKEY_CHAR    0
#define ARROWKEY_CHAR       224
#define BOARD_ROW           9
#define BOARD_COL           8

enum KEYS
{
    KEY_HOME = 71,
    KEY_UP = 72,
    KEY_PGUP = 73,
    KEY_UNKNOWN01 = 74,
    KEY_LEFT = 75,
    KEY_CENTER = 76,
    KEY_RIGHT = 77,
    KEY_UNKNOWN02 = 78,
    KEY_END = 79,
    KEY_DOWN = 80,
    KEY_PGDN = 81
};

struct KPoint
{
    int x;
    int y;

    KPoint()
    {
        x = 0;
        y = 0;
    }
    KPoint(int x_, int y_)
    {
        x = x_;
        y = y_;
    }
    KPoint Add(const KPoint p)
    {
        KPoint t = *this;
        t.x += p.x;
        t.y += p.y;
        return t;
    }
};

enum ECell
{
    CELL_EMPTY = 0,
    CELL_WALL = 1,
    CELL_GOAL = 2,
    CELL_BOX = 3,
    CELL_PLAYER = 4,
};

struct KCell
{
    int fixed; // CELL_WALL or CELL_GOAL
    int movable; // CELL_BOX or CELL_PLAYER

    KCell() {}
    KCell(int f, int m)
    {
        fixed = f;
        movable = m;
    }
};

void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

void gotoxy(int x, int y)
{
    COORD coord;

    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

using DrawBoardCallback = void (*)(KCell board[BOARD_ROW][BOARD_COL]);

void DrawBoard(KCell board[BOARD_ROW][BOARD_COL])
{
    for (int row = 0; row < BOARD_ROW; ++row)
    {
        for (int col = 0; col < BOARD_COL; ++col)
        {

            if (board[row][col].fixed == CELL_WALL)
            {
                gotoxy(col, row);
                printf("#");
            }
            else if (board[row][col].fixed == CELL_GOAL)
            {
                gotoxy(col, row);
                printf("0");
            }
            else if (board[row][col].fixed == 0)
            {
                gotoxy(col, row);
                printf(".");
            }

            if (board[row][col].movable == CELL_BOX)
            {
                gotoxy(col, row);
                if (board[row][col].fixed == CELL_GOAL)
                {
                    printf("*");
                }
                else
                {
                    printf("$");
                }
            }
            else if (board[row][col].movable == CELL_PLAYER)
            {
                gotoxy(col, row);
                if (board[row][col].fixed == CELL_GOAL)
                {
                    printf("+");
                }
                else {
                    printf("P");
                }
            }
        }
    }
}

class KBoard
{
private:
    KCell  m_board[BOARD_ROW][BOARD_COL];

public:
    KBoard() {}
    void InitializeBoard(KCell board[BOARD_ROW][BOARD_COL])
    {
        memcpy_s(m_board, sizeof(KCell) * BOARD_ROW * BOARD_COL
            , board, sizeof(KCell) * BOARD_ROW * BOARD_COL);
    }
    int GetFixedValue(KPoint p)
    {
        return m_board[p.y - 1][p.x - 1].fixed;
    }
    int GetMovableValue(KPoint p)
    {
        return m_board[p.y - 1][p.x - 1].movable;
    }
    void SetMovableValue(KPoint p, int m)
    {
        m_board[p.y - 1][p.x - 1].movable = m;
    }
    void DrawBoard(DrawBoardCallback callback)
    {
        callback(m_board);
    }
};

class KGameWorld
{
private:
    KPoint m_playerPos = { 3,3 };
    KCell  m_board[BOARD_ROW][BOARD_COL];
    KBoard m_kBoard;
    KPoint m_offsetVector;
    int m_lastKey = 0;
    int turnsTaken = 0;
    int totalGoals = 0;
public:
    KGameWorld() {}
    void Initialize()
    {
        this->readFromFile("maps/level1.txt");
        m_kBoard.InitializeBoard(m_board);
        m_kBoard.SetMovableValue(m_playerPos, CELL_PLAYER);
        for (int i = 0; i < BOARD_ROW; i++)
        {
            for (int j = 0; j < BOARD_COL; j++)
            {
                KPoint *tempPos = new KPoint(i, j);
                if (m_kBoard.GetFixedValue(*tempPos) == CELL_GOAL)
                {
                    totalGoals++;
                }
            }
           
        }
    }
    void UpdateInput();
    void Update();
    void DrawBoard()
    {
        gotoxy(0, 0);
        m_kBoard.DrawBoard(::DrawBoard);
    }
    int GetLastKey()
    {
        return m_lastKey;
    }
    int getTurns()
    {
        return turnsTaken;
    }
    int getGoals()
    {
        return totalGoals;
    }
    void readFromFile(const char* fileName)
    {
        std::ifstream mapFile;
        const char* fileNameAddress = fileName;
        mapFile.open(fileNameAddress);
        if (!mapFile)
        {
            std::cerr << "Unable to read map from disk";
            exit(1);
        }
        int fixed;
        int movable;
        int x = 0;
        int y = 0;
        KCell temp;

        while (mapFile >> fixed >> movable) {

            KCell* tempCell = new KCell(fixed, movable);
            m_board[x][y] = *tempCell;
            if (x < BOARD_ROW)
            {
                if (y < BOARD_COL)
                {
                    y++;
                }
                else {
                    y = 1;
                    x++;
                }
            }
        }

    }
};

void KGameWorld::UpdateInput()
{
    m_lastKey = _getch();
    if (m_lastKey == ARROWKEY_CHAR || m_lastKey == FUNCTIONKEY_CHAR)
    {
        KPoint oldPos = m_playerPos;
        m_offsetVector = KPoint(0, 0);

        m_lastKey = _getch();
        if (m_lastKey == KEYS::KEY_LEFT)
        {
            m_offsetVector = KPoint(-1, 0);
        }
        else if (m_lastKey == KEYS::KEY_RIGHT)
        {
            m_offsetVector = KPoint(+1, 0);
        }
        else if (m_lastKey == KEYS::KEY_UP)
        {
            m_offsetVector = KPoint(0, -1);
        }
        else if (m_lastKey == KEYS::KEY_DOWN)
        {
            m_offsetVector = KPoint(0, +1);
        }
    }
}

void KGameWorld::Update()
{
    m_kBoard.SetMovableValue(m_playerPos, 0);
    KPoint playersNewPos = m_playerPos.Add(m_offsetVector);
    if (m_kBoard.GetMovableValue(playersNewPos) == CELL_BOX)
    {
        KPoint boxPos = playersNewPos;
        if (m_kBoard.GetFixedValue(boxPos.Add(m_offsetVector)) != CELL_WALL && m_kBoard.GetMovableValue(boxPos.Add(m_offsetVector)) == 0)
        {
            KPoint boxNewPos = boxPos.Add(m_offsetVector);
            if (m_kBoard.GetFixedValue(boxPos) == CELL_GOAL)
            {
                totalGoals++;
            }
            
            boxPos = boxNewPos;

            m_kBoard.SetMovableValue(boxPos, CELL_BOX);
            m_playerPos = playersNewPos;

            if (m_kBoard.GetFixedValue(boxPos) == CELL_GOAL)
            {
                totalGoals--;
            }
            turnsTaken++;
        }
    }
    else if (m_kBoard.GetFixedValue(playersNewPos) != CELL_WALL)
    {
 
        m_playerPos = playersNewPos;
        turnsTaken++;
    }
    m_kBoard.SetMovableValue(m_playerPos, CELL_PLAYER);
    
}



int main()
{
    KGameWorld gameWorld;
    gameWorld.Initialize();
    gameWorld.DrawBoard();
    gotoxy(BOARD_ROW / 2, BOARD_COL + 1);
    printf(" Total amount of turns taken: %i\r\n", gameWorld.getTurns());
    printf(" Total amount of goals left: %i", gameWorld.getGoals());

    ShowConsoleCursor(false);
    int ch = 0;
    while (ch != ESC || gameWorld.getGoals() > 0)
    {
        gameWorld.UpdateInput();
        ch = gameWorld.GetLastKey();
        gameWorld.Update();
        gameWorld.DrawBoard();
        gotoxy(BOARD_ROW / 2, BOARD_COL + 1);
        printf(" Total amount of turns taken: %i\r\n", gameWorld.getTurns());
        printf(" Total amount of goals left: %i", gameWorld.getGoals());
    }
    ShowConsoleCursor(true);
}
