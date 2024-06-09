#include <windows.h>
#include "tools.h"
#include <graphics.h> 
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <vector>

#pragma comment( lib, "Msimg32.lib" )//TransparentBlt，VC环境下用

using namespace std;

#define WIN_WIDTH 1200 //窗口宽度
#define WIN_HEIGHT 500 //窗口高度
#define SLEEP_TIME 5 //刷新时间
#define WIN_SCORE 100 //胜利的分数
#define HERO_JUMP_HEIGHT 10  //英雄跳跃的高度
#define OBSTACLE_COUNT 10  //怪物的数量
#define HERO_WIDTH 144
#define HERO_HEIGHT 144
#define HERO_DOWN_WIDTH 128
#define HERO_DOWN_HEIGHT 71
#define HERO_BLOOD 10

#define TORTOISE_POWER 5
#define LION_POWER 10
#define HOOK_POWER 20

bool gameOver = false;
bool gameStart = true;

int update = true; //表示是否需要马上刷新画面

HBITMAP imgBackgrounds[5]; //背景图片
int bgX[5] = {0}; //背景图片的x坐标
int bgSpeed[5] = { 3, 6, 9, 6, 12 }; //背景图片的移动速度

HBITMAP imgHeros[12]; //英雄图片资源
int heroX; //英雄x坐标
int heroY; //英雄y坐标
int currentHeroIndex; //当前显示的英雄帧序号
int heroBlood;


bool heroJump; //表示英雄正在跳跃
int heroJumpMaxHeight; //英雄跳跃的最高高度
int heroJumpHeight; //英雄每次跳跃的高度

HBITMAP imgHeroDown[2]; //英雄下蹲资源
bool heroDown; //表示英雄正在下蹲

int frameCount = 0;

typedef enum {
    TORTOISE, //乌龟 0
    LION, //狮子 1
    HOOK1,
    HOOK2,
    HOOK3,
    HOOK4,
    OBSTACLE_TYPE_COUNT //2
} OBSTACLE_TYPE;


vector<vector<HBITMAP>> obstacleImgs; //存放所有障碍物的各个图片


typedef struct Obstacle {
    OBSTACLE_TYPE obstacleType; //障碍物类型
    int width;
    int height;
    int imgIndex; //当前显示的图片的序号
    int x, y; //障碍物的坐标 
    int speed; //速度
    int power; //杀伤力
    bool exist;
    bool hited; // 是否发生碰撞
    bool passed; //表示是否被跨越
}Obstacle;

Obstacle* obstacles[OBSTACLE_COUNT];

int lastObsIndex;

int currentScore;


HBITMAP imgScore[10];
HBITMAP imgBlood[10];

//全局变量声明
HBITMAP dra, bg ;
HDC		hdc, mdc, bufdc;
HWND	hWnd;
DWORD	tPre, tNow;
int		num, x, y;

//全局函数声明
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void MyPaint(HDC hdc);
void gameLoop(); //游戏循环
void createObstacle(); //创建障碍物
void checkHit(); //碰撞检测
void checkOver(); //判断游戏是否结束
void checkScore(); //检测得分
void levelIncrease(); //难度提升



//第三方函数2
BOOL MyTransparentBlt2(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
    int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, UINT crTransparent)

{
    if (nWidthDest < 1)
        return false;
    if (nWidthSrc < 1)
        return false;
    if (nHeightDest < 1)
        return false;
    if (nHeightSrc < 1)
        return false;

    HDC dc = CreateCompatibleDC(NULL);
    HBITMAP bitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, GetDeviceCaps(dc, BITSPIXEL), NULL);
    if (bitmap == NULL)
    {
        DeleteDC(dc);
        return false;
    }
    HBITMAP oldBitmap = (HBITMAP)SelectObject(dc, bitmap);
    if (!BitBlt(dc, 0, 0, nWidthSrc, nHeightSrc, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY))
    {
        SelectObject(dc, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(dc);
        return false;
    }
    HDC maskDC = CreateCompatibleDC(NULL);
    HBITMAP maskBitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, 1, NULL);
    if (maskBitmap == NULL)
    {
        SelectObject(dc, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(dc);
        DeleteDC(maskDC);
        return false;
    }
    HBITMAP oldMask = (HBITMAP)SelectObject(maskDC, maskBitmap);
    SetBkColor(maskDC, RGB(0, 0, 0));
    SetTextColor(maskDC, RGB(255, 255, 255));
    if (!BitBlt(maskDC, 0, 0, nWidthSrc, nHeightSrc, NULL, 0, 0, BLACKNESS))
    {
        SelectObject(maskDC, oldMask);
        DeleteObject(maskBitmap);
        DeleteDC(maskDC);
        SelectObject(dc, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(dc);
        return false;
    }
    SetBkColor(dc, crTransparent);
    BitBlt(maskDC, 0, 0, nWidthSrc, nHeightSrc, dc, 0, 0, SRCINVERT);
    SetBkColor(dc, RGB(0, 0, 0));
    SetTextColor(dc, RGB(255, 255, 255));
    BitBlt(dc, 0, 0, nWidthSrc, nHeightSrc, maskDC, 0, 0, SRCAND);
    HDC newMaskDC = CreateCompatibleDC(NULL);
    HBITMAP newMask;
    newMask = CreateBitmap(nWidthDest, nHeightDest, 1, GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);
    if (newMask == NULL)
    {
        SelectObject(dc, oldBitmap);
        DeleteDC(dc);
        SelectObject(maskDC, oldMask);
        DeleteDC(maskDC);
        DeleteDC(newMaskDC);
        DeleteObject(bitmap);
        DeleteObject(maskBitmap);
        return false;
    }
    SetStretchBltMode(newMaskDC, COLORONCOLOR);
    HBITMAP oldNewMask = (HBITMAP)SelectObject(newMaskDC, newMask);
    StretchBlt(newMaskDC, 0, 0, nWidthDest, nHeightDest, maskDC, 0, 0, nWidthSrc, nHeightSrc, SRCCOPY);
    SelectObject(maskDC, oldMask);
    DeleteDC(maskDC);
    DeleteObject(maskBitmap);
    HDC newImageDC = CreateCompatibleDC(NULL);
    HBITMAP newImage = CreateBitmap(nWidthDest, nHeightDest, 1, GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);
    if (newImage == NULL)
    {
        SelectObject(dc, oldBitmap);
        DeleteDC(dc);
        DeleteDC(newMaskDC);
        DeleteObject(bitmap);
        return false;
    }
    HBITMAP oldNewImage = (HBITMAP)SelectObject(newImageDC, newImage);
    StretchBlt(newImageDC, 0, 0, nWidthDest, nHeightDest, dc, 0, 0, nWidthSrc, nHeightSrc, SRCCOPY);
    SelectObject(dc, oldBitmap); DeleteDC(dc); DeleteObject(bitmap);
    BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, newMaskDC, 0, 0, SRCAND);
    BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, newImageDC, 0, 0, SRCPAINT);
    SelectObject(newImageDC, oldNewImage);
    DeleteDC(newImageDC);
    SelectObject(newMaskDC, oldNewMask);
    DeleteDC(newMaskDC);
    DeleteObject(newImage);
    DeleteObject(newMask);
    return true;
}

//***WinMain函数，程序入口点函数**************************************
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    MSG msg;

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    //游戏循环
    GetMessage(&msg, NULL, NULL, NULL);  //初始化msg
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            tNow = GetTickCount();
            if ((tNow - tPre >= 20) && !gameOver) {
                MyPaint(hdc);
            }
                
        }
    }
    return msg.wParam;
}

//***设计一个窗口类***
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "canvas";
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

//****初始化函数****
// 加载位图并设定各对象的初始值
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hWnd = CreateWindow("canvas", "动画演示", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd)
    {
        return FALSE;
    }

    MoveWindow(hWnd, 100, 100, WIN_WIDTH, WIN_HEIGHT, true);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    //1.初始化各种资源
    HBITMAP fullmap;
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);//小DC，for大图
    bufdc = CreateCompatibleDC(hdc);//小DC，for小图
    fullmap = CreateCompatibleBitmap(hdc, WIN_WIDTH, WIN_HEIGHT);//大图

    SelectObject(mdc, fullmap);

    dra = (HBITMAP)LoadImage(NULL, "dra.bmp", IMAGE_BITMAP, 760, 198, LR_LOADFROMFILE);
    bg = (HBITMAP)LoadImage(NULL, "res/bg000.bmp", IMAGE_BITMAP, WIN_WIDTH, WIN_HEIGHT, LR_LOADFROMFILE);

    char name[64];

    //1.加载背景资源
    imgBackgrounds[0] = (HBITMAP)LoadImage(NULL, "res/bg001.bmp", IMAGE_BITMAP, 2400, 500, LR_LOADFROMFILE);
    imgBackgrounds[1] = (HBITMAP)LoadImage(NULL, "res/bg002.bmp", IMAGE_BITMAP, 2400, 200, LR_LOADFROMFILE);
    imgBackgrounds[2] = (HBITMAP)LoadImage(NULL, "res/bg003.bmp", IMAGE_BITMAP, 2400, 300, LR_LOADFROMFILE);
    imgBackgrounds[3] = (HBITMAP)LoadImage(NULL, "res/bg004.bmp", IMAGE_BITMAP, 2400, 200, LR_LOADFROMFILE);
    imgBackgrounds[4] = (HBITMAP)LoadImage(NULL, "res/bg005.bmp",IMAGE_BITMAP ,2400, 400, LR_LOADFROMFILE);

    //2.加载英雄奔跑帧资源
    for (int i = 0; i < 12; i++)
    {
        sprintf_s(name, "res/hero/loading_%02d.bmp", i + 1);
        imgHeros[i] = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 144, 144, LR_LOADFROMFILE);
    }
    //2.加载英雄下蹲资源
    imgHeroDown[0] = (HBITMAP)LoadImage(NULL, "res/hero/down_01.bmp", IMAGE_BITMAP, 84, 90, LR_LOADFROMFILE);
    imgHeroDown[1] = (HBITMAP)LoadImage(NULL, "res/hero/down_02.bmp", IMAGE_BITMAP, 128, 71, LR_LOADFROMFILE);


    //3.加载乌龟资源
    HBITMAP imgTortoise;
    imgTortoise = (HBITMAP)LoadImage(NULL, "res/t1.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    vector<HBITMAP> imgTortoiseArray;
    imgTortoiseArray.push_back(imgTortoise);
    obstacleImgs.push_back(imgTortoiseArray);

    //4.加载狮子资源
    HBITMAP imgLion;
    vector<HBITMAP> imgLionArray;
    for (int i = 0; i < 6; i++) {
        sprintf_s(name, "res/p%d.bmp", i + 1);
        imgLion = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
        imgLionArray.push_back(imgLion);
    }
    obstacleImgs.push_back(imgLionArray);

    //5.加载柱子资源
    HBITMAP imgH;

    for (int i = 0; i < 4; i++) {
        vector<HBITMAP> imgHOOKArray;
        sprintf_s(name, "res/h%d.bmp", i + 1);
        imgH = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 60, 220, LR_LOADFROMFILE);
        imgHOOKArray.push_back(imgH);
        obstacleImgs.push_back(imgHOOKArray);
    }

    //5.加载分数资源
    for (int i = 0; i < 10; i++)
    {
        sprintf_s(name, "res/sz/%d.bmp", i);
        imgScore[i] = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 30, 30, LR_LOADFROMFILE);
    }

    for (int i = 0; i < 10; i++)
    {
        sprintf_s(name, "res/blood/%d.bmp", i);
        imgBlood[i] = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 30, 30, LR_LOADFROMFILE);
    }

    //6.初始化障碍物池子
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        Obstacle* obstacle = (Obstacle*)malloc(sizeof(Obstacle));
        obstacle->exist = false;
        obstacles[i] = obstacle;
    }

    //7.初始化英雄
    heroX = 300;
    heroY = 180;
    currentHeroIndex = 0;
    heroJump = false;
    heroDown = false;
    heroJumpMaxHeight = heroY - 120;
    heroJumpHeight = -HERO_JUMP_HEIGHT;
    
    heroBlood = HERO_BLOOD;


    lastObsIndex = -1;
    currentScore = 0;
    

    num = 0;    //显示图号
    x = 640;	//贴图起始X坐标
    y = 360;    //贴图起始Y坐标

    //2.开始绘图
    MyPaint(hdc);

    return TRUE;
}

//****自定义绘图函数*********************************
// 1.恐龙跑动图案的透明背景化
// 2.更新贴图坐标，实现动画效果
void MyPaint(HDC hdc)
{

    gameLoop();


    //1.背景移动
    SelectObject(bufdc, imgBackgrounds[0]);
    MyTransparentBlt2(mdc, bgX[0], 0, 2400, 500, bufdc, 0, 0, 2400, 500, RGB(255, 255, 255));
    SelectObject(bufdc, imgBackgrounds[1]);
    MyTransparentBlt2(mdc, bgX[1], 60, 2400, 200, bufdc, 0, 0, 2400, 200, RGB(255, 255, 255));
    SelectObject(bufdc, imgBackgrounds[2]);
    MyTransparentBlt2(mdc, bgX[2], 260, 2400, 300, bufdc, 0, 0, 2400, 300, RGB(255, 255, 255));
    SelectObject(bufdc, imgBackgrounds[3]);
    MyTransparentBlt2(mdc, bgX[3], 260, 2400, 200, bufdc, 0, 0, 2400, 200, RGB(255, 255, 255));
    SelectObject(bufdc, imgBackgrounds[4]);
    MyTransparentBlt2(mdc, bgX[4], 120, 2400, 400, bufdc, 0, 0, 2400, 400, RGB(255, 255, 255));


    //2.英雄奔跑
    if (!heroDown) {
        SelectObject(bufdc, imgHeros[currentHeroIndex]);
        MyTransparentBlt2(mdc, heroX, heroY, HERO_WIDTH, HERO_HEIGHT, bufdc, 0, 0, HERO_WIDTH, HERO_HEIGHT, RGB(255, 255, 255));
    }
    else {
        int y = 220;
        SelectObject(bufdc, imgHeroDown[currentHeroIndex]);
        if (currentHeroIndex == 0)
        {
            MyTransparentBlt2(mdc, heroX, y, 84, 90, bufdc, 0, 0, 84, 90, RGB(255, 255, 255));
        }
        else {
            MyTransparentBlt2(mdc, heroX, y, HERO_DOWN_WIDTH, HERO_DOWN_HEIGHT, bufdc, 0, 0, HERO_DOWN_WIDTH, HERO_DOWN_HEIGHT, RGB(255, 255, 255));
        }
        
    }


    //3.渲染所有怪物
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        Obstacle curObstacle = *obstacles[i];
        if (curObstacle.exist) {

            SelectObject(bufdc, obstacleImgs[curObstacle.obstacleType][curObstacle.imgIndex]);
            MyTransparentBlt2(mdc, curObstacle.x, curObstacle.y, curObstacle.width, curObstacle.height,
               bufdc, 0, 0, curObstacle.width, curObstacle.height, RGB(255, 255, 255));
            //putimagePNG2(curObstacle.x, curObstacle.y, WIN_WIDTH,
            //    &obstacleImgs[curObstacle.obstacleType][curObstacle.imgIndex]);
        }
    }

    //4.渲染分数
    char str[8];
    sprintf_s(str, "%d", currentScore);

    for (int i = 0; str[i]; i++)
    {
        int number = str[i] - '0';
        SelectObject(bufdc, imgScore[number]);
        MyTransparentBlt2(mdc, 1000 + 50*i, 50, 30, 30,
            bufdc, 0, 0, 30, 30, RGB(255, 255, 255));
        //putimagePNG2(WIN_WIDTH - 150, y, &imgScore[number]);
        //x += imgScore[number].getwidth() + 5;
    }

    //5.渲染血量
    sprintf_s(str, "%d", heroBlood);

    for (int i = 0; str[i]; i++)
    {
        int number = str[i] - '0';
        SelectObject(bufdc, imgBlood[number]);
        MyTransparentBlt2(mdc, 100 + 50 * i, 50, 30, 30,
            bufdc, 0, 0, 30, 30, RGB(255, 255, 255));
        //putimagePNG2(WIN_WIDTH - 150, y, &imgScore[number]);
        //x += imgScore[number].getwidth() + 5;
    }



    //判断游戏是否结束
    if (gameOver) {
        bg = (HBITMAP)LoadImage(NULL, "res/bg000.bmp", IMAGE_BITMAP, WIN_WIDTH, WIN_HEIGHT, LR_LOADFROMFILE);
        SelectObject(bufdc, bg);
        BitBlt(mdc, 0, 0, WIN_WIDTH, WIN_HEIGHT, bufdc, 0, 0, SRCCOPY);
    }


    //将最后的画面显示在窗口中
    //双缓冲2，再把大图画到大DC中显示
    BitBlt(hdc, 0, 0, WIN_WIDTH, WIN_HEIGHT, mdc, 0, 0, SRCCOPY);

    tPre = GetTickCount();     //记录此次绘图时间
}

//****消息处理函数***********************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:					//窗口结束消息，撤销各种DC
        DeleteDC(mdc);
        DeleteDC(bufdc);
        DeleteObject(dra);
        DeleteObject(bg);
        ReleaseDC(hWnd, hdc);
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:

        if (gameOver) {
            gameOver = false;
            break;
        }

        if (wParam == VK_SPACE && !heroJump) 
        {
            heroJump = true;
            heroDown = false;
            heroY = 180;
        }
        else if (wParam == 'K' && !heroDown) {
            heroDown = true;
            heroJump = false;
            currentHeroIndex = 0;
            heroY = 180;
        }
        break;
    default:							//其他消息
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//游戏循环
void gameLoop() {

    //背景图片移动
    for (int i = 0; i < 5; i++) {
        bgX[i] -= bgSpeed[i];

        if (bgX[i] < -WIN_WIDTH) {
            bgX[i] = 0;
        }
    }

    //英雄跳跃
    if (heroJump) {
        if (heroY < heroJumpMaxHeight) {
            heroJumpHeight = -heroJumpHeight;
        }

        heroY += heroJumpHeight;

        if (heroY > 320 - 144) {
            heroJump = false;
            heroJumpHeight = -HERO_JUMP_HEIGHT;
        }
    }
    //英雄下蹲
    else if (heroDown) {
        static int count = 0;
        int delays[2] = { 5, 40 };
        count++;
        if (count > delays[currentHeroIndex]) {
            count = 0;
            currentHeroIndex++;
            if (currentHeroIndex >= 2) {
                heroDown = false;
            }
        }

    }
    else { //不跳跃时才实现帧动画
        //实现帧动画
        currentHeroIndex = (++currentHeroIndex) % 12;
    }


    //创建障碍物
    frameCount++;
    static int enemyFre = 10;
    if (frameCount > enemyFre) {
        frameCount = 0;
        enemyFre = 30 + rand() % 15;
        if (currentScore > 0) {
            enemyFre = 50 + rand() & 50;
        }
        createObstacle();
    }


    //障碍物帧动画
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacles[i]->exist) {
            obstacles[i]->x -= obstacles[i]->speed + bgSpeed[2];
            //if (obstacles[i]->x < -2 * obstacleImgs[obstacles[i]->obstacleType][obstacles[i]->imgIndex].getwidth()) {
            //    obstacles[i]->exist = false;
            //}
            if (obstacles[i]->x < -2 * 100) {
                obstacles[i]->exist = false;
            }
            obstacles[i]->imgIndex = (obstacles[i]->imgIndex + 1) % obstacleImgs[obstacles[i]->obstacleType].size();
        }
    }


    //英雄和障碍物的 "碰撞检测" 处理
    checkHit();

    //检查得分
    checkScore();

    levelIncrease();
}

//创建障碍物
void createObstacle() {
    int i;
    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (!obstacles[i]->exist) {
            break;
        }
    }
    if (i >= OBSTACLE_COUNT) {
        return;
    }

    obstacles[i]->exist = true;
    obstacles[i]->imgIndex = 0;
    //obstacles[i]->obstacleType = (OBSTACLE_TYPE)(rand() % OBSTACLE_TYPE_COUNT);
    obstacles[i]->obstacleType = (OBSTACLE_TYPE)(rand() % 3);
    obstacles[i]->hited = false;
    obstacles[i]->passed = false;

    if (lastObsIndex >= 0 &&
        obstacles[lastObsIndex]->obstacleType >= HOOK1 &&
        obstacles[lastObsIndex]->obstacleType <= HOOK4 &&
        obstacles[i]->obstacleType == LION &&
        obstacles[lastObsIndex]->x > (WIN_WIDTH - 500))
    {
        obstacles[i]->obstacleType = TORTOISE;
    }
    lastObsIndex = i;

    //随机选取一种类型的柱子
    if (obstacles[i]->obstacleType == HOOK1) {
        obstacles[i]->obstacleType = (OBSTACLE_TYPE)((int)(obstacles[i]->obstacleType) + rand() % 4);
    }

    obstacles[i]->x = WIN_WIDTH;
    //obstacles[i]->y = 350 - obstacleImgs[obstacles[i]->obstacleType][0].getheight();
    obstacles[i]->y = 180 + HERO_HEIGHT;

    if (obstacles[i]->obstacleType == TORTOISE) {
        obstacles[i]->speed = 4;
        obstacles[i]->power = TORTOISE_POWER;
        obstacles[i]->width = 50;
        obstacles[i]->height = 50;  
        obstacles[i]->y -= 60;
    }
    else if (obstacles[i]->obstacleType == LION) {
        obstacles[i]->speed = 4;
        obstacles[i]->power = LION_POWER;
        obstacles[i]->height = 80;
        obstacles[i]->width = 80;
        obstacles[i]->y -= 90;
    }
    else if (obstacles[i]->obstacleType >= HOOK1 && obstacles[i]->obstacleType <= HOOK4) {
        obstacles[i]->speed = 0;
        obstacles[i]->power = HOOK_POWER;
        obstacles[i]->y = 0;
        obstacles[i]->width = 60;
        obstacles[i]->height = 220;

    }
}

//碰撞检测
void checkHit() {
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacles[i]->exist && !obstacles[i]->hited) {

            int a1x, a1y, a2x, a2y;
            int off = 30;
                
            //1.计算英雄的坐标
            if (!heroDown) {
                a1x = heroX + off;
                a1y = heroY;
                a2x = heroX + HERO_WIDTH - off;
                a2y = heroY + HERO_HEIGHT;
            }
            else {
                a1x = heroX + off;
                a1y = 345 - HERO_DOWN_HEIGHT;
                a2x = heroX + HERO_DOWN_WIDTH - off;
                a2y = 345;
            }

            //IMAGE img = obstacleImgs[obstacles[i]->obstacleType][obstacles[i]->imgIndex];
            //2.计算障碍物的坐标
            int b1x = obstacles[i]->x + off;
            int b1y = obstacles[i]->y + off;
            int b2x = obstacles[i]->x + obstacles[i]->width - off;
            int b2y = obstacles[i]->y + obstacles[i]->height - off;

            if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y)) {
                heroBlood -= obstacles[i]->power;

                checkOver();

                //3.障碍物已经发生碰撞, 无法继续碰撞
                obstacles[i]->hited = true;
            }
        }
    }
}

//检查是否结束
void checkOver() {
    if (heroBlood <= 0)
    {

        gameOver = true;
        heroBlood = 100;
        currentScore = 0;

    }
}

//检查得分
void checkScore() {
    for (int i = 0; i < OBSTACLE_COUNT; i++)
    {
        if (obstacles[i]->exist &&
            !obstacles[i]->passed &&
            !obstacles[i]->hited &&
            obstacles[i]->x + obstacles[i]->width < heroX)
        {
            currentScore++;
            obstacles[i]->passed = true;
        }
    }
}

//检查胜利
void checkWin() {
    if (currentScore >= WIN_SCORE)
    {
        mciSendString("play res/win.mp3", 0, 0, 0);
        Sleep(1000);
        loadimage(0, "res/win.png");
        FlushBatchDraw();
        mciSendString("stop res/bg.mp3", 0, 0, 0);
        system("pause");

        heroBlood = 100;
        currentScore = 0;
        mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
    }
}

//难度提升
void levelIncrease() {
    if (currentScore > 5 && currentScore <= 10){

        //int bgSpeed[5] = { 3, 6, 9, 6, 12 };
        bgSpeed[0] = 6;
        bgSpeed[1] = 12;
        bgSpeed[2] = 18;
        bgSpeed[3] = 12;
        bgSpeed[4] = 24;
        
    }
    else if (currentScore > 10)
    {
        bgSpeed[0] = 9;
        bgSpeed[1] = 18;
        bgSpeed[2] = 27;
        bgSpeed[3] = 18;
        bgSpeed[4] = 36;
    }

}