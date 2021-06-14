#ifndef VIEW_H__
#define VIEW_H__

#include <stdlib.h>

typedef struct view_t view_t;

enum { VIEWS_MAX  = 1024 }; // �����ڳ���
static void (*VIEW_DESTROY_DFT)(view_t*) = (void (*)(view_t*))free;
static void (*VIEW_DESTROY_NOTNEED)(view_t*) = (void (*)(view_t*))0x01;

struct view_t {
    char const* name;
    void (*display)(view_t *me);
    void (*destroy)(view_t *me);
};

enum {
    VIEW_OK = 0,        // ������������
    VIEW_NOTFOUND = -1, // û���ҵ���Ӧ�� view_name
    VIEW_TOOMANY = -2,  // view �������� VIEWS_MAX
    VIEW_INVALID = -3,  // ����������Ч
};

/**
 * ���ó�ʼ��� view
 * @name: view ������
 * return: VIEW_OK ���óɹ�
 *         VIEW_NOTFOUND û�ж�Ӧ�� view
 */
int view_default(char const *name);

/**
 * ���µ�ǰ��� view ҳ�浽��Ļ
 */
void view_display(void);

/**
 */
int view_switch(char const *name);

/**
 * return: VIEW_OK ��ӳɹ�
 *         VIEW_INVALID ���������Ч��������
 *         VIEW_TOOMANY ��ǰ��¼�� view �����Ѿ�����
 */
int view_add(view_t *view);

/**
 */
int view_remove(char const *name);

/**
 */
void view_destroy(void);

#endif // VIEW_H__
