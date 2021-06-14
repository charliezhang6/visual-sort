#ifndef UTILITY_H__
#define UTILITY_H__

#include <limits.h>     // import INT_MIN, INT_MAX
#include <stddef.h>     // import offsetof
#include <stdbool.h>    // import type bool
#include <stdio.h>

#define MAX(x, y) ((x)<(y)? (y): (x))
#define MIN(x, y) ((x)<(y)? (x): (y))

#define ARRAY_SIZE(a)   (int)(sizeof(a)/sizeof(a[0]))

#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

static inline
void swap(double *a, double *b)
{
    double t = *a;
    *a = *b;
    *b = t;
}
double min_nums(double const *nums, int size);
double max_nums(double const *nums, int size);
double *random_nums(int lo, int hi, int size);

double *load_nums(char const *ifile, int *psize, int *ptype);
double *load_nums_from(FILE *istream, int *psize, int *ptype);
bool save_nums(char const *ofile, double const *nums, int size, int type);
bool save_nums_at(FILE *ostream, double const *nums, int size, int type);

#endif // UTILITY_H__
