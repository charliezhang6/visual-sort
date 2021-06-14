#ifndef MAINVIEW_H__
#define MAINVIEW_H__

#include "view.h"

static char const *INPUT_DATA_FILE = "input.txt";
static char const *OUTPUT_DATA_FILE = "output.txt";
static char const *STATE_DATA_FILE = "sorter-state.bin";

view_t *main_view_create(char const *name);

#endif // MAINVIEW_H__
