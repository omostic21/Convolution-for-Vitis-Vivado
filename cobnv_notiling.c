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
#define TILING_FACTOR 4

// define the arrays that call from on-chip mem
#define IN_SIZE     (IN_DEPTH * IN_HEIGHT * IN_WIDTH)
#define KERNEL_SIZE (KERNEL_DEPTH * KERNEL_HEIGHT * KERNEL_WIDTH)
#define OUT_SIZE    (OUT_DEPTH * OUT_HEIGHT * OUT_WIDTH)

//begin convolution declaration
void convolve(float in[IN_SIZE], float kernel[KERNEL_SIZE], float out[OUT_SIZE]) {
/*
#pragma HLS AGGREGATE compact=auto variable=out
#pragma HLS TOP name=convolve


// on-chip memo arrays for the input, kernel, and output
#pragma HLS ARRAY_PARTITION variable=kernel cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=out cyclic factor=8 dim=1
*/

    // find the center position of the kernel
    int kcenter_w = KERNEL_WIDTH / 2;
    int kcenter_h = KERNEL_HEIGHT / 2;
    int kcenter_d = KERNEL_DEPTH / 2;
float sum = 0.0;
    // convolution
    for (int d = 0; d < OUT_DEPTH; d++) {
        for (int h = 0; h < OUT_HEIGHT; h++) {
            for (int w = 0; w < OUT_WIDTH; w++) {
                
              
                for (int z = 0; z < KERNEL_DEPTH; z++) { 
                   for (int y = 0; y < KERNEL_HEIGHT; y++) {

                        for (int x = 0; x < KERNEL_WIDTH; x++) {

                            int in_x = w + x - kcenter_w;
                            int in_y = h + y - kcenter_h;
                            int in_z = d + z - kcenter_d;
                            if (in_x < 0 || in_x >= IN_WIDTH ||
                                in_y < 0 || in_y >= IN_HEIGHT ||
                                in_z < 0 || in_z >= IN_DEPTH) {
                                continue;
                            }
                            
                            sum += in[in_z * IN_HEIGHT * IN_WIDTH + in_y * IN_WIDTH + in_x] *
                                   kernel[z * KERNEL_HEIGHT * KERNEL_WIDTH + y * KERNEL_WIDTH + x];
                        }
                    }
                }
                out[d * OUT_HEIGHT * OUT_WIDTH + h * OUT_WIDTH + w] = sum;
            }
        }
    }
}
