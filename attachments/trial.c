#include <stdio.h>  
#include <stdlib.h>  
  
int main() {  
    // 正确的二维整数数组声明  
    int matrix[10][10] = {  
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
    int ***lines = (int ***)malloc(10 * sizeof(int **));  
    for (int i = 0; i < 10; i++) {  
        lines[i] = (int **)malloc(10 * sizeof(int *));
        for (int j = 0; j < 10; j++) {  
        lines[j] = (int *)malloc(10 * sizeof(int)); // 假设每行最多有10个0  
    }  
  
    int srow = 0, erow = 10, scol = 0, ecol = 10, numcols = 10;  
    int count = 0; // 用于跟踪每行中0的数量  
  
    for (int i = srow; i < erow; i++) { // 修改循环条件  
        count = 0; // 重置计数器  
        int countt=0;
        for (int j = scol; j < ecol; j++) {  
            
            if (matrix[i][j] == 0) {  
                if (countt==0)count++;
                lines[i][count][countt++] = j; // 存储0的列索引  
            } 
            else if(matrix[i][j]==1){
                countt=0;
            }
        }  

    }  
  
    // 打印结果  
    for (int i = 0; i < 10; i++) {  
        for (int j = 0; lines[i][j] != -1; j++) {  
            printf("%d ", lines[i][j]);  
        }  
        printf("\n");  
    }  
  
    // 释放分配的内存  
    for (int i = 0; i < 10; i++) {  
        free(lines[i]);  
    }  
    free(lines);  
  
    return 0;  
}