/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#define _ocl_source

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "ocl_interface.h"
#include "ocl_utils.h"

extern ocl_data_t ocl;
extern int32_t ocl_warning;

/*----------------------------------------------------------------------------*
 * P.O.S. OS X Snow Leopard
 *----------------------------------------------------------------------------*/

#if !defined(HAVE_STRNDUP)
char * strndup(const char * s, size_t n) {
	char * result;
	size_t len = strlen(s);

	if(n<len) {
		len = n;
	} // if

	result = (char *)malloc(len+1);

	if(!result) { return NULL; }

	result[len] = '\0';

	return (char *)memcpy(result, s, len);
} // strndup
#endif

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

	message("\n");

} // print_device_info

/*----------------------------------------------------------------------------*
 * Add to free allocations.
 *----------------------------------------------------------------------------*/

void add_allocation(void * data, int32_t * index) {
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

void default_hint(ocl_kernel_hint_t * hint) {
	hint->local_size = hint->work_group_size;
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

// needed to initialize hash data before call to hcreate_r
#if !defined(__APPLE__)
static struct hsearch_data ocl_htab_init = {0};
#endif

/*----------------------------------------------------------------------------*
 * ocl_hash_init
 *----------------------------------------------------------------------------*/

int32_t ocl_hash_init() {
	// pad for hash efficiency as per manpage recommendation
	float pad = 1.25;

#if defined(__APPLE__)
	pad *= (OCL_MAX_PROGRAM_HASH_ENTRIES*OCL_MAX_KERNEL_HASH_ENTRIES +
		OCL_MAX_TIMER_HASH_ENTRIES);
	return !hcreate((size_t)pad);	
#else

#if defined(ENABLE_OPENCL_PROFILING)
	// initialize timer hash
	ocl.events = ocl_htab_init;
	int32_t err = !hcreate_r((size_t)(pad*OCL_MAX_TIMER_HASH_ENTRIES),
		&ocl.events);

	if(err != 0) { return 1; }
#endif

	// initialize table counter
	ocl.tables = 0;

	// zero program hash struct entries
	ocl.programs = ocl_htab_init;

	// return program hash creation
	return !hcreate_r(pad*OCL_MAX_PROGRAM_HASH_ENTRIES, &ocl.programs);
#endif
} // ocl_hash_init

/*----------------------------------------------------------------------------*
 * ocl_hash_add_program
 *----------------------------------------------------------------------------*/

void ocl_hash_add_program(const char * name, cl_program token) {
	ENTRY e, * ep = NULL;
	int32_t err = 0;

	// make sure the program doesn't already exist in the hash
	ep = ocl_hash_find_program(name);

	if(ep != NULL) {
		message("Error: hash entry \"%s\" already exists!\n", name);
		exit(1);
	} // if

	// allocate memory for hash data (this will be freed by
	// hdestroy or hdestroy_r)
	ocl_program_t * program = (ocl_program_t *)malloc(sizeof(ocl_program_t));

#if !defined(__APPLE__)
	// zero hash struct entries
	program->kernels = ocl_htab_init;
#endif

	// set ocl data
	program->token = token;

	// reentrant version needs to allocate hash table for each program
#if !defined(__APPLE__)
	// initialize kernel hash data for this program
	if(!hcreate_r(OCL_MAX_KERNEL_HASH_ENTRIES, &program->kernels) != 0) {
		message("Kernel hash initialization failed for %s\n", name);
		exit(1);
	} // if

	ocl.free_tables[ocl.tables++] = &program->kernels;
#endif

	// set the key
	e.key = strdup(name);

	// set hash data
	e.data = (void *)program;

	// set data to free
#if ! defined(__APPLE__)
	add_allocation(e.key, NULL);
#endif
	add_allocation(e.data, NULL);

	// add program entry
#if defined(__APPLE__)
	ep = hsearch(e, ENTER);
#else
	err = !hsearch_r(e, ENTER, &ep, &ocl.programs);
#endif

	// check errors
	if(err != 0 || ep == NULL) {
		message("Hash entry failed for %s\n", name);
		exit(1);
	} // if
} // ocl_hash_add_program

/*----------------------------------------------------------------------------*
 * ocl_hash_add_kernel
 *----------------------------------------------------------------------------*/

void ocl_hash_add_kernel(const char * program_name, const char * kernel_name,
	ocl_kernel_t token) {
	ENTRY e, * ep = NULL;
	int32_t err = 0;

	// make sure that the program exists
	ep = ocl_hash_find_program(program_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", program_name);
		exit(1);
	} // if

#if !defined(__APPLE__)
	// get the program hash data
	ocl_program_t * program = (ocl_program_t *)ep->data;
#endif

	// make sure that this kernel name does not already exist in the hash table
	ep = ocl_hash_find_kernel(program_name, kernel_name);

	if(ep != NULL) {
		message("Error: hash entry \"%s\" already exists!\n", kernel_name);
		exit(1);
	} // if

	// set the key
#if defined(__APPLE__)
	char buffer[1024];
	sprintf(buffer, "%s%s", program_name, kernel_name);
	e.key = strdup(buffer);
#else
	e.key = strdup(kernel_name);
#endif

	// set the data
	ocl_kernel_t * _token = (ocl_kernel_t *)malloc(sizeof(ocl_kernel_t));
	*_token = token;
	e.data = (void *)_token;

	// set data to free
#if ! defined(__APPLE__)
	add_allocation(e.key, NULL);
#endif
	add_allocation(e.data, NULL);

#if defined(__APPLE__)
	ep = hsearch(e, ENTER);
#else
	err = !hsearch_r(e, ENTER, &ep, &program->kernels);
#endif

	// check errors
	if(err != 0 || ep == NULL) {
		message("Hash entry failed for %s\n", kernel_name);
		exit(1);
	} // if
} // ocl_hash_add_kernel

/*----------------------------------------------------------------------------*
 * ocl_hash_find_program
 *----------------------------------------------------------------------------*/

ENTRY * ocl_hash_find_event(const char * event_name) {
	ENTRY e, * ep = NULL;

	e.key = strdup(event_name);

#if defined(__APPLE__)
	ep = hsearch(e, FIND);
#else
	hsearch_r(e, FIND, &ep, &ocl.events);
#endif

	free(e.key);

	return ep;
} // ocl_hash_find_event

/*----------------------------------------------------------------------------*
 * ocl_hash_find_program
 *----------------------------------------------------------------------------*/

ENTRY * ocl_hash_find_program(const char * program_name) {
	ENTRY e, * ep = NULL;

	e.key = strdup(program_name);

#if defined(__APPLE__)
	ep = hsearch(e, FIND);
#else
	hsearch_r(e, FIND, &ep, &ocl.programs);
#endif

	free(e.key);

	return ep;
} // ocl_hash_find_program

/*----------------------------------------------------------------------------*
 * ocl_hash_find_kernel
 *----------------------------------------------------------------------------*/

ENTRY * ocl_hash_find_kernel(const char * program_name,
	const char * kernel_name) {
	ENTRY e, * ep = NULL;

	// get the program (this is just a sanity check for the OS X implementation)
	ep = ocl_hash_find_program(program_name);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", program_name);
		exit(1);
	} // if

#if defined(__APPLE__)
	char buffer[1024];
	sprintf(buffer, "%s%s", program_name, kernel_name);
	e.key = strdup(buffer);
	ep = hsearch(e, FIND);
#else
	ocl_program_t * program = (ocl_program_t *)ep->data;

	e.key = strdup(kernel_name);
	hsearch_r(e, FIND, &ep, &program->kernels);
#endif

	free(e.key);

	return ep;
} // ocl_hash_find_kernel

/*----------------------------------------------------------------------------*
 * ocl_hash_destroy
 *----------------------------------------------------------------------------*/

void ocl_hash_destroy() {
#if defined(__APPLE__)
	hdestroy();
#else
	size_t i;

#if defined(ENABLE_OPENCL_PROFILING)
	// destroy timer hash
	hdestroy_r(&ocl.events);
#endif

	// destroy kernel tables
	for(i=0; i<ocl.tables; ++i) {
		hdestroy_r(ocl.free_tables[i]);
	} // for

	// destroy program hash
	hdestroy_r(&ocl.programs);
#endif
} // ocl_destroy_hash

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
	ENTRY e, * ep = NULL;
	int32_t ierr = 0;

	// check for existing key
	ep = ocl_hash_find_event(label);

	if(ep != NULL) {
		error("Hash entry already exists for %s\n", label);
		exit(1);
	} // if

	// allocate data (this will be freed when we destroy the hash)
	ocl_host_timer_data_t * data =
		(ocl_host_timer_data_t *)malloc(sizeof(ocl_host_timer_data_t));

	// initialize data
	data->start.tv_sec = 0.0;
	data->start.tv_usec = 0.0;
	data->duration = 0.0;

	// set hash values
	e.key = strdup(label);
	e.data = (void *)data;

	// add data to free
#if ! defined(__APPLE__)
	add_allocation(e.key, NULL);
#endif
	add_allocation(e.data, NULL);

#if defined(__APPLE__)
	ep = hsearch(e, ENTER);
#else
	ierr = !hsearch_r(e, ENTER, &ep, &ocl.events);
#endif

	if(ierr != 0 || ep == NULL) {
		error("Hash entry failed for %s\n", label);
		exit(1);
	} // if
	
	return ierr;
} // ocl_host_initialize_timer

/*----------------------------------------------------------------------------*
 * Clear timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_clear_timer(const char * label) {
	ENTRY * ep = NULL;
	int32_t ierr = 0;

	// lookup event
	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		error("Hash entry does not exist for %s\n", label);
		exit(1);
	} // if
	
	ocl_host_timer_data_t * data = (ocl_host_timer_data_t *)ep->data;
	
	data->start.tv_sec = 0.0;
	data->start.tv_usec = 0.0;
	data->duration = 0.0;

	return ierr;
} // ocl_host_clear_timer

/*----------------------------------------------------------------------------*
 * Start timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_start_timer(const char * label) {
	ENTRY * ep = NULL;
	ocl_host_timer_data_t tmp;
	int32_t ierr = 0;

	// lookup event
	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		error("Hash entry does not exist for %s\n", label);
		exit(1);
	} // if
	
	ocl_host_timer_data_t * data = (ocl_host_timer_data_t *)ep->data;
	
	if(gettimeofday(&tmp.start, NULL)) {
		warning("Start timer failed for %s\n", label);
	} // if

	data->start = tmp.start;

	return ierr;
} // ocl_host_start_timer

/*----------------------------------------------------------------------------*
 * Stop timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_stop_timer(const char * label) {
	ENTRY * ep = NULL;
	int32_t ierr = 0;
	struct timeval stop;

	if(gettimeofday(&stop, NULL)) {
		warning("Stop timer failed for %s\n", label);
	} // if

	// lookup event
	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		error("Hash entry does not exist for %s\n", label);
		exit(1);
	} // if
	
	ocl_host_timer_data_t * data = (ocl_host_timer_data_t *)ep->data;

	double duration = (stop.tv_sec*1000000.0 + stop.tv_usec);
	duration -= (data->start.tv_sec*1000000.0 + data->start.tv_usec);
	
	data->duration += duration;

	return ierr;
} // ocl_host_stop_timer

/*----------------------------------------------------------------------------*
 * Report timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_report_timer(const char * label) {
	ENTRY * ep = NULL;
	int32_t ierr = 0;

	// lookup event
	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		error("Hash entry does not exist for %s\n", label);
		exit(1);
	} // if
	
	ocl_host_timer_data_t * data = (ocl_host_timer_data_t *)ep->data;

	message("Timing results for %s:\n", label);
	message("\tduration %lf\n", data->duration/1000000.0);
	message("\n");

	return ierr;
} // ocl_host_report_timer

int32_t ocl_host_read_timer(const char * label, double * value) {
	ENTRY * ep = NULL;
	int32_t ierr = 0;

	// lookup event
	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		error("Hash entry does not exist for %s\n", label);
		exit(1);
	} // if
	
	ocl_host_timer_data_t * data = (ocl_host_timer_data_t *)ep->data;

	*value = data->duration/1000000.0;

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
	ENTRY e, * ep = NULL;
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

	ocl_timer_event_t * timer_event =
		(ocl_timer_event_t *)malloc(sizeof(ocl_timer_event_t));

	// how long did the event sit in the host queue?
	timer_event->queued = (submit-queued)*1e-9;

	// what was the invocation latency?
	timer_event->invocation = (start-submit)*1e-9;

	// what was the execution duration?
	timer_event->duration = (end-start)*1e-9;

	// aggregate of all time spent in event execution
	timer_event->aggregate = timer_event->queued + timer_event->invocation +
		timer_event->duration;

	ep = ocl_hash_find_event(label);

	// if the label exists, append new values
	if(ep != NULL) {
		((ocl_timer_event_t *)ep->data)->queued += timer_event->queued;
		((ocl_timer_event_t *)ep->data)->invocation += timer_event->invocation;
		((ocl_timer_event_t *)ep->data)->duration += timer_event->duration;
		((ocl_timer_event_t *)ep->data)->aggregate += timer_event->aggregate;
		free(timer_event);
	}
	// add new label
	else {
		// set the key
		e.key = strdup(label);

		// set the data
		e.data = (void *)timer_event;

		// add data to free
#if ! defined(__APPLE__)
		add_allocation(e.key, NULL);
#endif
		add_allocation(e.data, NULL);

		// add event to hash
#if defined(__APPLE__)
		ep = hsearch(e, ENTER);
#else
		ierr = !hsearch_r(e, ENTER, &ep, &ocl.events);
#endif

		// check errors
		if(ierr != 0 || ep == NULL) {
			message("Hash entry failed for %s\n", label);
			exit(1);
		} // if
	} // if

	return ierr;
} // ocl_add_timer

/*----------------------------------------------------------------------------*
 * ocl_clear_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_clear_timer(const char * label) {
	ENTRY * ep = NULL;
	int32_t ierr = 0;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	ep = ocl_hash_find_event(label);

	// if the label exists, append new values
	if(ep != NULL) {
		((ocl_timer_event_t *)ep->data)->queued = 0.0;
		((ocl_timer_event_t *)ep->data)->invocation = 0.0;
		((ocl_timer_event_t *)ep->data)->duration = 0.0;
		((ocl_timer_event_t *)ep->data)->aggregate = 0.0;
	}

	return ierr;
} // ocl_clear_timer

/*----------------------------------------------------------------------------*
 * ocl_report_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_report_timer(const char * label) {
	int32_t ierr = 0;
	ENTRY * ep = NULL;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	// make sure that this label doesn't already exist in the hash
	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", label);
		exit(1);
	} // if

	ocl_timer_event_t * timer_event = (ocl_timer_event_t *)ep->data;

	message("Timing results for %s:\n", label);
	message("\tqueued %lf\n", timer_event->queued);
	message("\tinvocation %lf\n", timer_event->invocation);
	message("\tduration %lf\n", timer_event->duration);
	message("\taggregate %lf\n", timer_event->aggregate);
	message("\n");

	return ierr;
} // ocl_report_timer

/*----------------------------------------------------------------------------*
 * ocl_read_timer
 *----------------------------------------------------------------------------*/

int32_t ocl_read_timer(const char * label, ocl_timer_attribute_t attribute,
	double * value) {
	ENTRY * ep = NULL;
	int32_t ierr = 0;

#if !defined(ENABLE_OPENCL_PROFILING)
	message("ERROR: You must enable profiling to use timers!\n");
	exit(1);
#endif

	ep = ocl_hash_find_event(label);

	if(ep == NULL) {
		message("Error: hash entry \"%s\" does not exist!\n", label);
		exit(1);
	} // if

	ocl_timer_event_t * timer_event = (ocl_timer_event_t *)ep->data;

	switch(attribute) {
		case OCL_TIMER_QUEUED:
			*value = timer_event->queued;
			break;
		case OCL_TIMER_INVOCATION:
			*value = timer_event->invocation;
			break;
		case OCL_TIMER_DURATION:
			*value = timer_event->duration;
			break;
		case OCL_TIMER_AGGREGATE:
			*value = timer_event->aggregate;
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
