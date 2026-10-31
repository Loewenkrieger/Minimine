/* Pixel reference from the released 0.1.0 world renderer. Test only. */
#include "render.h"

#include <gint/display.h>

_Static_assert(DWIDTH == SCREEN_W && DHEIGHT == SCREEN_H,
    "This renderer requires the ClassPad II display");

#define INK C_RGB(3, 5, 8)
#define PANEL C_RGB(3, 5, 9)
#define MUTED C_RGB(17, 21, 23)
#define ACCENT C_RGB(13, 29, 19)

static void outline(int x, int y, int width, int height, int color)
{
    drect(x, y, x + width - 1, y, color);
    drect(x, y + height - 1, x + width - 1, y + height - 1, color);
    drect(x, y, x, y + height - 1, color);
    drect(x + width - 1, y, x + width - 1, y + height - 1, color);
}

static void tile_draw(Tile tile, int x, int y, int variant)
{
    switch(tile) {
    case TILE_GRASS:
    case TILE_DIRT:
        drect(x, y, x + 15, y + 15, C_RGB(15, 10, 6));
        drect(x + 2 + variant, y + 7, x + 5 + variant, y + 8, C_RGB(20, 14, 8));
        drect(x + 10, y + 12, x + 12, y + 13, C_RGB(10, 7, 5));
        if(tile == TILE_GRASS) {
            drect(x, y, x + 15, y + 2, C_RGB(10, 24, 9));
            drect(x + 2, y + 3, x + 4, y + 4, C_RGB(7, 18, 7));
        }
        break;
    case TILE_STONE:
    case TILE_ORE:
    case TILE_BEDROCK:
        drect(x, y, x + 15, y + 15, tile == TILE_BEDROCK ?
            C_RGB(5, 6, 9) : C_RGB(12, 14, 17));
        drect(x + 1, y + 1, x + 13, y + 1, C_RGB(17, 19, 22));
        drect(x + 3, y + 8, x + 6, y + 8, C_RGB(8, 10, 13));
        drect(x + 11, y + 12, x + 14, y + 13, C_RGB(8, 10, 13));
        if(tile == TILE_ORE) {
            drect(x + 3, y + 4, x + 6, y + 6, C_RGB(27, 19, 9));
            drect(x + 9, y + 9, x + 11, y + 12, C_RGB(24, 15, 7));
        }
        break;
    case TILE_WOOD:
        drect(x, y, x + 15, y + 15, C_RGB(20, 13, 7));
        drect(x, y + 7, x + 15, y + 8, C_RGB(12, 8, 4));
        drect(x + 1, y + 1, x + 14, y + 1, C_RGB(25, 18, 10));
        break;
    case TILE_TRUNK:
        drect(x + 5, y, x + 10, y + 15, C_RGB(12, 8, 5));
        drect(x + 6, y, x + 7, y + 15, C_RGB(21, 14, 7));
        break;
    case TILE_LEAVES:
        drect(x + 1, y + 1, x + 14, y + 14, C_RGB(5, 15, 8));
        drect(x + 3, y + 2, x + 11, y + 8, C_RGB(7, 21, 10));
        drect(x + 5, y + 3, x + 9, y + 4, C_RGB(11, 25, 12));
        break;
    default: break;
    }
}

static void background(const Game *game)
{
    drect(0, VIEW_TOP, SCREEN_W - 1, VIEW_TOP + VIEW_H - 1, C_RGB(15, 23, 28));
    int horizon = 30 * TILE_SIZE - game->camera_y + VIEW_TOP;
    for(int i = -1; i < 12; ++i) {
        int x = i * 40 - (game->camera_x / 5) % 40;
        int h = 20 + ((i + game->camera_x / 200 + 128) * 17 % 45);
        drect(x, horizon - h, x + 40, horizon + 80, C_RGB(11, 19, 20));
    }
    int underground = 36 * TILE_SIZE - game->camera_y + VIEW_TOP;
    if(underground < VIEW_TOP) underground = VIEW_TOP;
    if(underground < VIEW_TOP + VIEW_H)
        drect(0, underground, SCREEN_W - 1, VIEW_TOP + VIEW_H - 1, C_RGB(6, 7, 10));
}

static void player_draw(const Game *game)
{
    const Player *p = &game->player;
    int x = p->x / FIXED_ONE - game->camera_x;
    int y = p->y / FIXED_ONE - game->camera_y + VIEW_TOP;
    int stride = p->vx && p->grounded ? (int)(game->ticks / 6 % 2) : 0;
    drect(x + 1, y, x + 8, y + 4, C_RGB(9, 6, 4));
    drect(x + 2, y + 3, x + 8, y + 8, C_RGB(29, 22, 15));
    dpixel(x + (p->facing > 0 ? 7 : 3), y + 5, INK);
    drect(x + 1, y + 9, x + 8, y + 15, C_RGB(28, 14, 6));
    drect(x, y + 10, x + 1, y + 16, C_RGB(29, 22, 15));
    drect(x + 8, y + 10, x + 9, y + 16, C_RGB(29, 22, 15));
    drect(x + 1, y + 16, x + 4, y + 21 - stride, C_RGB(5, 9, 17));
    drect(x + 6, y + 16, x + 8, y + 20 + stride, C_RGB(5, 9, 17));
}

void reference_world(const Game *game, const RenderInfo *info)
{
    struct dwindow previous = dwindow_set((struct dwindow){
        0, VIEW_TOP, SCREEN_W, VIEW_TOP + VIEW_H });
    background(game);
    int first_x = game->camera_x / TILE_SIZE;
    int first_y = game->camera_y / TILE_SIZE;
    int last_x = (game->camera_x + SCREEN_W - 1) / TILE_SIZE;
    int last_y = (game->camera_y + VIEW_H - 1) / TILE_SIZE;
    for(int y = first_y; y <= last_y; ++y)
        for(int x = first_x; x <= last_x; ++x)
            tile_draw(game_tile(game, x, y), x * TILE_SIZE - game->camera_x,
                y * TILE_SIZE - game->camera_y + VIEW_TOP, (x + y) % 3);
    player_draw(game);
    if(!info->benchmark) {
        int tx, ty;
        game_target(game, &tx, &ty);
        outline(tx * TILE_SIZE - game->camera_x,
            ty * TILE_SIZE - game->camera_y + VIEW_TOP,
            TILE_SIZE, TILE_SIZE, C_WHITE);
    }
    dwindow_set(previous);
}

