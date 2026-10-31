#ifndef HOST_DISPLAY_H
#define HOST_DISPLAY_H

#include <stdint.h>

#define DWIDTH 320
#define DHEIGHT 528
#define C_RGB(r,g,b) (((r) << 11) | ((g) << 6) | (b))
enum { C_WHITE = 0xffff, C_BLACK = 0, C_RED = 0xf800 };
typedef uint16_t color_t;
struct dwindow { int left, top, right, bottom; };

extern uint16_t *gint_vram;
void host_check_guards(void);
struct dwindow dwindow_set(struct dwindow window);
void dclear(color_t color);
void drect(int x1, int y1, int x2, int y2, int color);
void dpixel(int x, int y, int color);
void dtext(int x, int y, int color, const char *text);
void dprint(int x, int y, int color, const char *format, ...);
void dupdate(void);
void host_write_ppm(const char *path);
unsigned host_nonzero_pixels(void);

#endif

