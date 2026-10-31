#include <gint/display.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

static struct {
    uint16_t before[16];
    uint16_t pixels[DWIDTH * DHEIGHT];
    uint16_t after[16];
} storage = { .before = {0x1357}, .after = {0x2468} };
#define pixels storage.pixels
uint16_t *gint_vram = pixels;
void host_check_guards(void)
{
    assert(storage.before[0] == 0x1357 && storage.after[0] == 0x2468);
    for(int i=1;i<16;++i) assert(!storage.before[i] && !storage.after[i]);
}
static struct dwindow window = { 0, 0, DWIDTH, DHEIGHT };

struct dwindow dwindow_set(struct dwindow next)
{
    assert(next.left >= 0 && next.top >= 0 && next.right <= DWIDTH &&
        next.bottom <= DHEIGHT && next.left <= next.right && next.top <= next.bottom);
    struct dwindow previous = window;
    window = next;
    return previous;
}

void dpixel(int x, int y, int color)
{
    if(x < window.left || y < window.top || x >= window.right || y >= window.bottom)
        return;
    assert(x >= 0 && x < DWIDTH && y >= 0 && y < DHEIGHT);
    pixels[y * DWIDTH + x] = (uint16_t)color;
}

void dclear(color_t color)
{
    drect(0, 0, DWIDTH - 1, DHEIGHT - 1, color);
}

void drect(int x1, int y1, int x2, int y2, int color)
{
    if(x1 < window.left) x1 = window.left;
    if(y1 < window.top) y1 = window.top;
    if(x2 >= window.right) x2 = window.right - 1;
    if(y2 >= window.bottom) y2 = window.bottom - 1;
    for(int y = y1; y <= y2; ++y)
        for(int x = x1; x <= x2; ++x)
            dpixel(x, y, color);
}

/* Compact 5x7 test font for host screenshots, not linked into the add-in.
   The calculator uses gint's own font, so text spacing differs slightly. */
static const uint8_t digits[10][5] = {
    {62,81,73,69,62}, {0,66,127,64,0}, {66,97,81,73,70},
    {33,65,69,75,49}, {24,20,18,127,16}, {39,69,69,69,57},
    {60,74,73,73,48}, {1,113,9,5,3}, {54,73,73,73,54}, {6,73,73,41,30}
};
static const uint8_t letters[26][5] = {
    {126,9,9,9,126}, {127,73,73,73,54}, {62,65,65,65,34},
    {127,65,65,34,28}, {127,73,73,73,65}, {127,9,9,9,1},
    {62,65,73,73,122}, {127,8,8,8,127}, {0,65,127,65,0},
    {32,64,65,63,1}, {127,8,20,34,65}, {127,64,64,64,64},
    {127,2,12,2,127}, {127,4,8,16,127}, {62,65,65,65,62},
    {127,9,9,9,6}, {62,65,81,33,94}, {127,9,25,41,70},
    {38,73,73,73,50}, {1,1,127,1,1}, {63,64,64,64,63},
    {31,32,64,32,31}, {127,32,24,32,127}, {99,20,8,20,99},
    {3,4,120,4,3}, {97,81,73,69,67}
};

static uint8_t glyph_column(char ch, int column)
{
    if(ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    if(ch >= 'A' && ch <= 'Z') return letters[ch - 'A'][column];
    if(ch >= '0' && ch <= '9') return digits[ch - '0'][column];
    switch(ch) {
    case '.': return column == 2 ? 64 : 0;
    case ':': return column == 2 ? 36 : 0;
    case '-': return 8;
    case '/': return (uint8_t)(1u << (6 - column));
    case '%': return (uint8_t)(column == 0 || column == 4 ? 99 : (1u << (6 - column)));
    case '=': return 20;
    case '(': return column == 2 ? 62 : 0;
    case ')': return column == 2 ? 62 : 0;
    default: return 0;
    }
}

void dtext(int x, int y, int color, const char *text)
{
    for(; *text; ++text, x += 7)
        for(int column = 0; column < 5; ++column) {
            uint8_t bits = glyph_column(*text, column);
            for(int row = 0; row < 7; ++row)
                if(bits & (1u << row)) dpixel(x + column, y + row, color);
        }
}

void dprint(int x, int y, int color, const char *format, ...)
{
    char text[256];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    assert(length >= 0 && (size_t)length < sizeof(text));
    dtext(x, y, color, text);
}

void (*host_present_hook)(void);

void dupdate(void)
{
    if(host_present_hook) host_present_hook();
}

unsigned host_nonzero_pixels(void)
{
    unsigned count = 0;
    for(unsigned i = 0; i < DWIDTH * DHEIGHT; ++i)
        if(pixels[i]) ++count;
    return count;
}

void host_write_ppm(const char *path)
{
    FILE *file = fopen(path, "wb");
    assert(file);
    fprintf(file, "P6\n%d %d\n255\n", DWIDTH, DHEIGHT);
    for(unsigned i = 0; i < DWIDTH * DHEIGHT; ++i) {
        unsigned color = pixels[i];
        fputc((int)((color >> 11 & 31) * 255 / 31), file);
        fputc((int)((color >> 5 & 63) * 255 / 63), file);
        fputc((int)((color & 31) * 255 / 31), file);
    }
    assert(fclose(file) == 0);
}

