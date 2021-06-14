#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "utility.h"
#include "sorter.h"
#include "quick-sorter.h"

enum {
    MAGIC_MAX = 64,
};
static char const MAGIC_TEXT[MAGIC_MAX] = "!!quick sort state file!!";

enum { STACK_MAX = 1024 };
enum {
    QUICK_INIT = 0,
    QUICK_SORT1,
    QUICK_PIVOT,
    QUICK_PARTITION_LO,
    QUICK_PARTITION_HI,
    QUICK_SWAP,
    QUICK_PARTITION_END,
    QUICK_PUSH_SEGEMENT,
    QUICK_STATE_MAX,
};
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

static
void next_step(sorter_t *b)
{
    if (!b || !(SORTER_SORTING == b->state || SORTER_PAUSED == b->state))
        return;

    b->frame_cnt = 0;

    quick_sorter_t *me = container_of(b, quick_sorter_t, base);

#define push(l, r) do { \
        me->stack[me->stack_size][0] = l; \
        me->stack[me->stack_size][1] = r; \
        ++me->stack_size; \
    } while (0)

#define pop(l, r) do { \
        --me->stack_size; \
        l = me->stack[me->stack_size][0]; \
        r = me->stack[me->stack_size][1]; \
    } while (0)

    double *nums = b->nums;
    int lo = me->lo;
    int hi = me->hi;
    int l = me->l, r = me->r;
    int pidx = me->pivot_idx;
    switch (me->state) {
    case QUICK_INIT:
        if (me->stack_size >= NUMS_SIZE_MAX)
            break;
        push(0, b->size - 1);
        me->state = QUICK_SORT1;
        break;

    case QUICK_SORT1:
        if (me->stack_size <= 0) {
            b->state = SORTER_SORTED;
            break;
        }
        pop(lo, hi);
        l = lo;
        r = hi;
        me->state = QUICK_PIVOT;
        break;

    case QUICK_PIVOT:
        pidx = r;
        --r;
        me->state = QUICK_PARTITION_LO;
        break;

    case QUICK_PARTITION_LO:
        if (l <= r && ((nums[l] < nums[pidx]) ^ (!b->ascend))) {
            ++l;
        } else {
            me->state = QUICK_PARTITION_HI;
        }
        break;

    case QUICK_PARTITION_HI:
        if (l <= r && ((nums[r] > nums[pidx]) ^ (!b->ascend))) {
            --r;
        } else {
            me->state = QUICK_SWAP;
        }
        break;

    case QUICK_SWAP:
        if (l > r) {
            me->state = QUICK_PARTITION_END;
            break;
        }
        swap(nums+l, nums+r);
        ++l;
        --r;
        me->state = QUICK_PARTITION_LO;
        break;

    case QUICK_PARTITION_END:
        swap(nums+l, nums+pidx);
        pidx = l;
        me->state = QUICK_PUSH_SEGEMENT;
        break;

    case QUICK_PUSH_SEGEMENT:
        me->done_list[me->done_size++] = pidx;
        if (lo <= pidx-1)
            push(lo, pidx-1);
        if (pidx+1 <= hi)
            push(pidx+1, hi);
        me->state = QUICK_SORT1;
        break;
    }
    me->lo = lo;
    me->hi = hi;
    me->l = l;
    me->r = r;
    me->pivot_idx = pidx;

#undef push
#undef pop
}

static
void display(sorter_t *b, double x, double y, double w, double h)
{
    // 没有数据
    if (!b || b->state == SORTER_UNREADY) {
        sorter_display_unready(x, y, w, h);
        return;
    }

    quick_sorter_t *me = container_of(b, quick_sorter_t, base);
    // 刚准备好
    if (b->state == SORTER_READY) {
        me->stack_size = 0;
        me->done_size = 0;
        me->pivot_idx = -1;
        me->state = QUICK_INIT;
    }


    // 可视化排序情况
    double margin = 0.2;
    x += margin;
    y += margin;
    w -= 2*margin;
    h -= 2*margin;
    double base_height = 0.5;   // 最小元素的高度
    double max_span = h - base_height; // 最大元素高度减去 base_height
    double delta = b->max - b->min;
    double unit_width = w / b->size;

#define draw_nth_num(idx, color) do { \
        double x1 = x + idx*unit_width; \
        double h1 = base_height + (nums[idx] - b->min)/delta * max_span; \
        drawRectangleColor(x1, y, unit_width, h1, true, color); \
        if (unit_width > 0.05) \
            drawRectangleColor(x1, y, unit_width, h1, false, "Black"); \
    } while (0)
    // 显示所有数据
    double const *nums = b->nums;
    int pidx = me->pivot_idx;
    for (int i = 0; i < b->size; ++i)
        draw_nth_num(i, "Light Gray");

    // 已经排好序的
    for (int i = 0; i < me->done_size; ++i)
        draw_nth_num(me->done_list[i], "Dark Gray");

    // 显示排序细节
    if (b->state == SORTER_SORTING || b->state == SORTER_PAUSED) {
        // 显示排序区间
        double lx = x + me->lo * unit_width;
        double rx = x + (me->hi+1) * unit_width;
        drawLineColor(lx, y, lx, y + base_height + max_span, "Black");
        drawLineColor(rx, y, rx, y + base_height + max_span, "Black");

        // 显示枢纽元
        if (pidx != -1) {
            draw_nth_num(pidx, "Brown");
            // 画枢纽元的高度线
            double pivoth = y+base_height + (nums[pidx] - b->min)/delta * max_span;
            drawLineColor(x, pivoth, x+w, pivoth, "Gray");
        }
        // 显示当前活动的元素
        if (me->l <= me->r) {
            draw_nth_num(me->l, "Orange");
            draw_nth_num(me->r, "Orange");
        }
        // 显示两个元素的交换
        if (me->state == QUICK_SWAP && me->l <= me->r) {
            draw_nth_num(me->r, "Red");
            draw_nth_num(me->l, "Red");
        }
    }
}

static
void restart(sorter_t *b)
{
    if (!b || b->state == SORTER_UNREADY)
        return;
    quick_sorter_t *me = container_of(b, quick_sorter_t, base);

    sorter_restart(b);
    me->stack_size = 0;
    me->done_size = 0;
    me->pivot_idx = -1;
    me->state = QUICK_INIT;
}

static
void destroy(sorter_t *b)
{
    if (!b) return;
    sorter_deconstruct(b);
    quick_sorter_t *me = container_of(b, quick_sorter_t, base);
    free(me);
}

static
bool save_state(sorter_t *b, char const *ofile)
{
    if (!b || !ofile) return false;

    bool ret = true;
    quick_sorter_t *me = container_of(b, quick_sorter_t, base);
    FILE *ostream = fopen(ofile, "w");
    if (!ostream) return false;

    if (fprintf(ostream, "%s\n", MAGIC_TEXT) < 0) {
        ret = false;
        goto closestream;
    }

    if (!sorter_save_state_at(b, ostream)) {
        ret = false;
        goto closestream;
    }

    if (fprintf(ostream, "stack_size: %d\n", me->stack_size) < 0) {
        ret = false;
        goto closestream;
    }

    for (int i = 0; i < me->stack_size; ++i) {
        if (fprintf(ostream, "(%d, %d)\n", me->stack[i][0], me->stack[i][1]) < 0) {
            ret = false;
            goto closestream;
        }
    }

    if (fprintf(ostream, "done_size: %d\n", me->done_size) < 0) {
        ret = false;
        goto closestream;
    }
    for (int i = 0; i < me->done_size; ++i) {
        if (0 != i) fprintf(ostream, " ");
        fprintf(ostream, "%d", me->done_list[i]);
    }
    if (me->done_size > 0) fprintf(ostream, "\n");

    fprintf(ostream, "state: %d\n", me->state);
    fprintf(ostream, "lo: %d hi: %d\n", me->lo, me->hi);
    fprintf(ostream, "l: %d r: %d\n", me->l, me->r);
    fprintf(ostream, "pivot_idx: %d\n", me->pivot_idx);

closestream:
    fclose(ostream);
    return ret;
}

sorter_t *quick_sorter_create(void)
{
    quick_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    sorter_construct(&me->base);
    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    me->stack_size = 0;
    me->done_size = 0;
    me->pivot_idx = -1;
    me->state = QUICK_INIT;


    return &me->base;
}

sorter_t *quick_sorter_load(char const *state_file)
{
#define IGN_SPACES "%*[ \t\n]"
    if (!state_file) return NULL;
    quick_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    FILE *istream = fopen(state_file, "r");
    if (!istream)
        goto freeme;

    char magic_buff[MAGIC_MAX] = "";
    if (!fgets(magic_buff, MAGIC_MAX-1, istream))
        goto closestream;
    /* 减一是为了移除\n */
    if (0 != strncmp(MAGIC_TEXT, magic_buff, strlen(magic_buff)-1))
        goto closestream;
    if (!sorter_load_from(&me->base, istream))
        goto closestream;

    if (1 != fscanf(istream, IGN_SPACES"stack_size: %d", &me->stack_size)
            || me->stack_size < 0 || me->stack_size >= STACK_MAX) {
        goto closestream;
    }
    for (int i = 0; i < me->stack_size; ++i) {
        if (2 != fscanf(istream, IGN_SPACES"(%d, %d)", &me->stack[i][0], &me->stack[i][1])
                || (me->stack[i][0] < 0 || me->stack[i][0] >= me->base.size)
                || (me->stack[i][1] < 0 || me->stack[i][1] >= me->base.size)
                || (me->stack[i][0] > me->stack[i][1])) {
            goto closestream;
        }
    }

    if (1 != fscanf(istream, IGN_SPACES"done_size: %d", &me->done_size)
            || me->done_size < 0 || me->done_size > me->base.size) {
        goto closestream;
    }
    for (int i = 0; i < me->done_size; ++i) {
        if (1 != fscanf(istream, IGN_SPACES"%d", &me->done_list[i])
                || me->done_list[i] < 0 || me->done_list[i] >= me->base.size) {
            goto closestream;
        }
    }

    if (1 != fscanf(istream, IGN_SPACES"state: %d", &me->state)
            || me->state < 0 || me->state >= QUICK_STATE_MAX) {
        goto closestream;
    }

    if (2 != fscanf(istream, IGN_SPACES"lo: %d hi: %d", &me->lo, &me->hi)
            || me->lo < 0 || me->lo >= me->base.size
            || me->hi < 0 || me->hi >= me->base.size
            || me->lo > me->hi) {
        goto closestream;
    }

    if (2 != fscanf(istream, IGN_SPACES"l: %d r: %d", &me->l, &me->r)
            || me->l < 0 || me->l >= me->base.size
            || me->r < 0 || me->r >= me->base.size) {
        goto closestream;
    }

    if (1 != fscanf(istream, IGN_SPACES"pivot_idx: %d\n", &me->pivot_idx)
            || me->pivot_idx >= me->base.size) {
        goto closestream;
    }

    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    return &me->base;

closestream:
    fclose(istream);
freeme:
    free(me);
    return NULL;
#undef IGN_SPACES
}
