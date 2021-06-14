# visual-sort
基于libgraphics和imgui的排序可视化算法

### 术语说明

sorter：指的是排序算法对象，sorter是一个结构体，其包含了一些字段和函数指针，不同的排序算法都要包含这个结构体，同时实现相关函数并给函数指针赋值，以实现某种多态性。本程序实现了五个具体的排序算法，冒泡排序 bubble sorter、快速排序 quick sorter、归并排序 merge sorter、插入排序 insertion sorter和选择排序 select sorter，同时可以方便添加其他排序算法，只要符合接口即可。具体见后文数据结构设计。 

view：指的是视图，界面看见的部分称为 view，程序支持多个视图，通过函数 view_switch(view_name) 切换到不同的视图，类似与手机页面的切换。同时，view结构体也类似上述排序对象作为一个基类，本程序具体实现了 main-view 视图和 about-view 视图。前者是主要的排序界面，后者是程序的关于说明的页面。 

menu: 指的是菜单栏或者菜单下拉框。

## 总体框架设计

![图片 1](https://tva1.sinaimg.cn/large/008i3skNly1grhwgmxbtqj30o20foq5p.jpg)

## 功能模块设计

为何实现高内聚低耦合，主要划分了视图模块、排序算法模块、通用工具模块和设置模块。

视图模块主要是由view.h、view.c、main-view.h、main-view.c、about-view.h和about-view.c构成。view.h和view.c分别是视图的基础结构和接口，以及视图操作的基本函数的实现；main-view.h和main-view.c是主视图（也就是排序页面）的实现；about-view.h和about-view.c是关于视图的实现，主要内容是作者名单。当然，后续可以继续添加新的视图，只需要实现view.h中的接口就可以。

排序模块有sorter.h、sorter.c、bubble-sorter.h、bubble-sorter.c、quick-sorter.h、quick-sorter.c、select-sorter.h、select-sorter.c、merge-sorter.h、merge-sorter.c、insertion-sorter.h、insertion-sorter.c文件构成，后续可以继续添加其他排序算法的可视化。和view.h一样，sorter.h、sorter.c是排序算法的基础结构和共同接口，bubble-sorter.c是冒泡排序的实现，quick-sorter.c是快速排序算法的实现，select-sorter.c是选择排序的实现，merge-sorter.c是归并排序的实现，insertion-sorter.c是插入排序的实现。

通用工具模块是 utility.h和utility.c实现，主要包括了一些通用的算法，比如最大值、最小值和随机数生成等算法的实现。

设置模块是 setting.h，主要定义了程序的一些基本属性，比如窗口大小等。

最后是主模块，也叫入口模块，只有一个文件main.c，里面是一些初始化工作，注册各个视图，通过定时器驱动页面的刷新等。

## 数据结构设计

主要的数据结构有视图相关的`` view_t`、`main_view_t`和`about_view_t`、排序算法相关的`sorter_t`、`bubble_sorter_t`、`quick_sorter_t`、`merge_sorter_t`、`insertion_sorter_t`和`select_sorter_t`。

`view_t `结构体定义如下：

```c
struct view_t {
    char const* name;
    void (*display)(view_t *me);
    void (*destroy)(view_t *me);
};
```

其中``name`是视图名字，后续通过此名字来确定视图。`display`是函数指针，指向一个所谓的显示函数，用于把确定如何把此视图内容呈现到屏幕上，`destroy`是也是函数指针用于释放自定义视图的内存。具体如何工作起来的见下文对`main_view`和`about_view_t`的分析。

`main_view_t`这是最为主要的视图，也就是排序页面的视图，定义如下：

```c
enum {
    DATASRC_RANDOM = 1,
    DATASRC_ASCEND,
    DATASRC_DESCEND,
    DATASRC_LOADFILE,
};
enum { SIZE_TEXT_MAX = 64 };
typedef struct main_view_t {
    view_t base;
    sorter_t *sorter;
    int src_idx;
    double *nums;
int size;
int type;
    char size_text[SIZE_TEXT_MAX];
} main_view_t;
```

可以看见``main_view_t`结构体内嵌了`view_t`结构体，取名为` base `意味基类，这样可以让`main_view_t`具有`view_t`的性质；`sorter`的指针指向一个排序算法结构体，用于表明当前使用什么排序算法，具体来说可能是冒泡排序、快速排序、选择排序、归并排序、插入排序。`src_idx`是用于表明数据来源选择的是哪一个，取值为`DATASRC_RANDOM`表示随机生成，`DATASRC_ASCEND`表示随机生成升序数据，`DATASRC_DESCEND`表示随机生成降序数据，`DATASRC_LOADFILE`表示数据从文件加载，具体从当前目录下的“input.txt”读取数据；`nums`指针和`size`表明当前生成或加载的数据数组的地址和大小；`type`表示数据类型，可以取值为`SORTER_INT` 表明数据类型是整数，`SORTER_FLOAT`表示数据类型是浮点数，`SORTER_DOUBLE`表示数据类型是双精度浮点数；`size_text`是为了从界面上交互显示并输入`size`大小的缓冲区，最大长度为`SIZE_TEXT_MAX`长。这里常量使用 `enum`而不是通常的`#define`宏来定义，是为了方便编译后方便调试时具有符号，而不是简简单单的数字。

`about_view_t`结构体很简单

```c
typedef struct about_view_t {
    view_t base;
} about_view_t;
/*
和main_view_t结构体一样，不做介绍
*/
```

`sorter_t`排序算法结构体，思想和`view_t`结构体思想一致

```c
enum {
    NUMS_SIZE_MIN = 10,  // 自动生成的数据最少个数
    NUMS_SIZE_DFT = 30,  // 默认生成的数据个数
    NUMS_SIZE_MAX = 1024,// 最多的数据个数
};
enum {
    SORTER_UNREADY = 0,
    SORTER_READY,
    SORTER_SORTING,
    SORTER_PAUSED,
    SORTER_SORTED,
};
enum {
    SORTER_INT = 0,
    SORTER_FLOAT,
    SORTER_DOUBLE,
    SORTER_TYPE_MAX,
};
struct sorter_t {
    double *nums;
    double *nums_back;
    int size;
    double min, max;
    int type;

    double speed;
    int frame_cnt;
    int state;
    bool ascend;

    void (*next_step)(sorter_t *me);
    void (*display)(sorter_t *me, double x, double y, double w, double h);
    void (*restart)(sorter_t *me);
    void (*destroy)(sorter_t *me);
    bool (*save_state)(sorter_t *me, char const *ofile);
};
```

`nums`和``size`都是用于记录排序的数据的信息的，`nums_back`是对未排序前的`nums`的数据的备份，用于实现重新排序的功能。正如注释提到的一样 `nums `指向的内存所有权不是`sorter_t`的而是外部的，需要外界释放内容，但是`nums_back`是此结构体分配的，所以需要自己释放。`min`和`max`分别记录数据中的最大值和最小值，用于后面绘图时使用。type表示被排序的数据类型取值为`SORTER_INT`、`SORTER_FLOAT`、`SORTER_DOUBLE`分别表示int类型，float类型和double类型的数据。

`speed`记录可视化排序过程的快慢，每秒多少步。``frame_cnt`记录从上一步到现在界面刷新了多少帧（多少次画面）。`state`记录排序算法的状态，取值有`SORTER_UNREADY`，表示排序算法为准备好，也就是数据没有被提供；`SORTER_READY`表示排序算法准备好了，下一步可以进行排序；`SORTER_SORTING`表示算法正在排序过程中；`SORTER_PAUSED`表示排序算法被暂停，后续可以恢复继续排序；`SORTER_SORTED`表示排序过程结束，数据已经排好序了。`ascend`表示排序的顺序，升序还是降序，当`ascend`为真表示按照升序排序（也是默认排序方式），当`ascend`为假表示按照降序排序。

函数指针`next_step`用于记录子结构的实现下一步操作的函数的地址；`display`用于实现如何可视化排序的内部情况；`restart`用于重新操作结构体内部变量，实现重新排序的功能；`destroy`用于记录如何释放结构体内存；`save_state`用于存储当前状态到文件里。

bubble_sorter_t结构体是sorter_t的子结构体，是对冒泡排序算法的具体实现：

```c
typedef struct bubble_sorter_t {
    sorter_t base;

    int k;  // 排序好了多少个元素
    int i;  // 当前这一轮进行到哪个元素
} bubble_sorter_t;
```

`base`是排序算法的基类，k和i变量的含义正如注释里写的一样。



快速排序是一个比较复杂的排序算法，所以其对应结构体也较为复杂：

```c
enum { STACK_MAX = 1024 };
typedef struct quick_sorter_t {
    sorter_t base;
    int stack[STACK_MAX][2];
    int stack_size;
    int done_list[NUMS_SIZE_MAX];
    int done_size;
    int state;
    int lo, hi;
    int l, r;
    int pivot_idx;
} quick_sorter_t;
```

同样的`base`是基类结构体，二维数据`stack`用于记录递归的情况，最大递归深度`STACK_SIZE`为1024，`stack[i][0]`表示递归深度为i时左边界，``stack[i][1]``表示递归深度为i时右边界。``done_list`和`done_size`用于记录哪些下标的元素已经被排好序了，方便可视化时特别展示出来。`state`同样用于记录快速排序的内部状态。`lo`和`hi`记录当前轮快速排序正在处理的元素的左右边界下标分别是多少，`l`和`r`时记录元素划分时左边和右边的下标，`pivot_idx`记录枢纽元的下标是多少。



`select_sorter_t`对应的是选择排序

```c
typedef struct select_sorter_t {
    sorter_t base;

    int k;  // 排序好了多少个元素
int i;  // 当前这一轮进行到哪个元素
int max;  // 当前这一轮最值下标
} select_sorter_t;
```

比起冒泡排序多了一个`max`值用于标记当前这轮所确定的最值下标



`merge_sorter_t`结构体对应的是归并排序，具体如下

```c
typedef struct merge_sorter_t {
    sorter_t base;

    int doing_size;   
double doing_list[NUMS_SIZE_MAX];   
int start,end,mid;  
int state;
int i,j,k;
} merge_sorter_t;
```

与上述结构相同`base`是基类结构体，`doing_size`记录正在排序的元素个数，`doing_list[]`用于记录正在排序的元素，`start`,`end`,`mid`用于标记正在排序部分的始末和中间下标，`state`用于记录归并排序的内部状态，`i`，`j`，`k`用于记录当前正在比较的元素下标。



`insertion_sorter_t`结构体对应的是插入排序，具体如下

```c
typedef struct insertion_sorter_t {
    sorter_t base;
  int k;
  int i;  
  int flag; 
  int state;
double doing_list[NUMS_SIZE_MAX];
} insertion_sorter_t;
```

与上述结构相同`base`是基类结构体，`k`记录当前排序元素移动的位置，`i`记录当前排序元素的起始下标，`flag`记录绘制数据的状态，`state`用于记录插入排序的内部状态，`doing_list[]`用于记录正在排序的元素。

### 多文件构成机制

为了实现多文件编译，每个头文件都定义了`#ifndef FILENAME_H`__，`#define FILENAME_H__`和`#endif`宏，把正常内容包含其中以实现每次`#include`只被引入一次，以防止出现重复声明和定义。同时每个*.c文件都有对应的*.h文件，里面声明了可以从*.c中使用的外部函数的原型，如果是 *.c 文件中的私有函数（不希望外部可以使用的函数）加上 static 限制作用域为本*.c文件。如果是*.h头文件中定义的变量，通常都是希望是私有的所以有static修饰作用域为本文件以防止出现链接错误。对于内联函数比如`void swap(double *a, double \*b)`也是用`static`修饰，以防止出现链接时多次定义，本程序没有使用外部变量，所以不需要外部变量的链接问题。虽然在\*.c文件中定义了外部变量，但是这些变量都是默认为本文件内使用，所以不会被其他文件所感知到。

# 函数设计描述

### main.c文件中函数的说明：

 `void Mian(void);`

此函数主要完成初始化工作，包括调用 InitGraphic()以初始化graphics环境，调用 initGUI完成ImGUI库的初始化，调用 initialize() 函数完成本程序其他模块的初始化，注册destroy()函数到程序退出时候以释放资源，设置窗口标题为 Visual Sort，把ImGUI的uiGetChar()、uiGetKeyboard()、uiGetMouse()注册到graphic库中，以获得鼠标键盘等情况，同时注册一个定时器id为1的定时器。

 

`  static void main_loop_timer_handler(int timeid);`

函数首先调用DisplayClear()函数清屏，然后调用 view_display()函数显示当前活动的view页面到屏幕上。

timeid: 是此函数注册到定时器id

 

  `static void initialize(void);`

创建并添加两个视图 main 和 about到视图管理模块中，并设置默认主题颜色为4号

 

  `static void destroy(void);`

调用 view_destroy() 释放所有视图的内存

 

view.c文件中函数的说明：在这个文件中有三个文件内作用域的全局变量 view_t *views[VIEWS_MAX]、 int view_cnt和    view_t *active，第一个和第二个变量共同记录所有的视图，active指向当前活动的视图。

  `static void view_lookup_by_name(char const *name);`

查找视图名字为name的视图的下标，没找到返回 VIEW_NOTFOUND (-1)。

name: 需要查找的视图的名字

 

 ` int view_default(char const *name);`

设置默认活动的视图，内部间接调用 view_switch(name) 实现，具体见下面分析。

name: 视图的名字

 

 ` void view_display(void);`

调用当前活动的视图的display()，在屏幕上显示视图的内容。

 

 ` int view_switch(char const *name);`

切换名为name的视图为活动视图，内部使用 view_lookup_by_name(name) 找到视图下标，然后修改 active = views[idx] 以便设置为活动视图。

name: 视图名字

返回值：VIEW_NOTFOUND 名为name的视图没有找到

​    VIEW_OK 切换成功

 

 ` int view_add(view_t *view);`

通过查找 views 是否有空余位置，然后把 view 添加到里面去。

view: 待添加的视图

返回值：VIEW_INVALID 视图 view 为空指针，不合法

​    VIEW_TOOMANY 添加的视图过多

​    VIEW_OK 添加视图成功

 

  `static void view_remove_by_idx(int idx);`

移除下标为 idx 处的视图，并调用其注册的销毁函数，同时把 views[idx] 置为NULL

idx: 视图的下标

 

  `int view_remove(char const *name);`

通过使用 view_lookup_by_name(name) 找到视图的索引 idx，然后使用 view_remove_by_idx(idx)删除这个视图。

name: 视图名字

返回值：VIEW_NOTFOUND 名为name视图没有找到

​    VIEW_OK 删除成功

 

  `void view_destroy(void);`

销毁所有记录在views里的视图，并置view_cnt为0，active为NULL

 

### main-view.c中函数的说明：

  `view_t *main_view_create(char const *name);`

创建名为 name 的 main_view_t 类型结构体，内部会为 main_view_t 分配空间并初始化，包括初始化基类 view_t，设置 view_t 结构体里的display和destroy函数指针为本文定义的display和 destroy函数。设置默认的排序算法为冒泡排序。

name: 视图名字

返回值：main_view_t 结构体内部成员基类 base 的地址，这个类型是 view_t 类型的

 

  `static void display(view_t *b);`

显示视图，对于main_view_t 类型而言，首先调用 container_of从b计算出子类的地址赋值给me变量。然后分别调用menubar_display()、leftside_column_display()、drawLine()显示界面的菜单栏、左边控制部分界面和左右部分分界线。之后使用sorter_update(me->sorter)更新排序算法的内部，随后调用sorter->display()把排序可视化到屏幕。

b: 视图的基类，main_view_t内包含的base成员的地址

 

  `static void destroy(view_t *b);`

释放生成的数据，释放sorter结构体，释放本结构所占的内存。

b: 视图的基类，main_view_t内包含的base成员的地址

 

  `static void leftside_column_display(main_view_t *me, double x, double y, double w, double h);`

显示左侧控制按钮。内部调用了data_source_display()显示获取数据的控件，显示了选择算法的下拉菜单，排序顺序的按钮，控件排序速度的控件，排序状态、控制状态和保存状态的按钮。同时这些按钮根据排序状态的不同可能是不可以交互的，比如当数据没有准备好时，排序按钮不会显示，当排序进行过程中，选择数据源和排序算法的下拉菜单不可以交互，直到重新排序或者排序完成才可以重新交互。

me: main_view_t 类型的结构体地址

x, y, w, h: 在坐标(x,y)处显示，长宽分别是w, h英寸

 

  `static double data_source_display(main_view_t *me, double x, double y, double w, double h);`

首先显示“Load Saved State”的按钮，如果点击此按钮就会尝试从“sorter-staet.bin”文件加载保存的排序状态，并加载。然后显示数据来源的下拉菜单，如果选择随机生成还会显示文本框提示数据数据量大小，如果时选择从文件中加载那么尝试自动加载名为“input.txt”的文件内的数据。

me: main_view_t 类型的结构体地址

x, y, w, h: 在坐标(x,y)处显示，长宽分别是w, h英寸

返回值：控件下边界的y坐标

 

  `static void control_buttons_display(main_view_t *me, double x, double y, double w, double h);`

显示减速、加速、下一步和速度值的按钮。

me: main_view_t 类型的结构体地址

x, y, w, h: 在坐标(x,y)处显示，长宽分别是w, h英寸

 

  `static int double_less(void const *a, void const *b);`

给qsort函数使用的回调函数，返回 a – b的值，用于按照从小到大排序

a：指向第一个数的地址

b：指向第二个数的地址

返回值：<0：a < b

​     0：a == b

​    \>0：a > b

  `static int double_great(void const *a, void const *b);`

同 int_less，只是此函数按照从大到小排序，等价于调用 int_less(b, a)

 

  `static bool generate_nums(main_view_t *me);`

根据me->alg_idx决定生成什么类型的数据，如果时DATASRC_RANDOM则调用random_nums(0, 4*size, size)生成me中指定数据量为size的数据，如果为DATASRC_ASCEND或DATASRC_DESCEND再调用qsort()对数据排序；如果是DATASRC_LOADFILE，那么使用load_nums()从文件“input.txt”中加载数据，然后使用sorter_set_nums()给排序算法添加数据。

me: main_view_t类型的结构体指针

返回值：true：操作成功

​    false：操作失败

 

### about-view.c中函数的说明：

  `static void display(view_t *me);`

和main-view.c中的display类似，显示关于视图到屏幕上，具体显示的内容为作者列表。

me: about_view_t结构体内base成员的地址

 

 ` view_t *about_view_create(void);`

和main_view_create类似，只是给destroy成员赋值为VIEW_DESTROY_DFT，表明不需要特殊的释放内存方式，使用view_t内部默认的释放内存方式即可。

返回值： about_view_t 结构体成员base的地址

 

### sorter.c中函数的说明：

  `void sorter_update(sorter_t *me);`

如果当前排序状态不是SORTER_SORTING，直接返回。

让me->frame_cnt加一，如果frame_cnt乘以帧与帧之间间隔的时间大于等于1000除以速度时，调用me->next_step(me)更新到下一步。

me: 排序算法结构体指针

 

  `void sorter_speed_ctl(sorter_t *me, bool inc);`

如果inc为真，增加速度最大值时SPEED_MAX(50)，如果inc为假，减少速度值，最小值为SPEED_MIN。

me: 排序算法结构体指针

inc: true增加速度，false减少速度

 

  `sorter_t *sorter_construct(sorter_t *me);`

初始化me结构体的成员，包括设置速度为默认值SPEED_DFT、frame_cnt为0，状态为SORTER_UNREADY、排序按照升序排序等。

me: 排序算法结构体指针

返回值：me

 

  `sorter_t *sorter_load_from(sorter_t *me, FILE *istream);`

从文件输入流istream中读取数据并解析校验通过后，给me结构体进行初始化。首先读取被排序的元素的个数size，然后依次读取已经被部分排序的素组和原始数据的数组，然后再读取速度speed、帧计数frame_cnt、状态state、升序与否ascend，之后计算出最大值和最小值给me初始化。整个过程成功返回me，失败返回空指针。

me: 排序算法结构体指针

istream: 包含有待待解析的数据的输入流

返回值：me：读取并初始化成功

   NULL：读取或者初始化失败

 

  `void sorter_deconstruct(sorter_t *me);`

释放me结构体中nums和nums_back的空间。

 

  `bool sorter_set_nums(sorter_t *me, double *nums, int size, int type);`

为排序算法设置数据，包括数据量大小和数据类型，设置当前数据的备份，采取的是从nums复制数据到me->nums和me->nums_back，并求出最大值最小值。

me: 排序算法结构体

nums, size: 输入的数据

type: 输入数据的类型

返回值：true: 设置成功

​    false: 设置失败

 

  `void sorter_restart(sorter_t *me);`

重置排序算法的状态，包括从nums_back复制数据到nums数组，frame_cnt设置为0，状态为SORTER_READY

me: 排序算法结构体指针

 

 ` void sorter_display_unready(double x, double y, double w, double h);`

在起点为x, y宽高分别是w, h的区域显示数据没有准备好的提示

x, y: 区域起始坐标，单位英寸

w, h: 区域宽高，单位英寸

 

  `bool sorter_save_state_at(sorter_t *me, FILE *ostream);`

格式sorter_t类型的结构体me到输出流ostream中，此格式可以被sorter_load_from()函数解析用以还原此结构体。

me: 排序算法结构体

ostream: 输出流

返回值：true：格式化输出成功

​    false：格式化输出失败

 

 

### bubble-sorter.c中函数的说明：

  `static void next_step(sorter_t *b);`

冒泡排序执行下一步操作。具体来说，当排序算法没有处于SORTER_SORTING或者SORTER_PAUSED状态时，直接返回。设置frame_cnt为0，根据冒泡排序过程比较nums[i]和nums[i+1]的大小，当nums[i]>nums[i+1]和ascned同时为真（升序排序），或者同时为假时（降序排序）交换nums[i]和nums[i+1]。然后me->i加一，完成冒泡排序的一步。

b: 冒泡排序成员base的地址

 

  `static void display(sorter_t *b);`

如果状态是SORTER_UNREADY时，调用sorter_display_unready()函数并返回。然后按照柱状图形式可视化当前排序的状态，对于已经排好序的数据绘制为深灰色Dark Gray，对于未排好序的数据绘制为浅灰色Light Gray，当前排序进行到的数据绘制为橘黄色Orange，同时如果数据量过多导致柱宽度小于0.05那么不绘制边框。

b: 冒泡排序成员base的地址

 

  `static void restart(sorter_t *b);`

调用 sorter_restart(b)，然后设置i和k为0，以表示重新排序。

b: 冒泡排序成员base的地址

 

  `static void destroy(sorter_t *b);`

释放内存，通过container_of计算出包含b的结构体起始地址然后调用free()释放内存。

b: 冒泡排序成员base的地址

 

  `static bool save_state(sorter_t *b, char const *ofile);`

保存冒泡排序的状态到文件 ofile 里。具体来说，首先调用 sorter_save_state_at() 存储sorter_t结构体信息到文件里，然后存储i和k到文件里。

b: b: 冒泡排序成员base的地址

ofile: 输出到的文件名

返回值：true：存储成功，false：存储失败

 

  `sorter_t *bubble_sorter_create(void);`

同main_view_create()类似，分配一个bubble_sort_t结构体，并初始化成员，然后返回成员base的地址

返回值：成员base的地址，类型是sorter_t

 

  `sorter_t *bubble_sorter_load(char const *state_file);`

从文件 state_file 中读取、解析并初始化一个 bubble_sorter_t 的结构体，然后返回其结构体内部 base 成员的地址。冒泡排序文件格式，第一行是“!!bubble sort state file!!”，然后是 sorter_t 结构体保存到文件的数据，然后是bubble_sorter_t结构体中i和k的值。

state_file：状态文件

返回值：成员base的地址，类型是sorter_t

 

quick-sorter.c中函数的说明，quick-sorter中的这些函数名字和bubble-sorter中名字类似，功能也是一致的，只是实现细节和排序算法相关：

  `static void next_step(sorter_t *b);`

由于快速排序的复杂性，这个函数是快速排序模块最为复杂的函数，相当于把递归的快速排序用非递归来实现，同时又把其成一步步运行的状态。函数内部定义了两个宏push和pop用于操作栈，整个排序过程分为了多个内部状态QUICK_INIT初始状态，处于这个状态只把0和size-1这个区间压入栈，然后进入 QUICK_SORT1，这个状态首先判断栈是不是为空，如果是那么排序完成，否则弹出元素，并进入状态QUICK_PIVOT，准备根据枢纽元划分区间，此状态选择区间最右边元素作为枢纽元设置me->pivot_idx为r，然后进入QUICK_PARTITION_LO状态，移动l指针一步，然后保持这个状态，直到多次调用next_step使得l和r交错或者nums[l]大于nums[pivot_idx]进入QUICK_PARTITION_HI以便移动h指针，类似减少h指针直到l和r交错或者nums[h]小于nums[pivot_idx]，然后进入状态QUICK_SWAP，如果l和r交错那么进入QUICK_PARTITION_END状态，否则交换nums[l]和nums[r]然后继续进入QUICK_PARTITION_LO状态进行划分；处于QUICK_PARTITION_END状态时，把枢纽元交换到nums[l]处，然后进入压入当前区间的状态QUICK_PUSH_SEGMENT，处于此状态添加枢纽元下标到done_list中，然后更具lo和pivot_idx以及pivot_idx和hi的关于决定是否把左半区间压入栈或者把右半区间压入栈，然后状态进入QUICK_SORT1开始继续处理。

b: 快速排序结构体成员base的地址

 

  `static void display(sorter_t *b);`

和冒泡排序算法的display()类似，对排序好的数据绘制为深灰色，未排序好的数据为浅灰色，当前活动的数据为橘黄色，此外还增加了绘制枢纽元颜色为棕色Brown，绘制枢纽元高度的水平线，绘制当前活动区间的两条竖线，如果正在交互两个元素，这两个元素为红色以展示交换过程。

b: 快速排序结构体成员base的地址

 

  `static void restart(sorter_t *b);`

重置快速排序过程，调用sorter_restart(b)然后重置stack_size为0，done_size 为0，pivot_idx 为-1，状态为QUICK_INIT状态。

b: 快速排序结构体成员base的地址

 

  `static void destroy(sorter_t *b);`

和冒泡排序的destroy函数功能完全一样，只是释放内存。

b: 快速排序结构体成员base的地址

 

  `static bool save_state(sorter_t *b, char const *ofile);`

保存快速排序算法的状态到文件ofile。类似于bubble-sorter.c中的save_state一样。第一行输出“!!quick sort state file!!”，然后调用sorter_save_state_at()保存成员变量base的状态，之后输出stack_size和栈的所有数据；输出done_size和done_list数组的值，输出状态state，lo, hi, l, r和pivot_idx的值。

b: 快速排序结构体成员base的地址

ofile: 存储状态的文件

返回值：true: 保存成功，false：保存失败

 

  `sorter_t *quick_sorter_create(void);`

分配quick_sort_t类型结构体，并初始化。包括设置base成员的各个函数指针以及stack_size设置为0，done_size设置为0，pivot_idx为-1，状态为QUICK_INIT

返回值： 快速排序结构体成员base的地址，如果分配内存失败返回NULL

 

  `sorter_t *quick_sorter_load(char const *state_file);`

从状态文件中读取数据并初始化快速排序结构体 quick_sorter_t。具体来说，首先读取第一行判断是不是为“!!quick sort state file!!”，成功的话继续使用 sorter_load_from()从文件流中继续读取数据并初始化base成员，之后继续读取stack_size和栈的数据，读取done_size和done_list数组的数据，读取状态state，读取lo, hi, l, r和pivot_idx的值。

state_file：状态文件名

返回值：初始化好的快速排序结构体内部base成员的地址；如果加载或初始化失败返回NULL

 

### select-sorter.c中函数的说明：

  `static void next_step(sorter_t *b);`

选择排序执行下一步操作。具体来说，当排序算法没有处于SORTER_SORTING或者SORTER_PAUSED状态时，直接返回。设置frame_cnt为0。根据选择排序过程，比较nums[max]和nums[i]的大小，如果nums[max]小于nums[i]且排序方式为升序的话，则把i赋值给max。一轮过后交换max和i的值。

b: 选择排序成员base的地址

 

  `static void display(sorter_t *b);`

如果状态是SORTER_UNREADY时，调用sorter_display_unready()函数并返回。然后按照柱状图形式可视化当前排序的状态，对于已经排好序的数据绘制为深灰色Dark Gray，对于未排好序的数据绘制为浅灰色Light Gray，当前排序进行到的数据绘制为橘黄色Orange，当前的最值数据绘制为红色Red,同时如果数据量过多导致柱宽度小于0.05那么不绘制边框。

b: 选择排序成员base的地址

 

  `static void restart(sorter_t *b);`

调用 sorter_restart(b)，然后设置i和k为0，以表示重新排序。

b: 选择排序成员base的地址

 

  `static void destroy(sorter_t *b);`

释放内存，通过container_of计算出包含b的结构体起始地址然后调用free()释放内存。

b: 选择排序成员base的地址

 

  `static bool save_state(sorter_t *b, char const *ofile);`

保存选择排序的状态到文件 ofile 里。具体来说，首先调用 sorter_save_state_at() 存储sorter_t结构体信息到文件里，然后存储i、max和k到文件里。

b: b: 选择排序成员base的地址

ofile: 输出到的文件名

返回值：true：存储成功，false：存储失败

 

  `sorter_t *select_sorter_create(void);`

同main_view_create()类似，分配一个select_sort_t结构体，并初始化成员，然后返回成员base的地址

返回值：成员base的地址，类型是sorter_t

 

  `sorter_t *select_sorter_load(char const *state_file);`

从文件 state_file 中读取、解析并初始化一个 select_sorter_t 的结构体，然后返回其结构体内部 base 成员的地址。选择排序文件格式，第一行是“!!select sort state file!!”，然后是 sorter_t 结构体保存到文件的数据，然后是select_sorter_t结构体中i和k以及max的值。

state_file：状态文件

返回值：成员base的地址，类型是sorter_t

 

### merge-sorter.c中函数的说明：

  `static void next_step(sorter_t *b);`

该函数实现归并排序执行下一步操作的功能，利用非递归的方式实现递归的归并排序算法，使之能够单步进行。具体而言，当排序算法不处于SORTER_SORTING或者SORTER_PAUSED状态时，直接返回，反之继续，并设置frame_cnt为0。该函数将整个排序过程分为三个内部状态：MERGE_STEP1、MERGE_STEP2、MERGE_STEP3。处于MERGE_STEP1时，函数首先判断排序是否完成，若未完成，则利用while循环找出本次需要排序的子列（子列本身未完成排序，但其前后两部分均已完成排序），然后进入MERGE_STEP2。处于MERGE_STEP2时，函数将需要排序的子列分为前后两个部分，并依序比较两部分中各元素大小，将比较结果储存至me->doing_list[]中，然后进入MERGE_STEP3。在MERGE_STEP3状态下，函数将me->doing_list[]中的元素赋值给nums[]中的对应子列，最后重新进入MERGE_STEP1状态寻找下一个需要排序的子列。至此该函数已完成一次子列排序，之后函数将重复这三个过程，直到排序完成。

b: 归并排序结构体成员base的地址

 

  `static void display(sorter_t *b);`

如果状态是SORTER_UNREADY时，调用sorter_display_unready()函数并返回，反之继续。该函数以柱状图的形式可视化当前排序状态，其将绘图区域分为上下两部分，上半部分绘制me->doing_list[]中的数据，下半部分绘制nums[]中的数据。对于已经排序过的数据绘制为深灰色Dark Gray，对于未排序过的数据绘制为浅灰色Light Gray，对于正在排序的数据绘制为橘黄色Orange，对于正在进行比较的数据， 分别绘制为红色Red和绿色Green。同时如果数据量过多导致柱宽度小于0.05那么不绘制边框。

b: 归并排序成员base的地址

 

 `static void restart(sorter_t *b);`

重置归并排序过程，调用sorter_restart(b)然后重置i、j、k为0，状态为MERGE_STEP1状态。

b: 归并排序结构体成员base的地址

 

  `static void destroy(sorter_t *b);`

和冒泡排序的destroy函数功能完全一样，只是释放内存。

b: 归并排序结构体成员base的地址

 

  `static int Check(double Arr[],int StartIndex, int EndIndex, bool flag);`

检查输入数组是否已完成排序。

返回值：1：已完成排序；0：未完成排序

 

  `static bool save_state(sorter_t *b, char const *ofile);`

保存归并排序算法的状态到文件ofile。类似于bubble-sorter.c中的save_state一样。第一行输出“!!merge sort state file!!”，然后调用sorter_save_state_at()保存成员变量base的状态，之后输出状态state；输出start，end，mid的值；输出doing_list数组的值；输出i，j，k的值。

b: 归并排序结构体成员base的地址

ofile: 存储状态的文件

返回值：true: 保存成功，false：保存失败

 

  `sorter_t *merge_sorter_create(void);`

分配merge_sort_t类型结构体，并初始化。包括设置base成员的各个函数指针以及i，j，k设置为0，state设置为MERGE_STEP1

返回值： 归并排序结构体成员base的地址，如果分配内存失败返回NULL

 

  `sorter_t *merge_sorter_load(char const *state_file);`

从状态文件中读取数据并初始化归并排序结构体 merge_sorter_t。具体来说，首先读取第一行判断是不是为“!!merge sort state file!!”，成功的话继续使用 sorter_load_from()从文件流中继续读取数据并初始化base成员，之后继续读取状态state，读取start，end，mid的值，读取doing_list数组的数据，读取i，j，k的值。

state_file：状态文件名

返回值：初始化好的归并排序结构体内部base成员的地址；如果加载或初始化失败返回NULL

 

### insertion-sorter.c中函数的说明：

  `static void next_step(sorter_t *b);`

插入排序执行下一步操作。具体来说，当排序算法没有处于SORTER_SORTING或者SORTER_PAUSED状态时，直接返回，反之继续，并设置frame_cnt为0。排序过程分为3个内部状态：INSERTION_STEP1, INSERTION_STEP2, INSERTION_STEP3。处于INSERTION_STEP1时，其将nums[i]的值赋值给doing_list[i]，并进入INSERTION_STEP2。在INSERTION_STEP1状态中，函数将待排序数据（doing_list[k]）与相邻数据(nums[k-1]])进行比较，若顺序有误，则交换数据，以此来确定待排序数据的位置，然后进入INSERTION_STEP3。在INSERTION_STEP1状态下，函数将doing_list[k]的值赋值给nums[k]。重复上述三个状态，直到所有元素完成排序

b: 插入排序结构体成员base的地址

 

  `static void display(sorter_t *b);`

如果状态是SORTER_UNREADY时，调用sorter_display_unready()函数并返回，反之继续。该函数以柱状图的形式可视化当前排序状态，其将绘图区域分为上下两部分，上半部分绘制me->doing_list[]中的数据，下半部分绘制nums[]中的数据。对于未排序过的数据绘制为浅灰色Light Gray，对于正在排序的数据绘制为橘黄色Orange，对于正在进行比较的数据， 分别绘制为红色Red和绿色Green，排序完成后数据绘制为深灰色Dark Gray。同时如果数据量过多导致柱宽度小于0.05那么不绘制边框。

b: 插入排序成员base的地址

 

  `static void restart(sorter_t *b);`

重置插入排序过程，调用sorter_restart(b)然后重置i，k为0，flag为0，状态为INSERTION_STEP1状态。

b: 插入排序结构体成员base的地址

 

  `static void destroy(sorter_t *b);`

释放内存，通过container_of计算出包含b的结构体起始地址然后调用free()释放内存。

b: 插入排序结构体成员base的地址

 

  `static bool save_state(sorter_t *b, char const *ofile);`

保存快速排序算法的状态到文件ofile。类似于bubble-sorter.c中的save_state一样。第一行输出“!!insertion sort state file!!”，然后调用sorter_save_state_at()保存成员变量base的状态，之后输出i，k的值；输出doing_list数组的值，输出状态state，flag的值。

b: 插入排序结构体成员base的地址

ofile: 存储状态的文件

返回值：true: 保存成功，false：保存失败

 

  `sorter_t *insertion_sorter_create(void);`

分配insertion_sort_t类型结构体，并初始化。包括设置base成员的各个函数指针以及i，k设置为0，flag设置为1，状态为INSERTION_STEP1

返回值： 插入排序结构体成员base的地址，如果分配内存失败返回NULL

 

  `sorter_t *insertion_sorter_load(char const *state_file);`

从状态文件中读取数据并初始化插入排序结构体 insertion_sorter_t。具体来说，首先读取第一行判断是不是为“!!insertion sort state file!!”，成功的话继续使用 sorter_load_from()从文件流中继续读取数据并初始化base成员，之后继续读取i，k的值，读取doing_list数组的数据，读取状态state，读取flag的值。

state_file：状态文件名

返回值：初始化好的插入排序结构体内部base成员的地址；如果加载或初始化失败返回NULL

 

### utility.h中宏的说明：

  `ARRAY_SIZE(arr)`

求出数组元素个数，使用 sizeof(arr) 求出数组占的内存大小，除以sizeof(arr[0])一个元素占的大小，得出有多少个元素

 

  `container_of(ptr, type, member)`

根据type类型中member成员的地址求出type类型的地址，具体就是ptr地址减去member在type类型中的偏移量使用 ptr – offset(type, member)来求出

 

### utility.c中函数的说明：

  `double min_nums(double const *nums, int size);`

求出数组中的最小值

nums, size: 用于表示数组

返回值：最小值

 

  `double max_nums(double const *nums, int size);`

类似double min_nums(); 只是返回的是最大值

 

  `static int random_int(int lo, int hi);`

使用公式 rand()/(double)RAND_MAX * (1+hi – lo) + lo生成[lo, hi]区间的随机整数

lo, hi: 生成的随机数最小值和最大值

返回值：生成的随机数

 

  `double *random_nums(int lo, int hi, int size);`

反复调用random_int(lo, hi) 生成size个随机整数并放到动态分配内存的数组里。

lo, hi: 生成随机整数的最小值和最大值

size: 生成的个数

返回值：含有size个随机整数的数组

 

  `double *load_nums(char const *name, int *size, int *ptype);`

从名为name的文件里读取数据，然会数据数组的地址并把数据大小和数据类型分别存到size指针和ptype里，具体来说首先打开文件name为输入流 istream，然后简介调用load_nums_from()完成具体解析操作。数据文件里数据格式为课程文档中规定的格式。

name: 文件名

size: 指向存储数据大小的指针

返回值：动态分配的数组的起始地址，如果加载失败返回NULL

 

  `double *load_nums_from(FILE *istream, int *psize, int *type);`

从文件流中读取并解析数据，首先读取一个字符串判断是否是int、float和double中之一，相应给type设置为SORTER_INT、SORTER_FLOAT、SORTER_DOUBLE，紧接着根据类型读取数据，直到读取数据结束，设置psize的值为读取的数据个数。然后返回数组。

istream: 输入流

psize: 存储文件大小的地址

type: 存储文件类型的地址

返回值：NULL读取失败；读取成功返回数组地址

 

 

  `bool save_nums_at(FILE *ostream, double const *nums, int size, int type)`

把地址时nums大小为size的数组的数字存入流ostream中。首先根据type决定输出int、float还是double字符串，紧接着输出nums中的数并按照空格隔开。

ostream: 输出流

nums: 数组地址

size: 数组大小

type: 数据类型

返回值：true写入成功，false写入失败

 

  `bool save_nums(char const *ofile, double const *nums, int size, int type);`

存储数据到文件，内部首先打开文件为输出流ostream，然后调用save_nums_at()实现真正的存储操作。

ofile: 输出保存的文件名

nums, size：输出的数据

type: 数据的类型

返回值：true 存储成功，false 存储失败