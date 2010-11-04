#include "util.hpp"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


void
die (const char *errmsg, ...)
{
	va_list ap;
	va_start(ap, errmsg);
	vfprintf(stderr, errmsg, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}


void
setup_opencl (cl_platform_id *pid, cl_device_id *dev, cl_context *ctx,
		cl_command_queue *cmdq)
{
	cl_int err;

	if (clGetPlatformIDs(1, pid, NULL) != CL_SUCCESS)
		die("ERROR: No OpenCL Platform presen!\n");

	if (clGetDeviceIDs(*pid, DEVICE_TYPE, 1, dev, NULL) != CL_SUCCESS)
		die("ERROR: Could not open OpenCL device\n");

	cl_context_properties props[] = {CL_CONTEXT_PLATFORM,
		(cl_context_properties)*pid, 0};
	*ctx = clCreateContext(props, 1, dev, NULL, NULL, &err);
	if (!ctx || (err != CL_SUCCESS))
		die("ERROR: Could not create context (%d)\n", err);

	*cmdq = clCreateCommandQueue(*ctx, *dev, 0, &err);
	if (!cmdq || (err != CL_SUCCESS))
		die("ERROR: Could not create command queue on device\n");
}


void
compile_kernel (const char *fname, const char *krnlname, const cl_context *ctx,
		cl_program *prog, cl_kernel *kernel)
{
	cl_int err;
	const char *src;
	struct stat sb;
	int fd;

	if (stat(fname, &sb) < 0)
		die("ERROR: Could not open file %s\n", fname);

	if ((fd = open(fname, O_RDONLY)) < 0)
		die("ERROR: Could not open file %s\n", fname);
	src = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	size_t len[] = {strlen(src)};
	*prog = clCreateProgramWithSource(*ctx, 1, &src, len, &err);
	if (err != CL_SUCCESS)
		die("ERROR: Could not create program object from source %s\n",
				fname);

	if (clBuildProgram(*prog, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS)
		die("ERROR: Could not compile program\n");

	*kernel = clCreateKernel(*prog, krnlname, &err);
	if (err != CL_SUCCESS)
		die("ERROR: Could not create kernel object (%d)\n", err);

	munmap((void*)src, sb.st_size);
	close(fd);
}


void
eat_whitespace (unsigned int maxlen, unsigned int *i, const char *ppm)
{
	while (((*i) < maxlen) && (ppm[(*i)] >= 0) && (ppm[(*i)] <= ' '))
		++(*i);
}


void
eat_comments  (unsigned int maxlen, unsigned int *i, const char *ppm)
{
	while (((*i) < maxlen) && (ppm[(*i)] >= 0) && (ppm[(*i)] == '#'))
		while (((*i) < maxlen) && (ppm[(*i)] >= 0) && (ppm[(*i)] != '\n'))
			++(*i);
}

long int
get_scalar (unsigned int *i, const char *ppm)
{
	long int result = 0;
	errno = 0;
	char *p = NULL;

	result = strtol(ppm + (*i), &p, 10);
	if (errno != 0 || !p || p == ppm)
		die("ERROR: Could not read value from PPM file\n");
	*i = p - ppm;

	return result;
}

void
read_ppm (const char *fname, long int *w, long int *h, float *data[])
{
	int fd = -1;
	unsigned int i;
	const char *ppm;
	struct stat sb;

	if ((stat(fname, &sb) < 0) || ((fd = open(fname, O_RDONLY)) < 0))
		die("ERROR: Could not open file %s\n", fname);

	ppm = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (!ppm)
		die("ERROR: Could not read from file %s\n", fname);

	i = 2;
	if (strncmp(ppm, "P6", 2))
		die("ERROR: PPM not in a valid format\n");

	eat_whitespace(sb.st_size, &i, ppm);
	eat_comments(sb.st_size, &i, ppm);
	eat_whitespace(sb.st_size, &i, ppm);
	*w = get_scalar(&i, ppm);
	eat_whitespace(sb.st_size, &i, ppm);
	*h = get_scalar(&i, ppm);
	eat_whitespace(sb.st_size, &i, ppm);

	long int maxval = get_scalar(&i, ppm);
	i++;

	size_t npixels = (*w) * (*h);
	*data = (float*)malloc(sizeof(float) * npixels * 3);

	for (size_t j = 0; j < npixels * 3; j++, i++) {
		float v = (float)(unsigned char)ppm[i] / (float)maxval;
		(*data)[j] = (v > 1.0) ? 1.0 : v;
	}

	munmap((void*)ppm, sb.st_size);
	close(fd);
}


void
write_ppm (const char *fname, long int w, long int h, float *data)
{
	FILE *f = fopen(fname, "wb");
	if (!f)
		die("ERROR: Coult not open file %s to write\n", fname);

	size_t npixels = w * h;
	unsigned char *buf =
		(unsigned char*)malloc(sizeof(unsigned char) * npixels * 3);

	for (size_t i = 0; i < npixels * 3; i++) {
		float v = data[i] * 255.f;
		buf[i] = (unsigned char)v;
	}

	fprintf(f, "P6\n%ld %ld\n255\n", w, h);
	int result = fwrite(buf, sizeof(unsigned char) * npixels * 3, 1, f);
	if (result != 1)
		die("ERROR: Could not write data to file\n");
	free(buf);
	fclose(f);
}



void
get_pinned_memory (const cl_context &ctx, cl_mem *memobj, size_t size,
		cl_mem_flags flags)
{
	cl_int err;
	*memobj = clCreateBuffer(ctx, flags | CL_MEM_ALLOC_HOST_PTR, size, NULL,
			&err);
	if (err != CL_SUCCESS)
		die("ERROR: Could not allocate pinned memory\n");
}

