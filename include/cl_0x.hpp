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
#ifndef __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16
#define __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16

#include <CL/cl.h>

namespace cl {


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
	err = clSetKernelArg(k, n, sizeof(T), (void*)&arg);
	return err |
	       set_kernel_args(k, n+1, args...);
}


template <typename... Args>
cl_int
set_kernel_args (cl_kernel &k, const Args&... args)
{
	return set_kernel_args(k, 0, args...);
}













/*
 * forward declarations --------------------------------------------------------
 */
#if 0
namespace cl {
	struct Kernel;

	template <typename... Args>
	cl_int set_kernel_args (Kernel &k, const Args&... args);

	template <typename... Args>
	cl_int set_kernel_args (cl_kernel &kernel, const Args&... args);
};
#endif



/*
 * implementation --------------------------------------------------------------
 */



/*
 * classes
 */


struct Kernel {
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
	Kernel (cl_kernel kernel, bool release_on_destroy = false)
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

#if 0

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
	 */
	/*
	template <typename T>
	cl_int
	set_arg (unsigned int n, T arg)
	{
		return set_kernel_args(kernel, n, arg);
	}
	*/
#endif
};



/*
 * functions
 */

/*
cl_int
set_kernel_args (cl_kernel &, unsigned int)
{
	return CL_SUCCESS;
}


template <typename T, typename... Args>
cl_int
set_kernel_args (cl_kernel &kernel, unsigned int n, const T& value, const Args&... args)
{
	cl_int err;
	err = clSetKernelArg(kernel, n, sizeof(T), (void*)&value);
	return err |
	       set_kernel_args(kernel, n + 1, args...);
}


template <typename... Args>
cl_int
set_kernel_args (cl_kernel &kernel, const Args&... args)
{
	return set_kernel_args(kernel, 0, args...);
}
*/

/*
template <typename... Args>
cl_int
set_kernel_args (Kernel &kernel, const Args&... args)
{
	return set_kernel_args(kernel.k, 0, args...);
}

*/


} // namespace cl


#endif /* __CL0X_HPP__CAC0CF09_2899_40BA_B8A4_8A664E92CA16 */

