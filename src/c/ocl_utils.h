/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#if !defined(_ocl_source) && !defined(_include_ocl_h)
#error "Error: do not include this file directly, use #include <ocl.h>"
#endif

#ifndef ocl_utils_h
#define ocl_utils_h

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ocl_data.h"

/*------------------------------------------------------------------------------
 * Useful defines
 *----------------------------------------------------------------------------*/

#define OCL_SYNCHRONOUS 1
#define OCL_ASYNCHRONOUS 0

#ifdef ENABLE_OCL_ASSERTIONS
#define ASSERT(v, s)									\
	if((v) == 0) {										\
		message("Assertion Faile: %s\n", s);	\
		exit(1);											\
	}
#else
#define ASSERT(v, s)
#endif

#define STRINGIFY(token) #token
#define DEFINE_TO_STRING(define) STRINGIFY(define)

#if defined(__APPLE__)
#define CL_ULONG_FMT "llu"
#else
#define CL_ULONG_FMT "lu"
#endif

/*------------------------------------------------------------------------------
 * P.O.S. OS X Snow Leopard
 *----------------------------------------------------------------------------*/

#if !defined(HAVE_STRNDUP)
char * strndup(const char * s, size_t n);
#endif

/*------------------------------------------------------------------------------
 * Output
 *----------------------------------------------------------------------------*/

static inline void message(const char * format, ...) {
// only output messages if we're doing ENABLE_OCL_VERBOSE
#if defined(ENABLE_OCL_VERBOSE)
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
#endif
} // message

void warning(const char * format, ...);

static inline void error(const char * format, ...) {
	va_list args;
	va_start(args, format);

	fprintf(stderr, "Error: ");
	vfprintf(stderr, format, args);
} // error

/*------------------------------------------------------------------------------
 * OpenCL error codes
 *----------------------------------------------------------------------------*/

const char * error_to_string(int err);

/*------------------------------------------------------------------------------
 * Print device info structure.
 *----------------------------------------------------------------------------*/

void print_device_info(ocl_device_info_t * info, const char * label);

/*------------------------------------------------------------------------------
 * Add to free allocations.
 *----------------------------------------------------------------------------*/

void add_allocation(void * data, int32_t * index);

/*------------------------------------------------------------------------------
 * Remove from free allocations.
 *----------------------------------------------------------------------------*/

void free_allocation(void * data, int32_t * index);

/*------------------------------------------------------------------------------
 * Convert the contents of a file into a string, allocating memory for the
 * returned char * (this must be freed by the user.
 *----------------------------------------------------------------------------*/

char * file_to_string(const char * filename);

/*------------------------------------------------------------------------------
 * Compute size hint.
 *----------------------------------------------------------------------------*/

#ifndef KERNEL_HINT_FUNCTION
#define KERNEL_HINT_FUNCTION default_hint
#endif

void default_hint(ocl_kernel_hint_t * hint);

/*------------------------------------------------------------------------------
 * Hash functions
 *
 * This interface is necessary because OS X does not support the renetrant
 * version of the hsearch interface.  We could lose this if they fix it.
 *
 *----------------------------------------------------------------------------*/

int32_t ocl_hash_init();
void ocl_hash_add_program(const char * program_name, cl_program token);
void ocl_hash_add_kernel(const char * program_name, const char * kernel_name,
	ocl_kernel_t token);
ENTRY * ocl_hash_find_event(const char * event_name);
ENTRY * ocl_hash_find_program(const char * program_name);
ENTRY * ocl_hash_find_kernel(const char * program_name,
	const char * kernel_name);
void ocl_hash_destroy();

/*******************************************************************************
 * OpenCL kernel source interface
 ******************************************************************************/

/*!
\page ocl_add_from_file
Append or prepend OpenCL C source code from a file.

\par C Version:
ierr ocl_add_from_file(file, source, prepend)

@param file The input file containing the source to read \n
const char * (\b C) \n
@param source The source read from the file will be appended or prepended to \b source \n
char ** (\b C) \n
@param prepend If true (\b prepend \b == \b 1), prepend the source instead of appending it \n
int32_t (\b C) \n

\par Description:
This function appends or prepends OpenCL C source code read from \b file to \b source, allocating memory up to \b OCL_MAX_SOURCE_LENGTH as necessary.  The users is responsible for freeing the memory allocated for \b source.
*/

int32_t ocl_add_from_file(const char * file, char ** source, int32_t prepend);

/*!
\page ocl_add_from_string
Append or prepend OpenCL C source code from a string.

\par C Version:
ierr ocl_add_from_string(string, source, prepend)

@param string The input string containing the source to add \n
const char * (\b C) \n
@param source The source from \b string will be appended or prepended to \b source \n
char ** (\b C) \n
@param prepend If true (\b prepend \b == \b 1), prepend the source instead of appending it \n
int32_t (\b C) \n

\par Description:
This function appends or prepends OpenCL C source code from \b string to \b source, allocating memory up to \b OCL_MAX_SOURCE_LENGTH as necessary.  The users is responsible for freeing the memory allocated for \b source.
*/

int32_t ocl_add_from_string(const char * string, char ** source,
	int32_t prepend);

/*******************************************************************************
 * Host-side timer interface
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * Begin timer
 *----------------------------------------------------------------------------*/

int32_t ocl_host_initialize_timer(const char * label);
int32_t ocl_host_clear_timer(const char * label);
int32_t ocl_host_start_timer(const char * label);
int32_t ocl_host_stop_timer(const char * label);
int32_t ocl_host_report_timer(const char * label);
int32_t ocl_host_read_timer(const char * label, double * value);

/*******************************************************************************
 * OpenCL timer interface
 ******************************************************************************/

int32_t ocl_add_timer_list(const char * label,
	const ocl_event_wait_list_t * list);

/*------------------------------------------------------------------------------
 * Add a timer event.
 *----------------------------------------------------------------------------*/

/*!
\page ocl_add_timer
Add or append to a timer event.

\par C Version:
ierr ocl_add_timer(label, event)

\par Fortran Version:
ocl_add_timer(label, event, ierr)

@param label Label hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param event Event handle \n
ocl_event_t * (\b C), type(ocl_allocation_t) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine adds timer information under the hash key defined by \b label.  If \b label already exists, timing inforamtion is appended to the existing data.  If \b label does not exist, a new timing event is created.  The \b event argument passed to this subroutine must have been initialized using the \b ocl_initialize_event routine, and must have been passed to a routine that tracks event information, e.g., \b ocl_enqueue_kernel_ndrange.
*/

int32_t ocl_add_timer(const char * label, const ocl_event_t * event);

/*------------------------------------------------------------------------------
 * Clear timer information.
 *----------------------------------------------------------------------------*/

/*!
\page ocl_clear_timer
Clear timer information.

\par C Version:
ierr ocl_clear_timer(label)

\par Fortran Version:
ocl_clear_timer(label, ierr)

@param label Label hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
Clear the timer inforamtion associated with the given hash key.
*/

int32_t ocl_clear_timer(const char * label);

/*------------------------------------------------------------------------------
 * Print a timer.
 *----------------------------------------------------------------------------*/

/*!
\page ocl_report_timer
Print timer information to standard output.

\par C Version:
ierr ocl_report_timer(label)

\par Fortran Version:
ocl_report_timer(label, ierr)

@param label Label hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
This subroutine prints the timer information associated with \b label to standard output.  The output will have the following form: \n\n
Timing results for 'label': \n
\t queued 0.000006 \n
\t invocation 0.000017 \n
\t duration 0.000354 \n\n
\b queued \n
\t Time in ms that the host waited before acting on the request.\n\n
\b invocation \n
\t Kernel execution latency in ms.\n\n
\b duration \n
\t Kernel duration in ms.
*/

int32_t ocl_report_timer(const char * label);

typedef int32_t ocl_timer_attribute_t;

#define OCL_TIMER_QUEUED 0
#define OCL_TIMER_INVOCATION 1
#define OCL_TIMER_DURATION 2
#define OCL_TIMER_AGGREGATE 3

/*!
\page ocl_read_timer
Read timer information.

\par C Version:
ierr ocl_read_timer(label, attribute, value)

\par Fortran Version:
ocl_read_timer(label, attribute, value, ierr)

@param label Label hash key \n
const char * (\b C), character(kind=c_char), dimension(*) (\b Fortran) \n
@param attribute Timer attribute to read \n
ocl_timer_attribute_t (\b C), integer(int32_t) (\b Fortran)
@param value Value that will be filled with the timer information \n
double * (\b C), real(c_double) (\b Fortran)
@param ierr Error status \n
int32_t (\b C), integer(int32_t) (\b Fortran)

\par Description:
Read the given attribute from the timer.  Supported attribute values are \b OCL_TIMER_QUEUED, \b OCL_TIMER_INVOCATION, \b OCL_TIMER_DURATION and \b OCL_TIMER_AGGREGATE.
*/

int32_t ocl_read_timer(const char * label, ocl_timer_attribute_t attribute,
	double * value);

// Fortran 90 bindings
int32_t ocl_add_timer_f90(const char * label, const ocl_allocation_t * event);
int32_t ocl_clear_timer_f90(const char * label);
int32_t ocl_report_timer_f90(const char * label);
int32_t ocl_read_timer_f90(const char * label,
	ocl_timer_attribute_t attribute, double * value);

/*------------------------------------------------------------------------------
 * Macro definitions
 *----------------------------------------------------------------------------*/

#define CALLER_ARGS_DECL const char *caller_function,	\
	const char *caller_filename,								\
	unsigned caller_linenumber

#define CALLER_ARGS caller_function,caller_filename,caller_linenumber
#define CALLER_SELF										\
	const char * caller_function = __FUNCTION__;	\
	const char * caller_filename = __FILE__;		\
	unsigned caller_linenumber = __LINE__;

#define CL_ABORT()														\
	message("%s in file %s line %d\n",								\
		caller_function, caller_filename, caller_linenumber);	\
	exit(1);

#define CL_ABORTmalloc()												\
	message("Failed malloc in %s in file %s line %d\n",		\
		caller_function, caller_filename, caller_linenumber);	\
	exit(1);

#define CL_ABORTsanity(s) 														\
	message("%s\nSanity check failed in %s in file %s line %d\n",	\
		s, caller_function, caller_filename, caller_linenumber);		\
	exit(1);

#define CL_ABORTerr(call, err) 													\
	message("OpenCL call " #call													\
		" failed with %s(%d) in %s in file %s line %d\n",					\
		error_to_string(err), (err), caller_function, caller_filename,	\
		caller_linenumber);															\
	exit(1);

#define CL_ABORTkernel(call, err, name)							\
	message("OpenCL kernel %s failed in " #call					\
		" with %s(%d) in %s in file %s line %d\n",				\
		(name), error_to_string(err), (err), caller_function,	\
		caller_filename, caller_linenumber);						\
	exit(1);

#define CL_ABORTcreateKernel(err,name)			\
	CL_ABORTkernel(clCreateKernel,err,name)

#define CL_CHECKerr(call,...) {								\
		cl_int err;													\
		if((err = call(__VA_ARGS__)) != CL_SUCCESS) {	\
			CL_ABORTerr(call, err);								\
		}																\
	}

#define CL_CHECKerrKernel(kernel,call,...) {				\
		cl_int err;													\
		if ((err = call (__VA_ARGS__)) != CL_SUCCESS) {	\
			char name[256];										\
			size_t size;											\
			CL_CHECKerr(clGetKernelInfo,						\
							 kernel,									\
							 CL_KERNEL_FUNCTION_NAME,			\
							 sizeof(name), name, &size);		\
			CL_ABORTkernel(call,err,name);					\
		}																\
	}

#endif // ocl_utils_h
