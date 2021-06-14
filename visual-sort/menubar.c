#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "utility.h"
#include "view.h"
#include "menubar.h"

void menubar_display(double x, double y, double w, double h)
{
    char *themes[] = {
        "Themes",
        "Blue | Ctrl-1",
        "Orange | Ctrl-2",
        "Orange with Fill | Ctrl-3",
        "Light Gray | Ctrl-4",
        "Light Gray with Fill (dft) | Ctrl-5",
        "Brown | Ctrl-6",
        "Brown with Fill | Ctrl-7",
    };
    char *help[] = {
        "Help",
        "Help | Ctrl-H",
        "About | Ctrl-A",
    };

    double margin = 0.2;
    double lenest;
    double themew, helpw;
    int idx;

    // 边框
    drawMenuBar(x, y, w, h);

    themew = lenest = margin + TextStringWidth(themes[0]);
    for (int i = 1; i < ARRAY_SIZE(themes); ++i) {
        double len = margin + TextStringWidth(themes[i]);
        lenest = MAX(len, lenest);
    }
    idx = menuList(GenUUID(0), x, y, themew, lenest, h, themes, ARRAY_SIZE(themes));
    if (idx > 0)
        usePredefinedColor(idx-1);

    helpw = lenest = margin + TextStringWidth(help[0]);
    for (int i = 1; i < ARRAY_SIZE(help); ++i) {
        double len = margin + TextStringWidth(help[i]);
        lenest = MAX(len, lenest);
    }
    idx = menuList(GenUUID(0), x+themew, y, helpw, lenest, h, help, ARRAY_SIZE(help));
    switch (idx) {
    case 1:
        // TODO 展示帮助页面
        break;

    case 2: // 切换到关于页面
        view_switch("about");
        break;
    }
}

