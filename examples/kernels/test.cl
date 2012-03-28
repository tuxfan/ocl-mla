/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
 *
 * $Revision: 43 $
 * $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
 * $Author: bergen $
\******************************************************************************/

#if defined(__APPLE__)
	#define FORMAT_STR(f) (const char *)((f))
#else
	#define FORMAT_STR(f) ((f))
#endif

/*------------------------------------------------------------------------------
 * Trivial kernel
 *----------------------------------------------------------------------------*/

__kernel void test(__global float * data) {
	size_t gid = get_global_id(0);

	data[gid] = (float)gid;
} // test

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
