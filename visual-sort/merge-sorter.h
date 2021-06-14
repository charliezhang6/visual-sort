#ifndef Merge_SORTER_H__
#define Merge_SORTER_H__

#include "sorter.h"

sorter_t *merge_sorter_create(void);
sorter_t *merge_sorter_load(char const *state_file);

#endif // MERGE_SORTER_H__
