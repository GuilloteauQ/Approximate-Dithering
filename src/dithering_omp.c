#include <stdio.h>
#include <stdlib.h>
#include "Util.h"
#include "image_lib.h"
#include <omp.h>
#include <time.h>

#define RIGHT 7 / 16
#define BOT_LEFT 3 / 16
#define BOT 5 / 16
#define BOT_RIGHT 1 / 16

#define ITERATIONS 10

inline int16_t update_and_compute_error(int16_t* data, size_t i) {
    int16_t current_value = data[i];
    int16_t new_value = (current_value < 127) ? 0 : 255;
    // int16_t new_value = ((current_value - 127) / 127) * 255;
    data[i] = new_value;
    return current_value - new_value;
}

void propagate_error_exact(int16_t* data,
                           size_t rows,
                           size_t cols,
                           int16_t error,
                           size_t x,
                           size_t y) {
    if (x < cols - 1) {
        data[(y + 0) * cols + (x + 1)] =
            error * RIGHT + data[(y + 0) * cols + (x + 1)];
    }
    if (y < rows - 1) {
        if (x > 0) {
            data[(y + 1) * cols + (x - 1)] =
                error * BOT_LEFT + data[(y + 1) * cols + (x - 1)];
        }
        data[(y + 1) * cols + (x + 0)] =
            error * BOT + data[(y + 1) * cols + (x + 0)];
        if (x < cols - 1) {
            data[(y + 1) * cols + (x + 1)] =
                error * BOT_RIGHT + data[(y + 1) * cols + (x + 1)];
        }
    }
}


#define TOP_DEPS(n) int16_t* top##n = (y > 0) ? &data[(y - 1) * cols + x + n] : NULL;
#define TOP_NAME(n) *top##n



void floyd_steinberg_par_chunk(Image* image, size_t chunk) {
    int16_t* data = image->pixels;
    size_t rows = image->rows;
    size_t cols = image->cols;
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x += chunk) {
            int16_t* here      =  &data[y * cols + x];

            int16_t* left      =  (x + chunk > 0)                 ? &data[(y + 0) * cols + (x - chunk - 1)] : NULL;
            int16_t* top_left  =  (x + chunk > 0 && y > 0)        ? &data[(y - 1) * cols + (x - chunk - 1)] : NULL;
            int16_t* top_right =  (x < cols - 1 - chunk && y > 0) ? &data[(y - 1) * cols + (x + chunk + 1)] : NULL;
            int16_t* top       =  (y > 0)                         ? &data[(y - 1) * cols + (x + 0)] : NULL;


#pragma omp task depend(in:*top_left, *top, *top_right, *left) depend(out: *here)
            {
                for (size_t k = 0; k < chunk; k++) {
                    int16_t error = update_and_compute_error(data, y * cols + x + k);
                    propagate_error_exact(data, rows, cols, error, x + k, y);
                }
            }
        }
    }
}


void floyd_steinberg(Image* image) {
    int16_t* data = image->pixels;
    size_t rows = image->rows;
    size_t cols = image->cols;
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            int16_t error = update_and_compute_error(data, y * cols + x);
            propagate_error_exact(data, rows, cols, error, x, y);
        }
    }
}

int main(int argc, char** argv) {
    char* filename = argv[1];
    size_t chunk = atoi(argv[2]);

    clock_t t;
    int nb_threads = -1;

    for (int j = 0; j < ITERATIONS; j++) {
        Image* approx_image = read_image_from_file(filename);
        approx_image = duplicate(approx_image, 1);
        if (chunk == 0) {
            t = clock();
            floyd_steinberg(approx_image);
            t = clock() - t;
        } else {
            t = clock();
#pragma omp parallel
            {
            nb_threads = omp_get_num_threads();
#pragma omp single
            floyd_steinberg_par_chunk(approx_image, chunk);
            }
            t = clock() - t;
        }
        double time = ((double)t) / (CLOCKS_PER_SEC * nb_threads);
        write_image_to_file(approx_image, "image_out.pgm");
        free(approx_image->pixels);
        free(approx_image);
        printf("%d, %ld, %f\n", nb_threads, chunk, time);
    }

    return 0;
}
