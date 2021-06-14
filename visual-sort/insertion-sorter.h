#ifndef INSERTION_SORTER_H__
#define INSERTION_SORTER_H__

#include "sorter.h"

sorter_t *insertion_sorter_create(void);
sorter_t *insertion_sorter_load(char const *state_file);

#endif // INSERTION_SORTER_H__
