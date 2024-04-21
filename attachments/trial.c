#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Windows.h>
// 枚举一些关键常量,可以根据迷宫的不同而修改
enum state
{
	start='P', end = '*', road = ' ', wall = '0', visited = '1', successPath = '#', currentPosition = '@'
}State;
//路径操作符枚举
enum operate {
	up = 'w', right = 'd', down = 's', left = 'a'
}Operate;
//保存路径
struct path
{
	int len=0;
	unsigned char arr[1000]={0};
}Path;




//输入路径
void inputPath(unsigned char op)
{
	Path.arr[Path.len] = op;
	Path.len++;
}
//输出路径
void printPath()
{
	printf("\nPath:");
	while (Path.len > 0)
	{
		Path.len--;
		putchar(Path.arr[Path.len]);
	}
	printf("\n");
}
//判断是否在迷宫范围内以及是否可以走这一步
bool isLegal(int x,int y,int row,int col, unsigned char* p)	
{
	if (x >= 0 && y >= 0)
		if (x < row  && y < col)
			return (p[x * col + y ] ==road||p[x*col+y]==end);
	return false;
}
//输入迷宫图
//支持以矩阵形式输入,也可以输入一整行,自动处理换行符,直到读取到整个迷宫图为止
void inputMaze(unsigned char* p, int row, int col)
{
	unsigned char ch;
	printf("请输入迷宫图:\n");	
	for (int i = 0; i < row * col; i++)
	{
		if ((ch = getchar()) != '\n')
			p[i] = ch;
		else
			--i;
	}
 
}
//打印迷宫图
void printMaze(unsigned char* p, int row, int col) {
	printf("\n迷宫图如下:\n");
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
			printf("%c", p[i * col + j]);
		printf("\n");
	}
 
}
//走迷宫
//递归查询,这里由于递归是倒序输出路径,所以需要一个倒序操作
bool walkMaze(int row,int col, unsigned char* p,int x,int y)
{
	int pos =x * col + y;	//当前位置
	if (p[pos] == end)		//到达终点
		return true;
	if (isLegal(x - 1, y, row, col,p))	//上
	{
		//printMaze(p,row,col); //如果需要可以逐步输出迷宫图
		p[pos] = visited;	//设置访问标识,防止无限递归
		if (walkMaze(row, col, p, x - 1, y))	//该路径可行,输出操作符
		{
			inputPath(up);
			p[pos] = successPath;			//用于显示该路径
			return true;
		}
	}
	if (isLegal(x, y + 1, row, col, p))	//右
	{
		//printMaze(p,row,col);
		p[pos] = visited;
		if (walkMaze(row, col, p, x , y+1))
		{
			inputPath(right);
			p[pos] = successPath;
			return  true;
		}
	}
	if (isLegal(x + 1, y, row, col, p))	//下
	{
		//printMaze(p,row,col);
		p[pos] = visited;
		if (walkMaze(row, col, p, x +1, y))
		{
			inputPath(down);
			p[x * col + y] = successPath;
			return true;
		}
	}
	if (isLegal(x, y - 1, row, col, p))	//左
	{
		//printMaze(p,row,col);
		p[pos] = visited;
		if (walkMaze(row, col, p, x , y-1))
		{
			inputPath(left);
			p[pos] = successPath;
			return  true;
		}
	}
	p[pos] = visited;
	return false;	//无路可走,该条路径不行
}
 

int main()
{
	int row=21, col=21,x=15,y=0;	//行和列,起点坐标
	unsigned char* Maze =(unsigned char*)malloc(row*col);	//分配空间
	inputMaze(Maze, row, col );		//输入迷宫
	printMaze(Maze, row, col);		//打印迷宫
	walkMaze(row,col,Maze,x,y);		//走迷宫
	Maze[x * col + y] = start;		//矫正起点字符
	printMaze(Maze, row, col);		//打印迷宫
	printPath();					//打印路径
	free(Maze);						//释放空间
	system("pause");
	return 0;
}