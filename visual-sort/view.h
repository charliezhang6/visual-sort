#ifndef VIEW_H__
#define VIEW_H__

#include <stdlib.h>

typedef struct view_t view_t;

enum { VIEWS_MAX  = 1024 }; // 编译期常量
static void (*VIEW_DESTROY_DFT)(view_t*) = (void (*)(view_t*))free;
static void (*VIEW_DESTROY_NOTNEED)(view_t*) = (void (*)(view_t*))0x01;

struct view_t {
    char const* name;
    void (*display)(view_t *me);
    void (*destroy)(view_t *me);
};

enum {
    VIEW_OK = 0,        // 函数正常返回
    VIEW_NOTFOUND = -1, // 没有找到对应的 view_name
    VIEW_TOOMANY = -2,  // view 数量超过 VIEWS_MAX
    VIEW_INVALID = -3,  // 函数参数无效
};

/**
 * 设置初始活动的 view
 * @name: view 的名字
 * return: VIEW_OK 设置成功
 *         VIEW_NOTFOUND 没有对应的 view
 */
int view_default(char const *name);

/**
 * 更新当前活动的 view 页面到屏幕
 */
void view_display(void);

/**
 */
int view_switch(char const *name);

/**
 * return: VIEW_OK 添加成功
 *         VIEW_INVALID 输入参数无效、有问题
 *         VIEW_TOOMANY 当前记录的 view 数量已经满了
 */
int view_add(view_t *view);

/**
 */
int view_remove(char const *name);

/**
 */
void view_destroy(void);

#endif // VIEW_H__
