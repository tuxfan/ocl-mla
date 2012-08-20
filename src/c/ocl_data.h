/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#if !defined(_ocl_source) && !defined(_include_ocl_h)
#error "Error: do not include this file directly, use #include <ocl.h>"
#endif

#ifndef ocl_data_h
#define ocl_data_h

#if defined(HAVE_CONFIG_H)
	#include <ocl_config.h>
#endif

#include <ocl_device_config.h>

#include <sys/time.h>

#include <ocl_local.h>

/*------------------------------------------------------------------------------
 * Preprocessor defines
 *----------------------------------------------------------------------------*/

/*!
\page ocl_preprocessor_defines
Preprocessor defines used by OCL-MLA to configure behavior

\par OCL_MAX_LOGICAL_DEVICES (default: 10)
Set the maximum number of logical devices that can be created by the user.  The default logical devices \b OCL_PERFORMANCE_DEVICE and \b OCL_AUXILIARY_DEVICE are not included in this number

\par OCL_MAX_PLATFORMS (default: 10)
Set the maximum number of OpenCL platforms that are supported.

\par OCL_MAX_PLATFORM_DEVICES (default: 10)
Set the maximum number of OpenCL devices that are supported per platform.

\par OCL_EVENT_LIST_BLOCK_SIZE (default: 32)
Set the granularity at which data are allocated for storing events in an event list

\par OCL_MAX_SOURCE_LENGTH (default: 16*1024)
Set the maximum length that will be allocated for source assembly functions (see \b ocl_add_from_file and \b ocl_add_from_string)

\par OCL_MAX_ALLOCATIONS (default: 1024)
Set the maximum number of memory allocations that can be mananged automatically for Fortran garbage collection.

\par OCL_MIN_WORK_GROUP_SIZE (default: 8)
Specifiy the minimum work group size that is allowed as output from \b ocl_ndrange_hints.

\par OCL_PREFER_LARGE_WORK_GROUP_SIZE (default: 0)
Specifiy whether or not \b ocl_ndrange_hints should favor larger work groups or fewer single elements.
*/

#ifndef DOXYGEN_SKIP

#ifndef OCL_MAX_LOGICAL_DEVICES
#define OCL_MAX_LOGICAL_DEVICES 10
#endif

#ifndef OCL_MAX_PLATFORMS
#define OCL_MAX_PLATFORMS 10
#endif

#ifndef OCL_MAX_PLATFORM_DEVICES
#define OCL_MAX_PLATFORM_DEVICES 10
#endif

#ifndef OCL_EVENT_LIST_BLOCK_SIZE
#define OCL_EVENT_LIST_BLOCK_SIZE 32
#endif

#ifndef OCL_MAX_SOURCE_LENGTH
#define OCL_MAX_SOURCE_LENGTH 16*1024
#endif

#ifndef OCL_MAX_ALLOCATIONS
#define OCL_MAX_ALLOCATIONS 1024
#endif

#ifndef OCL_MIN_WORK_GROUP_SIZE
#define OCL_MIN_WORK_GROUP_SIZE 8
#endif

#ifndef OCL_PREFER_LARGE_WORK_GROUP_SIZE
#define OCL_PREFER_LARGE_WORK_GROUP_SIZE 0
#endif

/*------------------------------------------------------------------------------
 * OpenCL reference information.  This is for the Fortran 90 interface to
 * provide a robust pointer reference.
 *----------------------------------------------------------------------------*/

typedef struct ocl_reference_ {
	void * data;
} ocl_reference_t;

/*------------------------------------------------------------------------------
 * OpenCL allocation information.  This is for the Fortran 90 interface to
 * help in book keeping for freeing memory allocations.
 *----------------------------------------------------------------------------*/

typedef struct ocl_allocation_ {
	int32_t index;
	void * data;
} ocl_allocation_t;

/*------------------------------------------------------------------------------
 * OpenCL device information.  This data structure will be extended to
 * provide information to be used in determining work-group size and
 * scheduling.
 *----------------------------------------------------------------------------*/

typedef struct ocl_device_info_ {
	char platform_name[256];
	char platform_defines[1024];
	uint32_t version_major;
	uint32_t version_minor;
	char name[256];
	cl_device_type type;
	cl_uint vendor_id;
	cl_uint max_compute_units;
	cl_uint max_clock_frequency;
	size_t max_work_group_size;
	cl_uint max_work_item_dimensions;
	size_t max_work_item_sizes[3];
	cl_ulong local_mem_size;
} ocl_device_info_t;

/*------------------------------------------------------------------------------
 * OpenCL device information.
 *----------------------------------------------------------------------------*/

typedef struct ocl_device_instance_ {
	cl_device_id id;
	cl_context context;
	cl_command_queue queue;
	ocl_device_info_t info;
} ocl_device_instance_t;

/*------------------------------------------------------------------------------
 * OpenCL event information.
 *----------------------------------------------------------------------------*/

typedef struct ocl_event_ {
	cl_uint num_events_in_wait_list;
	const cl_event * event_wait_list;
	cl_event event;
} ocl_event_t;

typedef struct ocl_event_wait_list_ {
	cl_uint num_events_in_wait_list;
	cl_event * event_wait_list;
	size_t allocated;
	size_t fixed_size;
	int32_t index;
} ocl_event_wait_list_t;

typedef struct ocl_host_timer_data_ {
	struct timeval start;
	double duration;
} ocl_host_timer_data_t;

typedef struct ocl_timer_event_ {
	double queued;
	double invocation;
	double duration;
	double aggregate;
} ocl_timer_event_t;

/*------------------------------------------------------------------------------
 * OpenCL kernel information.
 *----------------------------------------------------------------------------*/

typedef struct ocl_kernel_hints_ {
	size_t max_work_group_size;
	size_t local_mem_size;
} ocl_kernel_hints_t; 

typedef struct ocl_kernel_work_group_info_ {
	size_t global_work_size[3];
	size_t work_group_size;
	size_t compile_work_group_size[3];
	cl_ulong local_mem_size;
	size_t preferred_multiple;
	cl_ulong private_mem_size;
} ocl_kernel_work_group_info_t;

typedef struct ocl_kernel_ {
	cl_kernel token;
} ocl_kernel_t;

/*------------------------------------------------------------------------------
 * OpenCL buffer.
 *----------------------------------------------------------------------------*/

typedef struct ocl_buffer_ {
	cl_mem token;
} ocl_buffer_t;

/*------------------------------------------------------------------------------
 * OpenCL program.
 *----------------------------------------------------------------------------*/

typedef struct ocl_program_ {
	uint32_t device_id;
	cl_program token;
	size_t kernel_hash;
} ocl_program_t;

/*------------------------------------------------------------------------------
 * OpenCL device information.
 *----------------------------------------------------------------------------*/

#define OCL_DEVICE_TYPES 4
static const size_t OCL_CPU = 0;
static const size_t OCL_GPU = 1;
static const size_t OCL_ACCELERATOR = 2;
static const size_t OCL_CUSTOM = 3;

typedef struct ocl_data_ {
	ocl_device_instance_t devices[OCL_MAX_LOGICAL_DEVICES+2];
	size_t initialized_devices[OCL_DEVICE_TYPES];

	size_t device_hash[OCL_MAX_LOGICAL_DEVICES+2];
	size_t program_hash;
	size_t host_event_hash;
	size_t device_event_hash;
	
	int32_t slots;
	int32_t open_slots[OCL_MAX_ALLOCATIONS];
	int32_t allocations;
	void * free_allocations[OCL_MAX_ALLOCATIONS];
} ocl_data_t;

ocl_device_instance_t * ocl_device_instance(uint32_t id);

#endif // DOXYGEN_SKIP

#endif // ocl_data_h
