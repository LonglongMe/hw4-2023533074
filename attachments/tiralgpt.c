#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Include this for strdup
#include "utils.h"
#define MAX_LEVEL 3
#define MAX_GHOSTS 10
#define MS_PER_GHOST_TICK 500
#define MS_PER_TICK 20

typedef struct {  
    int row;  
    int col;  
} Coord;

typedef enum { Up, Left, Idle, Right, Down } Direction;

Direction oppositeDirection(Direction dir) { return (Direction)(4 - dir); }

Coord moveOneStep(Coord from, Direction dir) {
    static const int rowDelta[] = {-1, 0, 0, 0, 1};
    static const int colDelta[] = {0, -1, 0, 1, 0};
    return (Coord){from.row + rowDelta[dir], from.col + colDelta[dir]};
}

typedef struct {
    Coord pos;
    Direction direction;
    char itemBelow;
} Ghost;

typedef enum { GS_Preparing, GS_Running, GS_Win, GS_Lose } GameStatus;

enum state { start = 'S', end = 'E', road = ' ', wall = '0', visited = '1', successPath = 'R' } State;

typedef struct {
    GameStatus status;
    int level;
    int score;
    int numRows;
    int numCols;
    int ghostCnt;
    int foodsCnt;
    Coord pacmanPos;
    Ghost ghosts[MAX_GHOSTS];
    char* maze;
    char** grid;
} Game;

bool isWall(char c) { return c == 'B' || c == 'Y' || c == 'R' || c == 'G'; }

bool isFood(char c) { return c == '.'; }

bool isPacman(char c) { return c == 'C'; }

bool isGhost(char c) { return c == '@'; }

bool isLegal(int x, int y, int row, int col, unsigned char* p) {
    if (x >= 0 && y >= 0)
        if (x < row && y < col)
            return (p[x * col + y] == road || p[x * col + y] == end);
    return false;
}

Game createGame(int level, const char* mapFileName) {
    char line[256], **grid;
    int column, row, ghost, index = 0, food = 0;

    FILE* file;
    file = fopen(mapFileName, "r");
    fgets(line, sizeof(line), file);
    sscanf(line, "%d %d %d", &row, &column, &ghost);
    grid = (char**)malloc(row * sizeof(char*));
    for (int i = 0; i < row; ++i) {
        fgets(line, sizeof(line), file);
        grid[i] = strdup(line);
    }
    char* maze = (char*)malloc(row * column * sizeof(char));

    fclose(file);

    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < column; j++) {
            if (grid[i][j] == '.') {
                maze[i * column + j] = ' ';
            } else if (grid[i][j] == 'C') {
                maze[i * column + j] = ' ';
            } else if (grid[i][j] >= '0' && grid[i][j] <= '9') {
                grid[i][j] = ' ';
                maze[i * column + j] = ' ';
            } else {
                maze[i * column + j] = '0';
            }
        }
    }
    for(int i=0;i<row;i++){
        for(int j=0;j<column;j++){
        printf("%c",maze[i*column+j]);}
        printf("\n");
    }
    Game game;
    game.level = level;
    game.grid = grid;
    game.numCols = column;
    game.numRows = row;
    game.score = 0;
    game.status = GS_Preparing;
    game.ghostCnt = ghost;
    game.foodsCnt = food;
    game.maze = maze;

    return game;
}

bool walkMaze(int numrows, int numcols, unsigned char* maze, int row, int col, int* ans) {
    int pos = row * numcols + col;
    if (maze[pos] == end) {
        return true;
    }
    if (isLegal(row - 1, col, numrows, numcols, maze)) {
        if (walkMaze(numrows, numcols, maze, row - 1, col, ans)) {
            *ans = 0;
            return true;
        }
    }
    if (isLegal(row, col + 1, numrows, numcols, maze)) {
        if (walkMaze(numrows, numcols, maze, row, col + 1, ans)) {
            *ans = 3;
            return true;
        }
    }
    if (isLegal(row + 1, col, numrows, numcols, maze)) {
        if (walkMaze(numrows, numcols, maze, row + 1, col, ans)) {
            *ans = 4;
            return true;
        }
    }
    if (isLegal(row, col - 1, numrows, numcols, maze)) {
        if (walkMaze(numrows, numcols, maze, row, col - 1, ans)) {
            *ans = 1;
            return true;
        }
    }
    return false;
}

void moveGhosts(Game* pGame) {
    int i = 0;
    Coord newpos;
    char* maze = (char*)malloc(pGame->numRows * pGame->numCols * sizeof(char));
    memcpy(maze, pGame->maze, pGame->numRows * pGame->numCols);

    maze[pGame->pacmanPos.row * pGame->numCols + pGame->pacmanPos.col] = 'E';
    
    maze[pGame->ghosts[i].pos.row * pGame->numCols + pGame->ghosts[i].pos.col] = 'S';
    
    int ans = 1;
    walkMaze(pGame->numRows, pGame->numCols, (unsigned char*)maze, pGame->ghosts[i].pos.row, pGame->ghosts[i].pos.col, &ans);
    // Update ghost position based on the value of 'ans'
    
    printf("%d",ans);
}

int main(void) {
    prepare_game(); // Assuming this function is defined elsewhere
    char path[100];
    sprintf(path, "maps/level%d.txt", 0);
    Game game = createGame(0, path);
    moveGhosts(&game);
    return 0;
}
