/**
 * graph.c — 景区路径规划系统功能实现
 *
 * 包含：邻接表构建、邻接矩阵展示、DFS 导游线路、
 *       回路检测、Dijkstra 最短路径、Prim 最小生成树
 *
 * 作者：许嘉华  学号：202509604228
 */

#include "graph.h"

/* ================================================================
 *                        基础操作
 * ================================================================ */

/* 初始化图 */
void initGraph(Graph *g) {
    g->spotCount = 0;
    for (int i = 0; i < MAX_SPOTS; i++) {
        g->names[i][0] = '\0';
        g->adjList[i] = NULL;
        for (int j = 0; j < MAX_SPOTS; j++) {
            g->adjMatrix[i][j] = (i == j) ? 0 : INF;
        }
    }
}

/* 查找景点下标 */
int findSpotIndex(const Graph *g, const char *name) {
    for (int i = 0; i < g->spotCount; i++) {
        if (strcmp(g->names[i], name) == 0) return i;
    }
    return -1;
}

/* 添加景点 */
int addSpot(Graph *g, const char *name) {
    if (g->spotCount >= MAX_SPOTS) {
        printf("⚠️  景点数量已达上限（%d）！\n", MAX_SPOTS);
        return -1;
    }
    if (findSpotIndex(g, name) != -1) {
        printf("⚠️  景点 \"%s\" 已存在！\n", name);
        return -1;
    }
    strncpy(g->names[g->spotCount], name, MAX_NAME - 1);
    g->names[g->spotCount][MAX_NAME - 1] = '\0';
    g->adjList[g->spotCount] = NULL;
    g->spotCount++;
    printf("✅ 已添加景点：%s（编号：%d）\n", name, g->spotCount - 1);
    return g->spotCount - 1;
}

/* 添加无向边 */
int addEdge(Graph *g, int from, int to, int weight) {
    if (from < 0 || from >= g->spotCount ||
        to   < 0 || to   >= g->spotCount) {
        printf("⚠️  景点编号无效！\n");
        return -1;
    }
    if (from == to) {
        printf("⚠️  不允许添加自环边！\n");
        return -1;
    }
    if (weight <= 0) {
        printf("⚠️  距离必须为正整数！\n");
        return -1;
    }

    /* 检查是否已存在 */
    EdgeNode *p = g->adjList[from];
    while (p) {
        if (p->to == to) {
            printf("⚠️  边 \"%s\"—\"%s\" 已存在！\n",
                   g->names[from], g->names[to]);
            return -1;
        }
        p = p->next;
    }

    /* 添加 from -> to */
    EdgeNode *e1 = (EdgeNode *)malloc(sizeof(EdgeNode));
    e1->to = to;
    e1->weight = weight;
    e1->next = g->adjList[from];
    g->adjList[from] = e1;

    /* 添加 to -> from（无向图） */
    EdgeNode *e2 = (EdgeNode *)malloc(sizeof(EdgeNode));
    e2->to = from;
    e2->weight = weight;
    e2->next = g->adjList[to];
    g->adjList[to] = e2;

    printf("✅ 已添加边：%s ↔ %s（距离：%d）\n",
           g->names[from], g->names[to], weight);
    return 0;
}

/* 根据邻接表重建邻接矩阵 */
void rebuildMatrix(Graph *g) {
    int n = g->spotCount;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            g->adjMatrix[i][j] = (i == j) ? 0 : INF;
        }
    }
    for (int i = 0; i < n; i++) {
        EdgeNode *p = g->adjList[i];
        while (p) {
            g->adjMatrix[i][p->to] = p->weight;
            p = p->next;
        }
    }
}

/* 打印景点列表 */
void printSpots(const Graph *g) {
    printf("\n══════════════════════════════\n");
    printf("  景区景点列表（共 %d 个）\n", g->spotCount);
    printf("══════════════════════════════\n");
    for (int i = 0; i < g->spotCount; i++) {
        printf("  [%d]  %s\n", i, g->names[i]);
    }
    printf("\n");
}

/* ================================================================
 *                功能 2：展示邻接矩阵
 * ================================================================ */

void displayMatrix(const Graph *g) {
    if (g->spotCount == 0) {
        printf("⚠️  当前无景点数据，请先加载预设数据或手动添加景点。\n");
        return;
    }

    int n = g->spotCount;

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║     景区景点分布图 — 邻接矩阵        ║\n");
    printf("║     （∞ = %d，表示无连通路径）      ║\n", INF);
    printf("╚══════════════════════════════════════╝\n\n");

    /* 表头 */
    printf("%-8s", "景点");
    for (int i = 0; i < n; i++) {
        printf("%-8s", g->names[i]);
    }
    printf("\n");

    /* 分隔线 */
    printf("%-8s", "────");
    for (int i = 0; i < n; i++) printf("────────");
    printf("\n");

    /* 矩阵内容 */
    for (int i = 0; i < n; i++) {
        printf("%-8s", g->names[i]);
        for (int j = 0; j < n; j++) {
            int val = g->adjMatrix[i][j];
            if (val == INF) {
                printf("%-8s", "∞");
            } else {
                printf("%-8d", val);
            }
        }
        printf("\n");
    }
    printf("\n");
}

/* ================================================================
 *           功能 3：DFS 导游线路图（深度优先遍历）
 * ================================================================ */

static void dfsRecursive(Graph *g, int v, int *visited, int prev,
                         int *tourPath, int *pathLen) {

    visited[v] = 1;
    tourPath[(*pathLen)++] = v;

    EdgeNode *p = g->adjList[v];

    while (p) {
        if (!visited[p->to]) {
            dfsRecursive(g, p->to, visited, v, tourPath, pathLen);
            /* 导游从大门出发，走到最后一个景点结束，不走回头路 */
        }
        p = p->next;
    }
}

void dfsTour(Graph *g, int start, int *tourPath, int *pathLen) {
    if (g->spotCount == 0) {
        printf("⚠️  当前无景点数据！\n");
        *pathLen = 0;
        return;
    }
    if (start < 0 || start >= g->spotCount) {
        printf("⚠️  起始景点编号无效！\n");
        *pathLen = 0;
        return;
    }

    int visited[MAX_SPOTS] = {0};
    *pathLen = 0;

    dfsRecursive(g, start, visited, -1, tourPath, pathLen);

    /* 输出结果 */
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║      DFS 导游线路图                  ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    printf("  起始景点：%s\n\n", g->names[start]);
    printf("  导游线路（深度优先遍历，不走回头路）：\n\n    ");

    for (int i = 0; i < *pathLen; i++) {
        if (i > 0) printf(" → ");
        printf("%s", g->names[tourPath[i]]);
    }
    printf("\n\n");

    /* 统计 */
    int visitedCount = 0;
    for (int i = 0; i < g->spotCount; i++) {
        if (visited[i]) visitedCount++;
    }

    printf("  ─────────────────────────────\n");
    printf("  覆盖景点数：%d / %d\n", visitedCount, g->spotCount);
    printf("  线路步数  ：%d 步（每个景点仅访问一次）\n", *pathLen);
    printf("  💡 导游从 %s 出发，游览至 %s 结束，无需走回头路\n",
           g->names[tourPath[0]], g->names[tourPath[*pathLen - 1]]);
    printf("\n");

    if (visitedCount < g->spotCount) {
        printf("  ⚠️  以下景点不可达：");
        for (int i = 0; i < g->spotCount; i++) {
            if (!visited[i]) printf("%s  ", g->names[i]);
        }
        printf("\n\n");
    }
}

/* ================================================================
 *              功能 4：回路检测
 * ================================================================ */

void detectCycle(Graph *g, int *tourPath, int pathLen) {
    if (g->spotCount == 0 || pathLen == 0) {
        printf("⚠️  请先生成导游线路（DFS）！\n");
        return;
    }

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║      导游线路回路检测                ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    /* 统计每个景点的访问次数 */
    int visitCount[MAX_SPOTS] = {0};
    for (int i = 0; i < pathLen; i++) {
        visitCount[tourPath[i]]++;
    }

    /* 查找重复访问的景点 */
    int hasCycle = 0;
    for (int i = 0; i < g->spotCount; i++) {
        if (visitCount[i] > 1) {
            hasCycle = 1;

            /* 查找该景点在 tourPath 中第一次和第二次出现的位置 */
            int firstPos = -1, secondPos = -1;
            for (int j = 0; j < pathLen; j++) {
                if (tourPath[j] == i) {
                    if (firstPos == -1) firstPos = j;
                    else if (secondPos == -1) { secondPos = j; break; }
                }
            }

            printf("  🔁 检测到回路！\n\n");
            printf("  重复访问景点：%s（访问 %d 次）\n",
                   g->names[i], visitCount[i]);

            if (firstPos != -1 && secondPos != -1) {
                printf("\n  回路路径（%s → … → %s）：\n\n    ",
                       g->names[i], g->names[i]);
                for (int j = firstPos; j <= secondPos; j++) {
                    if (j > firstPos) printf(" → ");
                    printf("%s", g->names[tourPath[j]]);
                }
                printf("\n");
                printf("  回路长度：%d 步\n", secondPos - firstPos);
            }
            printf("\n");
        }
    }

    if (!hasCycle) {
        printf("  ✅ 导游线路中未检测到回路。\n");
        printf("  当前线路采用'不走回头路'模式，\n");
        printf("  每个景点在导游线路中仅访问一次。\n\n");
    }
}

/* ================================================================
 *          功能 5：Dijkstra 最短路径查询
 * ================================================================ */

void dijkstra(Graph *g, int start, int end) {
    if (g->spotCount == 0) {
        printf("⚠️  当前无景点数据！\n");
        return;
    }
    if (start < 0 || start >= g->spotCount ||
        end   < 0 || end   >= g->spotCount) {
        printf("⚠️  景点编号无效！\n");
        return;
    }
    if (start == end) {
        printf("⚠️  起点和终点相同，距离为 0。\n");
        return;
    }

    int n = g->spotCount;
    int dist[MAX_SPOTS];
    int prev[MAX_SPOTS];
    int visited[MAX_SPOTS] = {0};

    /* 初始化 */
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[start] = 0;

    /* Dijkstra 主循环 */
    for (int k = 0; k < n; k++) {
        /* 选取未访问中距离最小的顶点 */
        int u = -1, minDist = INF;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1) break;  /* 剩余顶点不可达 */
        visited[u] = 1;

        /* 松弛操作 */
        EdgeNode *p = g->adjList[u];
        while (p) {
            if (!visited[p->to] && dist[u] + p->weight < dist[p->to]) {
                dist[p->to] = dist[u] + p->weight;
                prev[p->to] = u;
            }
            p = p->next;
        }
    }

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║    Dijkstra 最短路径查询             ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    printf("  起点：%s\n", g->names[start]);
    printf("  终点：%s\n\n", g->names[end]);

    if (dist[end] == INF) {
        printf("  ❌ %s 和 %s 之间没有连通路径。\n\n",
               g->names[start], g->names[end]);
        return;
    }

    /* 回溯路径 */
    int path[MAX_SPOTS];
    int pathLen = 0;
    int cur = end;
    while (cur != -1) {
        path[pathLen++] = cur;
        cur = prev[cur];
    }

    printf("  最短路径：\n\n    ");
    for (int i = pathLen - 1; i >= 0; i--) {
        if (i < pathLen - 1) printf(" → ");
        printf("%s", g->names[path[i]]);
    }
    printf("\n\n");
    printf("  最短距离：%d\n\n", dist[end]);

    /* 输出途经景点详细信息 */
    if (pathLen > 2) {
        printf("  途经景点详情：\n");
        int totalDist = 0;
        for (int i = pathLen - 1; i > 0; i--) {
            int from = path[i], to = path[i - 1];
            int edgeWeight = g->adjMatrix[from][to];
            totalDist += edgeWeight;
            printf("    %s — %s ：%d\n",
                   g->names[from], g->names[to], edgeWeight);
        }
        printf("    ─────────────\n");
        printf("    总距离：%d\n\n", totalDist);
    }
}

/* ================================================================
 *         功能 6：Prim 最小生成树（扩展功能）
 * ================================================================ */

void primMST(Graph *g) {
    if (g->spotCount == 0) {
        printf("⚠️  当前无景点数据！\n");
        return;
    }

    int n = g->spotCount;
    int visited[MAX_SPOTS] = {0};
    int mstFrom[MAX_SPOTS] = {0};
    int mstTo[MAX_SPOTS] = {0};
    int mstWeight[MAX_SPOTS] = {0};
    int mstCount = 0;
    int totalWeight = 0;

    /* 从顶点 0 开始 */
    visited[0] = 1;

    for (int k = 0; k < n - 1; k++) {
        int minWeight = INF;
        int bestFrom = -1, bestTo = -1;

        /* 在所有已访问顶点中，找连接到未访问顶点的最小边 */
        for (int u = 0; u < n; u++) {
            if (!visited[u]) continue;
            EdgeNode *p = g->adjList[u];
            while (p) {
                if (!visited[p->to] && p->weight < minWeight) {
                    minWeight = p->weight;
                    bestFrom = u;
                    bestTo = p->to;
                }
                p = p->next;
            }
        }

        if (bestTo == -1) break;  /* 图不连通 */

        mstFrom[mstCount] = bestFrom;
        mstTo[mstCount] = bestTo;
        mstWeight[mstCount] = minWeight;
        totalWeight += minWeight;
        mstCount++;
        visited[bestTo] = 1;
    }

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║    Prim 最小生成树（扩展功能）       ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (mstCount < n - 1) {
        printf("  ⚠️  图不连通！仅生成 %d/%d 条边。\n\n", mstCount, n - 1);
    }

    printf("  MST 边列表：\n\n");
    for (int i = 0; i < mstCount; i++) {
        printf("  %2d.  %s — %s  距离：%d\n",
               i + 1,
               g->names[mstFrom[i]],
               g->names[mstTo[i]],
               mstWeight[i]);
    }
    printf("\n  ─────────────────────────────\n");
    printf("  总边数    ：%d\n", mstCount);
    printf("  最小总权重：%d\n\n", totalWeight);
    printf("  💡 说明：最小生成树可用于景区道路建设优化，\n");
    printf("     以最少道路总长度连通所有景点。\n\n");
}

/* ================================================================
 *                    加载预设数据
 * ================================================================ */

void loadPreset(Graph *g) {
    /* 重置图 */
    initGraph(g);

    /* 8 个预设景点 */
    const char *presetNames[] = {
        "大门", "碧玉潭", "红叶林", "赏花园",
        "飞云石", "九曲桥", "白虎岩", "观月阁"
    };
    int nSpots = 8;

    for (int i = 0; i < nSpots; i++) {
        addSpot(g, presetNames[i]);
    }

    /* 9 条预设边 */
    int edges[][3] = {
        {0, 1, 5},   /* 大门 — 碧玉潭 */
        {0, 2, 8},   /* 大门 — 红叶林 */
        {1, 3, 3},   /* 碧玉潭 — 赏花园 */
        {2, 3, 4},   /* 红叶林 — 赏花园 */
        {2, 4, 6},   /* 红叶林 — 飞云石 */
        {3, 4, 2},   /* 赏花园 — 飞云石 */
        {4, 5, 7},   /* 飞云石 — 九曲桥 */
        {5, 6, 4},   /* 九曲桥 — 白虎岩 */
        {6, 7, 5}    /* 白虎岩 — 观月阁 */
    };
    int nEdges = 9;

    printf("\n");
    for (int i = 0; i < nEdges; i++) {
        addEdge(g, edges[i][0], edges[i][1], edges[i][2]);
    }

    /* 重建矩阵 */
    rebuildMatrix(g);

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║   预设数据加载完成！                 ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("  景点数：%d\n", g->spotCount);
    printf("  边数  ：%d\n\n", nEdges);
}
