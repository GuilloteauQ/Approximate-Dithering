#include "image_lib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "Util.h"

Image* read_image_from_file(char* filename) {
    FILE* ppm_file = fopen(filename, "r");

    int ich1 = getc(ppm_file);
    if (ich1 == EOF)
        pm_erreur("EOF / read error / magic number");
    int ich2 = getc(ppm_file);
    int raw_byte = ich2 == '5';

    Image* image = malloc(sizeof(Image));

    image->cols = pm_getint(ppm_file);
    image->rows = pm_getint(ppm_file);
    image->max_val = pm_getint(ppm_file);

    image->pixels = malloc(sizeof(int16_t) * image->cols * image->rows);

    for (size_t i = 0; i < image->rows; i++) {
        for (size_t j = 0; j < image->cols; j++) {
            image->pixels[i * image->cols + j] = (int16_t)(
                (raw_byte) ? pm_getrawbyte(ppm_file) : pm_getint(ppm_file));
        }
    }

    fclose(ppm_file);
    return image;
}

void write_image_to_file(Image* image, char* filename) {
    FILE* output_file = fopen(filename, "w");
    fprintf(output_file, "P5\n");
    fprintf(output_file, "%lu %lu\n", image->cols, image->rows);
    fprintf(output_file, "%lu\n", image->max_val);
    for (size_t i = 0; i < image->cols; i++) {
        for (size_t j = 0; j < image->rows; j++) {
            int16_t p = image->pixels[i * image->rows + j];
            fprintf(output_file, "%c", p);
        }
    }
    fclose(output_file);
}

Image* duplicate(Image* image, size_t iter) {
    size_t new_cols = image->cols * iter;
    size_t new_rows = image->rows * iter;

    int16_t* new_pixels = malloc(sizeof(int16_t) * new_cols * new_rows);

    for (size_t j = 0; j < new_rows; j++) {
        for (size_t i = 0; i < new_cols; i++) {
            new_pixels[i + new_cols * j] =
                image->pixels[(i % image->cols) +
                              (j % image->rows) * image->cols];
        }
    }

    Image* new_image = malloc(sizeof(Image));
    new_image->cols = new_cols;
    new_image->rows = new_rows;
    new_image->max_val = image->max_val;
    new_image->pixels = new_pixels;

    free(image->pixels);
    free(image);

    return new_image;
}

double compute_psnr(Image* image1, Image* image2) {
    assert(image1->cols == image2->cols && image1->rows == image2->rows);
    uint32_t sum = 0;
    int16_t diff = 0;
    size_t rows = image1->rows;
    size_t cols = image1->cols;
    for (size_t j = 0; j < rows; j++) {
        for (size_t i = 0; i < cols; i++) {
            diff = image1->pixels[i + j * rows] - image2->pixels[i + j * rows];
            sum += diff * diff;
        }
    }
    return ((double)sum) / ((double)rows * cols);
}
