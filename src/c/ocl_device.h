/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#if !defined(_ocl_source) && !defined(_include_ocl_h)
#error "Error: do not include this file directly, use #include <ocl.h>"
#endif

#ifndef ocl_device_h
#define ocl_device_h

#if defined(HAVE_CONFIG_H)
	#include <ocl_config.h>
#endif

#include "ocl_data.h"

/*------------------------------------------------------------------------------
 * Function type definition.
 *----------------------------------------------------------------------------*/

typedef int32_t(*ocl_init_t)(ocl_device_instance_t *, const char *, size_t);

/*------------------------------------------------------------------------------
 * This function initializes a generic CPU device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_cpu(ocl_device_instance_t * instance,
	const char * platform, size_t thread);

/*------------------------------------------------------------------------------
 * This function initializes a generic GPU device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_gpu(ocl_device_instance_t * instance,
	const char * platform, size_t thread);

/*------------------------------------------------------------------------------
 * This function initializes a generic MIC device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_mic(ocl_device_instance_t * instance,
	const char * platform, size_t thread);

/*------------------------------------------------------------------------------
 * This function initializes a device instance as an generic multicore CPU.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_device(ocl_device_instance_t * instance,
	const char * platform, cl_uint device_type, size_t thread);

/*------------------------------------------------------------------------------
 * This function finalizes a device instance.
 *----------------------------------------------------------------------------*/

int32_t ocl_finalize_device(ocl_device_instance_t * instance);

#endif // ocl_device_h
