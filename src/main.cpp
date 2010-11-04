#include <iostream>
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


static void
cleanup_opencl ()
{
	RELEASEN(pinned, 2, clReleaseMemObject);
	RELEASEN(gpuArray, 2, clReleaseMemObject);
	RELEASE(prog, clReleaseProgram);
	RELEASE(cmdq, clReleaseCommandQueue);
	RELEASE(ctx, clReleaseContext);
}



int
main ()
{
	cl::Kernel kernel;

	atexit(cleanup_opencl);
	setup_opencl(&pid, &dev, &ctx, &cmdq);
	compile_kernel("cl/dotprod.cl", "dotprod", &ctx, &prog, &(kernel.k));

	const unsigned int dim = 10000;
	const size_t memsize = sizeof(cl_float) * dim;
	get_pinned_memory(ctx, &pinned[0], memsize);
	get_pinned_memory(ctx, &pinned[1], memsize);

	float *h[2] = {NULL, NULL};
	map(cmdq, pinned[0], &h[0], CL_MAP_WRITE, memsize);
	map(cmdq, pinned[1], &h[1], CL_MAP_WRITE, memsize);
	for (size_t i = 0; i < dim; i++)
		h[0][i] = h[1][i] = 2.0f;
	unmap(cmdq, pinned[1], h[1]);
	unmap(cmdq, pinned[0], h[0]);

	create_mem_obj(ctx, gpuArray[0], CL_MEM_WRITE_ONLY, memsize);
	create_mem_obj(ctx, gpuArray[1], CL_MEM_READ_ONLY, memsize);
	create_mem_obj(ctx, gpuArray[2], CL_MEM_READ_ONLY, memsize);

	// set_kernel_args(zonk, gpuArray[0]);

	cl::set_kernel_args(kernel.k, gpuArray[0], gpuArray[1], gpuArray[2], dim);





	// copy data to device
	//



	// then invoke the kernel

	// transfer results back from device to host
	return 0;
}
