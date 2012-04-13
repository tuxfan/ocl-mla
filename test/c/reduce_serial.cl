/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

/*------------------------------------------------------------------------------
 * Reduction kernel
 *----------------------------------------------------------------------------*/

#pragma OPENCL EXTENSION cl_amd_printf : enable

__kernel void reduce_serial(__global const int * elements,
	__global const float * a, __global float * acc) {
	int i;

	__local float _acc = 0.0f;

	for(i=0; i<(*elements); ++i) {
		_acc += a[i];
	} // for

	*acc = _acc;
} // reduce_serial

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