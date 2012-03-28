/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
 *
 * $Revision: 45 $
 * $Date: 2012-03-28 11:19:08 -0600 (Wed, 28 Mar 2012) $
 * $Author: bergen $
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
