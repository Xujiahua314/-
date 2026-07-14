/**
 * graph.h — 景区路径规划系统头文件
 *
 * 数据结构实训期末大作业
 * 作者：许嘉华  学号：202509604228
 * 专业：软件工程 5班
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ========== 常量定义 ========== */
#define MAX_SPOTS   50      /* 最大景点数量          */
#define MAX_NAME    32      /* 景点名称最大长度       */
#define INF         32767   /* 无穷大（无连通路径）    */

/* ========== 数据结构 ========== */

/* 邻接表边结点 */
typedef struct EdgeNode {
    int to;                   /* 邻接景点下标         */
    int weight;               /* 边的权重（距离）      */
    struct EdgeNode *next;    /* 指向下一条边         */
} EdgeNode;

/* 图结构：邻接表为主存储，邻接矩阵用于展示 */
typedef struct {
    char names[MAX_SPOTS][MAX_NAME];  /* 景点名称数组          */
    int  spotCount;                    /* 当前景点数量          */
    EdgeNode *adjList[MAX_SPOTS];     /* 邻接表（指针数组）      */
    int  adjMatrix[MAX_SPOTS][MAX_SPOTS]; /* 邻接矩阵          */
} Graph;

/* ========== 函数声明 ========== */

/* 初始化图 */
void initGraph(Graph *g);

/* 添加景点，返回下标；已存在则返回 -1 */
int  addSpot(Graph *g, const char *name);

/* 添加无向边，返回 0 成功 / -1 失败 */
int  addEdge(Graph *g, int from, int to, int weight);

/* 根据邻接表重建邻接矩阵 */
void rebuildMatrix(Graph *g);

/* 以表格形式输出邻接矩阵 */
void displayMatrix(const Graph *g);

/* 深度优先遍历：tourPath 由调用方提供（长度至少 2*MAX_SPOTS），
 * pathLen 返回实际长度 */
void dfsTour(Graph *g, int start, int *tourPath, int *pathLen);

/* 检测导游线路中的回路 */
void detectCycle(Graph *g, int *tourPath, int pathLen);

/* Dijkstra 最短路径 */
void dijkstra(Graph *g, int start, int end);

/* Prim 最小生成树（扩展功能） */
void primMST(Graph *g);

/* 加载预设示例数据（8 景点，9 条边） */
void loadPreset(Graph *g);

/* 打印所有景点列表 */
void printSpots(const Graph *g);

/* 按名称查找景点下标，未找到返回 -1 */
int  findSpotIndex(const Graph *g, const char *name);

#endif /* GRAPH_H */
