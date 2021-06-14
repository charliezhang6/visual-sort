#ifndef SORTER_H__
#define SORTER_H__

#include <stdbool.h>
#include <stdio.h>

typedef struct sorter_t sorter_t;

static double const SPEED_MIN = 0.3;
static double const SPEED_DFT = 4;
static double const SPEED_MAX = 50;
enum {
    NUMS_SIZE_MIN = 10,
    NUMS_SIZE_DFT = 30,
    NUMS_SIZE_MAX = 1024,
};

enum {
    SORTER_UNREADY = 0,
    SORTER_READY,
    SORTER_SORTING,
    SORTER_PAUSED,
    SORTER_SORTED,
};

/* 数据类型 */
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

void sorter_update(sorter_t *me);
void sorter_speed_ctl(sorter_t *me, bool inc);
sorter_t *sorter_construct(sorter_t *me);
sorter_t *sorter_load_from(sorter_t *me, FILE *istream);
void sorter_deconstruct(sorter_t *me);
bool sorter_set_nums(sorter_t *me, double *nums, int size, int type);
void sorter_restart(sorter_t *me);
void sorter_display_unready(double x, double y, double w, double h);
bool sorter_save_state_at(sorter_t *me, FILE *ostream);

#endif // SORTER_H__
