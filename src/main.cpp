#include <iostream>
#include <CL/cl.h>
#include "cl_0x.hpp"
#include "util.hpp"
#include "testing.hpp"
#include <typeinfo>

static cl_platform_id pid;
static cl_device_id dev;
static cl_context ctx;
static cl_command_queue cmdq;
static cl_program prog;
static cl_mem gpuArray[3];
static cl_mem pinned[2];
static cl_event ev[2];


static void
cleanup_opencl ()
{
	RELEASEN(ev, 2, clReleaseEvent);
	RELEASEN(pinned, 2, clReleaseMemObject);
	RELEASEN(gpuArray, 2, clReleaseMemObject);
	RELEASE(prog, clReleaseProgram);
	RELEASE(cmdq, clReleaseCommandQueue);
	RELEASE(ctx, clReleaseContext);
}



int
main ()
{
	cl_int err;
	cl::Kernel kernel;

	atexit(cleanup_opencl);
	setup_opencl(&pid, &dev, &ctx, &cmdq);
	compile_kernel("cl/dotprod.cl", "dotprod", &ctx, &prog, &(kernel.k));

	const unsigned int dim = 100000;
	const size_t memsize = sizeof(cl_float) * dim;
	get_pinned_memory(ctx, &pinned[0], memsize);
	get_pinned_memory(ctx, &pinned[1], memsize);

	float *h[2] = {NULL, NULL};
	map(cmdq, pinned[0], &h[0], CL_MAP_WRITE, memsize);
	map(cmdq, pinned[1], &h[1], CL_MAP_WRITE, memsize);
	for (size_t i = 0; i < dim; i++) {
		h[0][i] = 1.0f;
		h[1][i] = 3.0f;
	}
	unmap(cmdq, pinned[1], h[1]);
	unmap(cmdq, pinned[0], h[0]);

	create_mem_obj(ctx, gpuArray[0], CL_MEM_WRITE_ONLY, memsize);
	create_mem_obj(ctx, gpuArray[1], CL_MEM_READ_ONLY, memsize);
	create_mem_obj(ctx, gpuArray[2], CL_MEM_READ_ONLY, memsize);

	// set kernel arguments (the easy way)
	err = kernel.set_args(gpuArray[0], gpuArray[1], gpuArray[2], dim);
	if (err != CL_SUCCESS)
		die("ERROR: Could not set kernel arguments\n");

	// copy the data to the device
	err = clEnqueueCopyBuffer(cmdq, pinned[0], gpuArray[1], 0, 0, memsize,
			0, 0, &ev[0]);
	err |= clEnqueueCopyBuffer(cmdq, pinned[1], gpuArray[2], 0, 0, memsize,
			0, 0, &ev[1]);
	if (err != CL_SUCCESS)
		die("ERROR: Could not start data transfer to device\n");

	// wait for copy to succeed
	if (clWaitForEvents(2, ev) != CL_SUCCESS)
		die("ERROR: Could not wait for copy to complete\n");

	// call the kernel
	size_t threads = 1024;
	size_t tpb = 64;
	err = clEnqueueNDRangeKernel(cmdq, kernel.k, 1, NULL, &threads, &tpb, 0,
			NULL, &ev[0]);
	if (err != CL_SUCCESS)
		die("ERROR: Could not enqueue kernel\n");
	if (clWaitForEvents(1, &ev[0]) != CL_SUCCESS)
		die("ERROR: Could not wait for kernel\n");

	// copy data back to host
	err = clEnqueueCopyBuffer(cmdq, gpuArray[0], pinned[0], 0, 0, memsize,
			0, 0, &ev[0]);
	if (err != CL_SUCCESS || clWaitForEvents(1, &ev[0]) != CL_SUCCESS)
		die("ERROR: Could not transfer data to host\n");

	// finish
	double finalDotProduct = 0.0;
	map(cmdq, pinned[0], &h[0], CL_MAP_READ, memsize);
	for (size_t i = 0; i < threads; i++)
		finalDotProduct += h[0][i];
	unmap(cmdq, pinned[0], &h[0]);

	std::cout << finalDotProduct << std::endl;

	return 0;
}
