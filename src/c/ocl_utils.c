/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#define _ocl_source
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "ocl_interface.h"
#include "ocl_utils.h"

extern ocl_data_t ocl;
extern int32_t ocl_warning;

/*----------------------------------------------------------------------------*
 * I/O
 *----------------------------------------------------------------------------*/

void warning(const char * format, ...) {
// only output warnings if we're doing ENABLE_OCL_VERBOSE
#if defined(ENABLE_OCL_VERBOSE)
	// Note: ocl_warnings allow internal use of functions
	// without issueing a warning.
	if(ocl_warning) {
		va_list args;
		va_start(args, format);

		fprintf(stderr, "Warning: ");
		vfprintf(stdout, format, args);
	} // if

	// reset
	ocl_warning = 1;
#endif
} // warning

/*----------------------------------------------------------------------------*
 * OpenCL error codes
 *----------------------------------------------------------------------------*/

const char * ocl_error_codes[65] = {
	"CL_SUCCESS",
	"CL_DEVICE_NOT_FOUND",
	"CL_DEVICE_NOT_AVAILABLE",
	"CL_COMPILER_NOT_AVAILABLE",
	"CL_MEM_OBJECT_ALLOCATION_FAILURE",
	"CL_OUT_OF_RESOURCES",
	"CL_OUT_OF_HOST_MEMORY",
	"CL_PROFILING_INFO_NOT_AVAILABLE",
	"CL_MEM_COPY_OVERLAP",
	"CL_IMAGE_FORMAT_MISMATCH",
	"CL_IMAGE_FORMAT_NOT_SUPPORTED",
	"CL_BUILD_PROGRAM_FAILURE",
	"CL_MAP_FAILURE",
	"CL_MISALIGNED_SUB_BUFFER_OFFSET",
	"CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
	"", "", "", "", "", // Gap from 15 - 29
	"", "", "", "", "",
	"", "", "", "", "",
	"CL_INVALID_VALUE", // Index 30
	"CL_INVALID_DEVICE_TYPE",
	"CL_INVALID_PLATFORM",
	"CL_INVALID_DEVICE",
	"CL_INVALID_CONTEXT",
	"CL_INVALID_QUEUE_PROPERTIES",
	"CL_INVALID_COMMAND_QUEUE",
	"CL_INVALID_HOST_PTR",
	"CL_INVALID_MEM_OBJECT",
	"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
	"CL_INVALID_IMAGE_SIZE",
	"CL_INVALID_SAMPLER",
	"CL_INVALID_BINARY",
	"CL_INVALID_BUILD_OPTIONS",
	"CL_INVALID_PROGRAM",
	"CL_INVALID_PROGRAM_EXECUTABLE",
	"CL_INVALID_KERNEL_NAME",
	"CL_INVALID_KERNEL_DEFINITION",
	"CL_INVALID_KERNEL",
	"CL_INVALID_ARG_INDEX",
	"CL_INVALID_ARG_VALUE",
	"CL_INVALID_ARG_SIZE",
	"CL_INVALID_KERNEL_ARGS",
	"CL_INVALID_WORK_DIMENSION",
	"CL_INVALID_WORK_GROUP_SIZE",
	"CL_INVALID_WORK_ITEM_SIZE",
	"CL_INVALID_GLOBAL_OFFSET",
	"CL_INVALID_EVENT_WAIT_LIST",
	"CL_INVALID_EVENT",
	"CL_INVALID_OPERATION",
	"CL_INVALID_GL_OBJECT",
	"CL_INVALID_BUFFER_SIZE",
	"CL_INVALID_MIP_LEVEL",
	"CL_INVALID_GLOBAL_WORK_SIZE",
	"CL_INVALID_PROPERTY"
};

const char * error_to_string(int err) {
	err = -err;
	ASSERT(err>=0, "Invalid error code!");
	return ocl_error_codes[err];
}

/*----------------------------------------------------------------------------*
 * print_device_info
 *----------------------------------------------------------------------------*/

void print_device_info(ocl_device_info_t * info, const char * label) {
	size_t i;

	message("Device Info (%s)\n", label);

	message("\tName: %s\n", info->name);
	message("\tPlatform Name: %s\n", info->platform_name);
	message("\tVendor ID: %u\n", info->vendor_id);

	switch(info->type) {
		case CL_DEVICE_TYPE_CPU:
			message("\tType: CPU\n");
			break;
		case CL_DEVICE_TYPE_GPU:
			message("\tType: GPU\n");
			break;
		default:
			message("\tType: Unknown\n");
			break;
	} // switch

	message("\tMax Compute Units: %u\n",
		info->max_compute_units);

	message("\tMax Clock Frequency: %u\n",
		info->max_clock_frequency);

	message("\tMax Work Group Size: %lu\n",
		info->max_work_group_size);

	message("\tMax Work-Item Dimensions: %u\n",
		info->max_work_item_dimensions);

	for(i=0; i<info->max_work_item_dimensions; ++i) {
		message("\t\tDimension %lu: %lu\n",
			i, info->max_work_item_sizes[i]);
	} // for

	message("\tLocal Memory Size: %" CL_ULONG_FMT "\n",
		info->local_mem_size);

	message("\tDevice Extensions: \n");
	char * token = strtok(info->device_extensions, " ");

	while(token != NULL) {
		message("\t\t%s\n", token);
		token = strtok(NULL, " ");
	} // while

	message("\n");

} // print_device_info

/*----------------------------------------------------------------------------*
 * Add to free allocations.
 *----------------------------------------------------------------------------*/

void add_allocation(void * data, int32_t * index) {
	ASSERT(ocl.allocations+1 <= OCL_MAX_ALLOCATIONS,
		"Maximum allocations exceeded!!!");

	int32_t _index = ocl.slots > 0 ? ocl.open_slots[--ocl.slots] :
		ocl.allocations++;
	ocl.free_allocations[_index] = data;

	if(index != NULL) {
		*index = _index;
	} // if
} // add_allocation

/*----------------------------------------------------------------------------*
 * Remove from free allocations.
 *----------------------------------------------------------------------------*/

void free_allocation(int32_t * index) {
	ASSERT(index != NULL, "Bad index!");	
	free(ocl.free_allocations[*index]);
	ocl.open_slots[ocl.slots++] = *index;
	*index = -1;
} // free_allocation

size_t num_allocations() {
	return ocl.allocations;
} // num_allocations

/*----------------------------------------------------------------------------*
 * default_hint
 *----------------------------------------------------------------------------*/

void default_hint(const ocl_kernel_work_group_info_t * info,
	ocl_kernel_hints_t * hints) {
	// force power-of-two size
	size_t places = log2(info->work_group_size);

	hints->max_work_group_size = 1<<places;

} // default_hint

/*----------------------------------------------------------------------------*
 * file_to_string
 *----------------------------------------------------------------------------*/

char * file_to_string(const char * filename) {
	FILE * file = fopen(filename, "r");

	if(file == NULL) {
		message("Error: failed opening %s\n", filename);
		exit(1);
	} // if

	// get file size
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	// allocate space
	char * buffer = (char *)malloc(size+1);

	if(buffer == NULL) {
		message("Error: failed allocating %lu bytes\n", size);
		exit(1);
	} // if

	fread(buffer, 1, size, file);
	buffer[size] = '\0';

	fclose(file);

	return buffer;
} // file_to_string

/******************************************************************************
 * Hash implementation
 ******************************************************************************/

/*----------------------------------------------------------------------------*
 * ocl_hash_init
 *----------------------------------------------------------------------------*/

int32_t ocl_hash_init() {
	int32_t ierr = 0;

	// free data at exit
	hm_set_property(hm_free_data);

	// print error and exit if something goes wrong
	hm_set_property(hm_exit_on_error);

	// buffer tables are handled in built-source file

	// program hash table
	hm_add_table(&ocl.program_hash);

	// event hash tables
	hm_add_table(&ocl.host_event_hash);
	hm_add_table(&ocl.device_event_hash);

	return ierr;
} // ocl_hash_init

/*----------------------------------------------------------------------------*
 * ocl_hash_add_program
 *----------------------------------------------------------------------------*/

void ocl_hash_add_program(const char * name, uint32_t device_id,
	cl_program token) {
	// allocate storage for the program
	ocl_program_t * _program = (ocl_program_t *)malloc(sizeof(ocl_program_t));

	// initialize a kernel hash table for this program
	hm_add_table(&_program->kernel_hash);

	// set the program device id
	_program->device_id = device_id;

	// set the program token
	_program->token = token;

	// add program to hash
	hm_add(ocl.program_hash, name, (void *)_program);
} // ocl_hash_add_program

/*----------------------------------------------------------------------------*
 * ocl_hash_add_kernel
 *----------------------------------------------------------------------------*/

void ocl_hash_add_kernel(const char * program_name, const char * kernel_name,
	ocl_kernel_t token) {
	// find the program
	ocl_program_t * _program = hm_find(ocl.program_hash, program_name);

	// allocate memory for the kernel token
	ocl_kernel_t * _token = (ocl_kernel_t *)malloc(sizeof(ocl_kernel_t));
	*_token = token;

	// add to the kernel hash of this program
	hm_add(_program->kernel_hash, kernel_name, (void *)_token);
} // ocl_hash_add_kernel

/*----------------------------------------------------------------------------*
 * ocl_hash_add_buffer
 *----------------------------------------------------------------------------*/

void ocl_hash_add_buffer(uint32_t device_id, const char * name,
	cl_mem token) {
	// allocate storage for the buffer
	ocl_buffer_t * _buffer = (ocl_buffer_t *)malloc(sizeof(ocl_buffer_t));

	// set the buffer token
	_buffer->token = token;	

	// add buffer to device hash
	hm_add(ocl.device_hash[device_id], name, (void *)_buffer);
} // ocl_hash_add_buffer

/*----------------------------------------------------------------------------*
 * ocl_hash_find_program
 *----------------------------------------------------------------------------*/

ocl_program_t * ocl_hash_find_program(const char * program_name) {
	return hm_find(ocl.program_hash, program_name);
} // ocl_hash_find_program

/*----------------------------------------------------------------------------*
 * ocl_hash_find_kernel
 *----------------------------------------------------------------------------*/

ocl_kernel_t * ocl_hash_find_kernel(const char * program_name,
	const char * kernel_name) {
	ocl_program_t * _program = ocl_hash_find_program(program_name);
	return hm_find(_program->kernel_hash, kernel_name);
} // ocl_hash_find_kernel

/*----------------------------------------------------------------------------*
 * ocl_hash_find_buffer
 *----------------------------------------------------------------------------*/

ocl_buffer_t * ocl_hash_find_buffer(uint32_t device_id,
	const char * buffer_name) {
	return hm_find(ocl.device_hash[device_id], buffer_name);
} // ocl_hash_find_program

/*----------------------------------------------------------------------------*
 * ocl_hash_remove_program
 *----------------------------------------------------------------------------*/

void ocl_hash_remove_program(const char * program_name) {
	ocl_program_t * _program = ocl_hash_find_program(program_name);
	hm_remove_table(_program->kernel_hash, 1);
	hm_remove(ocl.program_hash, program_name, 1);
} // ocl_hash_remove_program

/*----------------------------------------------------------------------------*
 * ocl_hash_remove_kernel
 *----------------------------------------------------------------------------*/

void ocl_hash_remove_kernel(const char * program_name,
	const char * kernel_name) {
	ocl_program_t * _program = ocl_hash_find_program(program_name);
	hm_remove(_program->kernel_hash, kernel_name, 1);
} // ocl_hash_remove_program

/*----------------------------------------------------------------------------*
 * ocl_hash_remove_buffer
 *----------------------------------------------------------------------------*/

void ocl_hash_remove_buffer(uint32_t device_id, const char * buffer_name) {
	hm_remove(ocl.device_hash[device_id], buffer_name, 1);
} // ocl_hash_remove_buffer

/*----------------------------------------------------------------------------*
 * ocl_hash_destroy
 *----------------------------------------------------------------------------*/

void ocl_hash_finalize() {
} // ocl_hash_finalize

/******************************************************************************
 * OpenCL kernel source interface
 ******************************************************************************/

int32_t ocl_add_from_file(const char * file, char ** source, int32_t prepend) {
	int32_t ierr = 0;
	char buffer[OCL_MAX_SOURCE_LENGTH];
	char path_file[OCL_MAX_SOURCE_LENGTH];
	char * file_string = NULL;
	struct stat st;

	// test for file existence
	if(stat(file, &st) != -1) {
		file_string = file_to_string(file);
	}
	else {
		// try appending the kernel path
		char * _paths = getenv("OCL_KERNEL_PATH") == NULL ? NULL :
			strdup(getenv("OCL_KERNEL_PATH"));
		char * _free_chars = _paths;
		char * _path = NULL;
		int32_t found = 0;

		while((_path = strsep(&_paths, ":")) != NULL) {
			sprintf(path_file, "%s/%s", _path, file);

			if(stat(path_file, &st) != -1) {
				file_string = file_to_string(path_file);
				found = 1;
				break;
			} // if
		} // while

		free(_free_chars);

		if(found == 0) {
			warning("File %s does not exist!\n", file);
			ierr = 1;
		} // if
	} // if

	if(ierr != 1) {
		// copy everything into temporary buffer
		if(prepend == 1) {
			sprintf(buffer, "%s%s", file_string,
				(*source == NULL) ? "" : *source);
		}
		else {
			sprintf(buffer, "%s%s",
				(*source == NULL) ? "" : *source, file_string);
		} // if

		// allocate new storage
		free(*source);
		*source = strndup(buffer, strlen(buffer));
	} // if

	// free locally allocated data
	free(file_string);

	return ierr;
} // ocl_concat_from_file

int32_t ocl_add_from_string(const char * string, char ** source,
	int32_t prepend) {
	int32_t ierr = 0;
	char buffer[OCL_MAX_SOURCE_LENGTH];

	// copy everything into temporary buffer
	if(prepend == 1) {
		sprintf(buffer, "%s%s", string, (*source == NULL) ? "" : *source);
	}
	else {
		sprintf(buffer, "%s%s", (*source == NULL) ? "" : *source, string);
	} // if

	// free allocated data
	free(*source);

	// allocate new storage
	*source = strndup(buffer, strlen(buffer));

	return ierr;
} // ocl_append_from_string

/******************************************************************************
 * Host-side timer interface
 ******************************************************************************/

/*----------------------------------------------------------------------------*
 * Initialize timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_initialize_timer(const char * label) {
	int32_t ierr = 0;

	if(hm_key_exists(ocl.host_event_hash, label) == 1) {
		warning("Hash key already exists!!!  No action taken...");
		return ierr;
	} // if

	ocl_host_timer_data_t * _data =
		(ocl_host_timer_data_t *)malloc(sizeof(ocl_host_timer_data_t));

	// initialize data
	_data->start.tv_sec = 0.0;
	_data->start.tv_usec = 0.0;
	_data->duration = 0.0;

	hm_add(ocl.host_event_hash, label, (void *)_data);

	return ierr;
} // ocl_host_initialize_timer

/*----------------------------------------------------------------------------*
 * Clear timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_clear_timer(const char * label) {
	int32_t ierr = 0;

	ocl_host_timer_data_t * _data =
		(ocl_host_timer_data_t *)hm_find(ocl.host_event_hash, label);

	_data->start.tv_sec = 0.0;
	_data->start.tv_usec = 0.0;
	_data->duration = 0.0;

	return ierr;
} // ocl_host_clear_timer

/*----------------------------------------------------------------------------*
 * Start timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_start_timer(const char * label) {
	int32_t ierr = 0;
	ocl_host_timer_data_t tmp;

	ocl_host_timer_data_t * _data =
		(ocl_host_timer_data_t *)hm_find(ocl.host_event_hash, label);

	if(gettimeofday(&tmp.start, NULL)) {
		warning("Start timer failed for %s\n", label);
	} // if
	
	_data->start = tmp.start;

	return ierr;
} // ocl_host_start_timer

/*----------------------------------------------------------------------------*
 * Stop timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_stop_timer(const char * label) {
	int32_t ierr = 0;
	struct timeval stop;

	if(gettimeofday(&stop, NULL)) {
		warning("Stop timer failed for %s\n", label);
	} // if

	ocl_host_timer_data_t * _data =
		(ocl_host_timer_data_t *)hm_find(ocl.host_event_hash, label);

	double duration = (stop.tv_sec*1000000.0 + stop.tv_usec);
	duration -= (_data->start.tv_sec*1000000.0 + _data->start.tv_usec);
	
	_data->duration += duration;

	return ierr;
} // ocl_host_stop_timer

/*----------------------------------------------------------------------------*
 * Report timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_report_timer(const char * label) {
	int32_t ierr = 0;

	ocl_host_timer_data_t * _data =
		(ocl_host_timer_data_t *)hm_find(ocl.host_event_hash, label);

	message("Timing results for %s:\n", label);
	message("\tduration %lf\n", _data->duration/1000000.0);
	message("\n");

	return ierr;
} // ocl_host_report_timer

int32_t ocl_host_read_timer(const char * label, double * value) {
	int32_t ierr = 0;

	ocl_host_timer_data_t * _data =
		(ocl_host_timer_data_t *)hm_find(ocl.host_event_hash, label);

	*value = _data->duration/1000000.0;

	return ierr;
} // ocl_host_read_timer

/****************************************************************************** 
 * OpenCL timer implementation
 ******************************************************************************/

/*----------------------------------------------------------------------------* 
 * ocl_add_timer_list
 *----------------------------------------------------------------------------*/

int32_t ocl_add_timer_list(const char * label,
	const ocl_event_wait_list_t * list) {
	size_t i;
	int32_t ierr = 0;

	ocl_event_t e;
	ocl_initialize_event(&e);

	for(i=0; i<list->num_events_in_wait_list; ++i) {
		e.event = list->event_wait_list[i];
		ocl_add_timer(label, &e);
	} // for

	return ierr;
} // ocl_add_timer_list

/*----------------------------------------------------------------------------* 
 * ocl_add_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_add_timer(const char * label, const ocl_event_t * event) {
	cl_ulong queued, submit, start, end;
	int32_t ierr = 0;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	// read when event was enqueued on host
	clGetEventProfilingInfo(event->event, CL_PROFILING_COMMAND_QUEUED,
		sizeof(cl_ulong), &queued, NULL);

	// read when event was submitted to device
	clGetEventProfilingInfo(event->event, CL_PROFILING_COMMAND_SUBMIT,
		sizeof(cl_ulong), &submit, NULL);

	// read when event began execution on device
	clGetEventProfilingInfo(event->event, CL_PROFILING_COMMAND_START,
		sizeof(cl_ulong), &start, NULL);

	// read when event ended execution on device
	clGetEventProfilingInfo(event->event, CL_PROFILING_COMMAND_END,
		sizeof(cl_ulong), &end, NULL);

	ocl_timer_event_t * _timer_event =
		(ocl_timer_event_t *)malloc(sizeof(ocl_timer_event_t));

	// how long did the event sit in the host queue?
	_timer_event->queued = (submit-queued)*1e-9;

	// what was the invocation latency?
	_timer_event->invocation = (start-submit)*1e-9;

	// what was the execution duration?
	_timer_event->duration = (end-start)*1e-9;

	// aggregate of all time spent in event execution
	_timer_event->aggregate = _timer_event->queued + _timer_event->invocation +
		_timer_event->duration;

	if(hm_key_exists(ocl.device_event_hash, label) == 1) {
		ocl_timer_event_t * _data =
			(ocl_timer_event_t *)hm_find(ocl.device_event_hash, label);

		_data->queued += _timer_event->queued;
		_data->invocation += _timer_event->invocation;
		_data->duration += _timer_event->duration;
		_data->aggregate += _timer_event->aggregate;
		free(_timer_event);
	}
	else {
		hm_add(ocl.device_event_hash, label, (void *)_timer_event);
	} // if

	return ierr;
} // ocl_add_timer

/*----------------------------------------------------------------------------*
 * ocl_clear_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_clear_timer(const char * label) {
	int32_t ierr = 0;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	ocl_timer_event_t * _data =
		(ocl_timer_event_t *)hm_find(ocl.device_event_hash, label);

	// timer doesn't exist: do nothing
	if(_data == NULL) { return ierr; }
	
	_data->queued = 0.0;
	_data->invocation = 0.0;
	_data->duration = 0.0;
	_data->aggregate = 0.0;

	return ierr;
} // ocl_clear_timer

/*----------------------------------------------------------------------------*
 * ocl_report_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_report_timer(const char * label) {
	int32_t ierr = 0;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	ocl_timer_event_t * _data =
		(ocl_timer_event_t *)hm_find(ocl.device_event_hash, label);

	message("Timing results for %s:\n", label);
	message("\tqueued %lf\n", _data->queued);
	message("\tinvocation %lf\n", _data->invocation);
	message("\tduration %lf\n", _data->duration);
	message("\taggregate %lf\n", _data->aggregate);
	message("\n");

	return ierr;
} // ocl_report_timer

/*----------------------------------------------------------------------------*
 * ocl_read_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_read_timer(const char * label, ocl_timer_attribute_t attribute, double * value) { int32_t ierr = 0;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	ocl_timer_event_t * _data =
		(ocl_timer_event_t *)hm_find(ocl.device_event_hash, label);

	switch(attribute) {
		case OCL_TIMER_QUEUED:
			*value = _data->queued;
			break;
		case OCL_TIMER_INVOCATION:
			*value = _data->invocation;
			break;
		case OCL_TIMER_DURATION:
			*value = _data->duration;
			break;
		case OCL_TIMER_AGGREGATE:
			*value = _data->aggregate;
			break;
		default:
			message("Error: Unknown timer attribute %d!\n", attribute);
			exit(1);
			break;
	} // switch

	return ierr;
} // ocl_read_timer

/******************************************************************************
 ******************************************************************************
 * Fortran 90
 ******************************************************************************
 ******************************************************************************/

/*----------------------------------------------------------------------------*
 * Initialize timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_initialize_timer_f90(const char * label) {
	return ocl_host_initialize_timer(label);
} // ocl_host_initialize_timer_f90

/*----------------------------------------------------------------------------*
 * Clear timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_clear_timer_f90(const char * label) {
	return ocl_host_clear_timer(label);
} // ocl_host_clear_timer_f90

/*----------------------------------------------------------------------------*
 * Start timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_start_timer_f90(const char * label) {
	return ocl_host_start_timer(label);
} // ocl_host_start_timer_f90

/*----------------------------------------------------------------------------*
 * Stop timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_stop_timer_f90(const char * label) {
	return ocl_host_stop_timer(label);
} // ocl_host_stop_timer_f90

/*----------------------------------------------------------------------------*
 * Report timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_report_timer_f90(const char * label) {
	return ocl_host_report_timer(label);
} // ocl_host_report_timer_f90

/*----------------------------------------------------------------------------*
 * Report timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_read_timer_f90(const char * label, double * value) {
	return ocl_host_read_timer(label, value);
} // ocl_host_read_timer_f90

/*----------------------------------------------------------------------------*
 * ocl_add_timer_list_f90
 *----------------------------------------------------------------------------*/

int32_t ocl_add_timer_list_f90(const char * label,
	const ocl_allocation_t * list) {
	return ocl_add_timer_list(label, (ocl_event_wait_list_t *)list->data);
} // ocl_add_timer_list_f90

/*----------------------------------------------------------------------------*
 * ocl_add_timer_f90
 *----------------------------------------------------------------------------*/

int32_t ocl_add_timer_f90(const char * label,
	const ocl_allocation_t * event) {
	return ocl_add_timer(label, (ocl_event_t *)event->data);
} // ocl_add_timer_f90

/*----------------------------------------------------------------------------*
 * ocl_clear_timer_f90
 *----------------------------------------------------------------------------*/

int32_t ocl_clear_timer_f90(const char * label) {
	return ocl_clear_timer(label);
} // ocl_clear_timer_f90

/*----------------------------------------------------------------------------*
 * ocl_report_timer_f90
 *----------------------------------------------------------------------------*/

int32_t ocl_report_timer_f90(const char * label) {
	return ocl_report_timer(label);
} // ocl_report_timer_f90
