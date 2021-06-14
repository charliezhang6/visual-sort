#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "view.h"
#include "main-view.h"
#include "about-view.h"
#include "setting.h"

int const MAIN_LOOP_TIMEID = 0x01;
static
void main_loop_timer_handler(int timeid)
{
    if (timeid != MAIN_LOOP_TIMEID) return;

    DisplayClear();
    view_display();
}

static
void initalize()
{
    view_add(about_view_create("about"));
    view_add(main_view_create("main"));
    view_default("main");

    usePredefinedColor(4);
}

static
void destroy(void)
{
    view_destroy();
}

/**
 * 入口函数，只会被调用一次
 * 整体工作方式通过注册时间到 libgraphics 库中运行
 */
void Main()
{
    InitGraphics(); // 初始化 ImGUI 使用的底层绘图库
    initGUI();      // 初始化 ImGUI 控件库

    initalize();        // 初始化程序相关资源
    atexit(destroy);    // 程序退出时销毁各种资源

    //InitConsole();  // 初始化控制台
    SetWindowTitle("Visual Sort");

    RegisterCharEvent(uiGetChar);           // GUI 字符输入
	RegisterKeyboardEvent(uiGetKeyboard);   // 键盘
    RegisterMouseEvent(uiGetMouse);         // 鼠标事件
    RegisterTimerEvent(main_loop_timer_handler);
    StartTimer(MAIN_LOOP_TIMEID, fps2interval(MAIN_LOOP_FPS));
}
