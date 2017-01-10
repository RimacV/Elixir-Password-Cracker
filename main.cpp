/***
* Excerpted from "Seven Concurrency Models in Seven Weeks",
* published by The Pragmatic Bookshelf.
* Copyrights apply to this code. It may not be used to create training material,
* courses, books, articles, and the like. Contact us if you are in doubt.
* We make no guarantees that this code is fit for any purpose.
* Visit http://www.pragmaticprogrammer.com/titles/pb7con for more book information.
***/
#ifdef __APPLE__
#include <OpenCL/cl.h>
#include <mach/mach_time.h>
#else  
#include <CL/cl.h>
#include <Windows.h>

#endif

#include <stdio.h>
#include<iostream>
#include <inttypes.h>
#include <chrono>

#define NUM_ELEMENTS (4)
#define NUM_ELEMENTS_INT (2)
#define WIDTH_OUTPUT (10)
#define HEIGHT_OUTPUT (1014)
#define SHA256_RESULT_SIZE (8)

char* read_source(const char* filename) {
	FILE *h = fopen(filename, "r");
	fseek(h, 0, SEEK_END);
	size_t s = ftell(h);
	rewind(h);
	char* program = (char*)malloc(s + 1);
	fread(program, sizeof(char), s, h);
	program[s] = '\0';
	fclose(h);
	return program;
}

void random_fill(cl_char array[], cl_int startIndex[], cl_int endindex[], size_t size) {
	startIndex[0] = 0;
	for (int i = 0; i < size; i++) {
		endindex[i] = startIndex[i] + 5;
		for (int j = startIndex[i]; j < endindex[i]; j++) {
			array[j] = 'd';
		}
		startIndex[i+1] = endindex[i];
	}

}

int main() {
	//Status for Errorhandling
	cl_int status;

	//Identify Platform
	cl_platform_id platform;
	clGetPlatformIDs(1, &platform, NULL);

	//Get Id of GPU
	cl_device_id device;
	cl_uint num_devices = 0;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);

	// Create Context
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);


	//Use context to create Command Queue
	//Que enables us to send commands to the gpu device
	cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, NULL);

	//Load Kernel
	char* source = read_source("multiply_arrays.cl");
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char**)&source, NULL, &status);
	free(source);

	// Build Program
	status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

	size_t len;
	char *buffer;
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
	buffer = (char *) malloc(len);
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
	printf("%s\n", buffer);

	//Create Kernel
	cl_kernel kernel = clCreateKernel(program, "multiply_arrays", &status);

	// Create Arrays with random Numbers
	cl_char a[NUM_ELEMENTS];
	cl_int b[NUM_ELEMENTS_INT];
	cl_int c[NUM_ELEMENTS_INT];
	a[0] = '1';
	a[1] = '\0';
	a[2] = '1';
	a[3] = '\0';
	b[0] = 0;
	c[0] = 2;
	b[1] = 3;
	c[1] = 5;
 
	//uint64_t startGPU = mach_absolute_time();
	auto start = std::chrono::high_resolution_clock::now();


	//Create Readonly input Buffers with value from a and b
	cl_mem inputA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_char) * NUM_ELEMENTS, a, NULL);
		cl_mem inputB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_int) * NUM_ELEMENTS_INT, b, NULL);
	cl_mem inputC = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_int) * NUM_ELEMENTS_INT, c, NULL);

	//Create Output buffer write Only
	cl_mem buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * SHA256_RESULT_SIZE *NUM_ELEMENTS_INT, NULL, NULL);

	//set Kernel Arguments
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputA);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputB);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &inputC);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &buffer_out);

	cl_event timing_event;
	size_t work_units = NUM_ELEMENTS_INT;
	status = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units,
		NULL, 0, NULL,&timing_event);

	cl_uint *results;
	cl_mem  pinned_partial_hashes;
	pinned_partial_hashes = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_uint) * SHA256_RESULT_SIZE, NULL, &status);
	results = (cl_uint *)clEnqueueMapBuffer(queue, pinned_partial_hashes, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE, 0, NULL, NULL, &status);
	memset(results, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE);

	//Calculate Results and copy from output buffer to results
	status = clEnqueueReadBuffer(queue, buffer_out, CL_TRUE, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE * NUM_ELEMENTS_INT,
		results, 0, NULL, NULL);

	//Print Sha values
	char outpoutHex[65];
	int shaIndex = 0;
	int shaEnd = 8;
	for (int j = 0; j < NUM_ELEMENTS_INT; j++) {
		int k = 0;
		for (int i = shaIndex; i<shaEnd; i++)
		{
			sprintf(outpoutHex + k * 8, "%08x", results[i]);
			k++;
		}
		shaIndex += 8;
		shaEnd += 8;
		printf("%s\n", outpoutHex);
	}

	std::cout << results[0];

	//uint64_t endGPU = mach_absolute_time();
	auto finish = std::chrono::high_resolution_clock::now();
	//printf("Total (GPU): %lu ns\n\n", (unsigned long)(endGPU - startGPU));
	std::cout << "Total(GPU) :"<< std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";

	cl_ulong starttime;
	clGetEventProfilingInfo(timing_event, CL_PROFILING_COMMAND_START,
		sizeof(cl_ulong), &starttime, NULL);
	cl_ulong endtime;
	clGetEventProfilingInfo(timing_event, CL_PROFILING_COMMAND_END,
		sizeof(cl_ulong), &endtime, NULL);
	printf("Elapsed (GPU): %lu ns\n\n", (unsigned long)(endtime - starttime));
	clReleaseEvent(timing_event);
	clReleaseMemObject(inputA);
	clReleaseMemObject(inputB);
	clReleaseMemObject(buffer_out);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	//uint64_t startCPU = mach_absolute_time();
	start = std::chrono::high_resolution_clock::now();
	for (int id = 0; id < NUM_ELEMENTS_INT; id++) {
		int start = b[id];
		int end = c[id];
		for (int i = start; i < end; i++) {
			results[i] = a[i];
		}
	}
	//uint64_t endCPU = mach_absolute_time();
	finish = std::chrono::high_resolution_clock::now();
	//printf("Elapsed (CPU): %lu ns\n\n", (unsigned long)(endCPU - startCPU));
	std::cout << "Elapsed (CPU) :" << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";
	return 0;
}