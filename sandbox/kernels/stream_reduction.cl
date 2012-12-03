/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

/*----------------------------------------------------------------------------*
 * Test kernel
 *----------------------------------------------------------------------------*/

__kernel void reduce(__global const unsigned * count,
	__global unsigned * offset) {
	size_t lid = get_local_id(0);
	size_t lsize = get_local_size(0);

	__local unsigned _count[8];
	__local unsigned _offset[8];

	_count[lid] = count[lid];

	phz_reduce(lid, lsize, _count, _offset);

	offset[lid] = _offset[lid];

	if(count[lid] > 0) {
		printf("thread %d has %d values and writes to offset %d\n",
			lid, count[lid], offset[lid]);
	} // if
} // phz_reduce

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
