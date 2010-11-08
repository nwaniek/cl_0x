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
 *
 *
 *  incomplete TODO list
 * ======================
 * - function name consistency (CamelCasing, underscore_names)
 *
 */



#ifndef __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16
#define __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16

#include <CL/cl.h>
#include <fstream>
#include <string>
#include <cstring>

#ifdef DEBUG
#include <cstdio>
#endif

namespace cl_0x {


// forward declarations (needed for argument size and pointer deduction)
template <typename CLType, cl_int (*RFunc)(CLType)> struct CLObjContainer;
template <typename T> struct Buffer;


/**
 * class to pass local memory to the set_kernel_args function
 */
struct LocalMemory
{
	size_t size;

	LocalMemory(size_t size) : size(size) {}
};


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

	static size_t
	size (const T &)
	{
		return sizeof(T);
	}
};


template <>
struct CLTypeTraits <LocalMemory>
{
	static size_t
	size (const LocalMemory &arg)
	{
		return arg.size;
	}
};


template <typename CLType, cl_int (*RFunc)(CLType)>
struct CLTypeTraits <CLObjContainer<CLType, RFunc>>
{
	static size_t
	size (const CLObjContainer<CLType, RFunc> &)
	{
		return sizeof(CLType);
	}
};


template <typename T>
struct CLTypeTraits <Buffer<T>>
{
	static size_t
	size (const Buffer<T> &)
	{
		return sizeof(cl_mem);
	}
};


/**
 * Argument Wrapper to make it possible to pass local memory to the kernel by
 * set_kernel_args
 */
template <typename T>
struct KernelArg
{
	static const void*
	ptr (const T &arg)
	{
		return &arg;
	}
};


template <>
struct KernelArg<LocalMemory>
{
	static const void*
	ptr (const LocalMemory &) {
		return NULL;
	}
};


template <typename CLType, cl_int (*RFunc)(CLType)>
struct KernelArg <CLObjContainer<CLType, RFunc>>
{
	static const void*
	ptr (const CLObjContainer<CLType, RFunc> &arg)
	{
		return &(arg.cl_obj);
	}
};


template <typename T>
struct KernelArg <Buffer<T>>
{
	static const void*
	ptr (const Buffer<T> &arg)
	{
		return &(arg.cl_obj);
	}
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
	err = clSetKernelArg(k, n,
			CLTypeTraits<T>::size(arg),
			KernelArg<T>::ptr(arg));
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
 * empty release function for classes inheriting from CLObjContainer that don't
 * need or have a release function
 */
template <typename T>
constexpr cl_int empty_release (T*) { return CL_SUCCESS; }


/**
 *
 */
template <typename CLType, cl_int (*RFunc)(CLType) = empty_release>
struct CLObjContainer
{
	typedef CLType cl_type;
	typedef CLObjContainer<CLType, RFunc> container_type;


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
	explicit
	CLObjContainer (CLType cl_obj = NULL, bool release_on_destroy = true)
		: cl_obj(cl_obj)
		, release_on_destroy(release_on_destroy)
	{}


	~CLObjContainer ()
	{
		if (release_on_destroy && this->cl_obj)
			RFunc(this->cl_obj);
	}


	CLType operator() () const
	{
		return this->cl_obj;
	}
};


struct Platform : CLObjContainer<cl_platform_id>
{
	cl_int
	select_first ()
	{
		return clGetPlatformIDs(1, &(this->cl_obj), NULL);
	}
};


struct Device : CLObjContainer<cl_device_id>
{
	cl_int
	select_first (const Platform &platform, cl_device_type devtype)
	{
		return clGetDeviceIDs(platform(), devtype, 1, &(this->cl_obj),
				NULL);
	}
};

struct DeviceJunction
{
	const Device *device;

	DeviceJunction () : device(NULL) {}

	void bind_to (const Device &device)
	{
		this->device = &device;
	}
};


struct Context : CLObjContainer<cl_context, clReleaseContext>
{
	cl_int
	create (const Platform &platform, Device &device)
	{
		cl_int err;
		cl_context_properties props[] = {CL_CONTEXT_PLATFORM,
			(cl_context_properties)platform(), 0};
		this->cl_obj = clCreateContext(props, 1, &device.cl_obj, NULL, NULL,
				&err);
		return err;
	}
};

struct ContextJunction
{
	const Context *context;

	ContextJunction() : context(NULL) {}

	void bind_to (const Context &context)
	{
		this->context = &context;
	}
};


struct CommandQueue : CLObjContainer<cl_command_queue, clReleaseCommandQueue>
		, ContextJunction, DeviceJunction
{
	cl_int
	create (const Device &device, const Context &context)
	{
		cl_int err;
		this->cl_obj = clCreateCommandQueue(context(), device(), 0,
				&err);
		return err;
	}
};

struct CommandQueueJunction
{
	const CommandQueue *command_queue;

	CommandQueueJunction () : command_queue(NULL) {}

	void bind_to (const CommandQueue &q)
	{
		this->command_queue = &q;
	}
};


struct Program : CLObjContainer<cl_program, clReleaseProgram>
		 , ContextJunction
{
	cl_int
	build_from_source (const Context &context, const char *src)
	{
		cl_int err;
		size_t len[] = {strlen(src)};
		this->cl_obj = clCreateProgramWithSource(context(), 1, &src,
				len, &err);
		if (err != CL_SUCCESS)
			return err;

		return clBuildProgram(this->cl_obj, 0, NULL, NULL, NULL, NULL);
	}


	cl_int
	build_from_file (const Context &context, const char *fname)
	{
		cl_int err;
		std::ifstream f(fname);
		if (!f.is_open())
			return CL_INVALID_PROGRAM;

		std::string str((std::istreambuf_iterator<char>(f)),
				std::istreambuf_iterator<char>());
		err = build_from_source(context, str.c_str());
		f.close();

		return err;
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
		, CommandQueueJunction
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


	/**
	 * create a kernel by name from a given program object
	 */
	cl_int
	create (const Program &program, const char *kernel_name)
	{
		cl_int err;
		this->cl_obj = clCreateKernel(program(), kernel_name, &err);
		return err;
	}


	/**
	 * run the kernel on the associated context/commandq.
	 */
	cl_int
	run (cl_uint work_dim, const size_t *global_work_size,
			const size_t *local_work_size,
			const size_t *global_work_offset = 0,
			cl_uint num_events_in_wait_list = 0,
			const cl_event 	*event_wait_list = NULL,
			cl_event *event = NULL)
	{

		if (!(this->command_queue))
			return CL_INVALID_COMMAND_QUEUE;

		return clEnqueueNDRangeKernel(this->command_queue->cl_obj,
				this->cl_obj, work_dim, global_work_offset,
				global_work_size, local_work_size,
				num_events_in_wait_list, event_wait_list,
				event);
	}
};


template <typename T>
struct Buffer: CLObjContainer<cl_mem, clReleaseMemObject>
	       , CommandQueueJunction
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
		return mallocHost(ctx(), size, flags);
	}



	cl_int
	mallocDevice (const cl_context ctx, size_t size,
			cl_mem_flags flags = CL_MEM_READ_WRITE)
	{
		cl_int err;
		cl_obj = clCreateBuffer(ctx, flags, size, NULL, &err);
		this->size = size;
		return err;
	}


	cl_int
	mallocDevice (const Context &ctx, size_t size,
			cl_mem_flags flags = CL_MEM_READ_WRITE)
	{
		return mallocDevice(ctx(), size, flags);
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
		return this->map(q(), err, flags, blocked);
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
		return this->unmap(q(), event);
	}



	// TODO: extend for events! reflect in other copy_to functions
	// TODO: provide operator= for copy operation
	cl_int
	copy_to (const cl_command_queue q, Buffer<T> &buffer, size_t size = 0,
		size_t src_offset = 0, size_t dst_offset = 0)
	{
		if (size == 0)
			size = this->size;

		return clEnqueueCopyBuffer(q, this->cl_obj, buffer(),
				src_offset, dst_offset, size, 0, NULL, NULL);

	}


	cl_int
	copy_to (const CommandQueue &q, Buffer<T> &buffer, size_t size = 0,
		size_t src_offset = 0, size_t dst_offset = 0)
	{
		return copy_to(q(), buffer, size, src_offset, dst_offset);
	}


	/**
	 * copy_to function when the buffer is already bound to a command queue
	 */
	cl_int
	copy_to (Buffer<T> &buffer, size_t size = 0, size_t src_offset = 0,
			size_t dst_offset = 0)
	{
		if (!(this->command_queue))
			return CL_INVALID_COMMAND_QUEUE;

		return copy_to(*(this->command_queue), buffer, size, src_offset,
				dst_offset);
	}


};














} // namespace cl


#endif /* __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16 */

