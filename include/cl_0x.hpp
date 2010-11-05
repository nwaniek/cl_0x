/*
 * Copyright © 2010 Nicolai Waniek <nicolai dot waniek at sphere71 dot com>
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
 * This file is considered to be a W.I.P. and will be extended as needed
 */

#ifndef __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16
#define __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16

#include <CL/cl.h>


namespace cl_0x {


template <typename T>
struct CLTypeTraits
{
	typedef T type;
	static const size_t size = sizeof(T);
};


template <>
struct CLTypeTraits<float>
{
	static const size_t size = sizeof(cl_float);
};


template <>
struct CLTypeTraits<double>
{
	static const size_t size = sizeof(cl_double);
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
 * struct Kernel - wrapping the cl_kernel object into some templated functions
 * to reduce direct OpenCL function invocation/typing.
 *
 * @k:			the contained cl_kernel object
 * @release_on_destroy:	determines if the contained cl_kernel object is released
 *			by clReleaseKernel when the cl::Kernel object is
 *			destroyed
 */
struct Kernel
{
	cl_kernel k;

	bool release_on_destroy;


	/**
	 * Create a new kernel instance.
	 *
	 * @release_on_destroy:	Decide whether the contained cl_kernel object
	 *			shall be released when cl::Kernel object is
	 *			destroyed
	 */
	Kernel (bool release_on_destroy = true)
		: release_on_destroy(release_on_destroy)
	{}


	/**
	 * create a new kernel instance by passing the cl_kernel object to it.
	 *
	 * @kernel:		An existing cl_kernel object
	 * @release_on_destroy:	Decide whether the contained cl_kernel object
	 *			shall be released when cl::Kernel object is
	 *			destroyed
	 */
	Kernel (cl_kernel &kernel, bool release_on_destroy = false)
		: k(kernel)
		, release_on_destroy(release_on_destroy)
	{}


	/**
	 * according to the member release_on_destroy, the cl_kernel object
	 * might get release via clReleaseKernel
	 */
	~Kernel ()
	{
		if (release_on_destroy)
			clReleaseKernel(k);
	}


	/**
	 * set one or many kernel arguments. consecutive calls to this function
	 * might overwrite previously set arguments as the argument index is
	 * deduced from argument count
	 */
	template <typename... Args>
	cl_int
	set_args (const Args&... args)
	{
		return set_kernel_args(this->k, args...);
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
		return set_kernel_args(this->k, n, arg);
	}
};


} // namespace cl


#endif /* __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16 */

