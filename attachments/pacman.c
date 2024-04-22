#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define MAX_LEVEL 3

// To enable more ghosts, we will need to update our map file format.
#define MAX_GHOSTS 10
#define MS_PER_GHOST_TICK 500
#define MS_PER_TICK 20

typedef struct
{
  int row;
  int col;
} Coord;

// Remark: Each enum item corresponds to an integer, starting from 0.
typedef enum
{
  Up,
  Left,
  Idle,
  Right,
  Down
} Direction;

Direction oppositeDirection(Direction dir) { return (Direction)(4 - dir); } // 如果碰壁，往回退一格并修改方向

Coord moveOneStep(Coord from, Direction dir)
{ // 将方向与坐标数值变化一一对应
  static const int rowDelta[] = {-1, 0, 0, 0, 1};
  static const int colDelta[] = {0, -1, 0, 1, 0};
  return (Coord){from.row + rowDelta[dir], from.col + colDelta[dir]};
}

typedef struct
{ // ghost的参数，与ghost绑定，储存并接受外部函数的修改
  Coord pos;
  Direction direction;
  char itemBelow;
} Ghost;

typedef enum
{
  GS_Preparing,
  GS_Running,
  GS_Win,
  GS_Lose
} GameStatus;

typedef struct
{ // 游戏运行所需要的全部数据，游戏状态，游戏地图，玩家和怪物参数
  GameStatus status;
  int level;
  int score;

  int numRows;
  int numCols;
  int ghostCnt;
  int foodsCnt;

  Coord pacmanPos;
  Ghost ghosts[MAX_GHOSTS];

  char *maze;
  char **grid;
} Game;
// 获取地图信息
bool isWall(char c) { return c == 'B' || c == 'Y' || c == 'R' || c == 'G'; }

bool isFood(char c) { return c == '.'; }

bool isPacman(char c) { return c == 'C'; }

bool isGhost(char c) { return c == '@'; }

Game createGame(int level, const char *mapFileName)
{ // 初始化游戏，根据抓取的txt数据，创建一个游戏对象

  char line[256], **grid;
  int column, row, ghost, index = 0, food = 0;
  // 开始阅读文件
  FILE *file;
  file = fopen(mapFileName, "r");
  fgets(line, sizeof(line), file);
  sscanf(line, "%d %d %d", &row, &column, &ghost);
  grid = (char **)malloc(row * sizeof(char *));
  for (int i = 0; i < row; ++i)
  {
    fgets(line, sizeof(line), file);
    grid[i] = strdup(line);
  }

  Game game;
  while (fgets(line, sizeof(line), file) != NULL)
  { // 读取和分配初始方向
    if (line[0] == 'u')
      game.ghosts[index++].direction = Up;
    else if (line[0] == 'l')
      game.ghosts[index++].direction = Left;
    else if (line[0] == 'r')
      game.ghosts[index++].direction = Right;
    else if (line[0] == 'd')
      game.ghosts[index++].direction = Down;
  }
  fclose(file); // 关闭文件
  // 计算和分配foodcnt和坐标
  for (int i = 0; i < row; ++i)
  {
    for (int j = 0; j < column; j++)
    {
      if (grid[i][j] == '.')
      {
        food++;
      }
      else if (grid[i][j] == 'C')
      {
        game.pacmanPos.row = i;
        game.pacmanPos.col = j;
      }
      else if (grid[i][j] >= '0' && grid[i][j] <= '9')
      {
        game.ghosts[grid[i][j] - '0'].pos.row = i;
        game.ghosts[grid[i][j] - '0'].pos.col = j;
        grid[i][j] = '@';
      }
    }
  }
  for (int i = 0; i < ghost; i++)
  {
    game.ghosts[i].itemBelow = ' ';
  }
  // 分配game对象的各个属性
  game.level = level;
  game.grid = grid;
  game.numCols = column;
  game.numRows = row;
  game.score = 0;
  game.status = GS_Preparing;
  game.ghostCnt = ghost;
  game.foodsCnt = food;
  printf("hi");
  return game;
}

void destroyGame(Game *pGame)
{
  for (int i = pGame->numRows - 1; i >= 0; --i)
    free(pGame->grid[i]);
  free(pGame->grid);
}

void printInitialGame(const Game *pGame)
{

  clear_screen();
  for (int i = 0; i != pGame->numRows; ++i)
  {
    for (int j = 0; j != pGame->numCols; ++j)
    {
      switch (pGame->grid[i][j])
      {
      case 'C':
        printf(BRIGHT_YELLOW_TEXT("C"));
        break;
      case 'B':
        printf(BLUE_TEXT("#"));
        break;
      case 'Y':
        printf(YELLOW_TEXT("#"));
        break;
      case 'R':
        printf(RED_TEXT("#"));
        break;
      case 'G':
        printf(GREEN_TEXT("#"));
        break;
      default:
        putchar(pGame->grid[i][j]);
        break;
      }
    }
    putchar('\n');
  }
  putchar('\n');
  printf("Level %d\n", pGame->level);
  assert(pGame->score == 0);
  printf("Score: 0\n");
  printf("Food remaining: %d\n", pGame->foodsCnt);
  printf("Pacman wants food! (control by w/a/s/d)\n");
  for (int i = 0; i < pGame->ghostCnt; i++)
  {
    move_cursor(pGame->ghosts[i].pos.row, pGame->ghosts[i].pos.col);
    printf("@");
  }
}

void printScoreUpdate(const Game *pGame)
{
  move_cursor(pGame->numRows + 2, 7);
  printf("%d   ", pGame->score);
}

void printFoodUpdate(const Game *pGame)
{
  move_cursor(pGame->numRows + 3, 16);
  printf("%d          ", pGame->foodsCnt);
}

void moveGhosts(Game *pGame)
{
  for (int i = 0; i < pGame->ghostCnt; i++)
  {
    // ghost重叠方案：每次更新时全部清除，如果重合，后来者取前者itembelow而不是@
    pGame->grid[pGame->ghosts[i].pos.row][pGame->ghosts[i].pos.col] = pGame->ghosts[i].itemBelow;
    move_cursor(pGame->ghosts[i].pos.row, pGame->ghosts[i].pos.col);
    putchar(pGame->ghosts[i].itemBelow);
  }

  for (int i = 0; i < pGame->ghostCnt; i++)
  {
    // ghost移动代码
    Ghost *gho = &pGame->ghosts[i];
    Coord targetpos = pGame->pacmanPos;
    Coord newpos;
    // move ghosts
    if (gho->pos.col > targetpos.col)
      newpos = moveOneStep(gho->pos, (Direction)(1));
    else if (gho->pos.col < targetpos.col)
      newpos = moveOneStep(gho->pos, (Direction)(3)); // 异列变列
    else if (gho->pos.col == targetpos.col)
    { // 同列变行
      if (gho->pos.row > targetpos.row)
        newpos = moveOneStep(gho->pos, (Direction)(0));
      else if (gho->pos.row < targetpos.row)
        newpos = moveOneStep(gho->pos, (Direction)(4));

      if (!isWall(pGame->grid[newpos.row][newpos.col]))
      {
        gho->pos = newpos;
      }

      else
      { // 同列异行触墙 随机运动
        for (int i = 0; i < 4; i++)
        {
          if (i != 2)
          {
            newpos = moveOneStep(gho->pos, (Direction)(i));
            if (!isWall(pGame->grid[newpos.row][newpos.col]))
            {
              gho->pos = newpos;
            }
          }
        }
      }
    }
    if (isWall(pGame->grid[newpos.row][newpos.col]) && !(gho->pos.col == targetpos.col))
    { // 异列触墙
      if (gho->pos.row > targetpos.row)
        newpos = moveOneStep(gho->pos, (Direction)(0)); // 异行变行
      else if (gho->pos.row < targetpos.row)
        newpos = moveOneStep(gho->pos, (Direction)(4));
      if (!isWall(pGame->grid[newpos.row][newpos.col]))
      {
        gho->pos = newpos;
      } // 异列异行触墙
    }
    else if (!isWall(pGame->grid[newpos.row][newpos.col]) && !(gho->pos.col == targetpos.col))
    {
      gho->pos = newpos;
    }

    if (pGame->grid[gho->pos.row][gho->pos.col] == '@')
    {
      for (int j = 0; j < pGame->ghostCnt; j++)
      {
        if (j != i && pGame->ghosts[j].pos.col == pGame->ghosts[i].pos.col && pGame->ghosts[j].pos.row == pGame->ghosts[i].pos.row)
        {
          pGame->ghosts[i].itemBelow = pGame->ghosts[j].itemBelow;
        }
      }
    }
    else
    {

      gho->itemBelow = pGame->grid[gho->pos.row][gho->pos.col];
    }
    if (isPacman(pGame->grid[gho->pos.row][gho->pos.col]))
    {
      pGame->status = GS_Lose;
    }
    move_cursor(gho->pos.row, gho->pos.col);
    printf("@");
    pGame->grid[gho->pos.row][gho->pos.col] = '@';
  }
}

Direction getPacmanMovement(int row)
{
  if (kbhit())
  {
    switch (getch())
    {
      // 将当前移动方向展示在屏幕上
    case 'w':
      move_cursor(row, 40);
      printf("Up   ");
      return Up;
    case 's':
      move_cursor(row, 40);
      printf("Down ");
      return Down;
    case 'a':
      move_cursor(row, 40);
      printf("Left ");
      return Left;
    case 'd':
      move_cursor(row, 40);
      printf("Right");
      return Right;
    }
  }
  return Idle;
}

void movePacman(Game *pGame)
{

  Direction pacmanDirection;
  pacmanDirection = getPacmanMovement(pGame->numRows + 4);

  move_cursor(pGame->pacmanPos.row, pGame->pacmanPos.col);
  printf(" ");
  pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col] = ' ';
  Coord newpos = moveOneStep(pGame->pacmanPos, pacmanDirection);

  if (isWall(pGame->grid[newpos.row][newpos.col]) || newpos.col == pGame->numCols || newpos.row == pGame->numRows)
  {
    ;
  }
  else
  {
    pGame->pacmanPos = newpos;
    int emergency = 0;
    for (int i = 0; i < pGame->ghostCnt; i++) // 调查emergency
    {
      if (abs((pGame->pacmanPos.row) - (pGame->ghosts[i].pos.row)) + abs((pGame->pacmanPos.col) - (pGame->ghosts[i].pos.col)) <= 4)
      {
        emergency = 2; // 任意一个ghost的emergency是2就立刻退出返回warning
        break;
      }
      else if (abs((pGame->pacmanPos.row) - (pGame->ghosts[i].pos.row)) +
                   abs((pGame->pacmanPos.col) - (pGame->ghosts[i].pos.col)) <=
               8)
      {
        emergency = 1;
      }
      else
      {
        if (emergency == 1)
        {
          ;
        }
        else
        {
          emergency = 0;
        }
      }
    }

    if (emergency == 2)
    {
      move_cursor(pGame->numRows + 2, 12);
      printf(RED_TEXT("WARNING!    "));
    }
    else if (emergency == 1)
    {
      move_cursor(pGame->numRows + 2, 12);
      printf(YELLOW_TEXT("CAREFUL    "));
    }
    else
    {
      move_cursor(pGame->numRows + 2, 12);
      printf(GREEN_TEXT("SAFE      "));
    }

    if (isFood(pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col]))
    {
      pGame->foodsCnt--;
      pGame->score++;
    }
    else if (isGhost(pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col]))
    {
      pGame->status = GS_Lose;
    }
  }
  pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col] = 'C';
  move_cursor(pGame->pacmanPos.row, pGame->pacmanPos.col);
  printf(BRIGHT_YELLOW_TEXT("C"));
  if (pGame->foodsCnt == 0)
  {
    pGame->status = GS_Win;
  }
}

bool pacmanDies(const Game *pGame)
{
  return pGame->status == GS_Lose;
}

bool gameEnds(Game *pGame)
{

  if (pacmanDies(pGame))
  {
    pGame->status = GS_Lose;
    return true;
  }
  if (pGame->foodsCnt == 0)
  {
    pGame->status = GS_Win;
    return true;
  }
  return false;
}

int getinput(int result, int level)
{
  char c;
  while (1)
  {
    if (kbhit())
    {
      c = getch();
      if (c == 'q')
        return 1;
      if (c == 'n' && result == 1 && level != 3)
        return 2;
      if (c == 'r')
        return 3;
    }
  }

  return 0;
}
void runGame(Game *pGame)
{
  printInitialGame(pGame);
  pGame->status = GS_Running;
  int ticks = 0;
  while (1)
  {
    sleep_ms(MS_PER_TICK);
    ++ticks;
    movePacman(pGame);
    printFoodUpdate(pGame);
    if (gameEnds(pGame))
      break;
    if (ticks == MS_PER_GHOST_TICK / MS_PER_TICK)
    {
      ticks = 0;
      --pGame->score;
      printScoreUpdate(pGame);
      moveGhosts(pGame);
      if (gameEnds(pGame))
        break;
    }
  }
}

int runLevel(int level)
{
  char path[100];
  int result;
  sprintf(path, "maps/level%d.txt", level);
  Game game = createGame(level, path);
  runGame(&game);
  assert(game.status == GS_Lose || game.status == GS_Win);
  move_cursor(game.numRows + 4, 0);
  if (game.status == GS_Lose)
  {
    printf(RED_TEXT("Pacman dies!                                      "));
    move_cursor(game.numRows + 5, 0);
    printf("Press q to quit, Press r to restart");
    result = 2;
  }
  else
  {
    printf(RED_TEXT("You win!                                          "));
    move_cursor(game.numRows + 5, 0);
    if (level == 3)
    {
      printf("Press q to quit, Press r to try again ,Its end of the game");
    }
    else
    {
      printf("Press q to quit, Press r to try again, Press n to play next level");
    }
    result = 1;
  }

  destroyGame(&game);
  return getinput(result, level);
}

int main(void)
{
  prepare_game();
  int level = 0, result = 0, done = 0;

  while (1)
  {
    result = runLevel(level);
    if (result == 1)
    {
      break;
    }
    else if (result == 2)
    {
      level++;
    }
    else if (result == 3)
      ;
  }

  return 0;
}
