/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#if !defined(_ocl_source) && !defined(_include_ocl_h)
#error "Error: do not include this file directly, use #include <ocl.h>"
#endif

#ifndef ocl_data_h
#define ocl_data_h

#include <search.h>
#include <sys/time.h>

#include "ocl_local.h"

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

\par OCL_MAX_PROGRAM_HASH_ENTRIES (default: 8)
Set the maximum number of hash entries that can be used for storing program objects

\par OCL_MAX_KERNEL_HASH_ENTRIES (default: 32)
Set the maximum number of hash entries that can be used for storing kernel objects.  This is on a \b per-program basis, i.e., the total memory allocated will be \b OCL_MAX_PROGRAM_HASH_ENTRIES \b * \b OCL_MAX_KERNEL_HASH_ENTRIES

\par OCL_MAX_TIMER_HASH_ENTRIES (default: 32)
Set the maximum number of hash entries that can be used for storing timer objects

\par OCL_MAX_FORTRAN_ALLOCATIONS (default: 1024)
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

#ifndef OCL_MAX_PROGRAM_HASH_ENTRIES
#define OCL_MAX_PROGRAM_HASH_ENTRIES 8
#endif

#ifndef OCL_MAX_KERNEL_HASH_ENTRIES
#define OCL_MAX_KERNEL_HASH_ENTRIES 32
#endif

#ifndef OCL_MAX_TIMER_HASH_ENTRIES
#define OCL_MAX_TIMER_HASH_ENTRIES 32
#endif

#ifndef OCL_MAX_FORTRAN_ALLOCATIONS
#define OCL_MAX_FORTRAN_ALLOCATIONS 1024
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

typedef struct ocl_kernel_hint_ {
	size_t local_size;
	size_t work_group_size;
	cl_ulong local_mem_size;
	size_t preferred_multiple;
} ocl_kernel_hint_t; 

typedef struct ocl_kernel_ {
	cl_kernel token;
	ocl_kernel_hint_t hint;
} ocl_kernel_t;

/*------------------------------------------------------------------------------
 * OpenCL program.
 *----------------------------------------------------------------------------*/

typedef struct ocl_program_ {
	cl_program token;

#if !defined(__APPLE__)
	struct hsearch_data kernels;
#endif
} ocl_program_t;

/*------------------------------------------------------------------------------
 * OpenCL device information.
 *----------------------------------------------------------------------------*/

static const uint32_t OCL_PERFORMANCE_DEVICE = 0;
static const uint32_t OCL_AUXILIARY_DEVICE = 1;

typedef struct ocl_data_ {
	ocl_device_instance_t devices[OCL_MAX_LOGICAL_DEVICES+2];

#if !defined(__APPLE__)
	// timer hash
	struct hsearch_data events;

	// program hash
	struct hsearch_data programs;
	int32_t tables;
	struct hsearch_data * free_tables[OCL_MAX_PROGRAM_HASH_ENTRIES];
#endif
	
	int32_t slots;
	int32_t open_slots[OCL_MAX_FORTRAN_ALLOCATIONS];
	int32_t allocations;
	void * free_allocations[OCL_MAX_FORTRAN_ALLOCATIONS];
} ocl_data_t;

ocl_device_instance_t * ocl_device_instance(uint32_t id);

#endif // DOXYGEN_SKIP

#endif // ocl_data_h
