#include <stdio.h>

// pre-defined size of the convolution kernel
#define IN_DEPTH    64
#define IN_HEIGHT   64
#define IN_WIDTH    64
#define KERNEL_DEPTH    4
#define KERNEL_HEIGHT   8
#define KERNEL_WIDTH    4
#define OUT_DEPTH   (IN_DEPTH - KERNEL_DEPTH + 1)
#define OUT_HEIGHT  (IN_HEIGHT - KERNEL_HEIGHT + 1)
#define OUT_WIDTH   (IN_WIDTH - KERNEL_WIDTH + 1)

// define a tiling factor
#define TILING_FACTOR 8

// define the arrays that call from on-chip mem
#define IN_SIZE     (IN_DEPTH * IN_HEIGHT * IN_WIDTH)
#define KERNEL_SIZE (KERNEL_DEPTH * KERNEL_HEIGHT * KERNEL_WIDTH)
#define OUT_SIZE    (OUT_DEPTH * OUT_HEIGHT * OUT_WIDTH)

//begin convolution declaration
void convolve(float in[IN_SIZE], float kernel[KERNEL_SIZE], float out[OUT_SIZE]) {
#pragma AGGREGATE compact=auto variable=out
#pragma TOP name=convolve


// on-chip memo arrays for the input, kernel, and output
#pragma ARRAY_PARTITION variable=kernel cyclic factor=8 dim=1
#pragma ARRAY_PARTITION variable=out cyclic factor=8 dim=1


    // find the center position of the kernel
    int kcenter_w = KERNEL_WIDTH / 2;
    int kcenter_h = KERNEL_HEIGHT / 2;
    int kcenter_d = KERNEL_DEPTH / 2;

    // convolution
#pragma UNROLL factor=TILING_FACTOR
for (int d_tile = 0; d_tile < OUT_DEPTH; d_tile += TILING_FACTOR) {
    for (int h_tile = 0; h_tile < OUT_HEIGHT; h_tile += TILING_FACTOR) {
        for (int w_tile = 0; w_tile < OUT_WIDTH; w_tile += TILING_FACTOR) {
            // Tile input array
            float tile_in[TILING_FACTOR][IN_HEIGHT][IN_WIDTH];
            #pragma ARRAY_PARTITION variable=tile_in cyclic factor=TILING_FACTOR dim=1

            // Load input tile
            for (int t = 0; t < TILING_FACTOR; t++) {
                int d = d_tile + t;
                if (d >= OUT_DEPTH) {
                    break;
                }
                for (int y = 0; y < IN_HEIGHT; y++) {
                    for (int x = 0; x < IN_WIDTH; x++) {
                        tile_in[t][y][x] = in[d * IN_HEIGHT * IN_WIDTH + y * IN_WIDTH + x];
                    }
                }
            }
            // Tile output array
            float tile_out[TILING_FACTOR][TILING_FACTOR][TILING_FACTOR];
            #pragma ARRAY_PARTITION variable=tile_out cyclic factor=TILING_FACTOR dim=1
            // Compute output tile
            for (t = 0; t < TILING_FACTOR; t++) {
                int d = d_tile + t;
                if (d >= OUT_DEPTH) {
                    break;
                }
                for (int h = 0; h < TILING_FACTOR; h++) {
                    int y = h_tile + h;
                    if (y >= OUT_HEIGHT) {
                        break;
                    }
                    for (int w = 0; w < TILING_FACTOR; w++) {
                        int x = w_tile + w;
                        if (x >= OUT_WIDTH) {
                            break;
                        }

                        float sum = 0.0;
                        for (int z = 0; z < KERNEL_DEPTH; z++) {
                            for (int k = 0; k < KERNEL_HEIGHT; k++) {
                                for (int l = 0; l < KERNEL_WIDTH; l++) {
                                    int in_x = x + l - kcenter_w;
                                    int in_y = y + k - kcenter_h;
                                    int in_z = d + z - kcenter_d;
                                    if (in_x >= 0 && in_x < IN_WIDTH &&
                                        in_y >= 0 && in_y < IN_HEIGHT &&
                                        in_z >= 0 && in_z < IN_DEPTH) {
                                        sum += tile_in[t][in_y][in_x] * kernel[z * KERNEL_HEIGHT * KERNEL_WIDTH + k * KERNEL_WIDTH + l];
                                    }
                                }
                            }
                        }
                        tile_out[t][h][w] = sum;
                    }
                }
            }

            // Store output tile
            for (t = 0; t < TILING_FACTOR; t++) {
                int d = d_tile + t;
                if (d >= OUT_DEPTH) {
                    break;
                }
                for (int h = 0; h < TILING_FACTOR; h++) {
                    int y = h_tile + h;
                    if (y >= OUT_HEIGHT) {
                        break;
                    }
                    for (int w = 0; w < TILING_FACTOR; w++) {
                        int x = w_tile + w;
                        if (x >= OUT_WIDTH) {
                            break;
                        }
                        out[d * OUT_HEIGHT * OUT_WIDTH + y * OUT_WIDTH + x] = tile_out[t][h][w];
                    }
                }
            }
}
    }}}
