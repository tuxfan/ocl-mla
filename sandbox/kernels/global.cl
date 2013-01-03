/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

//__constant int phz_ptr_address_lo;
//__constant int phz_ptr_address_hi;

__kernel void test(__global unsigned * value, __global unsigned * offset) {
	size_t lid = get_local_id(0);
	offset[lid] = lid;
	printf("lid: %d\n", (int)lid);
} // test

/*
 * vim: set syntax=c :
 */
