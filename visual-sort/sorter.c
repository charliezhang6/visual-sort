#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "sorter.h"
#include "utility.h"
#include "setting.h"

void sorter_speed_ctl(sorter_t *me, bool inc)
{
    if (!me) return;

    if (me->speed < 2) {
        me->speed += (inc? 0.1: -0.1);
        if (me->speed < SPEED_MIN)
            me->speed = SPEED_MIN;
    } else if (me->speed < 10) {
        me->speed += (inc? 0.5: -0.5);
    } else if (me->speed < 20) {
        me->speed += (inc? 2: -2);
    } else {
        me->speed += (inc? 5: -5);
        if (me->speed > SPEED_MAX)
            me->speed = SPEED_MAX;
    }
}

sorter_t *sorter_construct(sorter_t *me)
{
    if (!me) return NULL;

    memset(me, 0, sizeof(*me));

    me->speed = SPEED_DFT;
    me->frame_cnt = 0;
    me->state = SORTER_UNREADY;
    me->ascend = true;

    return me;
}

sorter_t *sorter_load_from(sorter_t *me, FILE *istream)
{
    // %*[ \t\n] ÊÇÎªÁËºöÂÔ¿Õ°××Ö·û
#define IGN_SPACES "%*[ \t\n]"

    if (!me || !istream)
        return NULL;
    memset(me, 0, sizeof(*me));

    int size, type;
    if (1 != fscanf(istream, "size: %d", &size) || size <= 0)
        return NULL;
    me->nums = load_nums_from(istream, &size, &type);
    if (!me->nums || type < 0 || type > SORTER_TYPE_MAX)
        return NULL;
    me->nums_back = load_nums_from(istream, &size, &type);
    if (!me->nums_back || type < 0 || type > SORTER_TYPE_MAX)
        goto freenums;
    me->size = size;
    me->type = type;

    if (1 != fscanf(istream, IGN_SPACES"speed: %lf", &me->speed))
        goto freeallnums;

    if (1 != fscanf(istream, IGN_SPACES"frame_cnt: %d", &me->frame_cnt))
        goto freeallnums;

    if (1 != fscanf(istream, IGN_SPACES"state: %d", &me->state))
        goto freeallnums;

    int ascend;
    if (1 != fscanf(istream, IGN_SPACES"ascend: %d", &ascend))
        goto freenums;
    me->ascend = ascend;

    me->min = min_nums(me->nums_back, size);
    me->max = max_nums(me->nums_back, size);

    return me;

freeallnums:
    free(me->nums_back);
    me->nums_back = NULL;
freenums:
    free(me->nums);
    me->nums = NULL;
    me->state = SORTER_UNREADY;
    return NULL;
#undef IGN_SPACES
}

void sorter_deconstruct(sorter_t *me)
{
    if (!me) return;
    free(me->nums);
    free(me->nums_back);
}

void sorter_update(sorter_t *me)
{
    if (!me || me->state != SORTER_SORTING)
        return;

    ++me->frame_cnt;
    if (me->frame_cnt * fps2interval(MAIN_LOOP_FPS) >= fps2interval(me->speed))
        me->next_step(me);
}

bool sorter_set_nums(sorter_t *me, double *nums, int size, int type)
{
    if (!me || !nums || size <= 0 || type < 0 || type > SORTER_TYPE_MAX)
        return false;

    me->nums = realloc(me->nums, size * sizeof(double));
    if (!me->nums) return false;
    me->nums_back = realloc(me->nums_back, size * sizeof(double));
    if (!me->nums_back) {
        free(me->nums);
        return false;
    }

    memcpy(me->nums, nums, size * sizeof(double));
    memcpy(me->nums_back, nums, size * sizeof(double));
    me->size = size;
    me->type = type;

    me->min = min_nums(nums, size);
    me->max = max_nums(nums, size);

    return true;
}

void sorter_restart(sorter_t *me)
{
    if (!me || me->state == SORTER_UNREADY)
        return;
    memcpy(me->nums, me->nums_back, me->size * sizeof(double));
    me->frame_cnt = 0;
    me->state = SORTER_READY;
}

void sorter_display_unready(double x, double y, double w, double h)
{
    char *nodata = "No Any Numbers Added, Please Add Numbers Firstly!!!";
    drawLabel(x + (w - TextStringWidth(nodata))/2, y + w/2, nodata);
}


bool sorter_save_state_at(sorter_t *me, FILE *ostream)
{
    if (!me || !ostream)
        return false;

    if (fprintf(ostream, "size: %d\n", me->size) < 0)
        return false;
    if (!save_nums_at(ostream, me->nums, me->size, me->type))
        return false;
    if (!save_nums_at(ostream, me->nums_back, me->size, me->type))
        return false;
    if (fprintf(ostream, "speed: %lf\n", me->speed) < 0)
        return false;
    if (fprintf(ostream, "frame_cnt: %d\n", me->frame_cnt) < 0)
        return false;
    if (fprintf(ostream, "state: %d\n", me->state) < 0)
        return false;
    if (fprintf(ostream, "ascend: %d\n", me->ascend) < 0)
        return false;

    return true;
}

