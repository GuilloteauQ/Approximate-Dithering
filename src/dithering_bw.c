#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Util.h"
#include "image_lib.h"
#include "strategies.h"

#define SAVE 0
#define RATIO 1

#define ITERATIONS 5

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Oops you forgot to give the pgm file!\n");
        exit(1);
    }
    srand(0);
    char* filename = argv[1];
    char* filename_compa = argv[2];
    char* strategy_name = argv[3];
    propagate_f strategy_f = get_desired_strategy(strategy_name);
    if (strategy_f == NULL) {
        fprintf(stderr, "Could not find strategy '%s'\n", strategy_name);
        exit(1);
    }

    size_t size_factor = RATIO;
    clock_t t;

    Image* ref_image = read_image_from_file(filename_compa);

    for (int j = 0; j < ITERATIONS; j++) {
        Image* approx_image = read_image_from_file(filename);
        approx_image = duplicate(approx_image, size_factor);
        t = clock();
        floyd_steinberg(approx_image, strategy_f);
        t = clock() - t;
        double time = ((double)t) / CLOCKS_PER_SEC;
        double psnr = compute_psnr(ref_image, approx_image);
        free(approx_image->pixels);
        free(approx_image);
        printf("%s, %f, %f\n", strategy_name, time, psnr);
    }

    return 0;
}
