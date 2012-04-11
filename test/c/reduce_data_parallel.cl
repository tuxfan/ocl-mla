/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

/*------------------------------------------------------------------------------
 * Return 1 if n is a power of 2.
 *----------------------------------------------------------------------------*/

#define pow2(s) (((s) & ((s) - 1)) == 0)

/*------------------------------------------------------------------------------
 * Return 1 if n is a power of 4.
 *----------------------------------------------------------------------------*/

STATIC inline int pow4(size_t n) {
	int count = 0;

	while(n>1) {
		n >>= 1;
		count += 1;
	} // while

	return (count%2 == 0) ? 1 : 0;
} // pow4

/*------------------------------------------------------------------------------
 * Reduction kernel
 *----------------------------------------------------------------------------*/

#pragma OPENCL EXTENSION cl_amd_printf : enable

__kernel void reduce_data_parallel(__global const float4 * a,
	__local float * s_a, __global float * acc) {
	const float4 one = { 1.0f, 1.0f, 1.0f, 1.0f };
	const size_t lid = get_local_id(0);
	const size_t gid = get_global_id(0);
	const size_t lsize = get_local_size(0);
	const size_t max_width = 0.25f*lsize;
	const size_t grpid = get_group_id(0);
//	__local float s_a[OCL_MAX_WGSIZE];

#if 0
	printf("lid: %d gid: %d grpid: %d lsize: %d\n",
		(int)lid, (int)gid, (int)grpid, (int)lsize);
#endif

	// redice float4
	s_a[lid] = dot(a[gid], one);

	barrier(CLK_LOCAL_MEM_FENCE);

#define REDUCE(_id, _a, _w) 								\
	if((_id)%((_w)*4) == 0) {								\
		const float4 tmp = {	(_a)[(_id)],				\
									(_a)[(_id)+1*(_w)],		\
									(_a)[(_id)+2*(_w)],		\
									(_a)[(_id)+3*(_w)] };	\
		(_a)[(_id)] = dot(tmp, one);						\
	} /* if */													\
																	\
	barrier(CLK_LOCAL_MEM_FENCE);

//		printf("id: %d width: %d\n", (int)_id, (int)_w);\

	size_t width = 1;

	// reduce by factors of 4 (SIMD width)
	while(width <= max_width) {
		REDUCE(lid, s_a, width);
		width *= 4;
	} // while

#undef REDUCE

	// Handle power of 2
	if(lid == 0 && pow4(lsize) == 0) {
		s_a[0] = s_a[0] + s_a[lsize/2];
	} // if

	// add to global accumulation
	if(lid == 0) {
		acc[grpid] = s_a[0];
	} // if
} // reduce_data_parallel

/*
 * Local Variables:
 * mode: c
 * c-basic-offset:3
 * c-file-offsets: ((arglist-intro . +))
 * indent-tabs-mode:t
 * tab-width:3
 * End:
 *
 * vim: set syntax=c : set ts=3 :
 */
