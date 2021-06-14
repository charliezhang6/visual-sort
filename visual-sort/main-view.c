#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "view.h"
#include "main-view.h"
#include "sorter.h"
#include "bubble-sorter.h"
#include "quick-sorter.h"
#include "merge-sorter.h"
#include "insertion-sorter.h"
#include "select-sorter.h"
#include "menubar.h"
#include "utility.h"

enum {
    ALG_BUBBLE_SORT = 1,
    ALG_QUICK_SORT,
    ALG_MERGE_SORT,
    ALG_INSERTION_SORT,
    ALG_SELECT_SORT,
};
char *alg_list[] = {
    "Bubble Sort",  // 默认排序算法
    "Bubble Sort",  // 冒泡排序算法
    "Quick Sort",   // 快速排序算法
    "Merge Sort",	// 归并排序算法 
    "Insertion Sort",	// 插入排序算法 
    "Select Sort",  //选择排序算法 
};

enum {
    DATASRC_RANDOM = 1,
    DATASRC_ASCEND,
    DATASRC_DESCEND,
    DATASRC_LOADFILE,
};
char *src_list[] = {
    "Select Source ?",// 提示
    "Random",       // 随机生成数据
    "Ascend",       // 生成升序数据
    "Descend",      // 生成降序数据
    "Load File",    // 从文件中加载数据
};

double const LEFTSIDE_WIDTH = 2;
double const MARGIN = 0.1;
double MENUBAR_HEIGHT;

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

static
void control_buttons_display(main_view_t *me, double x, double y, double w, double h)
{
    double sep = 0.15;
    double fonth = GetFontHeight();
    double fontd = GetFontDescent();
    double value_width = w - 3 * (sep+h);
    sorter_t *sorter = me->sorter;

    if (button(GenUUID(0), x, y, h, h, "-"))
        sorter_speed_ctl(sorter, false);

    char speed_text[10];
    snprintf(speed_text, 10, "%.1lf", sorter->speed);
    x += sep + h;
    drawLabel(x, y, speed_text);

    x += sep + value_width;
    if (button(GenUUID(0), x, y, h, h, "+"))
        sorter_speed_ctl(sorter, true);

    x += sep + h;
    if (button(GenUUID(0), x, y, h, h, ">>"))
        sorter->next_step(sorter);
}

static
int double_less(void const *a, void const *b)
{
    return *(double const *)a - *(double const *)b;
}

static
int double_great(void const *a, void const *b)
{
    return double_less(b, a);
}

static
bool generate_nums(main_view_t *me)
{
    sorter_t *sorter = me->sorter;

    double *nums;
    int size = me->size;
    int type = SORTER_INT;
    switch (me->src_idx) {
    case DATASRC_RANDOM:
        nums = random_nums(0, 4*size, size);
        break;

    case DATASRC_ASCEND:
        nums = random_nums(0, 4*size, size);
        qsort(nums, size, sizeof(double), double_less);
        break;

    case DATASRC_DESCEND:
        nums = random_nums(0, 4*size, size);
        qsort(nums, size, sizeof(double), double_great);
        break;

    case DATASRC_LOADFILE:
        size = -1;
        nums = load_nums(INPUT_DATA_FILE, &size, &type);
        if (!nums || size <= 0 || type < 0 || type > SORTER_TYPE_MAX) {
            printf("%s: %s\n", INPUT_DATA_FILE,
                    (errno == 0)? "Invalid input file": strerror(errno));
            return false;
        }
    }
    if (!nums) return false;
    if (!sorter_set_nums(sorter, nums, size, type))
        return false;
    free(me->nums);
    me->nums = nums;
    me->size = size;
    snprintf(me->size_text, SIZE_TEXT_MAX, "%d", me->size);
    me->type = type;
    return true;
}

static
double data_sources_display(main_view_t *me, double x, double y, double w, double h)
{
    double lineh = 1.2 * GetFontHeight();
    double sep = 0.05;
    sorter_t *(*states_loader[])(char const *ifile) = {
        NULL, bubble_sorter_load, quick_sorter_load, merge_sorter_load, insertion_sorter_load
    };
    sorter_t *sorter = me->sorter;

    y -= lineh + sep;
    if (button(GenUUID(0), x, y, w, lineh, "Load Saved State")) {
        sorter_t *new_sorter;
        for (int i = 1; i < ARRAY_SIZE(states_loader); ++i) {
            new_sorter = states_loader[i](STATE_DATA_FILE);
            if (new_sorter) {
                double *nums = malloc(sizeof(double) * new_sorter->size);
                if (!nums) {
                    new_sorter->destroy(new_sorter);
                    break;
                }
                memcpy(nums, new_sorter->nums_back, new_sorter->size * sizeof(double));
                free(me->nums);
                me->nums = nums;
                me->size = new_sorter->size;
                snprintf(me->size_text, SIZE_TEXT_MAX, "%d", me->size);
                me->type = new_sorter->type;
                sorter->destroy(sorter);
                me->sorter = sorter = new_sorter;
                alg_list[0] = alg_list[i];
                break;
            }
        }
    }

    y -= lineh + 2*sep;
    drawLabel(x, y, "Data Source:");

    y -= lineh + sep;
    if (sorter->state == SORTER_SORTING || sorter->state == SORTER_PAUSED) {
        drawBox(x, y, w, lineh, src_list[0], 'L', getMenuColor()->fill);
    } else {
        int selected = menuList(GenUUID(0), x, y, w, w, lineh, src_list, ARRAY_SIZE(src_list));
        if (selected > 0) {
            me->src_idx = selected;
            src_list[0] = src_list[selected];
            if (generate_nums(me))
                sorter->state = SORTER_READY;
            return y;
        }
    }

    // 需要显示数据量
    if (1 <= me->src_idx && me->src_idx <= 3) {
        y -= lineh + sep;
        drawLabel(x, y, "Amount:");
        y -= lineh + sep;
        if (sorter->state == SORTER_SORTING || sorter->state == SORTER_PAUSED) {
            double indent = GetFontAscent()/2;
            double texty = y+lineh/2-GetFontAscent()/2;
            drawRectangleColor(x, y, w, lineh,
                    getTextBoxColor()->fill,
                    getTextBoxColor()->frame);
            SetPenColor(getTextBoxColor()->label);
            MovePen(x+indent, texty);
            DrawTextString(me->size_text);
            return y;
        }

        if (GUI_ACCEPT == textbox(GenUUID(0), x, y, w, lineh, me->size_text, SIZE_TEXT_MAX)) {
            int size;
            sscanf(me->size_text, "%d", &size);
            if (size > NUMS_SIZE_MAX) size = NUMS_SIZE_MAX;
            if (size < NUMS_SIZE_MIN) size = NUMS_SIZE_MIN;
            me->size = size;
            snprintf(me->size_text, SIZE_TEXT_MAX, "%d", size);
            if (generate_nums(me))
                sorter->state = SORTER_READY;
        }
    }

    return y;
}

static void leftside_column_display(main_view_t *me, double x, double y, double w, double h)
{
    x += MARGIN;
    y += MARGIN;
    w -= 2*MARGIN;
    h -= 2*MARGIN;

    double fonth = GetFontHeight();
    double lineh = 1.2 * fonth;
    double sep = 0.05;
    sorter_t *sorter = me->sorter;
    y = y + h;

    y = data_sources_display(me, x, y, w, h);

    y -= lineh + 2*sep;
    drawLabel(x, y, "Algorithm:");
    y -= lineh + sep;
    if (sorter->state == SORTER_SORTING || sorter->state == SORTER_PAUSED) {
        drawBox(x, y, w, lineh, alg_list[0], 'L', getMenuColor()->fill);
    } else {
        int selected = menuList(GenUUID(0), x, y, w, w, lineh, alg_list, ARRAY_SIZE(alg_list));
        if (selected > 0) {
            alg_list[0] = alg_list[selected];
            sorter->destroy(sorter);
            switch (selected) {
            case ALG_BUBBLE_SORT:
                sorter = me->sorter = bubble_sorter_create();
                break;

            case ALG_QUICK_SORT:
                sorter = me->sorter = quick_sorter_create();
                break;
                
            case ALG_MERGE_SORT:
                sorter = me->sorter = merge_sorter_create();
                break;
        	case ALG_INSERTION_SORT:
                sorter = me->sorter = insertion_sorter_create();
                break;
            case ALG_SELECT_SORT :
			    sorter = me->sorter = select_sorter_create(); 
			    break;
            }
            if (sorter_set_nums(sorter, me->nums, me->size, me->type))
                sorter->state = SORTER_READY;
        }
    }

    y -= lineh + 2*sep;
    drawLabel(x, y, "Sort by Order:");
    double indent = TextStringWidth("Sort by Order:") + sep;
    char *ascend_text = sorter->ascend? "Ascend" : "Descend";
    if (sorter->state == SORTER_SORTING || sorter->state == SORTER_PAUSED) {
        drawLabel(x + indent, y, ascend_text);
    } else {
        if (button(GenUUID(0), x + indent, y, w - indent, lineh, ascend_text))
            sorter->ascend = !sorter->ascend;
    }

    y -= lineh + 2*sep;
    drawLabel(x, y, "Speed:");
    y -= lineh + sep;
    control_buttons_display(me, x, y, w, lineh);

    y -= lineh + 2*sep;
    drawLabel(x, y, "State:");
    indent = TextStringWidth("State:") + sep;
    char *state_str[] = {
        "Unready",
        "Ready",
        "Sorting ...",
        "Paused",
        "Completed",
    };
    drawLabel(x + indent, y, state_str[sorter->state]);
    if (sorter->state != SORTER_UNREADY)
        y -= lineh + sep;
    switch (sorter->state) {
    case SORTER_READY:
        if (button(GenUUID(0), x, y, w, lineh, "SORT!")) {
            sorter->state = SORTER_SORTING;
            snprintf(me->size_text, SIZE_TEXT_MAX, "%d", me->size);
        }
        break;

    case SORTER_SORTING:
        if (button(GenUUID(0), x, y, (w-sep)/2, lineh, "Pause"))
            sorter->state = SORTER_PAUSED;
        if (button(GenUUID(0), x + (w+sep)/2, y, (w-sep)/2, lineh, "Restart"))
            sorter->restart(sorter);
        break;

    case SORTER_PAUSED:
        if (button(GenUUID(0), x, y, (w - sep)/2, lineh, "Continue"))
            sorter->state = SORTER_SORTING;
        if (button(GenUUID(0), x + (w+sep)/2, y, (w-sep)/2, lineh, "Restart"))
            sorter->restart(sorter);
        break;

    case SORTER_SORTED:
        if (button(GenUUID(0), x, y, (w-sep)/2, lineh, "Save Data"))
            save_nums(OUTPUT_DATA_FILE, sorter->nums, sorter->size, sorter->type);
        if (button(GenUUID(0), x + (w+sep)/2, y, (w-sep)/2, lineh, "Restart"))
            sorter->restart(sorter);
        break;

    case SORTER_UNREADY:
    default:
        break;
    }

    if (sorter->state == SORTER_SORTING || sorter->state == SORTER_PAUSED) {
        y -= lineh + 2*sep;
        if (button(GenUUID(0), x, y, w, lineh, "Save State"))
            sorter->save_state(sorter, STATE_DATA_FILE);
    }

    y -= lineh + 2*sep;
    if (button(GenUUID(0), x, y, w, lineh, "Quit"))
        exit(0);
}

static
void display(view_t *b)
{
    main_view_t *me = container_of(b, main_view_t, base);
    double winw = GetWindowWidth();
    double winh = GetWindowHeight();
    double leftsidew = LEFTSIDE_WIDTH;
    double menubarh = MENUBAR_HEIGHT;
    menubar_display(0, winh - menubarh, winw, menubarh);

    leftside_column_display(me, 0, 0, leftsidew, winh - menubarh);

    /* 垂直分割线 */
    drawLine(leftsidew, 0, leftsidew, winh - menubarh);

    sorter_t *sorter = me->sorter;
    sorter_update(sorter);
    sorter->display(sorter, leftsidew, 0, winw - leftsidew, winh - menubarh);
}

static
void destroy(view_t *b)
{
    main_view_t *me = container_of(b, main_view_t, base);
    free(me->nums);
    me->sorter->destroy(me->sorter);
    free(me);
}

view_t *main_view_create(char const *name)
{
    MENUBAR_HEIGHT = 1.4 * GetFontHeight();

    main_view_t *me = malloc(sizeof(*me));
    if (!me) return NULL;
    me->base.name = name;
    me->base.display = display; 
    me->base.destroy = destroy;

    me->sorter = bubble_sorter_create();

    me->src_idx = -1;
    me->nums = NULL;
    me->size = NUMS_SIZE_DFT;
    me->type = SORTER_INT;
    snprintf(me->size_text, SIZE_TEXT_MAX, "%d", NUMS_SIZE_DFT);

    return &me->base;
}
