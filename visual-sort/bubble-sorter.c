#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "setting.h"
#include "utility.h"
#include "sorter.h"
#include "bubble-sorter.h"

enum {
    MAGIC_MAX = 64,
};
static char const MAGIC_TEXT[MAGIC_MAX] = "!!bubble sort state file!!";

typedef struct bubble_sorter_t {
    sorter_t base;

    int k;  // 排序好了多少个元素
    int i;  // 当前这一轮进行到哪个元素
} bubble_sorter_t;

static
void next_step(sorter_t *b)
{
    if (!b || !(SORTER_SORTING == b->state || SORTER_PAUSED == b->state))
        return;

    b->frame_cnt = 0;

    bubble_sorter_t *me = container_of(b, bubble_sorter_t, base);
    int i = me->i;
    int k = me->k;
    int size = b->size;
    double *nums = b->nums;
    if (i+1 >= b->size - k) {
        me->i = i = 0;
        me->k = k = k + 1;
    }
    if (k >= size) {
        b->state = SORTER_SORTED;
        return;
    }
    if ((nums[i] > nums[i+1]) ^ !(b->ascend))
        swap(nums + i, nums + i+1);
    ++me->i;
}

static
void display(sorter_t *b, double x, double y, double w, double h)
{
    // 没有数据
    if (!b || b->state == SORTER_UNREADY) {
        sorter_display_unready(x, y, w, h);
        return;
    }

    bubble_sorter_t *me = container_of(b, bubble_sorter_t, base);

    // 刚准备好
    if (b->state == SORTER_READY) {
        me->i = me->k = 0;
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

    double mousex = 0, mousey = 1;
    int hover_idx = 0;
    double const *nums = b->nums;
    for (int i = 0; i < b->size; ++i) {
        double x1 = x + i*unit_width;
        double h1 = base_height + (nums[i] - b->min)/delta * max_span;
        char *color = "Dark Gray";
        if (i == me->i && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Orange";
        else if (i < b->size - me->k)
            color = "Light Gray";
        drawRectangleColor(x1, y, unit_width, h1, true, color);
        if (unit_width > 0.05)
            drawRectangleColor(x1, y, unit_width, h1, false, "Black");
    }
}

static
void restart(sorter_t *b)
{
    if (!b || b->state == SORTER_UNREADY)
        return;
    bubble_sorter_t *me = container_of(b, bubble_sorter_t, base);
    sorter_restart(b);
    me->i = me->k = 0;
}

static
void destroy(sorter_t *b)
{
    if (!b) return;
    sorter_deconstruct(b);
    bubble_sorter_t *me = container_of(b, bubble_sorter_t, base);
    free(me);
}

static
bool save_state(sorter_t *b, char const *ofile)
{
    if (!b || !ofile) return false;

    bool ret = true;
    bubble_sorter_t *me = container_of(b, bubble_sorter_t, base);
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

    fprintf(ostream, "%d %d\n", me->i, me->k);

closestream:
    fclose(ostream);
    return ret;
}

sorter_t *bubble_sorter_create(void)
{
    bubble_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    sorter_construct(&me->base);
    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    me->i = me->k = 0;

    return &me->base;
}

sorter_t *bubble_sorter_load(char const *state_file)
{
    if (!state_file) return NULL;
    bubble_sorter_t *me = malloc(sizeof(*me));
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
    if (2 != fscanf(istream, "%d %d", &me->i, &me->k))
        goto closestream;

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
}
