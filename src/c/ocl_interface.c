/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#define _ocl_source
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <float.h>

#include "ocl_interface.h"
#include "ocl_device.h"
#include "ocl_utils.h"

extern ocl_data_t ocl;
extern int32_t ocl_warning;

/*----------------------------------------------------------------------------*\
 * ocl_init
\*----------------------------------------------------------------------------*/

int32_t ocl_init() {
	return ocl_init_threaded(0);
} // ocl_init

/*----------------------------------------------------------------------------*\
 * ocl_init
\*----------------------------------------------------------------------------*/

int32_t ocl_init_threaded(size_t thread) {
	int32_t ierr = 0;

	ocl_warning = 1;

	// initialize hash tables
	if(ocl_hash_init() != 0) {
		message("Hash initialization failed!\n");
		exit(1);
	} // if

#if defined(ENABLE_OCL_VERBOSE)
	message("Initializing OpenCL layer\n\n");
#endif

	for(size_t i=0; i<OCL_DEVICE_TYPES; ++i) {
		ocl.initialized_devices[i] = 0;
	} // for

	if(ocl_init_devices(thread) != 0) {
		message("Device Initialization failed!\n");
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
	ocl_finalize_devices();

	// cleanup hash table
	ocl_hash_finalize();

	// garbage collection for fortran
	for(i=0; i<ocl.allocations; ++i) {
		free(ocl.free_allocations[i]);
	} // for

	return ierr;
} // ocl_finalize

/*----------------------------------------------------------------------------*\
 * ocl_finalize
\*----------------------------------------------------------------------------*/

int32_t ocl_get_device_instance(uint32_t device_id,
	ocl_raw_instance_t * instance) {
	int32_t ierr = 0;

	ocl_device_instance_t * _instance = ocl_device_instance(device_id);

	instance->id = _instance->id;
	instance->context = _instance->context;
	instance->queue = _instance->queue;

	return ierr;
} // ocl_get_device_instance

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

int32_t ocl_initialize_event_wait_list(ocl_event_wait_list_t * list,
	cl_event * events, size_t num_events) {
	int32_t ierr = 0;

	list->event_wait_list = events;
	list->num_events_in_wait_list = 0;
	list->allocated = 0;
	list->fixed_size = events == NULL ? 0 : num_events;
	list->index = -1;

	return ierr;
} // ocl_initialize_event_wait_list

/*----------------------------------------------------------------------------*\
 * ocl_add_event_to_wait_list
\*----------------------------------------------------------------------------*/

int32_t ocl_add_event_to_wait_list(ocl_event_wait_list_t * list,
 	ocl_event_t * event) {
	int32_t ierr = 0;

	if(list->fixed_size) {
		if(list->num_events_in_wait_list+1 > list->fixed_size) {
			message("Number of events exceeds list size!!!\n");
			exit(1);
		} // if
	}
	else {
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
				// set the list data
				list->event_wait_list = (cl_event *)data;
				add_allocation(data, &list->index);
			} // if

			list->allocated += OCL_EVENT_LIST_BLOCK_SIZE;
		} // if
	} // if

	list->event_wait_list[list->num_events_in_wait_list++] = event->event;

	return ierr;
} // ocl_add_event_to_wait_list

/*----------------------------------------------------------------------------*\
 * ocl_set_event_list
\*----------------------------------------------------------------------------*/

int32_t ocl_set_event_list(ocl_event_t * event, ocl_event_wait_list_t * list) {
	int32_t ierr = 0;

	ASSERT(list->event_wait_list != NULL, "Invalid event list!")

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
	if(list->fixed_size == 0 && list->index != -1 &&
		list->event_wait_list != NULL) {
		free(ocl.free_allocations[list->index]);
		list->event_wait_list = NULL;
		ocl.free_allocations[list->index] = NULL;
		ocl.open_slots[ocl.slots++] = list->index;
	} // if

	ocl_initialize_event_wait_list(list, list->event_wait_list,
		list->fixed_size);

	return ierr;
} // ocl_clear_event_wait_list

/*----------------------------------------------------------------------------*\
 * ocl_wait_for_events
\*----------------------------------------------------------------------------*/

int32_t ocl_wait_for_events(ocl_event_wait_list_t * list) {
	int32_t ierr = 0;
	CALLER_SELF

	ASSERT(list->event_wait_list != NULL, "Invalid event list!")

	ierr = clWaitForEvents(list->num_events_in_wait_list,
		list->event_wait_list);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clWaitForEvents, ierr);
	} // if

	return ierr;
} // ocl_wait_for_events

/*----------------------------------------------------------------------------*\
 * ocl_create_buffer_raw
\*----------------------------------------------------------------------------*/

int32_t ocl_create_buffer_raw(uint32_t device_id, size_t size,
	cl_mem_flags flags, void * host_ptr, cl_mem * buffer) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	*buffer = clCreateBuffer(instance->context, flags, size, host_ptr, &ierr);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clCreateBuffer, ierr);
	} // if

	return ierr;
} // ocl_create_buffer_raw

/*----------------------------------------------------------------------------*\
 * ocl_create_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_create_buffer(uint32_t device_id, const char * buffer_name,
	size_t size, cl_mem_flags flags, void * host_ptr) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	cl_mem token = clCreateBuffer(instance->context, flags, size,
		host_ptr, &ierr);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clCreateBuffer, ierr);
	} // if

	ocl_hash_add_buffer(device_id, buffer_name, token);

	return ierr;
} // ocl_create_buffer

/*----------------------------------------------------------------------------*\
 * ocl_buffer_type_size
\*----------------------------------------------------------------------------*/

size_t ocl_buffer_type_size() {
	return sizeof(cl_mem);
} // ocl_buffer_size

/*----------------------------------------------------------------------------*\
 * ocl_buffer_reference
\*----------------------------------------------------------------------------*/

cl_mem * ocl_buffer_reference(uint32_t device_id, const char * buffer_name) {
	ocl_buffer_t * _buffer = ocl_hash_find_buffer(device_id, buffer_name);
	return &_buffer->token;
} // ocl_buffer_reference

/*----------------------------------------------------------------------------*\
 * ocl_release_buffer_raw
\*----------------------------------------------------------------------------*/

int32_t ocl_release_buffer_raw(cl_mem * buffer) {
	CALLER_SELF
	int32_t ierr;
	ierr = clReleaseMemObject(*buffer);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clReleaseMemObject, ierr);
	} // if

	return ierr;
} // ocl_release_buffer_raw

/*----------------------------------------------------------------------------*\
 * ocl_release_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_release_buffer(uint32_t device_id, const char * buffer_name) {
	CALLER_SELF
	int32_t ierr;

	ocl_buffer_t * _buffer = ocl_hash_find_buffer(device_id, buffer_name);

	ierr = clReleaseMemObject(_buffer->token);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clReleaseMemObject, ierr);
	} // if

	return ierr;
} // ocl_release_buffer

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_write_buffer_raw
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_write_buffer_raw(uint32_t device_id,
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
} // ocl_enqueue_write_buffer_raw

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_write_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_write_buffer(uint32_t device_id,
	const char * buffer_name, int32_t synchronous, size_t offset,
	size_t cb, void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ocl_buffer_t * _buffer = ocl_hash_find_buffer(device_id, buffer_name);

	if(event != NULL) {
		ierr = clEnqueueWriteBuffer(instance->queue, _buffer->token,
			synchronous, offset, cb, ptr, event->num_events_in_wait_list,
			event->event_wait_list, &event->event);
	}
	else {
		ierr = clEnqueueWriteBuffer(instance->queue, _buffer->token,
			synchronous, offset, cb, ptr, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueWriteBuffer, ierr);
	} // if

	return ierr;
} // ocl_enqueue_write_buffer

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_read_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_read_buffer_raw(uint32_t device_id,
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
} // ocl_enqueue_read_buffer_raw

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_read_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_read_buffer(uint32_t device_id,
	const char * buffer_name, int32_t synchronous, size_t offset,
	size_t cb, void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ocl_buffer_t * _buffer = ocl_hash_find_buffer(device_id, buffer_name);

	if(event != NULL) {
		ierr = clEnqueueReadBuffer(instance->queue, _buffer->token,
			synchronous, offset, cb, ptr, event->num_events_in_wait_list,
			event->event_wait_list, &event->event);
	}
	else {
		ierr = clEnqueueReadBuffer(instance->queue, _buffer->token,
			synchronous, offset, cb, ptr, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueReadBuffer, ierr);
	} // if

	return ierr;
} // ocl_enqueue_read_buffer

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_map_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_map_buffer_raw(uint32_t device_id, cl_mem buffer,
	int32_t synchronous, cl_mem_flags flags, size_t offset, size_t cb,
	void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ASSERT(!((synchronous == 0) && (event == NULL)),
		"Asynchronous map operations require a non-NULL event!");

	if(event != NULL) {
		ptr = clEnqueueMapBuffer(instance->queue, buffer, synchronous, flags,
			offset, cb, event->num_events_in_wait_list, event->event_wait_list,
			&event->event, &ierr);
	}
	else {
		ptr = clEnqueueMapBuffer(instance->queue, buffer, synchronous, flags,
			offset, cb, 0, NULL, NULL, &ierr);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueMapBuffer, ierr);
	} // if

	return ierr;
} // ocl_enqueue_map_buffer_raw

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_map_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_map_buffer(uint32_t device_id,
	const char * buffer_name, int32_t synchronous, cl_mem_flags flags,
	size_t offset, size_t cb, void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ocl_buffer_t * _buffer = ocl_hash_find_buffer(device_id, buffer_name);

	ASSERT(!((synchronous == 0) && (event == NULL)),
		"Asynchronous map operations require a non-NULL event!");

	if(event != NULL) {
		ptr = clEnqueueMapBuffer(instance->queue, _buffer->token,
			synchronous, flags, offset, cb, event->num_events_in_wait_list,
			event->event_wait_list, &event->event, &ierr);
	}
	else {
		ptr = clEnqueueMapBuffer(instance->queue, _buffer->token,
			synchronous, flags, offset, cb, 0, NULL, NULL, &ierr);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueMapBuffer, ierr);
	} // if

	return ierr;
} // ocl_enqueue_map_buffer

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_unmap_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_unmap_buffer_raw(uint32_t device_id, cl_mem buffer,
	void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	if(event != NULL) {
		ierr = clEnqueueUnmapMemObject(instance->queue, buffer, ptr,
			event->num_events_in_wait_list, event->event_wait_list,
			&event->event);
	}
	else {
		ierr = clEnqueueUnmapMemObject(instance->queue, buffer, ptr,
			0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueUnmapMemObject, ierr);
	} // if

	return ierr;
} // ocl_enqueue_unmap_buffer_raw

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_unmap_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_unmap_buffer(uint32_t device_id,
	const char * buffer_name, void * ptr, ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

	ocl_buffer_t * _buffer = ocl_hash_find_buffer(device_id, buffer_name);

	if(event != NULL) {
		ierr = clEnqueueUnmapMemObject(instance->queue, _buffer->token,
			ptr, event->num_events_in_wait_list, event->event_wait_list,
			&event->event);
	}
	else {
		ierr = clEnqueueUnmapMemObject(instance->queue, _buffer->token,
			ptr, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueUnmapMemObject, ierr);
	} // if

	return ierr;
} // ocl_enqueue_unmap_buffer

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

	// add ocl preprocessor defines
	char _compile_options[OCL_MAX_SOURCE_LENGTH];

	if(compile_options != NULL && strlen(compile_options) > 0) {
		sprintf(_compile_options, "%s %s", instance->info.platform_defines,
			compile_options);
	}
	else {
		sprintf(_compile_options, "%s", instance->info.platform_defines);
	} // if

	// build the program source
	err = clBuildProgram(token, 1, &instance->id, _compile_options, NULL, NULL);

	// capture compilation output
	char buffer[256*1024];
	size_t length;

	cl_uint berr = clGetProgramBuildInfo(token, instance->id,
		CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);

	if(berr != CL_SUCCESS) {
		CL_ABORTerr(clGetProgramBuildInfo, err);
	} // if

#if defined(ENABLE_OCL_COMPILER_LOG)
	FILE * log = fopen("ocl_compile.log", "w");	

	if(log == NULL) {
		message("Failed opening ocl_compile.log!\n");
		exit(1);
	} // if

	fprintf(log, "clBuildProgram Output\nCompile Options: %s\n%s\n",
		_compile_options, buffer);

	fclose(log);
#endif

	if(err == CL_BUILD_PROGRAM_FAILURE) {
		message("clBuildProgram failed:\n%s\n%s\n", buffer, _compile_options);
		exit(1);
	} // if

	// add program to the hash
	ocl_hash_add_program(program_name, device_id, token);

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
	int32_t ierr = 0;

	// retrieve program
	ocl_program_t * _program = ocl_hash_find_program(program_name);

	// create the kernel token
	ocl_kernel_t kernel;
	kernel.token = clCreateKernel(_program->token, kernel_source_name, &ierr);

	if(ierr != CL_SUCCESS) {
		CL_ABORTcreateKernel(ierr, kernel_name);
	} // if

#if 0
	// get kernel information
	size_t param_value_size = sizeof(size_t);
	ierr = clGetKernelWorkGroupInfo(kernel.token, ocl.devices[device_id].id,
		CL_KERNEL_WORK_GROUP_SIZE, param_value_size,
		(void *)&kernel.info.work_group_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	param_value_size = sizeof(cl_ulong);
	ierr = clGetKernelWorkGroupInfo(kernel.token, ocl.devices[device_id].id,
		CL_KERNEL_LOCAL_MEM_SIZE, param_value_size,
		(void *)&kernel.info.local_mem_size, NULL);

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
			(void *)&kernel.info.preferred_multiple, NULL);

		if(ierr != CL_SUCCESS) {
			CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
		} // if
	}
	else {
		kernel.info.preferred_multiple = 1;
	} // if
#endif

	// add kernel to hash table
	ocl_hash_add_kernel(program_name, kernel_name, kernel);

	return ierr;
} // ocl_add_kernel

int32_t ocl_kernel_work_group_info(uint32_t device_id,
	const char * program_name, const char * kernel_name,
	ocl_kernel_work_group_info_t * info) {
	CALLER_SELF
	int32_t ierr = 0;
	size_t param_value_size;

	// get requested device
	ocl_device_instance_t * _instance = ocl_device_instance(device_id);

	// get requested kernel
	ocl_kernel_t * _kernel = ocl_hash_find_kernel(program_name, kernel_name);

#if 0
	// VERSION 1.2
	// maximum global work size that can be enqueued for this kernel
	param_value_size = 3*sizeof(size_t);
	ierr = clGetKernelWorkGroupInfo(_kernel->token, _instance->id,
		CL_KERNEL_GLOBAL_WORK_SIZE, param_value_size,
		(void *)&info->global_work_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if
#else
	info->global_work_size[0] = 0;
	info->global_work_size[1] = 0;
	info->global_work_size[2] = 0;
#endif

	// maximum work group size that can be used for this kernel
	param_value_size = sizeof(size_t);
	ierr = clGetKernelWorkGroupInfo(_kernel->token, _instance->id,
		CL_KERNEL_WORK_GROUP_SIZE, param_value_size,
		(void *)&info->work_group_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	// compile-time specificed work group size
	param_value_size = 3*sizeof(size_t);
	ierr = clGetKernelWorkGroupInfo(_kernel->token, _instance->id,
		CL_KERNEL_COMPILE_WORK_GROUP_SIZE, param_value_size,
		(void *)&info->compile_work_group_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	// local memory used by this kernel
	param_value_size = sizeof(cl_ulong);
	ierr = clGetKernelWorkGroupInfo(_kernel->token, _instance->id,
		CL_KERNEL_LOCAL_MEM_SIZE, param_value_size,
		(void *)&info->local_mem_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	// preferred work group size multiple for this kernel
	if(ocl.devices[device_id].info.version_major >= 1 &&
		ocl.devices[device_id].info.version_minor >= 1) {

// dummy value to enable compilation on older OpenCL installations
#ifndef CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
#define CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE 0
#endif
		param_value_size = sizeof(size_t);
		ierr = clGetKernelWorkGroupInfo(_kernel->token, _instance->id,
			CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, param_value_size,
			(void *)&info->preferred_multiple, NULL);

		if(ierr != CL_SUCCESS) {
			CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
		} // if
	}
	else {
		info->preferred_multiple = 1;
	} // if

	// private memory used by this kernel
	param_value_size = sizeof(cl_ulong);
	ierr = clGetKernelWorkGroupInfo(_kernel->token, _instance->id,
		CL_KERNEL_PRIVATE_MEM_SIZE, param_value_size,
		(void *)&info->private_mem_size, NULL);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clGetKernelWorkGroupInfo, ierr);
	} // if

	return ierr;
} // ocl_kernel_work_group_info

/*----------------------------------------------------------------------------*\
 * ocl_kernel_token
\*----------------------------------------------------------------------------*/

int32_t ocl_kernel_token(const char * program_name, const char * kernel_name,
	ocl_kernel_t * token) {
	int32_t ierr = 0;

	// retrieve kernel
	*token = *(ocl_hash_find_kernel(program_name, kernel_name));

	if(token == NULL) {
		message("Error: NULL kernel token!\n", kernel_name);
		exit(1);
	} // if

	return ierr;
} // ocl_kernel_token

/*----------------------------------------------------------------------------*\
 * ocl_set_kernel_arg
\*----------------------------------------------------------------------------*/

int32_t ocl_set_kernel_arg(const char * program_name, const char * kernel_name,
	cl_uint index, size_t size, const void * value) {
	CALLER_SELF
	int32_t ierr = 0;

	// retrieve kernel
	ocl_kernel_t * _kernel = ocl_hash_find_kernel(program_name, kernel_name);

	ierr = clSetKernelArg(_kernel->token, index, size, value);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clSetKernelArg, ierr);
	} // if

	return ierr;
} // ocl_set_kernel_arg

/*----------------------------------------------------------------------------*\
 * ocl_set_kernel_arg_buffer
\*----------------------------------------------------------------------------*/

int32_t ocl_set_kernel_arg_buffer(const char * program_name,
	const char * kernel_name, const char * buffer_name, cl_uint index) {
	CALLER_SELF
	int32_t ierr = 0;

	// retrieve program
	ocl_program_t * _program = ocl_hash_find_program(program_name);

	// retrieve kernel
	ocl_kernel_t * _kernel = ocl_hash_find_kernel(program_name, kernel_name);

	// retrieve buffer
	ocl_buffer_t * _buffer = ocl_hash_find_buffer(_program->device_id,
		buffer_name);

	ierr = clSetKernelArg(_kernel->token, index, sizeof(cl_mem),
		&_buffer->token);

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clSetKernelArg, ierr);
	} // if

	return ierr;
} // ocl_set_kernel_arg_buffer

/*----------------------------------------------------------------------------*\
 * ocl_kernel_hints
\*----------------------------------------------------------------------------*/

int32_t ocl_kernel_hints(uint32_t device_id, const char * program_name,
	const char * kernel_name, ocl_kernel_hints_t * hints) {
	int32_t ierr = 0;

	ocl_kernel_work_group_info_t _info;

	ocl_kernel_work_group_info(device_id, program_name, kernel_name, &_info);
	
#if 0
	// retrieve kernel
	ocl_kernel_t * _kernel = ocl_hash_find_kernel(program_name, kernel_name);
#endif

	// compute hint
	KERNEL_HINT_FUNCTION(&_info, hints);

	hints->local_mem_size = _info.local_mem_size;

	return ierr;
} // ocl_kernel_hints

int32_t ocl_device_info(uint32_t device_id, ocl_device_info_t * info) {
	int32_t ierr = 0;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);
	*info = instance->info;

	return ierr;
} // ocl_device_info

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
	double work_group_weight, double single_element_weight,
	size_t * work_group_size, size_t * work_group_elements,
	size_t * single_elements) {
	int32_t ierr = 0;
	int i;

	// find nearest power of 2
	*work_group_size = max_work_group_size;
	while(*work_group_size > elements) { *work_group_size /= 2; }

	size_t tests = 1;
	size_t factor = 2;

	while((*work_group_size)/factor >= OCL_MIN_WORK_GROUP_SIZE) {
		++tests;
		factor *= 2;
	} // while

	double wg_ratio_max = -DBL_MAX;
	double wg_ratio_min = DBL_MAX;
	double s_ratio_max = -DBL_MAX;
	double s_ratio_min = DBL_MAX;

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define SIZE(m, s, r, rl, wgs, sz) \
	(m) == (s) ? (r) > (rl) ? (wgs) : (wgs)*2 : (sz)

	size_t wgsize = *work_group_size;
	for(i = 0; i<tests; ++i) {
		wg_ratio_max = MAX(wg_ratio_max, wgsize/(double)elements);
		wg_ratio_min = MIN(wg_ratio_min, wgsize/(double)elements);

		div_t d = div(elements, wgsize);
		const double s_ratio = (d.quot*wgsize)/(double)elements;
		s_ratio_max = MAX(s_ratio_max, s_ratio);
		s_ratio_min = MIN(s_ratio_min, s_ratio);

		wgsize /= 2;
	} // for

	// scale values into [0,1] range
	wg_ratio_max -= wg_ratio_max == wg_ratio_min ? 0.0 : wg_ratio_min;
	s_ratio_max -= s_ratio_max == s_ratio_min ? 0.0 : s_ratio_min;

	double score = 0.0;
	size_t size = 0;
#if OCL_PREFER_LARGE_WORK_GROUP_SIZE == 1
	double wg_ratio_last = 0.0;
#else
	double s_ratio_last = 0.0;
#endif

#if defined(ENABLE_OCL_VERBOSE)
	message("NDRange Hints\n");
#endif

	wgsize = *work_group_size;
	for(i = 0; i<tests; ++i) {
		const double wg_ratio = ((wgsize/(double)elements)-wg_ratio_min)/
			wg_ratio_max;

		div_t d = div(elements, wgsize);
		const double s_ratio = (((d.quot*wgsize)/(double)elements)-s_ratio_min)/
			s_ratio_max;

#if defined(ENABLE_OCL_VERBOSE)
		message("\ttrying %d\n", (int)wgsize);
		message("\twg_ratio: %lf s_ratio: %lf\n", wg_ratio, s_ratio);
#endif

		const double mean =
			(work_group_weight*wg_ratio + single_element_weight*s_ratio)/
			(work_group_weight + single_element_weight);

#if defined(ENABLE_OCL_VERBOSE)
		message("\tscore: %lf\n", (int)wgsize, mean);
		message("\twork group elements: %d\n", d.quot*wgsize);
		message("\tsingle elements: %d\n\n", d.rem);
#endif

		score = MAX(mean, score);

		// FIXME: This is probably broken for the case that two sizes
		// have the same score but are seperated by one that has
		// a different score.  In that case, the most recent high score
		// would be selected.
#if OCL_PREFER_LARGE_WORK_GROUP_SIZE == 1
		size = i > 0 ? SIZE(mean, score, wg_ratio, wg_ratio_last, wgsize, size) :
			wgsize;
		wg_ratio_last = wg_ratio;
#else
		size = i > 0 ? SIZE(mean, score, s_ratio, s_ratio_last, wgsize, size) :
			wgsize;
		s_ratio_last = s_ratio;
#endif

		wgsize /= 2;
	} // for

#undef MAX
#undef MIN
#undef SIZE

#if defined(ENABLE_OCL_VERBOSE)
		message("\tpicked %d\n", (int)size);
#endif

	*work_group_size = size;

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
	int32_t ierr = 0;

	// retrieve device instance
	ocl_device_instance_t * _instance = ocl_device_instance(device_id);

	// retrieve kernel
	ocl_kernel_t * _kernel = ocl_hash_find_kernel(program_name, kernel_name);

	if(event != NULL) {
		ierr = clEnqueueNDRangeKernel(_instance->queue, _kernel->token,
			dim, global_offset, global_size, local_size,
			event->num_events_in_wait_list, event->event_wait_list,
			&event->event);
	}
	else {
		ierr = clEnqueueNDRangeKernel(_instance->queue, _kernel->token,
			dim, global_offset, global_size, local_size, 0, NULL, NULL);
	} // if

	if(ierr != CL_SUCCESS) {
		CL_ABORTerr(clEnqueueNDRangeKernel, ierr);
	} // if

	return ierr;
} // ocl_enqueue_kernel_ndrange

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_kernel_ndrange_token
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_kernel_ndrange_token(uint32_t device_id,
	ocl_kernel_t * kernel, cl_uint dim, const size_t * global_offset,
	const size_t * global_size, const size_t * local_size,
	ocl_event_t * event) {
	CALLER_SELF
	int32_t ierr = 0;

	ocl_device_instance_t * instance = ocl_device_instance(device_id);

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
} // ocl_enqueue_kernel_ndrange_token

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

int32_t ocl_get_device_instance_f90(uint32_t device_id,
	ocl_allocation_t * instance) {
	int32_t ierr = 0;

	ocl_raw_instance_t * _instance =
		(ocl_raw_instance_t *)malloc(sizeof(ocl_raw_instance_t));

	instance->data = (void *)_instance;

	add_allocation((void *)_instance, &instance->index);

	ierr = ocl_get_device_instance(device_id, _instance);

	return ierr;
} // ocl_get_device_instance_f90

/*----------------------------------------------------------------------------*\
 * ocl_initialize_event_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_initialize_event_f90(ocl_allocation_t * event) {
	int32_t ierr = 0;

	ocl_event_t * _event = (ocl_event_t *)malloc(sizeof(ocl_event_t));

	event->data = (void *)_event;

	add_allocation((void *)_event, &event->index);

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

	add_allocation((void *)_list, &list->index);

	ierr = ocl_initialize_event_wait_list(_list, NULL, 0);

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
 * ocl_create_buffer_raw_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_create_buffer_raw_f90(uint32_t device_id, size_t size,
	cl_mem_flags flags, void * host_ptr, ocl_allocation_t * buffer) {
	int32_t ierr;

	cl_mem * _buffer = (cl_mem *)malloc(sizeof(cl_mem));

	ierr = ocl_create_buffer_raw(device_id, size, flags, host_ptr, _buffer);

	buffer->data = (void *)_buffer;

	add_allocation((void *)_buffer, &buffer->index);

	return ierr;
} // ocl_create_buffer_raw_f90

/*----------------------------------------------------------------------------*\
 * ocl_create_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_create_buffer_f90(uint32_t device_id, const char * buffer_name,
	size_t size, cl_mem_flags flags, void * host_ptr) {
	return ocl_create_buffer(device_id, buffer_name, size, flags, host_ptr);
} // ocl_create_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_buffer_reference_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_buffer_reference_f90(uint32_t device_id, const char * buffer_name,
	ocl_reference_t * reference) {
	int32_t ierr = 0;

	reference->data = ocl_buffer_reference(device_id, buffer_name);	

	return ierr;
} // ocl_buffer_reference_f90

/*----------------------------------------------------------------------------*\
 * ocl_release_buffer_raw_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_release_buffer_raw_f90(ocl_allocation_t * buffer) {
	int32_t ierr = 0;

	ocl_release_buffer_raw((cl_mem *)buffer->data);

	free(ocl.free_allocations[buffer->index]);
	ocl.free_allocations[buffer->index] = NULL;
	ocl.open_slots[ocl.slots++] = buffer->index;

	return ierr;
} // ocl_release_buffer_raw_f90

/*----------------------------------------------------------------------------*\
 * ocl_release_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_release_buffer_f90(uint32_t device_id, const char * buffer_name) {
	return ocl_release_buffer(device_id, buffer_name);
} // ocl_release_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_write_buffer_raw_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_write_buffer_raw_f90(uint32_t device_id,
	ocl_allocation_t * buffer, int32_t synchronous, size_t offset, size_t cb,
	void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_write_buffer_raw(device_id, *(cl_mem *)buffer->data,
		synchronous, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_write_buffer_raw_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_write_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_write_buffer_f90(uint32_t device_id,
	const char * buffer_name, int32_t synchronous, size_t offset,
	size_t cb, void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_write_buffer(device_id, buffer_name,
		synchronous, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_write_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_read_buffer_raw_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_read_buffer_raw_f90(uint32_t device_id,
	ocl_allocation_t * buffer, int32_t synchronous, size_t offset, size_t cb,
	void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_read_buffer_raw(device_id, *(cl_mem *)buffer->data,
		synchronous, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_read_buffer_raw_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_read_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_read_buffer_f90(uint32_t device_id,
	const char * buffer_name, int32_t synchronous, size_t offset,
	size_t cb, void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_read_buffer(device_id, buffer_name,
		synchronous, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_read_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_map_buffer_raw_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_map_buffer_raw_f90(uint32_t device_id,
	ocl_allocation_t * buffer, int32_t synchronous, cl_mem_flags flags,
	size_t offset, size_t cb, void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_map_buffer_raw(device_id, *(cl_mem *)buffer->data,
		synchronous, flags, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_map_buffer_raw_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_map_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_map_buffer_f90(uint32_t device_id,
	const char * buffer_name, int32_t synchronous, cl_mem_flags flags,
	size_t offset, size_t cb, void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_map_buffer(device_id, buffer_name, synchronous,
		flags, offset, cb, ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_map_buffer_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_unmap_buffer_raw_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_unmap_buffer_raw_f90(uint32_t device_id,
	ocl_allocation_t * buffer, void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_unmap_buffer_raw(device_id, *(cl_mem *)buffer->data,
		ptr, (ocl_event_t *)event->data);
} // ocl_enqueue_unmap_buffer_raw_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_unmap_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_unmap_buffer_f90(uint32_t device_id,
	const char * buffer_name, void * ptr, ocl_allocation_t * event) {
	return ocl_enqueue_unmap_buffer(device_id, buffer_name, ptr,
		(ocl_event_t *)event->data);
} // ocl_enqueue_unmap_buffer_f90

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
 * ocl_set_kernel_arg_buffer_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_set_kernel_arg_buffer_f90(const char * program_name,
	const char * kernel_name, const char * buffer_name, cl_uint index) {
	return ocl_set_kernel_arg_buffer(program_name, kernel_name,
		buffer_name, index);
} // ocl_set_kernel_arg_buffer_f90

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

int32_t ocl_kernel_hints_f90(uint32_t device_id, const char * program_name,
	const char * kernel_name, ocl_kernel_hints_t * hints) {
	return ocl_kernel_hints(device_id, program_name, kernel_name, hints);
} // ocl_kernel_hint_f90

/*----------------------------------------------------------------------------*\
 * ocl_ndrange_hints_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_ndrange_hints_f90(const size_t * indeces,
	const size_t * max_work_group_size, double * work_group_weight,
	double * single_element_weight, size_t * work_group_size,
	size_t * work_group_indeces, size_t * single_indeces) {
	return ocl_ndrange_hints(*indeces, *max_work_group_size,
		*work_group_weight, *single_element_weight, work_group_size,
		work_group_indeces, single_indeces);
} // ocl_ndrange_hints_f90

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_kernel_ndrange_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_kernel_ndrange_f90(uint32_t device_id,
	const char * program_name, const char * kernel_name, cl_uint dim,
	const size_t * global_offset, const size_t * global_size,
	const size_t * local_size, ocl_allocation_t * event) {
	return ocl_enqueue_kernel_ndrange(device_id, program_name,
		kernel_name, dim, global_offset, global_size, local_size,
		(ocl_event_t *)event->data);
} // ocl_enqueue_kernel_ndrange

/*----------------------------------------------------------------------------*\
 * ocl_initialize_kernel_token_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_initialize_kernel_token_f90(ocl_allocation_t * token) {
	int32_t ierr = 0;

	ocl_kernel_t * _kernel = (ocl_kernel_t *)malloc(sizeof(ocl_kernel_t));

	token->data = (void *)_kernel;

	add_allocation((void *)_kernel, &token->index);

	return ierr;
} // ocl_initialize_kernel_token_f90

/*----------------------------------------------------------------------------*\
 * ocl_kernel_token_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_kernel_token_f90(const char * program_name,
	const char * kernel_name, ocl_allocation_t * token) {

	return ocl_kernel_token(program_name, kernel_name,
		(ocl_kernel_t *)token->data);
} // ocl_kernel_token

/*----------------------------------------------------------------------------*\
 * ocl_enqueue_kernel_ndrange_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_enqueue_kernel_ndrange_token_f90(uint32_t device_id,
	ocl_allocation_t * kernel, cl_uint dim, const size_t * global_offset,
	const size_t * global_size, const size_t * local_size,
	ocl_allocation_t * event) {
	return ocl_enqueue_kernel_ndrange_token(device_id,
		(ocl_kernel_t *)kernel->data, dim, global_offset, global_size,
		local_size, (ocl_event_t *)event->data);
} // ocl_enqueue_kernel_ndrange_token

/*----------------------------------------------------------------------------*\
 * ocl_finish_f90
\*----------------------------------------------------------------------------*/

int32_t ocl_finish_f90(uint32_t device_id) {
	return ocl_finish(device_id);
} // ocl_finish_f90
