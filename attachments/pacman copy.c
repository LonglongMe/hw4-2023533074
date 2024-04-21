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

typedef struct {  int row;  int col;  } Coord;

// Remark: Each enum item corresponds to an integer, starting from 0.
typedef enum { Up, Left, Idle, Right, Down } Direction;

Direction oppositeDirection(Direction dir) { return (Direction)(4 - dir); }//如果碰壁，往回退一格并修改方向

Coord moveOneStep(Coord from, Direction dir) {//将方向与坐标数值变化一一对应
  static const int rowDelta[] = {-1, 0, 0, 0, 1};
  static const int colDelta[] = {0, -1, 0, 1, 0};
  return (Coord){from.row + rowDelta[dir], from.col + colDelta[dir]};
}

typedef struct {//ghost的参数，与ghost绑定，储存并接受外部函数的修改
  Coord pos;
  Direction direction;
  char itemBelow;
} Ghost;

typedef enum { GS_Preparing, GS_Running, GS_Win, GS_Lose } GameStatus;

enum state{start='S', end = 'E', road = ' ', wall = '0', visited = '1',successPath='R'}State;



typedef struct {int len;  char* anslist;
}Ans;

typedef struct {//游戏运行所需要的全部数据，游戏状态，游戏地图，玩家和怪物参数
  GameStatus status;
  int level;  int score;

  int numRows;  int numCols;  int ghostCnt; int foodsCnt;

  Coord pacmanPos;    Ghost ghosts[MAX_GHOSTS];

  char* maze;   char **grid;
} Game;
//获取地图信息
bool isWall(char c) { return c == 'B' || c == 'Y' || c == 'R' || c == 'G'; }

bool isFood(char c) { return c == '.'; }

bool isPacman(char c) { return c == 'C'; }

bool isGhost(char c) { return c == '@'; }

bool isLegal(int x,int y,int row,int col, unsigned char* p)	
{
	if (x >= 0 && y >= 0)
		if (x < row  && y < col)
			return (p[x * col + y ] ==road||p[x*col+y]==end);
	return false;
}

/**
 * @brief Create a Game object. Level information including the map is read from
 * the file @c mapFileName. Return the Game object created.
 *
 * @param level Level ID.
 * @param mapFileName Path to the map file.
 * @return Game The created Game object.
 */
Game createGame(int level, const char *mapFileName) {//初始化游戏，根据抓取的txt数据，创建一个游戏对象
     
    char line[256],**grid;char* maze;
    int column,row,ghost,index=0,food=0;
    //开始阅读文件
    FILE *file; 
    file = fopen(mapFileName, "r");  
    fgets(line,sizeof(line),file);
    sscanf(line, "%d %d %d", &row, &column, &ghost);
    grid = (char **)malloc(row * sizeof(char *));  
    for (int i = 0; i < row; ++i) {fgets(line, sizeof(line), file) ;grid[i] = strdup(line);}
    maze=(char*)malloc(row*column*sizeof(char));
    Game game;
    while (fgets(line, sizeof(line), file) != NULL){//读取和分配初始方向
        if (line[0]=='u') game.ghosts[index++].direction=Up;
        else if (line[0]=='l') game.ghosts[index++].direction=Left;
        else if (line[0]=='r') game.ghosts[index++].direction=Right;
        else if (line[0]=='d') game.ghosts[index++].direction=Down;
    }
    fclose(file);//关闭文件
    //计算和分配foodcnt和坐标
    for(int i=0;i<row;++i){for(int j=0;j<column;j++){
      if(grid[i][j]=='.'){food++;maze[i*column+j]=' ';}
      else if(grid[i][j]=='C'){game.pacmanPos.row=i;game.pacmanPos.col=j;maze[i*column+j]=' ';}
      else if(grid[i][j]>='0' && grid[i][j]<='9'){game.ghosts[grid[i][j]-'0'].pos.row=i;
      game.ghosts[grid[i][j]-'0'].pos.col=j;grid[i][j]=' ';maze[i*column+j]=' ';}
      
      else {maze[i*column+j]='0';}}}
    for(int i=0;i<ghost;i++){game.ghosts[i].itemBelow=' ';}
    //分配game对象的各个属性
    game.level=level;
    game.grid=grid;
    game.numCols=column;
    game.numRows=row;
    game.score=0;
    game.status=GS_Preparing;
    game.ghostCnt=ghost;
    game.foodsCnt=food;
    game.maze=maze;
    return game;
    

  // TODO: Implement this function.
  // Create a Game object. The information needed should be obtained from the
  // file 'mapFileName'. Every member of the Game object should be correctly
  // set, some of which you need to pay special attention to:
  //   - 'status' should be set to GS_Preparing.
  //   - 'score' should be set to zero.
  //   - 'itemBelow' of each ghost should also be correctly set. This influences
  //   the way you move ghosts. Think about it carefully.
  //   - 'grid' should be a dynamically allocated "2d-array". The function
  //   'printInitialGame' below serves as a hint on the contents of 'grid'.
  // Do not forget to close the file you opened.
  

}

/**
 * @brief Destroy a Game object. Resources are released.
 *
 * @param pGame
 */
void destroyGame(Game *pGame) {
  for (int i = pGame->numRows - 1; i >= 0; --i)
    free(pGame->grid[i]);
  free(pGame->grid);
  // If your Game has some other resources, they should also be released.
}

void printInitialGame(const Game *pGame) {
  // Called at the beginning of 'runGame'.
  // Your 'createGame' should set the contents of 'grid' correctly to make this
  // function work.
  //clear_screen();
  for (int i = 0; i != pGame->numRows; ++i) {
    for (int j = 0; j != pGame->numCols; ++j) {
      switch (pGame->grid[i][j]) {
      case 'C': printf(BRIGHT_YELLOW_TEXT("C")); break;
      case 'B': printf(BLUE_TEXT("#")); break;
      case 'Y': printf(YELLOW_TEXT("#")); break;
      case 'R': printf(RED_TEXT("#")); break;
      case 'G': printf(GREEN_TEXT("#")); break;
      default: putchar(pGame->grid[i][j]); break;
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
  for(int i=0;i<pGame->ghostCnt;i++){
    move_cursor(pGame->ghosts[i].pos.row,pGame->ghosts[i].pos.col);
    printf("@");}
}

void printScoreUpdate(const Game *pGame) {
  move_cursor(pGame->numRows + 2, 7);
  printf("%d          ", pGame->score);
}

void printFoodUpdate(const Game *pGame) {
  move_cursor(pGame->numRows + 3, 16);
  printf("%d          ", pGame->foodsCnt);
}

/**
 * @brief Move all the ghosts by one step. Their @c pos should be updated, and
 * the contents in @c pGame->grid and things printed on the screen should also
 * be updated.
 *
 * @param pGame Pointer to the current Game object.
 */
/*
void redirection(Game *pGame,const int i,int* col,int*row,int*colable,int*rowable){//获取ghost和pacman的相对位置
    Ghost *gho=&pGame->ghosts[i];
    if (gho->pos.col>pGame->pacmanPos.col)*col= -1;
    else if (gho->pos.col==pGame->pacmanPos.col)*col= 0;
    else if (gho->pos.col<pGame->pacmanPos.col)*col= 1;

    if (gho->pos.row>pGame->pacmanPos.row)*row= -1;
    else if (gho->pos.row==pGame->pacmanPos.row)*row=0;
    else if (gho->pos.row<pGame->pacmanPos.row)*row=1;

    if (isWall(pGame->grid[gho->pos.row][gho->pos.col+*col]))*colable=0;
    if (isPacman(pGame->grid[gho->pos.row][gho->pos.col+*col]))pGame->status=GS_Lose;
    if (isWall(pGame->grid[gho->pos.row+*row][gho->pos.col]))*rowable=0;
    if (isPacman(pGame->grid[gho->pos.row+*row][gho->pos.col]))pGame->status=GS_Lose;
  }*/
  /*
void redirection(Game *pGame,int i){
  int srow=pGame->ghosts[i].pos.row;int scol=pGame->ghosts[i].pos.col;
  int erow=pGame->pacmanPos.row;int ecol=pGame->pacmanPos.col;
  if (srow<erow){int temp=srow;srow=erow;erow=temp;temp=scol;scol=ecol;ecol=temp;}
  int *lines[50][50];
  for(int i=0;i<srow-erow;i++){
    for(int j=0;j<pGame->numCols;j++){
      int tem;
      if(!isWall(pGame->grid[i+srow][j])){lines[i][tem++]=j;}
    }
  }//////if (gho->pos.col>pGame->pacmanPos.col)newpos=moveOneStep(gho->pos,(Direction)(1));
    else if (gho->pos.col<pGame->pacmanPos.col)newpos=moveOneStep(gho->pos,(Direction)(3));

    if (isWall(pGame->grid[newpos.row][newpos.col])||gho->pos.col==pGame->pacmanPos.col){

      if (gho->pos.row>pGame->pacmanPos.row)newpos=moveOneStep(gho->pos,(Direction)(0));
      else if (gho->pos.row<pGame->pacmanPos.row)newpos=moveOneStep(gho->pos,(Direction)(4));

      if (isWall(pGame->grid[newpos.row][newpos.col])){;}
      else {gho->pos=newpos;}
    }
    else {gho->pos=newpos;}//////
}*/

/*
bool walkMaze(int numrows,int numcols, unsigned char* maze,int row,int col, Ans *ans )

{
	int pos =row * numcols + col;	//当前位置
	if (maze[pos] == end)return true;
	if (isLegal(row - 1, col, numrows, numcols,maze))	//
	
    {maze[pos] = visited;
		if (walkMaze(numrows, numcols, maze, row - 1, col, ans))	//该路径可行,输出操作符
		{
			ans->len++;
      ans->anslist[ans->len]==0;
			maze[pos] = successPath;			//用于显示该路径
			return true;
		}
	}
	if (isLegal(row, col + 1, numrows, numcols, maze))	//右
	{

		maze[pos] = visited;
		if (walkMaze(numrows, numcols, maze, row ,col+1,ans))
		{
			ans->len++;
      ans->anslist[ans->len]==3;
			maze[pos] = successPath;
			return  true;
		}
	}
	if (isLegal(row + 1, col, numrows, numcols, maze))	//下
	{
		//printMaze(p,row,col);
		maze[pos] = visited;
		if (walkMaze(numrows, numcols, maze, row +1, col,ans))
		{
			ans->len++;
      ans->anslist[ans->len]==4;
			maze[row * numcols + col] = successPath;
			return true;
		}
	}
	if (isLegal(row, col - 1, numrows, numcols, maze))	//左
	{
		//printMaze(p,row,col);
		maze[pos] = visited;
		if (walkMaze(numrows, numcols, maze, row , col-1,ans))
		{
			ans->len++;
      ans->anslist[ans->len]==1;
			maze[pos] = successPath;
			return  true;
		}
	}
	maze[pos] = visited;
	return false;	//无路可走,该条路径不行
}
 */

bool walkMaze(int numrows,int numcols, unsigned char* maze,int row,int col, int* ans )

{
	int pos =row * numcols + col;	//当前位置
	if (maze[pos] == end){return true;}
	if (isLegal(row - 1, col, numrows, numcols,maze))	//上
    {if (walkMaze(numrows, numcols, maze, row - 1, col, ans))	//该路径可行,输出操作符
		{*ans=0;return true;}
	}
	if (isLegal(row, col + 1, numrows, numcols, maze))	//右
	{if (walkMaze(numrows, numcols, maze, row ,col+1,ans))
	    {*ans=3;return  true;}
	}
	if (isLegal(row + 1, col, numrows, numcols, maze))	//下
	  {if (walkMaze(numrows, numcols, maze, row +1, col,ans))
		    {*ans=4; return true;}
	}
	if (isLegal(row, col - 1, numrows, numcols, maze))	//左
	{if (walkMaze(numrows, numcols, maze, row , col-1,ans))
		{*ans=1; return  true;}
	}
	return false;	
}
 
/*
bool walkMaze(int numrows,int numcols, unsigned char* maze,int row,int col, int* ans )

{*ans=1;return true;}*/



void moveGhosts(Game *pGame) {
  // TODO: Implement this function.
  // Move all the ghosts by one step. You are encouraged to move ghosts in a
  // smarter way.
  // Note that ghosts may overlap, and they may also overlap with foods. Hints
  // on this:
  //  - The 'itemBelow' member of Ghost may be of great help.
  //  - A possible way to handle overlapping objects correctly is to first
  //  remove all the ghosts in reverse order, and then put the ghosts on their
  //  new positions in order.
  for(int i =0;i<pGame->ghostCnt;i++){
    Coord newpos;
    move_cursor(pGame->ghosts[i].pos.row,pGame->ghosts[i].pos.col);
    putchar(pGame->ghosts[i].itemBelow);
    //char *maze=pGame->maze;
    //char an[128]={0};
    int ans;
    unsigned char maze[100]={0};
    /*
    Ans ans ;
    ans.len=0;
    ans.anslist=an;*/

    //maze[(pGame->pacmanPos.row)*(pGame->numCols)+pGame->pacmanPos.col]='E';
    //maze[(pGame->ghosts[i].pos.row)*(pGame->numCols)+pGame->ghosts[i].pos.col]='S';
    //walkMaze(pGame->numRows,pGame->numCols,maze,pGame->ghosts[i].pos.row,pGame->ghosts[i].pos.col,&ans);
    walkMaze(pGame->numRows,pGame->numCols,maze,pGame->ghosts[i].pos.row,pGame->ghosts[i].pos.col,&ans);

    //moveOneStep(pGame->ghosts[i].pos,(Direction)(ans.anslist[ans.len]));
    pGame->ghosts[i].pos=moveOneStep(pGame->ghosts[i].pos,(Direction)(ans));
    
    pGame->ghosts[i].itemBelow=pGame->grid[pGame->ghosts[i].pos.row][pGame->ghosts[i].pos.col];
    move_cursor(pGame->ghosts[i].pos.row,pGame->ghosts[i].pos.col);
    printf("@");


  }
}

Direction getPacmanMovement(void) {
  if (kbhit()) {
    switch (getch()) {
    case 'w':move_cursor(15,30); printf("w");return Up;
    case 's':move_cursor(15,30); printf("s"); return Down;
    case 'a': move_cursor(15,30); printf("a");return Left;
    case 'd': move_cursor(15,30); printf("d");return Right;
    }
  }
  // Note that 'Idle' is also returned when no keyboard hit occurs.
  return Idle;
}

/**
 * @brief Move the Pacman by one step, following the user's control obtained by
 * @c getPacmanMovement(). Pacman's position should be updated, and the contents
 * in @c pGame->grid and things printed on the screen should also be updated. If
 * the Pacman eats a food through this step, the related information stored and
 * printed should also be updated.
 *
 * @param pGame Pointer to the current Game object.
 */
void movePacman(Game *pGame) {
  // TODO: Implement this function.
  // Note that Pacman may be moved to a position containing a food or a ghost.
  // These cases should be handled carefully.
  Direction pacmanDirection;  
  pacmanDirection = getPacmanMovement();  

  move_cursor(pGame->pacmanPos.row,pGame->pacmanPos.col);
  printf(" ");
  Coord newpos = moveOneStep(pGame->pacmanPos,pacmanDirection);
  if(isWall(pGame->grid[newpos.row][newpos.col])){;}
  else{pGame->pacmanPos=newpos;
  if(isFood(pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col])){
    pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col]=' ';
    pGame->foodsCnt--;}
  //else if(isGhost(pGame->grid[pGame->pacmanPos.row][pGame->pacmanPos.col])){pGame->status=GS_Lose;}}
  }
  move_cursor(pGame->pacmanPos.row,pGame->pacmanPos.col);
  printf("C");
  if(pGame->foodsCnt==0){pGame->status=GS_Win;}
  
}

/**
 * @brief Test if Pacman has died.
 */
bool pacmanDies(const Game *pGame) {
  return pGame->status==GS_Lose;
  // TODO: Implement this function.
}

/**
 * @brief Test if the game has ended. If so, update the @c status and return
 * true; otherwise return false.
 *
 * @param pGame Pointer to the current Game object.
 */
bool gameEnds(Game *pGame) {
  
  if (pacmanDies(pGame)) {
    pGame->status = GS_Lose;
    return true;
  }
  if (pGame->foodsCnt == 0) {
    pGame->status = GS_Win;
    return true;
  }
  return false;
}

void runGame(Game *pGame) {
  printInitialGame(pGame);
  pGame->status = GS_Running;
  int ticks = 0;
  while (1) {
    sleep_ms(MS_PER_TICK);
    ++ticks;
    movePacman(pGame);
    printFoodUpdate(pGame);
    if (gameEnds(pGame))
      break;
    if (ticks == MS_PER_GHOST_TICK / MS_PER_TICK) {
      ticks = 0;
      --pGame->score;
      printScoreUpdate(pGame);
      
      moveGhosts(pGame);

      if (gameEnds(pGame))
       break;
    }
  }
}

void runLevel(int level) {
  char path[100];
  sprintf(path, "maps/level%d.txt", level);
  Game game = createGame(level, path);
  runGame(&game);
  assert(game.status == GS_Lose || game.status == GS_Win);
  move_cursor(game.numRows + 4, 0);
  if (game.status == GS_Lose)
    printf("Pacman dies!");
  else
    printf("You win!");
  printf("                                       \n");
  destroyGame(&game);
}

int main(void) {
  prepare_game();
  // TODO: Run more levels?
  runLevel(0);
  return 0;
}