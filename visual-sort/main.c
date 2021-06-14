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
 * ��ں�����ֻ�ᱻ����һ��
 * ���幤����ʽͨ��ע��ʱ�䵽 libgraphics ��������
 */
void Main()
{
    InitGraphics(); // ��ʼ�� ImGUI ʹ�õĵײ��ͼ��
    initGUI();      // ��ʼ�� ImGUI �ؼ���

    initalize();        // ��ʼ�����������Դ
    atexit(destroy);    // �����˳�ʱ���ٸ�����Դ

    //InitConsole();  // ��ʼ������̨
    SetWindowTitle("Visual Sort");

    RegisterCharEvent(uiGetChar);           // GUI �ַ�����
	RegisterKeyboardEvent(uiGetKeyboard);   // ����
    RegisterMouseEvent(uiGetMouse);         // ����¼�
    RegisterTimerEvent(main_loop_timer_handler);
    StartTimer(MAIN_LOOP_TIMEID, fps2interval(MAIN_LOOP_FPS));
}
