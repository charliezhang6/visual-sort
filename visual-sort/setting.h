#ifndef THEME_H__
#define THEME_H__

// 加 static 限制作用域为本文件
static int const WINDOW_MIN_WEDTH = 10;
static int const WINDOW_MIN_HEIGHT = 7;

static double const MAIN_LOOP_FPS = 60.0;

/* 根据 fps 计算帧与帧之间的时间差，毫秒 */
#define fps2interval(fps)   (int)(1000.0/(fps))

#endif // THEME_H__
