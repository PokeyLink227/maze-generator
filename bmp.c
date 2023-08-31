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
    FILE *fp = fopen(file_name, "wb");
    fwrite(header, 1, 54, fp);
    fwrite(pixel_array, 1, *((int *)(header + 34)), fp);
    fclose(fp);
    return;
}

byte save_image(bmp_image img, char *file_name) {
    int padding = img.width * 3 % 4 == 0 ? 0 : 4 - img.width * 3 % 4;
    byte *img_raw = (byte *)malloc(img.height * (img.width * 3 + padding) + 54);
    for (int i = 0; i < 54; i++) img_raw[i] = 0x00;

    //bitmap header
    img_raw[0] = 'B';
    img_raw[1] = 'M';
    *((int *)(img_raw + 2)) = img.height * (img.width * 3 + padding) + 54;
    *((int *)(img_raw + 10)) = 54;
    //dib header
    *((int *)(img_raw + 14)) = 40;
    *((int *)(img_raw + 18)) = img.width;
    *((int *)(img_raw + 22)) = img.height;
    img_raw[26] = 0x01;
    img_raw[28] = 0x18;
    *((int *)(img_raw + 34)) = img.height * (img.width * 3 + padding);
    *((int *)(img_raw + 38)) = 2835;
    *((int *)(img_raw + 42)) = 2835;

    int pos = 54;
    for (int h = img.height - 1; h >= 0; h--) {
        for (int w = 0; w < img.width; w++) {
            img_raw[pos] = img.pixels[h * img.width + w].blue;
            img_raw[pos + 1] = img.pixels[h * img.width + w].green;
            img_raw[pos + 2] = img.pixels[h * img.width + w].red;
            pos += 3;
        }
        for (int i = 0; i < padding; i++) img_raw[pos++] = 0x00;
    }

    FILE *fp = fopen(file_name, "wb");
    fwrite(img_raw, 1, *((int *)(img_raw + 2)), fp);
    fclose(fp);
    free(img_raw);
    return 1;
}

bmp_image load_image(char *file_name) {
    bmp_image img = (bmp_image){0, 0, 0};
    FILE *fp = fopen(file_name, "rb");
    byte *header = malloc(54);

    if (fread(header, 1, 54, fp) != 54) return img;
    if (header[0] != 'B' || header[1] != 'M') return img;
    img.width = *((int *)(header + 18));
    img.height = *((int *)(header + 22));

    byte *pixeldata = malloc(*((int *)(header + 34)));
    if (fread(pixeldata, 1, *((int *)(header + 34)), fp) != *((unsigned int *)(header + 34))) return img;
    fclose(fp);
    free(header);

    img.pixels = malloc(sizeof(color_rgb) * img.width * img.height);
    int padding = img.width * 3 % 4 == 0 ? 0 : 4 - img.width * 3 % 4;
    int pos = 0;
    for (int h = img.height - 1; h >= 0; h--) {
        for (int w = 0; w < img.width; w++) {
            // might be faster to use p, p + 1, p + 2 then increment p because of cpu mem blocking NOTE: it is faster :D
            img.pixels[h * img.width + w].blue = pixeldata[pos];
            img.pixels[h * img.width + w].green = pixeldata[pos + 1];
            img.pixels[h * img.width + w].red = pixeldata[pos + 2];
            pos += 3;
        }
        pos += padding;
    }
    return img;
}
