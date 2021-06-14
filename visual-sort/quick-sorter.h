#ifndef QUICK_SORTER_H__
#define QUICK_SORTER_H__

#include "sorter.h"

sorter_t *quick_sorter_create(void);
sorter_t *quick_sorter_load(char const *state_file);

#endif // QUICK_SORTER_H__
