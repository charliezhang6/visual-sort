#ifndef SELECT_SORTER_H__
#define SELECT_SORTER_H__

#include "sorter.h"

sorter_t *select_sorter_create(void);
sorter_t *select_sorter_load(char const *state_file);

#endif // SELECT_SORTER_H__
