#ifndef __UTIL_HPP__862143F7_968A_412F_9E6C_219937FCD443
#define __UTIL_HPP__862143F7_968A_412F_9E6C_219937FCD443

#include <CL/cl.h>


#define RELEASE(OBJ, FUNC)		\
	if (OBJ)			\
		FUNC(OBJ)

#define RELEASEN(OBJ, N, FUNC)		\
	for (int i = 0; i < N; i++)	\
		RELEASE(OBJ[i], FUNC)


/*
 * print a message to stderr and exit the application with status EXIT_FAILURE
 */
void die (const char *errmsg, ...);


/*
 * set up an open cl context and command queue, platform and device will be the
 * first available
 */
void setup_opencl (cl_platform_id *pid, cl_device_id *dev, cl_context *ctx,
		cl_command_queue *cmdq);


/*
 * compile an OpenCL file for context ctx and store the resulting program in
 * prog, the resulting kernel in kernel. Note that this function can handle only
 * OpenCL files with one exported kernel
 */
void compile_kernel (const char *fname, const char *krnlname,
		const cl_context *ctx, cl_program *prog, cl_kernel *kernel);


/*
 * read a PPM file. don't forget to free data
 */
void read_ppm (const char *fname, long int *w, long int *h, float *data[]);


/*
 * write a PPM file
 */
void write_ppm (const char *fname, long int w, long int h, float *data);



/*
 * get 'pinned' memory. though this is not guaranteed by CL_MEM_ALLOC_HOST_PTR,
 * it is the case on NVIDIA hardware and might possibly be the case on ATI
 * hardware
 */
void get_pinned_memory (const cl_context &ctx, cl_mem *memobj, size_t size,
		cl_mem_flags flags = CL_MEM_READ_WRITE);


/*
 * map a pointer to pinned memory
 */
template <typename T>
void
map (const cl_command_queue &cmdq, cl_mem &memobj, T **p, cl_map_flags flags,
		size_t size)
{
	cl_int err;
	*p = (T*)clEnqueueMapBuffer(cmdq, memobj, CL_TRUE, flags, 0, size, 0,
			NULL, NULL, &err);
	if (err != CL_SUCCESS)
		die("ERROR: Could not map pointer to pinned memory\n");
}


/*
 * unmap a pointer from pinned memory
 */
template <typename T>
void
unmap (const cl_command_queue &cmdq, cl_mem &memobj, T *p)
{
	if (clEnqueueUnmapMemObject(cmdq, memobj, (void*)p, 0, NULL, NULL)
			!= CL_SUCCESS)
		die("ERROR: Could not unmap pointer\n");
}


#endif /* __UTIL_HPP__862143F7_968A_412F_9E6C_219937FCD443 */

