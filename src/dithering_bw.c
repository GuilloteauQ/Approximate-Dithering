#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Util.h"
#include "image_lib.h"

#define RIGHT 7 / 16
#define BOT_LEFT 3 / 16
#define BOT 5 / 16
#define BOT_RIGHT 1 / 16

#define SAVE 0
#define Q 16
#define RATIO 32
#define ERROR_THRESHOLD 100

#define ITERATIONS 5

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

typedef void (*propagate_f)(int16_t* data,
                            size_t rows,
                            size_t cols,
                            int16_t error,
                            size_t x,
                            size_t y);

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

void propagate_error_branchless_exact(int16_t* data,
                                      size_t rows,
                                      size_t cols,
                                      int16_t error,
                                      size_t x,
                                      size_t y) {
    int16_t cond_x_sup = (x < cols - 1);
    int16_t cond_x_inf = (x > 0);
    int16_t cond_y = (y < rows - 1);
    data[((y + 0) * cols + (x + 1)) * cond_x_sup] += cond_x_sup * error * RIGHT;
    data[((y + 1) * cols + (x + 0)) * cond_y] += cond_y * error * BOT;
    data[((y + 1) * cols + (x - 1)) * cond_x_inf * cond_y] +=
        cond_x_inf * cond_y * error * BOT_LEFT;
    data[((y + 1) * cols + (x + 1)) * cond_x_sup * cond_y] +=
        cond_x_sup * cond_y * error * BOT_RIGHT;
}

void propagate_error_branchless(int16_t* data,
                                size_t rows,
                                size_t cols,
                                int16_t error,
                                size_t x,
                                size_t y) {
    int16_t cond_x_sup = (x < cols - 1);
    int16_t cond_x_inf = (x > 0);
    int16_t cond_y = (y < rows - 1);
    int16_t error_div_16 = error / 16;
    data[((y + 0) * cols + (x + 1)) * cond_x_sup] +=
        cond_x_sup * error_div_16 * 7;
    data[((y + 1) * cols + (x + 0)) * cond_y] += cond_y * error_div_16 * 5;
    data[((y + 1) * cols + (x - 1)) * cond_x_inf * cond_y] +=
        cond_x_inf * cond_y * error_div_16 * 3;
    data[((y + 1) * cols + (x + 1)) * cond_x_sup * cond_y] +=
        cond_x_sup * cond_y * error_div_16;
}

void propagate_error_only_bot(int16_t* data,
                              size_t rows,
                              size_t cols,
                              int16_t error,
                              size_t x,
                              size_t y) {
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

void propagate_error_eps(int16_t* data,
                         size_t rows,
                         size_t cols,
                         int16_t error,
                         size_t x,
                         size_t y) {
    if (__builtin_abs(error) < ERROR_THRESHOLD)
        return;
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

void propagate_error_approximate(int16_t* data,
                                 size_t rows,
                                 size_t cols,
                                 int16_t error,
                                 size_t x,
                                 size_t y) {
    if (!((y % Q == 0) && (x % Q == 0)))
        return;
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

void propagate_error_branchless_exact_approximate(int16_t* data,
                                                  size_t rows,
                                                  size_t cols,
                                                  int16_t error,
                                                  size_t x,
                                                  size_t y) {
    if (!((y % Q == 0) && (x % Q == 0)))
        return;
    propagate_error_branchless_exact(data, rows, cols, error, x, y);
}

void propagate_error_branchless_meh_approximate(int16_t* data,
                                                size_t rows,
                                                size_t cols,
                                                int16_t error,
                                                size_t x,
                                                size_t y) {
    if (!((y % Q == 0) && (x % Q == 0)))
        return;
    propagate_error_branchless(data, rows, cols, error, x, y);
}

int16_t update_and_compute_error(int16_t* data, size_t i) {
    int16_t current_value = data[i];
    int16_t new_value = (current_value < 127) ? 0 : 255;
    *(data + i) = new_value;
    return current_value - new_value;
}

void floyd_steinberg_f(int16_t* data, size_t rows, size_t cols, propagate_f f) {
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            int16_t error = update_and_compute_error(data, y * cols + x);
            f(data, rows, cols, error, x, y);
        }
    }
}

void sequential_floyd_steinberg_approx(int16_t* data,
                                       size_t rows,
                                       size_t cols) {
    floyd_steinberg_f(data, rows, cols, propagate_error_approximate);
}

void floyd_steinberg(Image* image, propagate_f f) {
    size_t rows = image->rows;
    size_t cols = image->cols;
    floyd_steinberg_f(image->pixels, rows, cols, f);
}

double mean(double times[]) {
    double mean = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        mean += times[i];
    }
    return mean / (double)(ITERATIONS);
}

double std(double times[], double mean) {
    double std = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        std += (mean - times[i]) * (mean - times[i]);
    }
    return std / (double)(ITERATIONS);
}

typedef struct {
    const char* name;
    propagate_f f;
} approx;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Oops you forgot to give the pgm file!\n");
        exit(1);
    }
    char* filename = argv[1];
    srand(0);

    Image* ppm_image;
    // propagate_error_exact,

    approx approx_functions[] = {
        {.name = "appox", .f = propagate_error_approximate},
        {.name = "branchless", .f = propagate_error_branchless_exact},
        {.name = "branchless_neh", .f = propagate_error_branchless},
        {.name = "branchless_exact_approx",
         .f = propagate_error_branchless_exact_approximate},
        {.name = "branchless_meh_approx",
         .f = propagate_error_branchless_meh_approximate},
        {.name = "only_bot", .f = propagate_error_only_bot},
        {.name = "eps", .f = propagate_error_eps},
    };

    size_t size_factor = RATIO;
    clock_t t;

    // EXACT

    double times[ITERATIONS];

    for (int j = 0; j < ITERATIONS; j++) {
        ppm_image = read_image_from_file(filename);
        ppm_image = duplicate(ppm_image, size_factor);
        t = clock();
        floyd_steinberg(ppm_image, propagate_error_exact);
        t = clock() - t;
        // printf("[*] dithered exact\n");
        double time_seq = ((double)t) / CLOCKS_PER_SEC;
        times[j] = time_seq;
        // printf(" [*] time = %f\n", time_seq);
        free(ppm_image->pixels);
        free(ppm_image);
    }
    double mean_times = mean(times);
    double std_times = std(times, mean_times);
    printf("%s: %f (± %f)\n", "Seq", mean_times, std_times);

    // APPROX

    for (long unsigned int i = 0; i < ARRAY_LEN(approx_functions); i++) {
        for (int j = 0; j < ITERATIONS; j++) {
            // printf("ITER: %d/%d\n", j, ITERATIONS);
            Image* approx_image = read_image_from_file(filename);
            approx_image = duplicate(approx_image, size_factor);
            t = clock();
            floyd_steinberg(approx_image, approx_functions[i].f);
            t = clock() - t;
            double time_approx = ((double)t) / CLOCKS_PER_SEC;
            times[j] = time_approx;
            free(approx_image->pixels);
            free(approx_image);
        }
        double mean_times = mean(times);
        double std_times = std(times, mean_times);
        printf("%s: %f (± %f)\n", approx_functions[i].name, mean_times,
               std_times);
    }

    return 0;
}
