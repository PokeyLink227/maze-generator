#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef struct color {
    byte red, green, blue;
} color_rgb;

byte *generate_header(int, int);

byte *generate_pixel_array(byte *, color_rgb *);

void export_image(byte *, byte *);
