#ifndef KERNEL_H
#define KERNEL_H

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DllExport __declspec(dllexport)
#define THREADS_PER_BLOCKDIM 32

float DllExport *pixelSubtraction(float *h_input1, float *h_input2, int width, int height);
float DllExport *gaussianBlur(float *h_input, int width, int height);
float DllExport *differenceOfGaussian(float *h_input, int width, int height);
float DllExport *laplacian4Neib(float *h_input, int width, int height);
float DllExport *applyMask(float *h_input, int width, int height, int radius, float *h_mask);
float DllExport *applyThreshold(float *h_input, int width, int height, int threshold);
float DllExport *underSampling(float *h_input, int width, int height, float sizeFactor);
float DllExport *raiseImage(float *h_input, int width, int height, float factor);

#endif // KERNEL_H