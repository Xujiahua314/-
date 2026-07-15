/**
 * main.c — 景区路径规划系统主函数
 *
 * 控制台交互菜单，调用 graph.h / graph.c 中的功能
 *
 * 作者：许嘉华  学号：202509604228
 * 专业：软件工程 5班
 *
 * 编译：gcc -o scenic_guide main.c graph.c
 * 运行：./scenic_guide   （Windows: scenic_guide.exe）
 */

#include "graph.h"

/* 全局图变量 */
static Graph g;

/* 全局导游线路 */
static int tourPath[MAX_SPOTS * 2];  /* 足够大以容纳回溯路径 */
static int tourPathLen = 0;
static int tourGenerated = 0;        /* 标记是否已生成导游线路 */

/* ================================================================
 *                       辅助函数
 * ================================================================ */

/* 清屏 */
static void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* 暂停等待用户按键 */
static void pauseScreen(void) {
    printf("\n按 Enter 键继续...");
    getchar();
}

/* 安全读取整数 */
static int readInt(void) {
    int val;
    while (scanf("%d", &val) != 1) {
        printf("输入无效，请重新输入数字：");
        while (getchar() != '\n');
    }
    while (getchar() != '\n'); /* 清除缓冲区 */
    return val;
}

/* 安全读取字符串（去除换行） */
static void readString(char *buf, int maxLen) {
    int c;
    while ((c = getchar()) == '\n' || c == ' '); /* 跳过前导空白 */
    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);
    }
    fgets(buf, maxLen, stdin);
    /* 去除末尾换行 */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
}

/* ================================================================
 *                       功能菜单函数
 * ================================================================ */

/* 菜单 1：手动添加景点 */
static void menuAddSpot(void) {
    char name[MAX_NAME];
    printf("\n请输入景点名称：");
    readString(name, MAX_NAME);
    if (strlen(name) == 0) {
        printf("⚠️  名称不能为空！\n");
        return;
    }
    addSpot(&g, name);
    rebuildMatrix(&g);
    tourGenerated = 0;
}

/* 菜单 2：手动添加边 */
static void menuAddEdge(void) {
    if (g.spotCount < 2) {
        printf("⚠️  至少需要 2 个景点才能添加边！\n");
        return;
    }
    printSpots(&g);
    printf("请输入起点编号：");
    int from = readInt();
    printf("请输入终点编号：");
    int to = readInt();
    printf("请输入距离（正整数）：");
    int weight = readInt();
    addEdge(&g, from, to, weight);
    rebuildMatrix(&g);
    tourGenerated = 0;
}

/* 菜单 3：展示邻接矩阵 */
static void menuDisplayMatrix(void) {
    rebuildMatrix(&g);
    displayMatrix(&g);
}

/* 菜单 4：DFS 导游线路 */
static void menuDFSTour(void) {
    if (g.spotCount == 0) {
        printf("⚠️  请先加载数据！\n");
        return;
    }
    printSpots(&g);
    printf("请输入起始景点编号（直接回车默认从 0 开始）：");
    int c = getchar();
    int start;
    if (c == '\n') {
        start = 0;
    } else {
        ungetc(c, stdin);
        start = readInt();
    }
    tourPathLen = 0;
    dfsTour(&g, start, tourPath, &tourPathLen);
    tourGenerated = (tourPathLen > 0);
}

/* 菜单 5：回路检测 */
static void menuDetectCycle(void) {
    if (!tourGenerated) {
        printf("⚠️  请先生成导游线路（菜单 4）！\n");
        return;
    }
    detectCycle(&g, tourPath, tourPathLen);
}

/* 菜单 6：Dijkstra 最短路径 */
static void menuDijkstra(void) {
    if (g.spotCount < 2) {
        printf("⚠️  至少需要 2 个景点！\n");
        return;
    }
    printSpots(&g);
    printf("请输入起点编号：");
    int start = readInt();
    printf("请输入终点编号：");
    int end = readInt();
    dijkstra(&g, start, end);
}

/* 菜单 7：Prim 最小生成树 */
static void menuPrimMST(void) {
    if (g.spotCount < 2) {
        printf("⚠️  至少需要 2 个景点！\n");
        return;
    }
    rebuildMatrix(&g);
    primMST(&g);
}

/* 菜单 8：显示景点列表 */
static void menuListSpots(void) {
    if (g.spotCount == 0) {
        printf("⚠️  当前无景点数据。\n");
        return;
    }
    printSpots(&g);

    /* 同时显示边列表 */
    printf("  当前路径（边）列表：\n\n");
    int edgeNum = 0;
    for (int i = 0; i < g.spotCount; i++) {
        EdgeNode *p = g.adjList[i];
        while (p) {
            if (i < p->to) { /* 无向图每条边只输出一次 */
                printf("  %2d. %s — %s  距离：%d\n",
                       ++edgeNum, g.names[i], g.names[p->to], p->weight);
            }
            p = p->next;
        }
    }
    if (edgeNum == 0) {
        printf("  （暂无路径）\n");
    }
    printf("\n");
}

/* ================================================================
 *                         主菜单
 * ================================================================ */

static void printTitle(void) {
    printf("\n");
    printf("  ╔══════════════════════════════════╗\n");
    printf("  ║       景区路径规划系统           ║\n");
    printf("  ║   数据结构实训期末大作业         ║\n");
    printf("  ║   许嘉华  202509604228           ║\n");
    printf("  ╚══════════════════════════════════╝\n");
}

static void printMainMenu(void) {
    printf("\n");
    printf("  ┌─────────────────────────────────┐\n");
    printf("  │         主  菜  单              │\n");
    printf("  ├────┬────────────────────────────┤\n");
    printf("  │ 1  │ 🏛 管理端（录入维护数据）  │\n");
    printf("  │ 2  │ 🏔 游客端（游览查询功能）  │\n");
    printf("  │ 0  │ 退出系统                   │\n");
    printf("  └────┴────────────────────────────┘\n");
    printf("\n  当前景点数：%d | 请选择操作 [0-2]：", g.spotCount);
}

static void printAdminMenu(void) {
    printf("\n");
    printf("  ┌─────────────────────────────────┐\n");
    printf("  │      管理端 — 数据录入与维护   │\n");
    printf("  ├────┬────────────────────────────┤\n");
    printf("  │ 1  │ 加载预设数据（8景点9条边） │\n");
    printf("  │ 2  │ 手动添加景点               │\n");
    printf("  │ 3  │ 手动添加路径（边）         │\n");
    printf("  │ 4  │ 展示邻接矩阵               │\n");
    printf("  │ 5  │ 显示景点 & 边列表          │\n");
    printf("  │ 0  │ 返回主菜单                 │\n");
    printf("  └────┴────────────────────────────┘\n");
    printf("\n  当前景点数：%d | 请选择操作 [0-5]：", g.spotCount);
}

static void printVisitorMenu(void) {
    printf("\n");
    printf("  ┌─────────────────────────────────┐\n");
    printf("  │      游客端 — 游览与路径查询   │\n");
    printf("  ├────┬────────────────────────────┤\n");
    printf("  │ 1  │ DFS 导游线路图             │\n");
    printf("  │ 2  │ 回路检测                   │\n");
    printf("  │ 3  │ 最短路径查询（Dijkstra）   │\n");
    printf("  │ 4  │ 最小生成树（Prim·扩展）    │\n");
    printf("  │ 5  │ 显示景点 & 边列表          │\n");
    printf("  │ 0  │ 返回主菜单                 │\n");
    printf("  └────┴────────────────────────────┘\n");
    printf("\n  当前景点数：%d | 请选择操作 [0-5]：", g.spotCount);
}

/* ================================================================
 *                         主函数
 * ================================================================ */

int main(void) {
    int mainChoice, subChoice;

    /* 初始化 */
    initGraph(&g);

    /* 启动时自动加载预设数据 */
    clearScreen();
    printTitle();
    printf("\n  正在加载预设景区数据...\n");
    loadPreset(&g);
    tourGenerated = 0;

    printf("\n  按 Enter 键进入主菜单...");
    getchar();

    /* 主循环 */
    do {
        clearScreen();
        printTitle();
        printMainMenu();
        mainChoice = readInt();

        if (mainChoice == 0) {
            printf("\n  感谢使用景区路径规划系统！再见～\n\n");
            break;
        }

        if (mainChoice == 1) {
            /* ======== 管理端子菜单 ======== */
            do {
                clearScreen();
                printTitle();
                printf("\n  🏛 当前模式：管理端\n");
                printAdminMenu();
                subChoice = readInt();

                clearScreen();
                printTitle();
                printf("\n  🏛 当前模式：管理端\n");

                switch (subChoice) {
                case 1:
                    printf("\n  重新加载预设数据...\n");
                    loadPreset(&g);
                    tourGenerated = 0;
                    break;
                case 2:
                    menuAddSpot();
                    break;
                case 3:
                    menuAddEdge();
                    break;
                case 4:
                    menuDisplayMatrix();
                    break;
                case 5:
                    menuListSpots();
                    break;
                case 0:
                    break;  /* 返回主菜单 */
                default:
                    printf("\n  ⚠️  无效选项，请重新选择 [0-5]。\n");
                    break;
                }

                if (subChoice != 0) {
                    pauseScreen();
                }

            } while (subChoice != 0);

        } else if (mainChoice == 2) {
            /* ======== 游客端子菜单 ======== */
            do {
                clearScreen();
                printTitle();
                printf("\n  🏔 当前模式：游客端\n");
                printVisitorMenu();
                subChoice = readInt();

                clearScreen();
                printTitle();
                printf("\n  🏔 当前模式：游客端\n");

                switch (subChoice) {
                case 1:
                    menuDFSTour();
                    break;
                case 2:
                    menuDetectCycle();
                    break;
                case 3:
                    menuDijkstra();
                    break;
                case 4:
                    menuPrimMST();
                    break;
                case 5:
                    menuListSpots();
                    break;
                case 0:
                    break;  /* 返回主菜单 */
                default:
                    printf("\n  ⚠️  无效选项，请重新选择 [0-5]。\n");
                    break;
                }

                if (subChoice != 0) {
                    pauseScreen();
                }

            } while (subChoice != 0);

        } else {
            printf("\n  ⚠️  无效选项，请重新选择 [0-2]。\n");
            pauseScreen();
        }

    } while (1);

    /* 释放邻接表内存 */
    for (int i = 0; i < g.spotCount; i++) {
        EdgeNode *p = g.adjList[i];
        while (p) {
            EdgeNode *tmp = p;
            p = p->next;
            free(tmp);
        }
    }

    return 0;
}
