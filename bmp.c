#include "bmp.h"


byte *generate_header(int width, int height) {
    byte *header = (byte *)malloc(54);
    for (int i = 0; i < 54; i++) header[i] = 0x00;

    int padding = width * 3 % 4 == 0 ? 0 : 4 - width * 3 % 4;

    //bitmap header
    header[0] = 'B';
    header[1] = 'M';
    *((int *)(header + 2)) = height * (width * 3 + padding) + 54;
    *((int *)(header + 10)) = 54;
    //dib header
    *((int *)(header + 14)) = 40;
    *((int *)(header + 18)) = width;
    *((int *)(header + 22)) = height;
    header[26] = 0x01;
    header[28] = 0x18;
    *((int *)(header + 34)) = height * (width * 3 + padding);
    *((int *)(header + 38)) = 2835;
    *((int *)(header + 42)) = 2835;

    return header;
}

byte *generate_pixel_array(byte *header, color_rgb *pixels) {
    int width = *((int *)(header + 18)),
    height = *((int *)(header + 22)),
    padding = width * 3 % 4 == 0 ? 0 : 4 - width * 3 % 4;
    byte *pixel_array = (byte *)malloc(*((int *)(header + 34)));

    int pos = 0;
    for (int h = height - 1; h >= 0; h--) {
        for (int w = 0; w < width; w++) {
            pixel_array[pos++] = pixels[h * width + w].blue;
            pixel_array[pos++] = pixels[h * width + w].green;
            pixel_array[pos++] = pixels[h * width + w].red;
        }
        for (int i = 0; i < padding; i++) pixel_array[pos++] = 0x00;
    }
    return pixel_array;
}

void export_image(byte *header, byte *pixel_array, char *file_name) {
    FILE *fp;
    fp = fopen(file_name, "wb");
    fwrite(header, 1, 54, fp);
    fwrite(pixel_array, 1, *((int *)(header + 34)), fp);
    fclose(fp);
    return;
}

/*
int main() {
    color_rgb pixels[25] = {
        {0xff, 0x00, 0x00}, {0x00, 0xff, 0x00}, {0x00, 0x00, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff},
        {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff},
        {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff},
        {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff},
        {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff},
    };

    byte *header, *pixel_array;

    header = generate_header(5, 5);
    pixel_array = generate_pixel_array(header, pixels);
    export_image(header, pixel_array);

    free(header);
    free(pixel_array);
}
*/
