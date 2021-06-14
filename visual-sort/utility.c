#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <ctype.h>

#include "utility.h"
#include "sorter.h"

double min_nums(double const *nums, int n)
{
    if (!nums || n < 0) return DBL_MIN;
    double min = nums[0];
    for (int i = 1; i < n; ++i) {
        if (nums[i] < min)
            min = nums[i];
    }
    return min;
}

double max_nums(double const *nums, int n)
{
    if (!nums || n < 0) return DBL_MAX;
    int max = nums[0];
    for (int i = 1; i < n; ++i) {
        if (nums[i] > max)
            max = nums[i];
    }
    return max;
}

static
int random_int(int lo, int hi)
{
    return rand()/(double)RAND_MAX * (1 + hi - lo) + lo;
}

double *random_nums(int lo, int hi, int size)
{
    if (lo > hi || size <= 0)
        return NULL;
    double *nums = malloc(sizeof(double) * size);
    srand(time(NULL) ^ clock());

    for (int i = 0; i < size; ++i)
        nums[i] = random_int(lo, hi);

    return nums;
}

double *load_nums(char const *ifile, int *psize, int *ptype)
{
    if (!ifile || !psize || !ptype)
        return NULL;
    FILE *istream = fopen(ifile, "r");
    if (!istream)
        return NULL;
    double *nums = load_nums_from(istream, psize, ptype);
    fclose(istream);
    return nums;
}

double *load_nums_from(FILE *istream, int *psize, int *ptype)
{
    if (!istream || !psize || !ptype)
        return NULL;

    char type_buff[10] = "";
    int type;
    fscanf(istream, "%9s", type_buff);
    if (0 == strcmp("int", type_buff))
        type = SORTER_INT;
    else if (0 == strcmp("float", type_buff))
        type = SORTER_FLOAT;
    else if ( 0 == strcmp("double", type_buff))
        type = SORTER_DOUBLE;
    else
        return NULL;
    int cnt = 0;
    int size = 0;
    double *nums = NULL;
    double x;
    /* 如果给出数据大小的提示 */
    if (*psize > 0) {
        if (*psize > NUMS_SIZE_MAX)
            return NULL;
        size = *psize;
        nums = malloc(sizeof(double) * size);
        if (!nums) return NULL;
    } else {
        *psize = INT_MAX;
    }

    while ((cnt < *psize) && (1 == fscanf(istream, "%lf", &x))) {
        if (cnt >= size) {
            if (size >= NUMS_SIZE_MAX)
                goto freenums;
            size = size*2 + 1;
            nums = realloc(nums, sizeof(double) * size);
            if (!nums)
                goto freenums;
        }
        nums[cnt++] = x;
    }
    *psize = cnt;
    *ptype = type;
    return nums;

freenums:
    free(nums);
    return NULL;
}

bool save_nums(char const *ofile, double const *nums, int size, int type)
{
    if (!ofile || !nums)
        return false;
    FILE *ostream = fopen(ofile, "w");
    if (!ostream)
        return false;
    bool ret = save_nums_at(ostream, nums, size, type);
    fclose(ostream);
    return ret;
}

bool save_nums_at(FILE *ostream, double const *nums, int size, int type)
{
    if (!ostream || !nums || size <= 0 || type > SORTER_TYPE_MAX || type < 0)
        return false;
    bool state = true;
    char const *types[] = {
        "int", "float", "double",
    };
    if (fprintf(ostream, "%s ", types[type]) < 0)
        return false;
    for (int i = 0; i < size; ++i) {
        if (0 != i) fprintf(ostream, " ");
        if (type == SORTER_INT) {
            if (fprintf(ostream, "%d", (int)nums[i]) < 0)
                return false;
        } else {
            if (fprintf(ostream, "%f", nums[i]) < 0)
                return false;
        }
    }
    fprintf(ostream, "\n");
    return true;
}
