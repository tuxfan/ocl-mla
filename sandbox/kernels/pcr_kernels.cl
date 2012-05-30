/*
	Adapted for OCL-MLA from NVIDIA example.
 */

/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
 
 /*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 * 
 * Tridiagonal solvers.
 * Device code for parallel cyclic reduction (PCR).
 *
 * Original CUDA kernels: UC Davis, Yao Zhang & John Owens, 2009
 * 
 * NVIDIA, Nikolai Sakharnykh, 2009
 */

//#define NATIVE_DIVIDE

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

__kernel void pcr_branch_free_kernel(__global real_t *a_d,
	__global real_t *b_d, __global real_t *c_d, __global real_t *d_d,
	__global real_t *x_d, __local real_t *shared, int system_size,
	int num_systems, int iterations) {
	size_t thid = get_local_id(0);
	size_t blid = get_group_id(0);

	int delta = 1;

	__local real_t * a = shared;
	__local real_t * b = &a[system_size+1];
	__local real_t * c = &b[system_size+1];
	__local real_t * d = &c[system_size+1];
	__local real_t * x = &d[system_size+1];

	a[thid] = a_d[thid + blid * system_size];
	b[thid] = b_d[thid + blid * system_size];
	c[thid] = c_d[thid + blid * system_size];
	d[thid] = d_d[thid + blid * system_size];

	real_t aNew, bNew, cNew, dNew;
  
	barrier(CLK_LOCAL_MEM_FENCE);

	// parallel cyclic reduction
	for (int j = 0; j < iterations; j++) {
		int i = thid;

		int iRight = i+delta;
		iRight = iRight & (system_size-1);

		int iLeft = i-delta;
		iLeft = iLeft & (system_size-1);

#ifndef NATIVE_DIVIDE
		real_t tmp1 = a[i] / b[iLeft];
		real_t tmp2 = c[i] / b[iRight];
#else
		real_t tmp1 = native_divide(a[i], b[iLeft]);
		real_t tmp2 = native_divide(c[i], b[iRight]);
#endif

		bNew = b[i] - c[iLeft] * tmp1 - a[iRight] * tmp2;
		dNew = d[i] - d[iLeft] * tmp1 - d[iRight] * tmp2;
		aNew = -a[iLeft] * tmp1;
		cNew = -c[iRight] * tmp2;

		barrier(CLK_LOCAL_MEM_FENCE);
        
		b[i] = bNew;
 		d[i] = dNew;
		a[i] = aNew;
		c[i] = cNew;	
    
		delta *= 2;
		barrier(CLK_LOCAL_MEM_FENCE);
	} // for

	if (thid < delta) {
		int addr1 = thid;
		int addr2 = thid + delta;
		real_t tmp3 = b[addr2] * b[addr1] - c[addr1] * a[addr2];
#ifndef NATIVE_DIVIDE
		x[addr1] = (b[addr2] * d[addr1] - c[addr1] * d[addr2]) / tmp3;
		x[addr2] = (d[addr2] * b[addr1] - d[addr1] * a[addr2]) / tmp3;
#else
		x[addr1] = native_divide((b[addr2] * d[addr1] - c[addr1] *
			d[addr2]), tmp3);
		x[addr2] = native_divide((d[addr2] * b[addr1] - d[addr1] *
			a[addr2]), tmp3);
#endif
	} // if
    
	barrier(CLK_LOCAL_MEM_FENCE);
    
	x_d[thid + blid * system_size] = x[thid];
} // pcr_branch_free_kernel

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
