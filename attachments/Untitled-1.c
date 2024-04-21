#include <stdio.h>
#include <stdlib.h>

#define ROWS 10
#define COLS 10

int main() {
    // 正确的二维整数数组声明
    int matrix[ROWS][COLS] = {
        {0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,0,1,1,1,0,0},
        {0,0,1,0,0,1,0,0,0,0},
        {0,0,1,0,0,1,0,0,1,0},
        {0,0,1,0,0,0,0,1,0,0},
        {0,0,0,1,0,0,1,0,0,0},
        {0,0,1,0,0,1,1,0,0,0},
        {0,0,1,0,1,0,0,1,0,0},
        {0,0,1,0,1,1,1,1,1,0},
        {0,0,1,0,0,0,0,0,0,0}
    };

    // 使用动态分配为lines数组中的每个指针分配内存
    int **lines = (int **)malloc(ROWS * sizeof(int *));
    for (int i = 0; i < ROWS; i++) {
        lines[i] = (int *)malloc(COLS * sizeof(int));
    }

    int srow = 0, erow = ROWS, scol = 0, ecol = COLS;

    for (int i = srow; i < erow; i++) {
        for (int j = scol; j < ecol; j++) {
            lines[i][j] = matrix[i][j];
        }
    }

    // 遍历lines数组，将每行中连续的0合成一个小数组
    for (int row = 0; row < ROWS; row++) {
        int start_col = -1, end_col = -1;
        for (int col = 0; col < COLS; col++) {
            if (lines[row][col] == 0) {
                if (start_col == -1) {
                    start_col = col;
                }
                end_col = col;
            } else if (start_col != -1) {
                printf("Row %d: Continuous zeros from column %d to %d\n", row + 1, start_col + 1, end_col + 1);
                start_col = -1;
            }
        }
        if (start_col != -1) {
            printf("Row %d: Continuous zeros from column %d to %d\n", row + 1, start_col + 1, end_col + 1);
        }
    }

    // 释放分配的内存
    for (int i = 0; i < ROWS; i++) {
        free(lines[i]);
    }
    free(lines);

    return 0;
}