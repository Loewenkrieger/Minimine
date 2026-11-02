#ifndef BENCH_HOST_IMAGE_H
#define BENCH_HOST_IMAGE_H
#include <stdint.h>
typedef struct {int width,height,format;void *data;} image_t;
typedef image_t bopti_image_t;
typedef struct {int dummy;} font_t;
struct gint_image_box;
struct gint_image_cmd;
#endif
