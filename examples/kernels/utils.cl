/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#if defined(__OCL_APPLE__)
	#define FORMAT_STR(f) (const char *)((f))
#else
	#define FORMAT_STR(f) ((f))
#endif

#if defined(__OCL_INTEL__) || defined(__OCL_AMD__)
	#define STATIC
#else
	#define STATIC static
#endif

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
