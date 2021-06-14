#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "setting.h"
#include "utility.h"
#include "sorter.h"
#include "select-sorter.h"

enum {
    MAGIC_MAX = 64,
};
static char const MAGIC_TEXT[MAGIC_MAX] = "!!select sort state file!!";

typedef struct select_sorter_t {
    sorter_t base;
    int i;
    int k;  // 排序好了多少个元素 
    int max; 
} select_sorter_t;

static
void next_step(sorter_t *b)
{
    if (!b || !(SORTER_SORTING == b->state || SORTER_PAUSED == b->state))
        return;

    b->frame_cnt = 0;

    select_sorter_t *me = container_of(b, select_sorter_t, base);
    int k = me->k;
    int i= me->i;
    int max = me->max;
    int size = b->size;
    double *nums = b->nums;

    if (i+1 >= b->size - k) {
    	swap(nums+i,nums+max);
        me->i = i = 0;
        me->k = k = k + 1;
        me->max = max = 0;
    }
    if (k >= size) {
        b->state = SORTER_SORTED;
        return;
    }
    if ((nums[i] > nums[max]) ^ !(b->ascend))
        me->max=max=i;
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

    select_sorter_t *me = container_of(b, select_sorter_t, base);

    // 刚准备好
    if (b->state == SORTER_READY) {
        me->k = 0;
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
        if (i == me->max && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Red";
        else if (i == me->i && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
		    color="Orange"; 
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
    select_sorter_t *me = container_of(b, select_sorter_t, base);
    sorter_restart(b);
    me->k =me->i=me->max=0;
}

static
void destroy(sorter_t *b)
{
    if (!b) return;
    sorter_deconstruct(b);
    select_sorter_t *me = container_of(b, select_sorter_t, base);
    free(me);
}

static
bool save_state(sorter_t *b, char const *ofile)
{
    if (!b || !ofile) return false;

    bool ret = true;
    select_sorter_t *me = container_of(b, select_sorter_t, base);
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

    fprintf(ostream, "%d %d %d\n",me->i, me->k,me->max);

closestream:
    fclose(ostream);
    return ret;
}

sorter_t *select_sorter_create(void)
{
    select_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    sorter_construct(&me->base);
    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    me->k=me->i=me->max=0;

    return &me->base;
}

sorter_t *select_sorter_load(char const *state_file)
{
    if (!state_file) return NULL;
    select_sorter_t *me = malloc(sizeof(*me));
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
    if (2 != fscanf(istream, "%d %d %d",&me->i, &me->k,&me->max))
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
