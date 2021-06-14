#ifndef BUBBLE_SORTER_H__
#define BUBBLE_SORTER_H__

#include "sorter.h"

sorter_t *bubble_sorter_create(void);
sorter_t *bubble_sorter_load(char const *state_file);

#endif // BUBBLE_SORTER_H__
