/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#if !defined(_ocl_source) && !defined(_include_ocl_h)
#error "Error: do not include this file directly, use #include <ocl.h>"
#endif

#ifndef ocl_interface_h
#define ocl_interface_h

#include <search.h>

#include "ocl_local.h"
#include "ocl_data.h"

/*----------------------------------------------------------------------------*\
 * Global initialization of OpenCL layer.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_init
Initialize the OpenCL runtime layer.

\par C Version:
ierr ocl_init()

\par Fortran Version:
ocl_init(ierr)

@param ierr Error status \n int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine initializes the OpenCL runtime layer using the setup method
defined by the build environment and creates logical devices with
associated contexts and queues suitable for executing OpenCL kernels.
Additionally, the C version takes a pointer reference to be filled with
the global state data.
*/

int32_t ocl_init();

/*!
\page ocl_init_threaded
Initialize the OpenCL runtime layer for multiple threads.

\par C Version:
ierr ocl_init_threaded(thread)

\par Fortran Version:
ocl_init_threaded(thread, ierr)

@param thread The zero-based thread id of the calling thread.\n
size_t (\b C), integer(int32_t) (\b Fortran)
@param ierr Error status \n int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine initializes the OpenCL runtime layer using the setup method defined by the build environment and creates logical devices with associated contexts and queues suitable for executing OpenCL kernels.  Additionally, this function will attempt to evenly subscribe device resources using the thread information provided in \b thread.  If the user oversubscribes the hardware, a round-robin approach will be used.
*/

int32_t ocl_init_threaded(size_t thread);

/*----------------------------------------------------------------------------*\
 * Global shutdown of OpenCL layer.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_finalize
Shut-Down the OpenCL runtime layer.

\par C Version:
ierr ocl_finalize()

\par Fortran Version:
ocl_finalize(ierr)

@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine shuts down the OpenCL runtime layer.
*/

int32_t ocl_finalize();

/*----------------------------------------------------------------------------*\
 * Initialize event data
\*----------------------------------------------------------------------------*/

/*!
\page ocl_initialize_event
Initialize an ocl_event_t structure.

\par C Version:
ierr ocl_initialize_event(event)

\par Fortran Version:
ocl_initialize_event(event, ierr)

@param event Event data structure to initialize \n
ocl_event_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine initializes an ocl_event_t data structure for use in collecting
timing information.  The Fortran version additionally allocates a C-side data
structure.  This memory is automatically free'd when it is no longer in use.
*/

int32_t ocl_initialize_event(ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 * Release event data
\*----------------------------------------------------------------------------*/

/*!
\page ocl_release_event
Initialize an ocl_event_t structure.

\par C Version:
ierr ocl_release_event(event)

\par Fortran Version:
ocl_release_event(event, ierr)

@param event Event data structure to initialize \n
ocl_event_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This function releases the event data structure \b event.
*/

int32_t ocl_release_event(ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 * Initialize event wait list data
\*----------------------------------------------------------------------------*/

/*!
\page ocl_initialize_event_wait_list
Initialize an ocl_event_wait_list_t structure.

\par C Version:
ierr ocl_initialize_event_wait_list(list)

\par Fortran Version:
ocl_initialize_event_wait_list(list, ierr)

@param event Event wait list data structure to initialize \n
ocl_event_wait_list_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine initializes an ocl_event_wait_list_t data structure for use
in collecting timing information.  The Fortran version additionally allocates
a C-side data structure.  This memory is automatically free'd when it is no
longer in use.

\sa ocl_clear_event_wait_list
*/

int32_t ocl_initialize_event_wait_list(ocl_event_wait_list_t * list);

/*----------------------------------------------------------------------------*\
 * Add an event to a wait list
\*----------------------------------------------------------------------------*/

/*!
\page ocl_add_event_to_wait_list
Add an event to a wait list.

\par C Version:
ierr ocl_add_event_to_wait_list(list, event)

\par Fortran Version:
ocl_add_event_to_wait_list(list, event, ierr)

@param list Event wait list data structure \n
ocl_event_wait_list_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param event Event data structure to add to wait list \n
ocl_event__t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine adds \b event to \b list, allocating memory for storing
events as needed in blocks of \b OCL_EVENT_LIST_BLOCK_SIZE.  This memory is
automatically free'd when it is no longer in use.

\sa ocl_initialize_event, ocl_initialize_event_wait_list, ocl_clear_event_wait_list, ocl_clear_event
*/

int32_t ocl_add_event_to_wait_list(ocl_event_wait_list_t * list,
	ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 * Set the events list parameter
\*----------------------------------------------------------------------------*/

/*!
\page ocl_set_event_list
Set event wait list for an event.

\par C Version:
ierr ocl_set_event_list(event, list)

\par Fortran Version:
ocl_set_event_list(event, list, ierr)

@param event Event data structure\n
ocl_event__t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param list Event wait list data structure \n
ocl_event_wait_list_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine sets the \b event data structure's wait list to \b list.  When the event is used in execution calls that take an \b ocl_event_t argument, execution will block until the events in the list have completed.

\sa ocl_enqueue_kernel_ndrange, ocl_enqueue_write_buffer, ocl_enqueue_read_buffer
*/

int32_t ocl_set_event_list(ocl_event_t * event, ocl_event_wait_list_t * list);

/*----------------------------------------------------------------------------*\
 * ocl_clear_event_wait_list
\*----------------------------------------------------------------------------*/

/*!
\page ocl_clear_event_wait_list

\par C Version:
ierr ocl_clear_event_wait_list(list)

\par Fortran Version:
ocl_clear_event_wait_list(list, ierr)

@param list Event wait list data structure \n
ocl_event_wait_list_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine clears the provided wait list.

\sa SEE_ALSO
*/

int32_t ocl_clear_event_wait_list(ocl_event_wait_list_t * list);

// FIXME: man pages and headings

/*----------------------------------------------------------------------------*\
 * Clear an event.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_clear_event

\par C Version:
ierr ocl_clear_event(event)

\par Fortran Version:
ocl_clear_event(event, ierr)

@param event Event data structure\n
ocl_event__t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine clears the provided event.

\sa SEE_ALSO
*/

int32_t ocl_clear_event(ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 *
\*----------------------------------------------------------------------------*/

/*!
\page ocl_wait_for_events

\par C Version:
ierr ocl_wait_for_events(list)

\par Fortran Version:
ocl_wait_for_events(list, ierr)

@param list Event wait list data structure \n
ocl_event_wait_list_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n

\par Description:
Wait for the events specified in the list.

\sa SEE_ALSO
*/

int32_t ocl_wait_for_events(ocl_event_wait_list_t * list);

/*----------------------------------------------------------------------------*\
 * Create a buffer on a device instance.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_create_buffer
Create a device-side buffer.

\par C Version:
ierr ocl_create_buffer(device_id, size, flags, host_ptr, buffer)

\par Fortran Version:
ocl_create_buffer(device_id, size, flags, host_ptr, buffer, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param size Buffer size in bytes \n
size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param flags Buffer creation flags (See below)\n
cl_mem_flag (\b C), integer(cl_bitfield) (\b Fortran) \n
@param host_ptr Host-side buffer \n
void * (\b C), type(c_ptr) (\b Fortran) \n
@param buffer Device-side buffer handle \n
cl_mem (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine creates a device-side buffer on the given logical device.

\par Buffer Creation Flags:
\b CL_MEM_READ_WRITE, \b CL_MEM_WRITE_ONLY, \b CL_MEM_READ_ONLY,
\b CL_MEM_USE_HOST_PTR, \b CL_MEM_ALLOC_HOST_PTR, \b CL_MEM_COPY_HOST_PTR
*/

int32_t ocl_create_buffer(uint32_t device_id, size_t size,
	cl_mem_flags flags, void * host_ptr, cl_mem * buffer);

/*----------------------------------------------------------------------------*\
 * Release memory object.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_release_buffer
Release a device-side buffer.

\par C Version:
ierr ocl_release_buffer(buffer)

\par Fortran Version:
ocl_release_buffer(buffer, ierr)

@param buffer Device-side buffer handle \n
cl_mem (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine releases a device-side buffer, freeing any memory that was
allocated for it.
*/

int32_t ocl_release_buffer(cl_mem * buffer);

/*----------------------------------------------------------------------------*\
 * Write buffer to device.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_enqueue_write_buffer
Enqueue a write-buffer operation.

\par C Version:
ierr ocl_enqueue_write_buffer(device_id, buffer, synchronous, offset, cb, ptr, event)

\par Fortran Version:
ocl_enqueue_write_buffer(device_id, buffer, synchronous, offset, cb, ptr, event, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param buffer Device-side buffer handle \n
cl_mem (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param synchronous Flag for synchronous (blocking) execution \n
int32_t (\b C), integer(int32_t) (\b Fortran) \n
@param offset Byte offset within buffer to begin write \n
size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param cb Bytes to write \n
size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param ptr Source buffer \n
void * (\b C), type(c_ptr) (\b Fortran) \n
@param event Event handle (used to collect timing information) \n
ocl_event_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine enqueues a write-buffer operation on the given device.
*/

int32_t ocl_enqueue_write_buffer(uint32_t device_id, cl_mem buffer,
	int32_t synchronous, size_t offset, size_t cb, void * ptr,
	ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 * Read buffer from device.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_enqueue_read_buffer
Enqueue a read-buffer operation.

\par C Version:
ierr ocl_enqueue_read_buffer(device_id, buffer, synchronous, offset, cb, ptr, event)

\par Fortran Version:
ocl_enqueue_read_buffer(device_id, buffer, synchronous, offset, cb, ptr, event, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param buffer Device-side buffer handle \n
cl_mem (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param synchronous Flag for synchronous (blocking) execution \n
int32_t (\b C), integer(int32_t) (\b Fortran) \n
@param offset Byte offset within buffer to begin read \n
size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param cb Bytes to read \n
size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param ptr Source buffer \n
void * (\b C), type(c_ptr) (\b Fortran) \n
@param event Event handle (used to collect timing information) \n
ocl_event_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine enqueues a read-buffer operation on the given device.
*/

int32_t ocl_enqueue_read_buffer(uint32_t device_id, cl_mem buffer,
	int32_t synchronous, size_t offset, size_t cb, void * ptr,
	ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 * Add OpenCL program.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_add_program
Add an OpenCL program

\par C Version:
ierr ocl_add_program(device_id, program_name, program_source, compiler_options)

\par Fortran Version:
ocl_add_program(device_id, program_name, program_source, compiler_options, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param program_name Program name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param program_source Program source code \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param compile_options Program compile options \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine creates and compiles a program on the given device.  The program will be associated with the \b program_name hash key, and should be used in future calls that require a program. The program name hash key is arbitrary, but must be unique within the calling program space (in main).  The Fortran interface requires that all strings be manually NULL terminated using 'string // C_NULL_CHAR'.
*/

int32_t ocl_add_program(uint32_t device_id, const char * program_name,
	const char * program_source, const char * compile_options);

/*----------------------------------------------------------------------------*\
 * Add OpenCL kernel.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_add_kernel
Add an OpenCL kernel

\par C Version:
ierr ocl_add_kernel(device_id, program_name, kernel_source_name, kernel_name)

\par Fortran Version:
ocl_add_kernel(device_id, program_name, kernel_source_name, kernel_name, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param program_name Program name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param kernel_source_name Kernel name in program source \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param kernel_name Kernel name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine creates a kernel on the given device.  The \b kernel_source_name must be defined in the corresponding program designated by \b program_name, which must have already been created using \b ocl_add_program.  The kernel name hash key is unique to the corresponding program.
*/

int32_t ocl_add_kernel(uint32_t device_id, const char * program_name,
	const char * kernel_source_name, const char * kernel_name);

// FIXME: need man page
int32_t ocl_kernel_token(const char * program_name, const char * kernel_name,
	ocl_kernel_t * token);

/*----------------------------------------------------------------------------*\
 * Set kernel argument.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_set_kernel_arg
Add kernel argument.

\par C Version:
ierr ocl_set_kernel_arg(program_name, kernel_name, index, size, value)

\par Fortran Version:
ocl_set_kernel_arg(program_name, kernel_name, index, size, value, ierr)

@param program_name Program name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param kernel_name Kernel name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param index The argument index \n
cl_uint (\b C), integer(int32_t) (\b Fortran) \n
@param size Argument size in bytes \n
size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param value Argument address \n
void * (\b C), type(c_ptr) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
Add a kernel argument to the kernel specified by \b kernel_name in \b program_name.  The index of the argument should correspond to the order of the kernel parameters.
*/

int32_t ocl_set_kernel_arg(const char * program_name, const char * kernel_name,
	cl_uint index, size_t size, const void * value);

/*----------------------------------------------------------------------------*\
 * Get kernel hint.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_kernel_hint
Get the preferred/maximum workgroup size.

\par C Version:
ierr ocl_kernel_hint(program_name, kernel_name, hint)

\par Fortran Version:
ocl_kernel_hint(program_name, kernel_name, hint, ierr)

@param program_name Program name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param kernel_name Kernel name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param hint Pointer to hint data to be filled \n
size_t * (\b C), integer(c_size_t) (\b Fortran)

\par Description:
This subroutine fills \b hint with the preferred/maximum \b local_size to use in calls to ocl_enqueue_kernel_ndrange.  The method used to compute the hint may be architecture specific and can be overridden at compile time by defining the \b KERNEL_HINT_FUNCTION preprocessor varialbe.
*/

int32_t ocl_kernel_hint(const char * program_name,
	const char * kernel_name, size_t * hint);

/*----------------------------------------------------------------------------*\
 * Get maximum work group size
\*----------------------------------------------------------------------------*/

/*!
\page ocl_max_work_group_size
Get the device maximum work group size.

\par C Version:
ierr ocl_max_work_group_size(device_id, max_work_group_size)

\par Fortran Version:
ocl_max_work_group_size(device_id, max_work_group_size, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param max_work_group_size Maximum work group size for the device specified in \b device_id \n
size_t * (\b C), integer(c_size_t) (\b Fortran)

\par Description:
This function returns the maximum work group size supported by the device specified in \b device_id.  \n NOTE: the maximum returned by this function may be larger than the maximum for a given kernel.
*/

int32_t ocl_max_work_group_size(uint32_t device_id,
	size_t * max_work_group_size);

/*----------------------------------------------------------------------------*\
 * Get ndrange hints.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_ndrange_hints
Get hints for executing an ndrange for a given number of indeces.

\par C Version:
ierr ocl_ndrange_hints(indeces, max_work_group_size, work_group_size, work_group_indeces, single_indeces)

\par Fortran Version:
ocl_ndrange_hints(indeces, max_work_group_size, work_group_indeces, single_indeces, ierr)

@param indeces Number of indeces to be processed, i.e., global work size.
size_t (\b C), integer(c_size_t) (\b Fortran)
@param max_work_group_size Maximum work group size to use, i.e., desired block size to process \b indeces.
size_t (\b C), integer(c_size_t) (\b Fortran)
@param work_group_indeces Number of indeces that can be processed with the requested block size.
size_t * (\b C), integer(c_size_t) (\b Fortran)
@param single_indeces Number of indeces that must be processed with the a work group size of 1.
size_t * (\b C), integer(c_size_t) (\b Fortran)

/FIXME: add documentation

\par Description:
This function determines a work group size to process the given number of \b indeces.  The function returns the maximum number of work items that can be processed in blocks of \b work_group_size in \b work_group_indeces and the remainder that must be processed with a work group size of 1 in \b single_indeces.
*/

int32_t ocl_ndrange_hints(size_t indeces, size_t max_work_group_size,
	double work_group_weight, double single_element_weight,
	size_t * work_group_size, size_t * work_group_indeces,
	size_t * single_indeces);

/*----------------------------------------------------------------------------*\
 * Enqueue an ND Range kernel.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_enqueue_kernel_ndrange
Enqueue an OpenCL NDRange kernel.

\par C Version:
ierr ocl_enqueue_kernel_ndrange(device_id, program_name, kernel_name, dim, global_offset, global_size, local_size, event)

\par Fortran Version:
ocl_enqueue_kernel_ndrange(device_id, program_name, kernel_name, dim, global_offset, global_size, local_size, event, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param program_name Program name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param kernel_name Kernel name hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param dim The dimension of the execution topology \n
cl_uint (\b C), integer(int32_t) (\b Fortran) \n
@param global_offset An array of size \b dim holding the global offsets \n
const size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param global_size An array of size \b dim holding the global sizes \n
const size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param local_size An array of size \b dim holding the local sizes \n
const size_t (\b C), integer(c_size_t) (\b Fortran) \n
@param event Event handle (used to collect timing information) \n
ocl_event_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine enqueues an OpenCL NDRange kernel with the execution topology defined by the input parameters: \b dim, \b global_offset, \b global_size and \b local_size.  OpenCL supports execution topologies of up to three dimensions.  The offset and size parameters are each arrays of size \b dim.  Users should query their specific hardware to determine what ranges are supported.
*/

int32_t ocl_enqueue_kernel_ndrange(uint32_t device_id,
	const char * program_name, const char * kernel_name, cl_uint dim,
	const size_t * global_offset, const size_t * global_size,
	const size_t * local_size, ocl_event_t * event);

int32_t ocl_enqueue_kernel_ndrange_token(uint32_t device_id,
	ocl_kernel_t * kernel, cl_uint dim, const size_t * global_offset,
	const size_t * global_size, const size_t * local_size, ocl_event_t * event);

/*----------------------------------------------------------------------------*\
 * Block for pending events on a queue.
\*----------------------------------------------------------------------------*/

/*!
\page ocl_finish
Block a device.

\par C Version:
ierr ocl_finish(device_id)

\par Fortran Version:
ocl_finish(device_id, ierr)

@param device_id Device instance handle \n
uint32_t (\b C), type(ocl_reference_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine causes the queue associated with the given logical device to block until all operations have completed.
*/

int32_t ocl_finish(uint32_t device_id);

#endif // ocl_interface_h
