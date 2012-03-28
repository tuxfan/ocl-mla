/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
 *
 * $Revision: 43 $
 * $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
 * $Author: bergen $
\******************************************************************************/

#if !defined(_ocl_source) && !defined(_include_ocl_h)
#error "Error: do not include this file directly, use #include <ocl.h>"
#endif

#ifndef ocl_device_h
#define ocl_device_h

#include "ocl_data.h"

/*------------------------------------------------------------------------------
 * This function initializes a generic CPU device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_cpu(ocl_device_instance_t * instance,
	const char * platform);

/*------------------------------------------------------------------------------
 * This function initializes a generic GPU device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_gpu(ocl_device_instance_t * instance,
	const char * platform);

/*------------------------------------------------------------------------------
 * This function initializes a device instance as an generic multicore CPU.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_device(ocl_device_instance_t * instance,
	const char * platform, cl_uint device_type);

/*------------------------------------------------------------------------------
 * This function finalizes a device instance.
 *----------------------------------------------------------------------------*/

int32_t ocl_finalize_device(ocl_device_instance_t * instance);

#endif // ocl_device_h
