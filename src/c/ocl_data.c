/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
 *
 * $Revision: 43 $
 * $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
 * $Author: bergen $
\******************************************************************************/

#define _ocl_source

#include "ocl_data.h"

/*------------------------------------------------------------------------------
 * Global state data.
 *----------------------------------------------------------------------------*/

ocl_data_t ocl;

int32_t ocl_warning;

ocl_device_instance_t * ocl_device_instance(uint32_t id) {
	// FIXME: NEED TO CHECK VALID RANGE
	return &ocl.devices[id];
} // ocl_device_instance
