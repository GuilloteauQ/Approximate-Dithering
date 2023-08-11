#include <stdint.h>
#include <stdlib.h>

typedef struct image_t {
    size_t cols;
    size_t rows;
    size_t max_val;
    int16_t* pixels;
} Image;

Image* read_image_from_file(char* filename);

void write_image_to_file(Image* image, char* filename);

Image* duplicate(Image* image, size_t iter);

double compute_psnr(Image* image1, Image* image2);
