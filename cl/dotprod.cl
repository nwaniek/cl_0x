__kernel void
dotprod (__global float *dst, __global const float *a, __global const float *b,
		const unsigned int dim)
{
	// calculate how many items one thread has to compute
	size_t iter = floor(dim / get_global_size(0));
	size_t pos = get_global_id(0);
	const size_t idx = get_global_id(0);

	// clear the output
	dst[pos] = 0.0f;

	for (int i = 0; i < iter; i++) {
		dst[idx] += a[pos] * b[pos];
		pos += get_global_size(0);
	}

	// if there are any items left to be computed, do that now
	if (pos < dim)
		dst[idx] += a[pos] * b[pos];
}
