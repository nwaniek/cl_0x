/*
 * Copyright (c) 2010 Nicolai Waniek <rochus at rochus dot net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * This file is considered to be a W.I.P. and will be extended as needed and
 * might yet be changed heavily!
 */

#ifndef __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16
#define __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16

#include <CL/cl.h>


namespace cl_0x {


/**
 * the TypeTraits class encapsulates type size information for usage in
 * OpenCL functions like clSetKernelArg.
 *
 * @type:	The contained type
 * @size:	The type size as returned by sizeof
 *
 * specialize the CLTypeTraits class with a custom type when there is need.
 */
template <typename T>
struct CLTypeTraits
{
	typedef T type;
	static const size_t size = sizeof(T);
};


cl_int
set_kernel_args (cl_kernel &, unsigned int)
{
	return CL_SUCCESS;
}


template <typename T, typename... Args>
cl_int
set_kernel_args (cl_kernel &k, int n, const T &arg, const Args&... args)
{
	cl_int err;
	err = clSetKernelArg(k, n, CLTypeTraits<T>::size, (void*)&arg);
	return err |
	       set_kernel_args(k, n+1, args...);
}


template <typename... Args>
cl_int
set_kernel_args (cl_kernel &k, const Args&... args)
{
	return set_kernel_args(k, 0, args...);
}


/**
 *
 */
template <typename CLType, cl_int (*RFunc)(CLType)>
struct CLObjContainer
{
	CLType cl_obj;
	bool release_on_destroy;

	/**
	 * creates a new Container to hold a cl object of type CLType
	 *
	 * @obj:		The object, or therefore the pointer to an
	 *			OpenCL object struct
	 * @release_on_destroy: call the corresponding RFunc on the object when
	 *			the instance gets destroyed or not
	 */
	CLObjContainer (CLType cl_obj = NULL, bool release_on_destroy = true)
		: cl_obj(cl_obj)
		, release_on_destroy(release_on_destroy)
	{}

	~CLObjContainer ()
	{
		if (release_on_destroy)
			RFunc(cl_obj);
	}
};



/**
 * struct Kernel - wrapping the cl_kernel object into some templated functions
 * to reduce direct OpenCL function invocation/typing.
 *
 * @k:			the contained cl_kernel object
 * @release_on_destroy:	determines if the contained cl_kernel object is released
 *			by clReleaseKernel when the cl::Kernel object is
 *			destroyed
 */
struct Kernel : CLObjContainer<cl_kernel, clReleaseKernel>
{
	/**
	 * create a new kernel instance by passing the cl_kernel object to it.
	 *
	 * @kernel:		An existing cl_kernel object
	 * @release_on_destroy:	Decide whether the contained cl_kernel object
	 *			shall be released when cl::Kernel object is
	 *			destroyed
	 */
	Kernel (cl_kernel kernel = NULL, bool release_on_destroy = true)
		: CLObjContainer(kernel, release_on_destroy)
	{}


	/**
	 * set one or many kernel arguments. consecutive calls to this function
	 * might overwrite previously set arguments as the argument index is
	 * deduced from argument count
	 */
	template <typename... Args>
	cl_int
	set_args (const Args&... args)
	{
		return set_kernel_args(this->cl_obj, args...);
	}


	/**
	 * set a specific kernel argument
	 *
	 * @n:		Argument index (starting by 0)
	 * @arg:	Argument value
	 */
	template <typename T>
	cl_int
	set_arg (unsigned int n, T arg)
	{
		return set_kernel_args(this->cl_obj, n, arg);
	}
};


struct Context : CLObjContainer<cl_context, clReleaseContext>
{};


struct CommandQueue : CLObjContainer<cl_command_queue, clReleaseCommandQueue>
{};


template <typename T>
struct Buffer: CLObjContainer<cl_mem, clReleaseMemObject>
{
	//! mapping pointer
	T *ptr;

	//! size of memory buffer object
	size_t size;


	Buffer (cl_mem buffer = NULL, bool release_on_destroy = true)
		: CLObjContainer(buffer, release_on_destroy)
		, ptr(NULL)
		, size(0)
	{}


	cl_int
	mallocHost (const cl_context ctx, size_t size,
			cl_mem_flags flags = CL_MEM_READ_WRITE)
	{
		cl_int err;
		cl_obj = clCreateBuffer(ctx,
				CL_MEM_ALLOC_HOST_PTR | flags, size, NULL,
				&err);
		this->size = size;
		return err;
	}


	cl_int
	mallocHost (const Context &ctx, size_t size,
			cl_mem_flags flags = CL_MEM_READ_WRITE)
	{
		return mallocHost(ctx.cl_obj, size, flags);
	}



	T*
	map (const cl_command_queue q, cl_int *err = NULL,
			cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE,
			cl_bool blocked = true)
	{
		cl_int e;
		ptr = (T*)clEnqueueMapBuffer(q, this->cl_obj, blocked, flags, 0,
				this->size, 0, NULL, NULL, &e);
		if (err)
			*err = e;
		return ptr;
	}


	T*
	map (const CommandQueue &q, cl_int *err = NULL,
			cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE,
			cl_bool blocked = true)
	{
		return this->map(q.cl_obj, err, flags, blocked);
	}


	cl_int
	unmap (const cl_command_queue q, cl_event *event = NULL)
	{
		cl_int err;
		cl_event e;
		err = clEnqueueUnmapMemObject(q, this->cl_obj, (void*)ptr, 0,
				NULL, &e);
		if (event)
			*event = e;
		return err;
	}


	cl_int
	unmap (const CommandQueue &q, cl_event *event = NULL)
	{
		return this->unmap(q.cl_obj, event);
	}


};














} // namespace cl


#endif /* __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16 */

