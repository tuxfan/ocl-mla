/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#ifndef PHZ_MAX_LOCAL_ARRAY_SIZE
#define PHZ_MAX_LOCAL_ARRAY_SIZE 1024
#endif

/*----------------------------------------------------------------------------*
 * Prototypes
 *----------------------------------------------------------------------------*/

void phz_reduce(unsigned lid, unsigned lsize,
	__local unsigned * count, __local unsigned * offset);

void phz_scan(unsigned lid, __local unsigned * count,
	unsigned p, __local unsigned * sum);

/*----------------------------------------------------------------------------*
 * Stream reduction
 *----------------------------------------------------------------------------*/

void phz_reduce(unsigned lid, unsigned lsize,
	__local unsigned * count, __local unsigned * offset) {

	// store this work item's count to correct offset
	unsigned _count = count[lid];

	// initialize with the count at the current work item
	offset[lid] = count[lid];

	// add work item to the left if it exists
	if(lid>0) {
		offset[lid] += count[lid-1];
	} // if

	// block for consistency
	barrier(CLK_LOCAL_MEM_FENCE);

	for(size_t p=2; p<lsize; p*=2) {
		phz_scan(lid, offset, p, count);
		offset[lid] = count[lid];
		barrier(CLK_LOCAL_MEM_FENCE);
	} // for

	// write output
	offset[lid] = count[lid]-_count;
} // phz_reduce

/*----------------------------------------------------------------------------*
 * Summing scan
 *----------------------------------------------------------------------------*/

void phz_scan(unsigned lid, __local unsigned * count,
	unsigned p, __local unsigned * sum) {

	// carry the previous sum
	sum[lid] = count[lid];

	// add power of two to the left work item if it exists
	if(lid>=p) {
		sum[lid] += count[lid-p];	
	} // if

	// block for consistency
	barrier(CLK_LOCAL_MEM_FENCE);
} // phz_scan

/*
 * vim: set syntax=c :
 */
