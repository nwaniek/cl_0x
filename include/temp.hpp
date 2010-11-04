#ifndef __TEMP_HPP__756F0599_402B_40CF_AB58_233DC0A41522
#define __TEMP_HPP__756F0599_402B_40CF_AB58_233DC0A41522

#include <CL/cl.h>
#include <iostream>

template <typename T1, typename T2>
void set_kernel_args (cl_kernel k, T1 &p1, T2 &p2)
{
	cl_int err;

	err  = clSetKernelArg(k, 0, sizeof(T1), (void*)&p1);
	err |= clSetKernelArg(k, 0, sizeof(T2), (void*)&p2);
	if (err != CL_SUCCESS)
		std::cout << "Could not set kernel argument" << std::endl;
}


#endif /* __TEMP_HPP__756F0599_402B_40CF_AB58_233DC0A41522 */

