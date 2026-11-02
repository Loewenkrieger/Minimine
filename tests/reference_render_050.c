/* Frozen device-tested 0.5.0 renderer, used only for visual regression tests. */
#define render_invalidate legacy050_invalidate
#define render_world legacy050_world
#define render_game legacy050_game
#define render_inventory legacy050_inventory
#define render_help legacy050_help
#define render_results legacy050_results
#define render_error legacy050_error
#define render_menu legacy050_menu
#define render_progress legacy050_progress
#define render_slots legacy050_slots
#define render_confirm legacy050_confirm
#define render_message legacy050_message
#include "../benchmarks/1.0.0/render-reference.c"
