//===========================================================================
//
//  版权所有者： 刘新国，浙江大学计算机科学与技术学院
//                       CAD&CG国家重点实验室
//               xgliu@cad.zju.edu.cn
//  最近修改：2019年5月22日
//            添加菜单展开的矩形，在菜单展开和控件区域重叠是，优先处理菜单。
//  最近修改：2019年2月28日
//            添加了控件的颜色和填充的设置，
//            以及设置颜色的例子（在函数demoGuiALL.c的drawButtons函数里）
//  最近修改：2019年2月26日
//            添加了演示文本编辑演示
//            添加了动画演示
//            添加了textbox 文本输入控件
//            简化了菜单处理
//            改 uiGetInput 为 uiGetMouse,uiGetKey,uiGetKeyboard
//  最近修改：2019年2月18日
//  初次创建：2018年4月，用于<<程序设计专题>>课程教学
//
//===========================================================================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#include <conio.h>
#include <windows.h>
#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

#include "graphics.h"
#include "extgraph.h"
#include "genlib.h"
#include "simpio.h"

#include "imgui.h"


#define KMOD_SHIFT 0x01
#define KMOD_CTRL  0x02

#undef MIN
#undef MAX
#define MIN(x, y) ((x)<(y)? (x): (y))
#define MAX(x, y) ((x)<(y)? (y): (x))


/* 鼠标和空间状态 */
struct {
	double mousex;
	double mousey;
	int    mousedown;
	int    pressedItem;   // item that was clicked
	int    activeMenu;    // 哪个菜单被展开
	int    kbdItem;       // item that takes keyboard
	int    lastItem;      // item that had focus just before
	int    keyPress;      // input key
	int    charInput;     // input char
	int    keyModifiers;  // key modifier (shift, ctrl)
} UIState;

enum {
    MENULIST_MAX = 200,
    MENULIST_TEXT_MAX = 100,
};
struct {
    bool unfold;
    double x, y, w, wlist, h;
    char list[MENULIST_MAX][MENULIST_TEXT_MAX];
    int list_size;
    int ids[MENULIST_MAX];
    int ids_size;
    double unfoldRect[4];
} menuInfo;

/*
 *  libgraphics 预定义的颜色名称
 *
 *  DefineColor("Black", 0, 0, 0);
 *  DefineColor("Dark Gray", .35, .35, .35);
 *  DefineColor("Gray", .6, .6, .6);
 *  DefineColor("Light Gray", .75, .75, .75);
 *  DefineColor("White", 1, 1, 1);
 *  DefineColor("Brown", .35, .20, .05);
 *  DefineColor("Red", 1, 0, 0);
 *  DefineColor("Orange", 1, .40, .1);
 *  DefineColor("Yellow", 1, 1, 0);
 *  DefineColor("Green", 0, 1, 0);
 *  DefineColor("Blue", 0, 0, 1);
 *  DefineColor("Violet", .93, .5, .93);
 *  DefineColor("Magenta", 1, 0, 1);
 *  DefineColor("Cyan", 0, 1, 1);
 */

ItemColor PredefinedColors[] = {
	{"Blue",      "Blue",	"Red",	    "Red",   0 },
	{"Orange",    "Black",  "Green",    "Blue",  0 },
	{"Orange",    "White",  "Green",    "Blue",  1 }, // 填充
	{"Light Gray","Black",  "Dark Gray","Blue",  0 },
	{"Light Gray","Black",  "Dark Gray","Yellow",1 }, // 填充
	{"Brown",     "Red",    "Orange",   "Blue",  0 },
	{"Brown",     "Red",    "Orange",   "White", 1 }, // 填充
};
int const PredefinedColorsMax = sizeof(PredefinedColors)/sizeof(PredefinedColors[0]);

ItemColor DefaultColor = {
	"Blue",      "Blue",	"Red",	    "Red",   0, // 不填充
};
ItemColor MenuColor = {
	"Blue",      "Blue",	"Red",	    "Red",   0, // 不填充
};
ItemColor ButtonColor = {
	"Blue",      "Blue",	"Red",	    "Red",   0 , // 不填充
};
ItemColor TextboxColor = {
	"Blue",      "Blue",	"Red",	    "Red",   0 , // 不填充
};


/* 测试：坐标点(x,y)是否位于包围和 [x1,x2] X [y1,y2] 内部 */
static
bool inBox(double x, double y, double x1, double x2, double y1, double y2)
{
	return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

static
bool mouseInBox(double x, double y, double w, double h)
{
    return inBox(UIState.mousex, UIState.mousey, x, x+w, y, y+h);
}

/**
 * 强制绘制版本
 * 绘制带颜色的文本框，不顾是否和控件重合
 */
static
void _drawBoxColor(double x, double y, double w, double h, char *label,
        char xalign, bool fill,
        char *frameColor, char *labelColor);
static
void _drawLabelColor(double x, double y, char *label, char *color);
static
void _drawRectangleColor(double x, double y, double w, double h, bool fill, char *color);

/**
 * 当前控件是否和菜单下拉列表重叠
 */
static
bool overlapMenu(double x, double y, double w, double h)
{
    if (!menuInfo.unfold)
        return false;
    double eps = 1e-6;
    double x1 = x, x2 = x+w;
    double y1 = y, y2 = y+h;
    return ((MIN(x2, menuInfo.unfoldRect[1]) > eps+MAX(x1, menuInfo.unfoldRect[0]))
            && (MIN(y2, menuInfo.unfoldRect[3]) > eps+MAX(y1, menuInfo.unfoldRect[2])));
}

static
bool mouseInMenu()
{
    return menuInfo.unfold && inBox(UIState.mousex, UIState.mousey,
            menuInfo.unfoldRect[0], menuInfo.unfoldRect[1],
            menuInfo.unfoldRect[2], menuInfo.unfoldRect[3]);
}

static
bool mouseNotInMenu(void)
{
	return !mouseInMenu();
}

static
void addIdToMenuInfo(int id)
{
    if (menuInfo.ids_size >= MENULIST_MAX)
        return;
    menuInfo.ids[menuInfo.ids_size++] = id;
}

#define MENUINFO_ARGSNULL 0, 0, 0, 0, 0, 0, NULL, 0
static
void setMenuInfo(int id, double x, double y, double w, double wlist, double h,
        char **labels, int size)
{
    menuInfo.x = x;
    menuInfo.y = y;
    menuInfo.w = w;
    menuInfo.wlist = wlist;
    menuInfo.h = h;

    menuInfo.unfoldRect[0] = x;
    menuInfo.unfoldRect[1] = x + wlist;
    menuInfo.unfoldRect[2] = y - size * h;
    menuInfo.unfoldRect[3] = y;

    if (0 == id || !labels || size <= 0) {
        menuInfo.unfold = false;
        menuInfo.list_size = 0;
        menuInfo.ids_size = 0;
        return;
    }

    for (int i = 0; i < size; ++i) {
        strncpy(menuInfo.list[i], labels[i], MENULIST_TEXT_MAX);
        menuInfo.list[i][MENULIST_TEXT_MAX-1] = '\0';
    }
    menuInfo.list_size = size;

    menuInfo.ids_size = 0;
    addIdToMenuInfo(id);
}

static
bool isListedMenu(int id)
{
    if (!menuInfo.unfold || menuInfo.ids_size == 0)
        return false;
    for (int i = 1; i < menuInfo.list_size; ++i) {
        if (id == menuInfo.ids[i])
            return true;
    }
    return false;
}

/**
 * 判断的控件是否和活动的 menu list 重叠，
 * 如果重叠重新绘制 menu list
 */

static
void redrawMenu(void)
{
    double x = menuInfo.x;
    double y = menuInfo.y;
    double w = menuInfo.w;
    double h = menuInfo.h;
    double wlist = menuInfo.wlist;
    _drawBoxColor(x, y, w, h, menuInfo.list[0], 'L',
            MenuColor.fill, MenuColor.frame, MenuColor.label);
    for (int i = 1; i < menuInfo.list_size; ++i) {
        char *frameColor = MenuColor.frame;
        char *labelColor = MenuColor.label;
        if (mouseInBox(x, y-i*h, wlist, h)) {
            frameColor = MenuColor.hoverFrame;
            labelColor = MenuColor.hoverLabel;
        }
        _drawBoxColor(x, y-i*h, wlist, h, menuInfo.list[i], 'L',
                MenuColor.fill, frameColor, labelColor);
    }
}

static
bool boxOverlapMenu(int id, double x, double y, double w, double h)
{
    return menuInfo.unfold && overlapMenu(x, y, w, h) && !isListedMenu(id);
}

/**
 * 向量乘
 */
static
double crossProduct(double v1[2], double v2[2])
{
    return v1[0]*v2[1] - v2[1]*v1[0];
}

static
bool isCross(double l1[4], double l2[4])
{
    double v1[2] = {l1[2] - l1[0], l1[3] - l1[1]};
    double v2[2] = {l1[0] - l1[0], l2[1] - l1[1]};
    double v3[2] = {l1[2] - l1[0], l2[3] - l1[1]};
    double eps = 1e-6;
    return crossProduct(v1, v2) * crossProduct(v1, v3) <= eps;
}

static
bool lineCrossLine(double l1[4], double l2[4])
{
    return isCross(l1, l2) && isCross(l2, l1);
}

static
bool lineOverlapMenu(double x1, double y1, double x2, double y2)
{
    if (!menuInfo.unfold)
        return false;

    double line[4] = {x1, y1, x2, y2};
    double mx1 = menuInfo.unfoldRect[0];
    double mx2 = menuInfo.unfoldRect[1];
    double my1 = menuInfo.unfoldRect[2];
    double my2 = menuInfo.unfoldRect[3];
    double menuEdges[4][4] = {
        {mx1, my1, mx2, my1},
        {mx2, my1, mx2, my2},
        {mx2, my2, mx1, my2},
        {mx1, my2, mx1, my1},
    };
    // 直线在矩形内部
    if (inBox(x1, y1, mx1, mx2, my1, my2) && inBox(x2, y2, mx1, mx2, my1, my2))
        return true;
    // 直线和边界相交
    for (int i = 0; i < 4; ++i) {
        if (lineCrossLine(line, menuEdges[i]))
            return true;
    }
    return false;
}


    static
void mySetPenColor(char *color)
{
    if (color && strlen(color) > 0)
        SetPenColor(color);
}

void setDefaultColor(char *frame, char *label, char *hoverFrame, char *hoverLabel, bool fill)
{
    if(frame) strcpy(DefaultColor.frame, frame);
    if(label) strcpy(DefaultColor.label, label);
    if(hoverFrame) strcpy(DefaultColor.hoverFrame, hoverFrame);
    if(hoverLabel) strcpy(DefaultColor.hoverLabel ,hoverLabel);
    DefaultColor.fill = fill;
}

void setButtonColor(char *frame, char *label, char *hoverFrame, char *hoverLabel, bool fill)
{
    if(frame) strcpy(ButtonColor.frame, frame);
    if(label) strcpy(ButtonColor.label, label);
    if(hoverFrame) strcpy(ButtonColor.hoverFrame, hoverFrame);
    if(hoverLabel) strcpy(ButtonColor.hoverLabel ,hoverLabel);
    ButtonColor.fill = fill;
}

void setMenuColor(char *border, char *label, char *hotFrame, char *hotLabel, bool fill)
{
    if(border) strcpy(MenuColor.frame, border);
    if(label) strcpy(MenuColor.label, label);
    if(hotFrame) strcpy(MenuColor.hoverFrame, hotFrame);
    if(hotLabel) strcpy(MenuColor.hoverLabel ,hotLabel);
    MenuColor.fill = fill;
}

void setTextBoxColor(char *frame, char*label, char *hotFrame, char *hotLabel, bool fill)
{
    if(frame) strcpy(TextboxColor.frame, frame);
    if(label) strcpy(TextboxColor.label, label);
    if(hotFrame) strcpy(TextboxColor.hoverFrame, hotFrame);
    if(hotLabel) strcpy(TextboxColor.hoverLabel ,hotLabel);
    TextboxColor.fill = fill;
}

ItemColor *getDefaultColor(void)
{
    return &DefaultColor;
}

ItemColor *getMenuColor(void)
{
    return &MenuColor;
}

ItemColor *getTextBoxColor(void)
{
    return &TextboxColor;
}

void usePredefinedColor(int k)
{
    int N = PredefinedColorsMax;
    DefaultColor = PredefinedColors[k%N];
    MenuColor    = PredefinedColors[k%N];
    ButtonColor  = PredefinedColors[k%N];
    TextboxColor = PredefinedColors[k%N];
}
void usePredefinedButtonColor(int k)
{
    int N = PredefinedColorsMax;
    ButtonColor  = PredefinedColors[k%N];
}
void usePredefinedMenuColor(int k)
{
    int N = PredefinedColorsMax;
    MenuColor    = PredefinedColors[k%N];
}
void usePredefinedTexBoxColor(int k)
{
    int N = PredefinedColorsMax;
    TextboxColor = PredefinedColors[k%N];
}

/* 函数名：	initGUI
 *
 * 功能：初始化工作
 *
 * 用法：在窗口创建或字体设置之后调用
 */
void initGUI()
{
    memset(&UIState, 0, sizeof(UIState));

    menuInfo.unfold = false;
    menuInfo.list_size = 0;
    menuInfo.ids_size = 0;
}

/* 调用该函数,得到鼠标的状态 */
void uiGetMouse(int x, int y, int button, int event)
{
    UIState.mousex = ScaleXInches(x);/*pixels --> inches*/
    UIState.mousey = ScaleYInches(y);/*pixels --> inches*/

    switch (event) {
    case BUTTON_DOWN:
        UIState.mousedown = 1;
        break;
    case BUTTON_UP:
        UIState.mousedown = 0;
        break;
    }
}

/* 调用该函数,得到键盘的输入 */
void uiGetKeyboard(int key, int event)
{
    if (event == KEY_DOWN) {
        switch (key) {
        case VK_SHIFT:
            UIState.keyModifiers |= KMOD_SHIFT;
            break;
        case VK_CONTROL:
            UIState.keyModifiers |= KMOD_CTRL;
            break;
        default:
            UIState.keyPress = key;
        }
    } else if (event==KEY_UP) {
        switch (key) {
        case VK_SHIFT:
            UIState.keyModifiers &= ~KMOD_SHIFT;
            break;
        case VK_CONTROL:
            UIState.keyModifiers &= ~KMOD_CTRL;
            break;
        default:
            UIState.keyPress = 0;
        }
    }
}

/* 调用该函数,得到文本输入 */
void uiGetChar(int ch)
{
    UIState.charInput = ch;
}


/*
 * 函数名：button
 *
 * 功能：显示一个按钮（button）
 *
 * 用法：if (button(GenUUID(0), x, y, w, h, label)) {
 *           执行语句，响应用户按下该按钮
 *       }
 *
 *   id  - 唯一号
 *   x, y - 位置
 *   w, h - 宽度和高度
 *   label - 按钮上的文字标签
 *
 * 返回值
 *   GUI_NONE(0)  - 没有操作
 *   GUI_CLICK(1) - 点击按钮
 */
int button(int id, double x, double y, double w, double h, char *label)
{
    char *frameColor = ButtonColor.frame;
    char *labelColor = ButtonColor.label;
    double movement = 0.2*h;
    double shrink = 0.15*h;
    double sinkx = 0, sinky = 0;

    if (mouseNotInMenu() && mouseInBox(x, y, w, h)) {
        frameColor = ButtonColor.hoverFrame;
        labelColor = ButtonColor.hoverLabel;
        if (UIState.mousedown) {
            UIState.pressedItem = id;
            sinkx =   movement;
            sinky = - movement;
        }
    } else {
        if (UIState.pressedItem == id)
            UIState.pressedItem = 0;
    }

    // If no widget has keyboard focus, take it
    if (UIState.kbdItem == 0)
        UIState.kbdItem = id;
    // If we have keyboard focus, we'll need to process the keys
    if (UIState.kbdItem == id && UIState.keyPress == VK_TAB) {
        // If tab is pressed, lose keyboard focus.
        // Next widget will grab the focus.
        UIState.kbdItem = 0;
        // If shift was also pressed, we want to move focus
        // to the previous widget instead.
        if (UIState.keyModifiers & KMOD_SHIFT)
            UIState.kbdItem = UIState.lastItem;
        UIState.keyPress = 0;
    }
    UIState.lastItem = id;

    // draw the button
    _drawBoxColor(x+sinkx, y+sinky, w, h, label, 'C', ButtonColor.fill,
            frameColor, labelColor);

    // 画键盘提示, show a small ractangle frane
    if (UIState.kbdItem == id) {
        _drawRectangleColor(x+sinkx+shrink, y+sinky+shrink, w-2*shrink, h-2*shrink,
                false, labelColor);
    }

    int clicked = 0;
    // must be clicked before, but now mouse button is up
    if (UIState.pressedItem == id && !UIState.mousedown) {
        UIState.pressedItem = 0;
        UIState.kbdItem = id;
        clicked = 1;
        UIState.activeMenu = 0;      // menu lose focus
        setMenuInfo(MENUINFO_ARGSNULL);
    }

    if (boxOverlapMenu(id, x, y, w, h))
        redrawMenu();

    return clicked;
}

/*
 * 显示一个菜单项
 *   id  - 菜单项的唯一号
 *   x,y - 菜单项的位置
 *   w,h - 菜单项的大小
 *   label - 菜单项的标签文字
 *
 * 返回值
 *   GUI_NONE(0)  - 用户没有点击（按下并释放）此菜单项
 *   GUI_CLICK(1) - 点击了此菜单项
 */
static int menuItem(int id, double x, double y, double w, double h, char *label)
{
    char *frameColor = MenuColor.frame;
    char *labelColor = MenuColor.label;
    if (mouseInBox(x, y, w, h)
            && (!overlapMenu(x, y, w, h) || isListedMenu(id))) {
        frameColor = MenuColor.hoverFrame;
        labelColor = MenuColor.hoverLabel;
        if ((UIState.pressedItem == id || UIState.pressedItem == 0)
                && UIState.mousedown) {
            UIState.pressedItem = id;
        }
    } else {
        if (UIState.pressedItem == id)
            UIState.pressedItem = 0;
    }

    _drawBoxColor(x, y, w, h, label, 'L', MenuColor.fill, frameColor, labelColor);
    if (boxOverlapMenu(id, x, y, w, h))
        redrawMenu();

    // must be pressed before, but now mouse button is up
    if (UIState.pressedItem == id && !UIState.mousedown) {
        UIState.pressedItem = 0;
        UIState.kbdItem = 0;    // 输入框失去焦点
        return GUI_CLICK;
    }

    return GUI_NONE;
}

/*
 * 函数名：shortcutkey
 *
 * 功能：从菜单标签中提取“快捷键”大写字母
 *       要求快捷键标志 Ctrl-X 位于标签的结尾部分
 */
static char ToUpperLetter(char c)
{
    return (c>='a' && c<='z' ? c-'a'+'A' : c);
}

static char shortcutkey(char *s)
{
    char predStr[] = "Ctrl-";
    int M = strlen(predStr)+1;
    int n = s ? strlen(s) : 0;

    if (n < M || strncmp(s+n-M, predStr, M-1) != 0)
        return 0;

    return ToUpperLetter(s[n-1]);
}

/*
 * 函数名：menuList
 *
 * 功能：显示一个组菜单
 *
 * 用法：choice = menuList(GenUUID(0),x,y,w,h,labels,n);
 *
 *   id  - 菜单的唯一号
 *   x,y,w,h - 菜单类别的位置和大小
 *   wlist,h - 菜单列表的宽度和高度
 *   labels[] - 标签文字，[0]是菜单类别，[1...n-1]是菜单列表
 *   n   - 菜单项的个数
 *
 * 返回值
 *   -1    - 用户没有点击（按下并释放）按钮
 *   >=0   - 用户选中的菜单项 index （在labels[]中）
 *
 */
int menuList(int id, double x, double y, double w, double wlist, double h, char *labels[], int n)
{
    // 处理快捷键
    if (UIState.keyModifiers & KMOD_CTRL) {
        for (int k = 1; k < n; k++) {
            int kp = ToUpperLetter(UIState.keyPress);
            // 成功匹配快捷键
            if (kp && kp == shortcutkey(labels[k])) {
                UIState.keyPress = 0;
                menuInfo.unfold = false;
                return k;
            }
        }
    }

    // 处理鼠标
    if (mouseInBox(x, y, w, h) && !overlapMenu(x, y, w, h))
        UIState.activeMenu = id;

    if (menuItem(id, x, y, w, h, labels[0]))
        menuInfo.unfold = !menuInfo.unfold;

    int selected = -1;
    if (UIState.activeMenu == id && menuInfo.unfold) {
        UIState.charInput = 0; // disable text editing
        UIState.keyPress = 0;  // disable text editing
        setMenuInfo(id, x, y, w, wlist, h, labels, n);
        for (int k = 1; k < n; k++) {
            int list_id = GenUUID(k)+id;
            addIdToMenuInfo(list_id);
            if (menuItem(list_id, x, y-k*h, wlist, h, labels[k])) {
                UIState.activeMenu = 0;
                setMenuInfo(MENUINFO_ARGSNULL);
                selected = k;
            }
        }
    }

    if (boxOverlapMenu(id, x, y, w, h))
        redrawMenu();

    return selected;
}

void drawMenuBar(double x, double y, double w, double h)
{
    drawRectangleColor(x, y, w, h, MenuColor.fill, MenuColor.frame);
    drawRectangleColor(x, y, w, h, 0, MenuColor.label);
}


/*
 * 函数名：textbox
 *
 * 功能：显示一个文本编辑框，显示和编辑文本字符串
 *
 * 用法：textbox(GenUUID(0),x,y,w,h,textbuf,buflen);
 *       或者
 if( textbox(GenUUID(0),x,y,w,h,textbuf,buflen) ) {
 *           文本字符串被编辑修改了，执行相应语句
 *       }
 *
 *   id  - 唯一号，一般用GenUUID(0), 或用GenUUID（k)（k是循环变量）
 *   x,y - 文本框位置
 *   w,h - 文本框的宽度和高度
 *   textbuf - 被编辑的文本字符串（以\0结尾）
 *   buflen - 可存储的文本字符串的最大长度
 * 返回值
 *   GUI_NONE(0)   - 文本没有被编辑
 *   GUI_TYPE(6)   - 被编辑了
 *   GUI_ACCEPT(7) - 按了回车，文本被确认
 */
int textbox(int id, double x, double y, double w, double h, char textbuf[], int buflen)
{
    char * frameColor = TextboxColor.frame;
    char * labelColor = TextboxColor.label;
    int state = GUI_NONE;
    int len = strlen(textbuf);
    double indent = GetFontAscent()/2;
    double textPosY = y+h/2-GetFontAscent()/2;

    if (mouseNotInMenu() && mouseInBox(x, y, w, h)) {
        frameColor = TextboxColor.hoverFrame;
        labelColor = TextboxColor.hoverLabel;
        if (UIState.mousedown) {
            UIState.pressedItem = id;
        }
    }

    // If no widget has keyboard focus, take it
    if (UIState.kbdItem == 0)
        UIState.kbdItem = id;

    if (UIState.kbdItem == id)
        labelColor = TextboxColor.hoverLabel;

    // Render the text box
    _drawRectangleColor(x, y, w, h, TextboxColor.fill, frameColor);
    // show text
    mySetPenColor(labelColor);
    MovePen(x+indent, textPosY);
    DrawTextString(textbuf);
    // add cursor if we have keyboard focus
    if (UIState.kbdItem == id && ((clock()>>8) & 1)) {
        //MovePen(x+indent+TextStringWidth(textbuf), textPosY);
        DrawTextString("_");
    }

    // If we have keyboard focus, we'll need to process the keys
    if (UIState.kbdItem == id) {
        switch (UIState.keyPress) {
        case VK_RETURN:
            state = GUI_ACCEPT;
        case VK_TAB:
            // lose keyboard focus.
            UIState.kbdItem = 0;
            // If shift was also pressed, we want to move focus
            // to the previous widget instead.
            if ( UIState.keyModifiers & KMOD_SHIFT )
                UIState.kbdItem = UIState.lastItem;
            // Also clear the key so that next widget won't process it
            UIState.keyPress = 0;
            break;

        case VK_BACK:
            if (len > 0) {
                textbuf[--len] = 0;
                state = GUI_TYPE;
            }
            UIState.keyPress = 0;
            break;
        }

        // char input
        if (UIState.charInput >= 32 && UIState.charInput < 127 && len+1 < buflen) {
            textbuf[len] = UIState.charInput;
            textbuf[++len] = 0;
            UIState.charInput = 0;
            state = GUI_TYPE;
        }
    }

    UIState.lastItem = id;

    // must be clicked before, but now mouse button is up
    if (UIState.pressedItem == id && !UIState.mousedown) {
        UIState.pressedItem = 0;
        UIState.kbdItem = id;
        UIState.activeMenu = 0;         // menu lose focus
        setMenuInfo(MENUINFO_ARGSNULL);
    }

    if (boxOverlapMenu(id, x, y, w, h))
        redrawMenu();

    return state;
}


/* 画一个矩形 */
void _drawRectangleColor(double x, double y, double w, double h, bool fill, char *color)
{
    mySetPenColor(color);

    MovePen(x, y);
    if (fill) StartFilledRegion(1);

    DrawLine(0, h);
    DrawLine(w, 0);
    DrawLine(0, -h);
    DrawLine(-w, 0);

    if (fill) EndFilledRegion();
}
void drawRectangleColor(double x, double y, double w, double h, bool fill, char *color)
{
    _drawRectangleColor(x, y, w, h, fill, color);
    if (boxOverlapMenu(0, x, y, w, h))
        redrawMenu();
}

void drawRectangle(double x, double y, double w, double h, bool fill)
{
    drawRectangleColor(x, y, w, h, fill, DefaultColor.frame);
}

/* 画直线 */
    static
void _drawLineColor(double x1, double y1, double x2, double y2, char *color)
{
    MovePen(x1, y1);
    SetPenColor(color);
    DrawLine(x2 - x1, y2 - y1);
}

void drawLine(double x1, double y1, double x2, double y2)
{
    drawLineColor(x1, y1, x2, y2, getDefaultColor()->label);
}

void drawLineColor(double x1, double y1, double x2, double y2, char *color)
{
    _drawLineColor(x1, y1, x2, y2, color);
    if (lineOverlapMenu(x1, y1, x2, y2))
        redrawMenu();
}

/* 显示字符串标签 */
void _drawLabelColor(double x, double y, char *label, char *color)
{
    if (!label || strlen(label) <= 0 || !color)
        return;
    mySetPenColor(color);

    MovePen(x, y + GetFontDescent());
    DrawTextString(label);
}

void drawLabelColor(double x, double y, char *label, char *color)
{
    _drawLabelColor(x, y, label, color);
    if (boxOverlapMenu(0, x, y, TextStringWidth(label), GetFontHeight()))
        redrawMenu();
}


void drawLabel(double x, double y, char *label)
{
    drawLabelColor(x, y, label, DefaultColor.label);
}

/* 画一个矩形，并在其内部居中显示一个字符串标签label */
void _drawBoxColor(double x, double y, double w, double h, char *label,
        char xalign, bool fill,
        char *frameColor, char *labelColor)
{
    if (!label || strlen(label) <= 0 || !frameColor || !labelColor)
        return;
    double fa = GetFontAscent();
    // 矩形
    _drawRectangleColor(x, y, w, h, fill, frameColor);
    // 边框
    _drawRectangleColor(x, y, w, h, 0, labelColor);
    // 文本内容
    mySetPenColor(labelColor);
    if (xalign == 'L' )
        MovePen(x + fa/2, y + h/2 - fa/2);
    else if (xalign == 'R')
        MovePen(x+w-fa/2-TextStringWidth(label), y+h/2-fa/2);
    else // if( labelAlignment=='C'
        MovePen(x+(w-TextStringWidth(label))/2, y+h/2-fa/2);
    DrawTextString(label);
}

void drawBoxColor(double x, double y, double w, double h, char *label,
        char xalign, bool fill,
        char *frameColor, char *labelColor)
{
    _drawBoxColor(x, y, w, h, label, xalign, fill, frameColor, labelColor);
    if (boxOverlapMenu(0, x, y, w, h))
        redrawMenu();
}

void drawBox(double x, double y, double w, double h, char *label,
        char xalign, bool fill)
{
    drawBoxColor(x, y, w, h, label, xalign, fill,
            DefaultColor.frame, DefaultColor.label);
}
