#include <string.h>

#include "view.h"

/* 这些变量默认都是 0 */
view_t *views[VIEWS_MAX];
int view_cnt;
view_t *active;

static
int view_lookup_by_name(char const *name)
{
    for (int i = 0; i < view_cnt; ++i) {
        if (views[i] && (0 == strcmp(views[i]->name, name)))
            return i;
    }
    return VIEW_NOTFOUND;
}

int view_default(char const* name)
{
    return view_switch(name);
}

void view_display(void)
{
    if (!active) return;
    active->display(active);
}

int view_switch(char const *name)
{
    int i = view_lookup_by_name(name);
    if (i == VIEW_NOTFOUND)
        return VIEW_NOTFOUND;
    active = views[i];
    return VIEW_OK;
}

int view_add(view_t *view)
{
    if (!view) return VIEW_INVALID;
    int i;
    for (i = 0; views[i] && i < VIEWS_MAX; ++i)
        ;
    if (i == VIEWS_MAX) return VIEW_TOOMANY;
    views[i] = view;
    if (i == view_cnt)
        ++view_cnt;
    return VIEW_OK;
}

static
void view_remove_by_idx(int idx)
{
    if (VIEW_DESTROY_NOTNEED != views[idx]->destroy)
        views[idx]->destroy(views[idx]);
    views[idx] = NULL;
}

int view_remove(char const *name)
{
    int i = view_lookup_by_name(name);
    if (i == VIEW_NOTFOUND)
        return VIEW_NOTFOUND;
    view_remove_by_idx(i);
    if (i == view_cnt-1)
        --view_cnt;
    return VIEW_OK;
}

void view_destroy(void)
{
    for (int i =  0; i < view_cnt; ++i) {
        if (views[i])
            view_remove_by_idx(i);
    }
    view_cnt = 0;
    active = NULL;
}
