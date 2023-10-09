#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef struct color {
    byte red, green, blue;
} color_rgb;

typedef struct bmp_image {
    int width, height;
    color_rgb *pixels;
} bmp_image;

byte *generate_header(int, int);

byte *generate_pixel_array(byte *, color_rgb *);

void export_image(byte *, byte *, const char *);

byte save_image(bmp_image, const char *);

bmp_image load_image(const char *);
