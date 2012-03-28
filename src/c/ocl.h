/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
 *
 * $Revision: 46 $
 * $Date: 2012-03-28 12:39:08 -0600 (Wed, 28 Mar 2012) $
 * $Author: bergen $
\******************************************************************************/

#ifndef ocl_h
#define ocl_h

/*!
\page ocl
OpenCL Abstractions

\par Description:
OCL-MLA defines a mid-level abstraction layer to the OpenCL runtime for applications developers.  One of the primary features of the ocl interface is that it defines a set of logical devices (currently a \b performance device and an \b auxiliary device) that are able to run OpenCL C kernels.  Each logical device is created with an associated queue and context, and has a user interface for creating and manipulating data, arguments and timing events.  The current interface is available in \b C and \b Fortran.

\par Documentation:
Documentation is available for the individual subroutines, e.g., man ocl_init

\par Main Interface:
The main interface allows creation and manipulation of programs, kernels and buffers.  The main interface consists of the following subroutines: \n\n
\ref ocl_init \n
\ref ocl_finalize \n
\ref ocl_initialize_event \n
\ref ocl_create_buffer \n
\ref ocl_release_buffer \n
\ref ocl_enqueue_write_buffer \n
\ref ocl_enqueue_read_buffer \n
\ref ocl_add_program \n
\ref ocl_add_kernel \n
\ref ocl_set_kernel_arg \n
\ref ocl_enqueue_kernel_ndrange \n
\ref ocl_finish

\par Utilities Interface:
The utilities interface currently only allows creation and manipulation of timing events.  The interface consists of the following subroutines: \n\n
\ref ocl_add_timer \n
\ref ocl_clear_timer \n
\ref ocl_report_timer
*/

#ifdef __cplusplus
extern "C" {
#endif

#define _include_ocl_h

#include <ocl_data.h>
#include <ocl_device.h>
#include <ocl_interface.h>
#include <ocl_local.h>
#include <ocl_utils.h>

#ifdef __cplusplus
} // extern
#endif

#endif // ocl_h
