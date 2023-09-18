#ifndef STRATEGIES_H_
#define STRATEGIES_H_

#define RIGHT 7 / 16
#define BOT_LEFT 3 / 16
#define BOT 5 / 16
#define BOT_RIGHT 1 / 16

#define Q 16
#define ERROR_THRESHOLD 100

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

void propagate_error_branchless_imprecise(int16_t* data,
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

void propagate_error_branchless_imprecise_approximate(int16_t* data,
                                                      size_t rows,
                                                      size_t cols,
                                                      int16_t error,
                                                      size_t x,
                                                      size_t y) {
    if (((y % Q == 0) && (x % Q == 0))) propagate_error_branchless_imprecise(data, rows, cols, error, x, y);
}

inline int16_t update_and_compute_error(int16_t* data, size_t i) {
    int16_t current_value = data[i];
    int16_t new_value = (current_value < 127) ? 0 : 255;
    // int16_t new_value = ((current_value - 127) / 127) * 255;
    data[i] = new_value;
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

void floyd_steinberg(Image* image, propagate_f f) {
    size_t rows = image->rows;
    size_t cols = image->cols;
    floyd_steinberg_f(image->pixels, rows, cols, f);
}

typedef struct {
    const char* name;
    propagate_f f;
} strategy;

const strategy strategies[] = {
    {.name = "EXACT", .f = propagate_error_exact},
    {.name = "APPROX", .f = propagate_error_approximate},
    {.name = "BRANCHLESS", .f = propagate_error_branchless_exact},
    {.name = "BRANCHLESS_IMPRECISE", .f = propagate_error_branchless_imprecise},
    {.name = "BRANCHLESS_EXACT_APPROX",
     .f = propagate_error_branchless_exact_approximate},
    {.name = "BRANCHLESS_IMPRECISE_APPROX",
     .f = propagate_error_branchless_imprecise_approximate},
    {.name = "ONLY_BOT", .f = propagate_error_only_bot},
    {.name = "EPSILON", .f = propagate_error_eps},
};

#define NSTRATEGIES sizeof(strategies) / sizeof(strategies[0])

propagate_f get_desired_strategy(char* strategy_name) {
    for (long unsigned int i = 0; i < NSTRATEGIES; i++) {
        if (strcmp(strategy_name, strategies[i].name) == 0) {
            return strategies[i].f;
        }
    }
    return NULL;
}

#endif
