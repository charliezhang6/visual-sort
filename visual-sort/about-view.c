#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "utility.h"
#include "menubar.h"
#include "about-view.h"

typedef struct about_view_t {
    view_t base;
} about_view_t;

static
void display(view_t *b)
{
    double winw = GetWindowWidth();
    double winh = GetWindowHeight();
    double lineh = 1.4 * GetFontHeight();
    menubar_display(0, winh - lineh, winw, lineh);

    char *list = "Authors List:";
    double w = TextStringWidth(list);
    double sep = 0.2;
    char *authors[] = {
        "Íô½¡ÇÚ (Wang Jianqin)",
        "Íõ×Óî£ (Wang Zirui)",
        "ÕÅ³½Áú (Charlie Zhang)",
    };
    drawLabel((winw - w)/2, 0.7*winh, list);
    for (int i = 0; i < ARRAY_SIZE(authors); ++i) {
        w = TextStringWidth(authors[i]);
        drawLabel((winw - w)/2, 0.7*winh - (i+1)*(GetFontHeight() + sep), authors[i]);
    }
    char *back = "Go Back";
    w = TextStringWidth(back) + 0.3;
    if (button(GenUUID(0), (winw - w)/2, 0.3*winh, w, lineh, back))
        view_switch("main");
}

view_t *about_view_create(char const *name)
{
    about_view_t *view = malloc(sizeof(*view));
    if (!view) return NULL;
    view->base.name = name;
    view->base.display = display;
    view->base.destroy = VIEW_DESTROY_DFT;

    return &view->base;
}
