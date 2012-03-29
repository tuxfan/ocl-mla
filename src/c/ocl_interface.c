/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#define _ocl_source

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ocl_interface.h"
#include "ocl_device.h"
#include "ocl_utils.h"

extern ocl_data_t ocl;
extern int32_t ocl_warning;

/*----------------------------------------------------------------------------*\
 * Default performance platform and device
\*----------------------------------------------------------------------------*/

#ifndef OCL_PLATFORM_PERFORMANCE
	const char * ocl_platform_performance = "default";
#else
	const char * ocl_platform_performance =
		{ DEFINE_TO_STRING(OCL_PLATFORM_PERFORMANCE) };
#endif

#ifndef OCL_PERFORMANCE_INITIALIZATION
#define OCL_PERFORMANCE_INITIALIZATION ocl_init_generic_gpu
#endif

/*----------------------------------------------------------------------------*\
 * Default auxiliary platform and device
\*----------------------------------------------------------------------------*/

#ifndef OCL_PLATFORM_AUXILIARY
	const char * ocl_platform_auxiliary = "default";
#else
	const char * ocl_platform_auxiliary =
		{ DEFINE_TO_STRING(OCL_PLATFORM_AUXILIARY) };
#endif

#ifndef OCL_AUXILIARY_INITIALIZATION
#define OCL_AUXILIARY_INITIALIZATION ocl_init_generic_cpu
#endif

/*----------------------------------------------------------------------------*\
 * ocl_init
\*----------------------------------------------------------------------------*/

int32_t ocl_init() {
	int32_t ierr = 0;

	ocl_warning = 1;

#if defined(ENABLE_OCL_VERBOSE)
	message("Initializing OpenCL layer\n\n");
#endif

	// initialize the performance device
	if(OCL_PERFORMANCE_INITIALIZATION(&ocl.devices[OCL_PERFORMANCE_DEVICE],
		ocl_platform_performance) != 0) {
		message("Initialization of performance device failed!\n");
		exit(1);
	} // if

#if defined(ENABLE_OCL_VERBOSE)
	print_device_info(&ocl.devices[OCL_PERFORMANCE_DEVICE].info,
		"Performance");
#endif

	// initialize the auxiliary device
	if(OCL_AUXILIARY_INITIALIZATION(&ocl.devices[OCL_AUXILIARY_DEVICE],
		ocl_platform_auxiliary) != 0) {
		message("Initialization of auxiliary device failed!\n");
		exit(1);
	} // if

#if defined(ENABLE_OCL_VERBOSE)
	print_device_info(&ocl.devices[OCL_AUXILIARY_DEVICE].info,
		"Auxilliary");
#endif

	// initialize the hash table
	if(ocl_hash_init() != 0) {
		message("Hash initialization failed!\n");
		exit(1);
	} // if

	// initialize garbage collection for fortran
	ocl.slots = 0;
	ocl.allocations = 0;

	return ierr;
} // ocl_init

/*----------------------------------------------------------------------------*\
 * ocl_finalize
\*----------------------------------------------------------------------------*/

int32_t ocl_finalize() {
	int32_t ierr = 0;
	size_t i;

#if defined(ENABLE_OCL_VERBOSE)
	message("Finalizing OpenCL layer\n\n");
#endif

	// shutdown all devices
	ocl_finalize_device(&ocl.devices[OCL_PERFORMANCE_DEVICE]);
	ocl_finalize_device(&ocl.devices[OCL_AUXILIARY_DEVICE]);

	// cleanup hash table
	ocl_hash_destroy();

	// garbage collection for fortran
	for(i=0; i<ocl.allocations; ++i) {
		free(ocl.free_allocations[i]);
	} // for

	return ierr;
} // ocl_finalize

/*----------------------------------------------------------------------------*\
 * ocl_initialize_event
\*----------------------------------------------------------------------------*/

int32_t ocl_initialize_event(ocl_event_t * event) {
	int32_t ierr = 0;

	event->num_events_in_wait_list = 0;
	event->event_wait_list = NULL;

	return ierr;
} // ocl_initialize_event

/*----------------------------------------------------------------------------*\
 * ocl_release_event
\*----------------------------------------------------------------------------*/

int32_t ocl_release_event(ocl_event_t * event) {
	int32_t ierr = 0;

	clReleaseEvent(event->event);

	ocl_initialize_event(event);

	return ierr;
} // ocl_release_event

/*----------------------------------------------------------------------------*\
 * ocl_initialize_event_wait_list
\*----------------------------------------------------------------------------*/

int32_t ocl_initialize_event_wait_list(ocl_event_wait_list_t * list) {
	int32_t ierr = 0;

	list->index = -1;
	list->event_wait_list = NULL;
	list->num_events_in_wait_list = 0;
	list->allocated = 0;

	return ierr;
} // ocl_initialize_event_wait_list

/*----------------------------------------------------------------------------*\
 * ocl_add_event_to_wait_list
\*----------------------------------------------------------------------------*/

int32_t ocl_add_event_to_wait_list(ocl_event_wait_list_t * list,
 	ocl_event_t * event) {
	int32_t ierr = 0;
	
	if(list->num_events_in_wait_list+1 > list->allocated) {
		
		// allocate data
		void * data = (cl_event *)malloc(
			(list->allocated + OCL_EVENT_LIST_BLOCK_SIZE)*sizeof(cl_event));

		if(data == NULL) {
			message("Memory allocation failed for event list!\n");
			exit(1);
		} // if

		if(list->index != -1) {
			if(list->event_wait_list != NULL) {
				// copy existing data to the new allocation
				memcpy(data, list->event_wait_list,
					list->allocated*sizeof(cl_event));

				// free existing allocation
				free(ocl.free_allocations[list->index]);
				ocl.free_allocations[list->index] = NULL;
			} // if

			// set new data
			ocl.free_allocations[list->index] = data;
		}
		else {
			// set up garbage collection
			if(ocl.slots > 0) {
				// use existing slot
				list->index = ocl.open_slots[--ocl.slots];
				ocl.free_allocations[list->index] = data;
			}
			else {
				list->index = ocl.allocations;
				ocl.free_allocations[ocl.allocations++] = data;
			} // if

			// set the list data
			list->event_wait_list = (cl_event *)data;
		} // if

		list->allocated += OCL_EVENT_LIST_BLOCK_SIZE;
	} // if

	list->event_wait_list[list->num_events_in_wait_list++] = event->event;

	return ierr;
} // ocl_add_event_to_wait_list

/*----------------------------------------------------------------------------*\
 * ocl_set_event_list
\*----------------------------------------------------------------------------*/

int32_t ocl_set_event_list(ocl_event_t * event, ocl_event_wait_list_t * list) {
	int32_t ierr = 0;

	ASSERT(list->event_wait_list != NULL, "Invalid event list")

	event->event_wait_list = list->event_wait_list;
	event->num_events_in_wait_list = list->num_events_in_wait_list;

	return ierr;
} // ocl_set_event_list

/*----------------------------------------------------------------------------*\
 * ocl_clear_event
\*----------------------------------------------------------------------------*/

int32_t ocl_clear_event(ocl_event_t * event) {
	int32_t ierr = 0;

	event->event_wait_list = NULL;
	event->num_events_in_wait_list = 0;

	return ierr;
} // ocl_clear_event

/*----------------------------------------------------------------------------*\
 * ocl_clear_event_wait_list
\*----------------------------------------------------------------------------*/

int32_t ocl_clear_event_wait_list(ocl_event_wait_list_t * list) {
	int32_t ierr = 0;

	// free allocated memory
	if(list->index != -1 && list->event_wait_list != NULL) {
		free(ocl.free_allocations[list->index]);
		ocl.free_allocations[list->index] = NULL;
		ocl.open_slots[ocl.slots++] = list->index;
	} // if

	ocl_initialize_event_wait_list(list);

	return ierr;
} // ocl_clear_event_wait_list

/*----------------------------------------------------------------------------*\
 * ocl_wait_for_events
\*----------------------------------------------------------------------------*/

int32_t ocl_wait_for_events(ocl_event_wait_list_t * list) {
	int32_t ierr = 0;
	CALLER_SELF

	ASSERT(list->event_wait_list != NULL, "Invalid event list")

	ierr = clWaitForEvents(list->num_events_in_wait_list,
		list->event_wait_list);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clWaitForEvents, ierr);
	} // if

	return ierr;
} // ocl_wait_for_events

/*----------------------------------------------------------------------------*\
 * ocl_create_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_create_buffer(uint32_t device_id, size_t size, cl_mem_flags flags,
	void * host_ptr, cl_mem * buffer) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	*buffer = clCreateBuffer(instance->context, flags, size, host_ptr, &ierr);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clCreateBuffer, ierr);
	} // if

	return ierr;
} // ocl_create_buffer

/*----------------------------------------------------------------------------*\
 * ocl_release_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_release_buffer(cl_mem * buffer) {
	CALLER_SELF
	int32_t ierr;
	ierr = clReleaseMemObject(*buffer);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clReleaseMemObject, ierr);
	} // if

	return ierr;
} // ocl_release_buffer

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_write_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_write_buffer(uint32_t device_id,
	cl_mem buffer, int32_t synchronous, size_t offset, size_t cb,
	void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	if(event != NULL) {
		ierr = clEnqueueWriteBuffer(instance->queue, buffer, synchronous,
			offset, cb, ptr, event->num_events_in_wait_list,
			event->event_wait_list, &event->event);
	}
	else {
		ierr = clEnqueueWriteBuffer(instance->queue, buffer, synchronous,
			offset, cb, ptr, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueWriteBuffer, ierr);
	} // if

	return ierr;
} // ocl_enqueue_write_buffer

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_read_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_read_buffer(uint32_t device_id,
	cl_mem buffer, int32_t synchronous, size_t offset, size_t cb,
	void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	if(event != NULL) {
		ierr = clEnqueueReadBuffer(instance->queue, buffer, synchronous,
			offset, cb, ptr, event->num_events_in_wait_list,
			event->event_wait_list, &event->event);
	}
	else {
		ierr = clEnqueueReadBuffer(instance->queue, buffer, synchronous,
			offset, cb, ptr, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueReadBuffer, ierr);
	} // if

	return ierr;
} // ocl_enqueue_read_buffer

/*----------------------------------------------------------------------------*\
 * ocl_add_program
\*----------------------------------------------------------------------------*/

int32_t ocl_add_program(uint32_t device_id, const char * program_name,
	const char * program_source, const char * compile_options) {
	CALLER_SELF
	cl_int err = 0;

	// try to determine if program_name is a file or a string
	char * _program_source = NULL;

	// try base program_source as a file name
	ocl_warning = 0;
	if(ocl_add_from_file(program_source, &_program_source, 0) != 0) {
		_program_source = strdup(program_source);
	} // if

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	// create the program token using the context from the instance argument
	cl_program token = clCreateProgramWithSource(instance->context, 1,
		(const char **)&_program_source, NULL, &err);

	if(err != CL_SUCCESS) {
		CL_ABORTerr(clCreateProgramWithSource, err);
	} // if

	// build the program source
	err = clBuildProgram(token, 1, &instance->id, compile_options, NULL, NULL);

	// capture failed output, print to stdout and exit
	if(err == CL_BUILD_PROGRAM_FAILURE) {
		char buffer[256*1024];
		size_t length;

		clGetProgramBuildInfo(token, instance->id,
			CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);

		message("clBuildProgram failed:\n%s\n%s\n", buffer, compile_options);
		exit(1);
	} // if

	// add program to the hash
	ocl_hash_add_program(program_name, token);

	// free memory
	free(_program_source);

	return err;
} // ocl_add_program

/*----------------------------------------------------------------------------*\
 * ocl_add_kernel
\*----------------------------------------------------------------------------*/

int32_t ocl_add_kernel(uint32_t device_id, const char * program_name,
	const char * kernel_source_name, const char * kernel_name) {
	CALLER_SELF
	ENTRY * ep = NULL;
	int32_t ierr = 0;

	// get the program hash data
	ep = ocl_hash_find_program(program_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", program_name);
		exit(1);
	} // if

	// get the program data
	ocl_program_t * program = (ocl_program_t *)ep->data;

	// create the kernel token
	ocl_kernel_t kernel;
	kernel.token = clCreateKernel(program->token, kernel_source_name, &ierr);

	if(ierr != CL_SUCCESS) {
		CL_ABORTcreateKernel(ierr, kernel_name);
	} // if

	// get kernel information
	size_t param_value_size = sizeof(size_t);
	ierr = clGetKernelWorkGroupInfo(kernel.token, ocl.devices[device_id].id,
		CL_KERNEL_WORK_GROUP_SIZE, param_value_size,
		(void *)&kernel.hint.work_group_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	param_value_size = sizeof(cl_ulong);
	ierr = clGetKernelWorkGroupInfo(kernel.token, ocl.devices[device_id].id,
		CL_KERNEL_LOCAL_MEM_SIZE, param_value_size,
		(void *)&kernel.hint.local_mem_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	if(ocl.devices[device_id].info.version_major >= 1 &&
		ocl.devices[device_id].info.version_minor >= 1) {

// dummy value to enable compilation on older OpenCL installations
#ifndef CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
#define CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE 0
#endif
		param_value_size = sizeof(size_t);
		ierr = clGetKernelWorkGroupInfo(kernel.token, ocl.devices[device_id].id,
			CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, param_value_size,
			(void *)&kernel.hint.preferred_multiple, NULL);

		if(ierr != CL_SUCCESS) {
			CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
		} // if
	}
	else {
		kernel.hint.preferred_multiple = 1;
	} // if

	// compute hint
	KERNEL_HINT_FUNCTION(&kernel.hint);

	// add kernel to hash table
	ocl_hash_add_kernel(program_name, kernel_name, kernel);

	return ierr;
} // ocl_add_kernel

/*----------------------------------------------------------------------------*\
 * ocl_set_kernel_arg
\*----------------------------------------------------------------------------*/

int32_t ocl_set_kernel_arg(const char * program_name, const char * kernel_name,
	cl_uint index, size_t size, const void * value) {
	CALLER_SELF
	ENTRY *ep = NULL;
	int32_t ierr = 0;

	// check that the program exists
	ep = ocl_hash_find_program(program_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", program_name);
		exit(1);
	} // if

	// check that the kernel exists
	ep = ocl_hash_find_kernel(program_name, kernel_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", kernel_name);
		exit(1);
	} // if

	// set the actual kernel argument
	ocl_kernel_t * kernel = (ocl_kernel_t *)ep->data;
	ierr = clSetKernelArg(kernel->token, index, size, value);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clSetKernelArg, ierr);
	} // if

	return ierr;
} // ocl_set_kernel_arg

/*----------------------------------------------------------------------------*\
 * ocl_kernel_hint
\*----------------------------------------------------------------------------*/

int32_t ocl_kernel_hint(const char * program_name,
	const char * kernel_name, size_t * hint) {
	ENTRY *ep = NULL;
	int32_t ierr = 0;
	
	// check that the program exists
	ep = ocl_hash_find_program(program_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", program_name);
		exit(1);
	} // if

	// check that the kernel exists
	ep = ocl_hash_find_kernel(program_name, kernel_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", kernel_name);
		exit(1);
	} // if

	ocl_kernel_t * kernel = (ocl_kernel_t *)ep->data;
	*hint = kernel->hint.local_size;

	return ierr;
} // ocl_kernel_hint

/*----------------------------------------------------------------------------*\
 * ocl_max_work_group_size
\*----------------------------------------------------------------------------*/

int32_t ocl_max_work_group_size(uint32_t device_id,
	size_t * max_work_group_size) {
	int32_t ierr = 0;

	*max_work_group_size = ocl.devices[device_id].info.max_work_group_size;

	return ierr;
} // ocl_max_work_group_size

/*----------------------------------------------------------------------------*\
 * ocl_ndrange_hints
\*----------------------------------------------------------------------------*/

int32_t ocl_ndrange_hints(size_t elements, size_t max_work_group_size,
	size_t * work_group_size, size_t * work_group_elements,
	size_t * single_elements) {
	int32_t ierr = 0;

	*work_group_size = max_work_group_size;

	// find nearest power of 2
	while(*work_group_size > elements) { *work_group_size /= 2; }

	div_t q = div(elements, *work_group_size);

	*work_group_elements = q.quot*(*work_group_size);
	*single_elements = q.rem;

	return ierr;
} // ocl_ndrange_hints

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_kernel_ndrange
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_kernel_ndrange(uint32_t device_id,
	const char * program_name, const char * kernel_name, cl_uint dim,
	const size_t * global_offset, const size_t * global_size,
	const size_t * local_size, ocl_event_t * event) {
	CALLER_SELF
	ENTRY *ep = NULL;
	int32_t ierr = 0;

	// check that the program exists
	ep = ocl_hash_find_program(program_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", program_name);
		exit(1);
	} // if

	// check that the kernel exists
	ep = ocl_hash_find_kernel(program_name, kernel_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", kernel_name);
		exit(1);
	} // if

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ocl_kernel_t * kernel = (ocl_kernel_t *)ep->data;

	if(event != NULL) {
		ierr = clEnqueueNDRangeKernel(instance->queue, kernel->token,
			dim, global_offset, global_size, local_size,
			event->num_events_in_wait_list, event->event_wait_list,
			&event->event);
	}
	else {
		ierr = clEnqueueNDRangeKernel(instance->queue, kernel->token,
			dim, global_offset, global_size, local_size, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueNDRangeKernel, ierr);
	} // if

	return ierr;
} // ocl_enqueue_kernel_ndrange

/*----------------------------------------------------------------------------*\
 * ocl_finish
\*----------------------------------------------------------------------------*/

int32_t ocl_finish(uint32_t device_id) {
	CALLER_SELF
	int32_t ierr = 0;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ierr = clFinish(instance->queue);
	
	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clFinish, ierr);
	} // if

	return ierr;
} // ocl_finish

/******************************************************************************\
 ******************************************************************************
 * Fortran 90
 ******************************************************************************
\******************************************************************************/

/*----------------------------------------------------------------------------*\
 * ocl_init_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_init_f90() {
	return ocl_init();
} // ocl_init_f90

/*----------------------------------------------------------------------------*\
 * ocl_finalize_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_finalize_f90() {
	return ocl_finalize();
} // ocl_finalize_f90

/*----------------------------------------------------------------------------*\
 * ocl_initialize_event_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_initialize_event_f90(ocl_allocation_t * event) {
	int32_t ierr = 0;

	ocl_event_t * _event = (ocl_event_t *)malloc(sizeof(ocl_event_t));

	event->data = (void *)_event;

	// try to use existing slots
	if(ocl.slots > 0) {
		event->index = ocl.open_slots[--ocl.slots];
		ocl.free_allocations[ocl.slots] = (void *)_event;
	}
	else {
		event->index = ocl.allocations;
		ocl.free_allocations[ocl.allocations++] = (void *)_event;
	} // if

	ierr = ocl_initialize_event(_event);

	return ierr;
} // ocl_initialize_event_f90

/*----------------------------------------------------------------------------*\
 * ocl_release_event_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_release_event_f90(ocl_allocation_t * event) {
	int32_t ierr = 0;

	ocl_release_event((ocl_event_t *)event->data);

	free(ocl.free_allocations[event->index]);
	ocl.free_allocations[event->index] = NULL;
	ocl.open_slots[ocl.slots++] = event->index;

	return ierr;
} // ocl_release_event_f90

/*----------------------------------------------------------------------------*\
 * ocl_initialize_event_wait_list_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_initialize_event_wait_list_f90(ocl_allocation_t * list) {
	int32_t ierr = 0;

	ocl_event_wait_list_t * _list =
		(ocl_event_wait_list_t *)malloc(sizeof(ocl_event_wait_list_t));

	list->data = (void *)_list;

	// try to use existing slots
	if(ocl.slots > 0) {
		list->index = ocl.open_slots[--ocl.slots];
		ocl.free_allocations[ocl.slots] = (void *)_list;
	}
	else {
		list->index = ocl.allocations;
		ocl.free_allocations[ocl.allocations++] = (void *)_list;
	} // if

	ierr = ocl_initialize_event_wait_list(_list);

	return ierr;

} // ocl_initialize_event_wait_list_f90

/*----------------------------------------------------------------------------*\
 * ocl_add_event_to_wait_list
\*----------------------------------------------------------------------------*/

int32_t ocl_add_event_to_wait_list_f90(ocl_allocation_t * list,
 	ocl_allocation_t * event) {
	return ocl_add_event_to_wait_list((ocl_event_wait_list_t *)list->data,
		(ocl_event_t *)event->data);
} // ocl_add_event_to_wait_list_f90

/*----------------------------------------------------------------------------*\
 * ocl_set_event_list
\*----------------------------------------------------------------------------*/

int32_t ocl_set_event_list_f90(ocl_allocation_t * event,
	ocl_allocation_t * list) {
	return ocl_set_event_list((ocl_event_t *)event->data,
		(ocl_event_wait_list_t *)list->data);
} // ocl_set_event_list_f90

/*----------------------------------------------------------------------------*\
 * ocl_clear_event
\*----------------------------------------------------------------------------*/

int32_t ocl_clear_event_f90(ocl_allocation_t * event) {
	return ocl_clear_event((ocl_event_t *)event->data);
} // ocl_clear_event_f90

/*----------------------------------------------------------------------------*\
 * ocl_clear_event_wait_list
\*----------------------------------------------------------------------------*/

int32_t ocl_clear_event_wait_list_f90(ocl_allocation_t * list) {
	return ocl_clear_event_wait_list((ocl_event_wait_list_t *)list->data);
} // ocl_clear_event_wait_list_f90

/*----------------------------------------------------------------------------*\
 * ocl_wait_for_events
\*----------------------------------------------------------------------------*/

int32_t ocl_wait_for_events_f90(ocl_allocation_t * list) {
	return ocl_wait_for_events((ocl_event_wait_list_t *)list->data);
} // ocl_wait_for_events_f90

/*----------------------------------------------------------------------------*\
 * ocl_mem_size_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_mem_size_f90() {
	return (int32_t)sizeof(cl_mem);
} // ocl_mem_size_f90

/*----------------------------------------------------------------------------*\
 * ocl_create_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_create_buffer_f90(uint32_t device_id, size_t size,
	cl_mem_flags flags, void * host_ptr, ocl_allocation_t * buffer) {
	CALLER_SELF
	cl_int err;

	cl_mem * _buffer = (cl_mem *)malloc(sizeof(cl_mem));

	ocl_device_instance_t * instance = ocl_device_instance(device_id);
	*_buffer = clCreateBuffer(instance->context, flags, size,
		host_ptr, &err);

	buffer->data = (void *)_buffer;

	// try to use existing slots
	if(ocl.slots > 0) {
		buffer->index = ocl.open_slots[--ocl.slots];
		ocl.free_allocations[ocl.slots] = (void *)_buffer;
	}
	else {
		buffer->index = ocl.allocations;
		ocl.free_allocations[ocl.allocations++] = (void *)_buffer;
	} // if

	if(err != CL_SUCCESS) {
		CL_ABORTerr(clCreateBuffer, err);
	} // if

	return 0;
} // ocl_create_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_release_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_release_buffer_f90(ocl_allocation_t * buffer) {
	int32_t ierr = 0;

	ocl_release_buffer((cl_mem *)buffer->data);

	free(ocl.free_allocations[buffer->index]);
	ocl.free_allocations[buffer->index] = NULL;
	ocl.open_slots[ocl.slots++] = buffer->index;

	return ierr;
} // ocl_release_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_write_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_write_buffer_f90(uint32_t device_id,
	ocl_allocation_t * buffer, int32_t synchronous, size_t offset, size_t cb,
	void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_write_buffer(device_id, *(cl_mem *)buffer->data,
		synchronous, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_write_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_read_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_read_buffer_f90(uint32_t device_id,
	ocl_allocation_t * buffer, int32_t synchronous, size_t offset, size_t cb,
	void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_read_buffer(device_id, *(cl_mem *)buffer->data,
		synchronous, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_read_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_add_program_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_add_program_f90(uint32_t device_id,
	const char * program_name, const char * program_source,
	const char * compile_options) {
	return ocl_add_program(device_id, program_name, program_source,
		compile_options);
} // ocl_add_program_f90

/*----------------------------------------------------------------------------*\
 * ocl_add_kernel_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_add_kernel_f90(uint32_t device_id, const char * program_name,
	const char * kernel_source_name, const char * kernel_name) {
	return ocl_add_kernel(device_id, program_name, kernel_source_name,
		kernel_name);
} // ocl_add_kernel_f90

/*----------------------------------------------------------------------------*\
 * ocl_set_kernel_arg_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_set_kernel_arg_f90(const char * program_name,
	const char * kernel_name, cl_uint index, size_t size, const void * value) {
	return ocl_set_kernel_arg(program_name, kernel_name, index, size, value);
} // ocl_set_kernel_arg_f90

/*----------------------------------------------------------------------------*\
 * ocl_set_kernel_arg_allocation_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_set_kernel_arg_allocation_f90(const char * program_name,
	const char * kernel_name, cl_uint index, size_t size,
	const ocl_allocation_t * value) {
	return ocl_set_kernel_arg(program_name, kernel_name, index, size,
		value->data);
} // ocl_set_kernel_arg_f90

/*----------------------------------------------------------------------------*\
 * ocl_kernel_hint_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_kernel_hint_f90(const char * program_name,
	const char * kernel_name, size_t * hint) {
	return ocl_kernel_hint(program_name, kernel_name, hint);
} // ocl_kernel_hint_f90

/*----------------------------------------------------------------------------*\
 * ocl_ndrange_hints_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_ndrange_hints_f90(const size_t * indeces,
	const size_t * max_work_group_size, size_t * work_group_size,
	size_t * work_group_indeces, size_t * single_indeces) {
	return ocl_ndrange_hints(*indeces, *max_work_group_size, work_group_size,
		work_group_indeces, single_indeces);
} // ocl_ndrange_hints_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_kernel_ndrange_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_kernel_ndrange_f90(uint32_t device_id,
	const char * program_name, const char * kernel_name, cl_uint dim,
	const size_t * global_offset, const size_t * global_size,
	const size_t * local_size, ocl_allocation_t * event) {
	return ocl_enqueue_kernel_ndrange(device_id, program_name, kernel_name,
		dim, global_offset, global_size, local_size, (ocl_event_t *)event->data);
} // ocl_enqueue_kernel_ndrange

/*----------------------------------------------------------------------------*\
 * ocl_finish_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_finish_f90(uint32_t device_id) {
	return ocl_finish(device_id);
} // ocl_finish_f90
