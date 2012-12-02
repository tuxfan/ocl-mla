/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#define MAX_SIZE 1024

void scan(unsigned lid, __local unsigned * value, __local unsigned * count,
	unsigned p);

__kernel void phz_reduce(__global unsigned * value,
	__global unsigned * count) {
	size_t lid = get_local_id(0);
	size_t lsize = get_local_size(0);

	__local unsigned _count[MAX_SIZE];
	__local unsigned _value[MAX_SIZE];

	// read values into local memory
	_value[lid] = value[lid];

	// initialize with the value at the current work item
	_count[lid] = _value[lid];

	// add work item to the left if it exists
	if(lid>0) {
		_count[lid] += _value[lid-1];
	} // if

	for(size_t v=2, p=1; v<lsize; v*=2, ++p) {
		if(lid>=p) {
			scan(lid, _value, _count, p);
		} // if
	} // for
} // phz_reduce

void scan(unsigned lid, __local unsigned * value, __local unsigned * count, unsigned p) {
	// carry the previous sum
	count[lid] = value[lid];

	// compute power of 2 for left-hand offset
	unsigned factor = 1<<p;

	if(lid>factor) {
		count[lid] += value[lid-factor];	
	} // if
} // scan

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
