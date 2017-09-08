#include "kernel.cuh"

__global__ void raiseImageKernel(float *output, float *input, int width, int height, float factor)
{
	int i = threadIdx.x + blockIdx.x*blockDim.x;
	int j = threadIdx.y + blockIdx.y*blockDim.y;

	if (i >= width || j >= height) return;

	int idx = j*width + i;

	output[idx] = factor * input[idx];
}

float DllExport *raiseImage(float *h_input, int width, int height, float factor);

__global__ void pixelSubtractionKernel(float *output, float *input1, float *input2, int width, int height)
{
	int i = threadIdx.x + blockIdx.x*blockDim.x;
	int j = threadIdx.y + blockIdx.y*blockDim.y;

	int idx = j*width + i;

	if (i < width && j < height)
		output[idx] = input1[idx] - input2[idx];
}

float DllExport *pixelSubtraction(float *h_input1, float *h_input2, int width, int height)
{
	int elementCount = width*height;
	int size = elementCount*sizeof(float);

	float *h_output1 = (float*)malloc(size), *h_output2;
	float *dev_input1, *dev_input2, *dev_output;

	cudaMalloc((void**)&dev_input1, size);
	cudaMalloc((void**)&dev_input2, size);
	cudaMalloc((void**)&dev_output, size);

	cudaMemcpy(dev_input1, h_input1, size, cudaMemcpyHostToDevice);
	cudaMemcpy(dev_input2, h_input2, size, cudaMemcpyHostToDevice);

	dim3 threadsPerBlock(THREADS_PER_BLOCKDIM, THREADS_PER_BLOCKDIM);
	dim3 blockCount(((width - 1) / THREADS_PER_BLOCKDIM) + 1, ((height - 1) / THREADS_PER_BLOCKDIM) + 1);

	pixelSubtractionKernel << <blockCount, threadsPerBlock >> >(dev_output, dev_input1, dev_input2, width, height);

	cudaMemcpy(h_output1, dev_output, size, cudaMemcpyDeviceToHost);
	float factor = 0.0;
	for (int i = 0; i < elementCount; i++)
		if (factor < h_output1[i])
			factor = h_output1[i];
	if (factor == 0.0) return h_output1;
	factor = 1.0 / factor;

	raiseImageKernel << <blockCount, threadsPerBlock >> >(dev_input1, dev_output, width, height, factor);
	h_output2 = (float*)malloc(size);
	cudaMemcpy(h_output2, dev_input1, size, cudaMemcpyDeviceToHost);

	free(h_output1);
	cudaFree(dev_input1);
	cudaFree(dev_input2);
	cudaFree(dev_output);

	return h_output2;
}

__global__ void gaussianBlurKernel(float *output, float *input, int width, int height)
{
	const int radius = 2;

	int i = threadIdx.x + blockIdx.x*blockDim.x;
	int j = threadIdx.y + blockIdx.y*blockDim.y;

	if (i >= width || j >= height) return;

	int idx = j*width + i;
	int ix, jy;
	float total = 0.0, totalFactor = 0.0, tmpFactor;
	int tmp;

	for (int x = -radius; x < radius; x++)
	{
		ix = i + x;
		if ((ix >= 0) && (ix < width))
		{
			for (int y = -radius; y < radius; y++)
			{
				jy = j + y;
				if ((jy >= 0) && (jy < height))
				{
					tmp = x*x + y*y;
					switch (tmp)
					{
					case 4:			// Apparait 8 fois
						tmpFactor = 5.0;
						break;
					case 5:
						tmpFactor = 4.0;
						break;
					case 8:
						tmpFactor = 2.0;
						break;
					case 1:
						tmpFactor = 12.0;
						break;
					case 2:
						tmpFactor = 9.0;
						break;
					default:		// Apparait 1 fois
						tmpFactor = 15.0;
						break;
					}
					totalFactor += tmpFactor;
					total += tmpFactor*input[ix + jy*width];
				}
			}
		}
	}

	output[idx] = total / totalFactor;
}

float DllExport *gaussianBlur(float *h_input, int width, int height)
{
	int elementCount = height*width;
	int size = elementCount*sizeof(float);

	float *h_output = (float*)malloc(size);
	float *dev_input, *dev_output;

	cudaMalloc((void**)&dev_input, size);
	cudaMalloc((void**)&dev_output, size);

	cudaMemcpy(dev_input, h_input, size, cudaMemcpyHostToDevice);

	dim3 threadsPerBlock(THREADS_PER_BLOCKDIM, THREADS_PER_BLOCKDIM);
	dim3 blockCount(((width - 1) / THREADS_PER_BLOCKDIM) + 1, ((height - 1) / THREADS_PER_BLOCKDIM) + 1);

	gaussianBlurKernel << < blockCount, threadsPerBlock >> > (dev_output, dev_input, width, height);

	cudaMemcpy(h_output, dev_output, size, cudaMemcpyDeviceToHost);

	cudaFree(dev_input);
	cudaFree(dev_output);

	return h_output;
}

float DllExport *differenceOfGaussian(float *h_input, int width, int height)
{
	return pixelSubtraction(gaussianBlur(h_input, width, height), h_input, width, height);
}

__global__ void underSamplingKernel(float *output, float *input, int newWidth, int newHeight, float sizeFactor)
{
	int i = threadIdx.x + blockIdx.x*blockDim.x;
	int j = threadIdx.y + blockIdx.y*blockDim.y;
	int width = newWidth*sizeFactor;

	if (i >= newWidth || j >= newHeight) return;

	int idxOutput = j*newWidth + i;
	int idxInput = (j*width + i)*sizeFactor;

	output[idxOutput] = (input[idxInput] + input[idxInput + 1] + input[idxInput + width] + input[idxInput + width +1]) / 4;
}

float DllExport *underSampling(float *h_input, int width, int height, float sizeFactor)
{
	if (sizeFactor < 1.0) return NULL;
	int newWidth = width / sizeFactor;
	int newHeight = height / sizeFactor;

	int elementCount = height*width;
	int newElementCount = newWidth*newHeight;

	int size = elementCount*sizeof(float);
	int newSize = newElementCount*sizeof(float);

	float *h_output = (float*)malloc(newSize);
	float *dev_input, *dev_output;

	cudaMalloc((void**)&dev_input, size);
	cudaMalloc((void**)&dev_output, newSize);

	cudaMemcpy(dev_input, h_input, size, cudaMemcpyHostToDevice);

	dim3 threadsPerBlock(THREADS_PER_BLOCKDIM, THREADS_PER_BLOCKDIM);
	dim3 blockCount(((newWidth - 1) / THREADS_PER_BLOCKDIM) + 1, ((newHeight - 1) / THREADS_PER_BLOCKDIM) + 1);

	underSamplingKernel << < blockCount, threadsPerBlock >> > (dev_output, dev_input, newWidth, newHeight, sizeFactor);

	cudaMemcpy(h_output, dev_output, newSize, cudaMemcpyDeviceToHost);

	cudaFree(dev_input);
	cudaFree(dev_output);

	return h_output;
}

float DllExport *laplacian4Neib(float *h_input, int width, int height);
float DllExport *applyMask(float *h_input, int width, int height, int radius, float *h_mask);
float DllExport *applyThreshold(float *h_input, int width, int height, int threshold);

