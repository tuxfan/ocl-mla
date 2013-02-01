/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "ocl.h"
#include <example_strings.h>

int main(int argc, char ** argv) {

	size_t i;

	const int32_t ELEMENTS = 16;
	size_t global_offset = 0;
	size_t global_size = ELEMENTS;
	size_t local_size = 1;
	size_t offset = 0;
	size_t bytes;

	float h_array[ELEMENTS];

	ocl_event_t event;
	ocl_event_wait_list_t wait_list;
	cl_event events[10];

	// step (1)
	// data size for buffer copying
	bytes = ELEMENTS*sizeof(float);

	// step (2)
	// initizize array
	for(i= 0; i<ELEMENTS; ++i) {
		h_array[i] = 0.0;
	} // for

	// step (3)
	// initialize the OpenCL layer
	ocl_init();

	// step (4)
	// create a device-side buffer
	ocl_create_buffer(OCL_DEFAULT_DEVICE, "array", bytes,
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, h_array);

	// step (5)
	// add program and build
	char * source = NULL;
	ocl_add_from_string(utils_PPSTR, &source, 0);
	ocl_add_from_string(test_PPSTR, &source, 0);

	ocl_add_program(OCL_DEFAULT_DEVICE, "program",
		source, "-DMY_DEFINE");
	free(source);

	// step (6)
	// add kernel
	ocl_add_kernel(OCL_DEFAULT_DEVICE, "program", "test", "my test");

	ocl_kernel_hints_t hints;
	size_t work_group_indeces;
	size_t single_indeces;

	ocl_kernel_hints(OCL_DEFAULT_DEVICE, "program", "my test", &hints);
	ocl_ndrange_hints(global_size, hints.max_work_group_size,
		0.5, 0.5, &local_size, &work_group_indeces, &single_indeces);

	if(single_indeces != 0) {
		error("ELEMENTS must be an even multiple of work group size %d\n",
			(int)local_size);
	} // if

	// step (7)
	// set kenerl argument
	ocl_set_kernel_arg_buffer("program", "my test", "array", 0);

	// step (8)
	// initialize event for timings
	ocl_initialize_event(&event);
	ocl_initialize_event_wait_list(&wait_list, events, 10);

	// step (9)
	// invoke kernel
	ocl_enqueue_kernel_ndrange(OCL_DEFAULT_DEVICE, "program",
		"my test", 1, &global_offset, &global_size, &local_size, &event);

	// step (10)
	// block for kernel completion
	ocl_finish(OCL_DEFAULT_DEVICE);

	ocl_add_event_to_wait_list(&wait_list, &event);

	// step (11)
	// add a timer event for the kernel invocation
	ocl_add_timer("kernel", &event);

	// step (12)
	// read data from device
	ocl_enqueue_read_buffer(OCL_DEFAULT_DEVICE, "array", 1, offset,
		ELEMENTS*sizeof(float), h_array, &event);

	ocl_add_event_to_wait_list(&wait_list, &event);

#if 1
	// step (13)
	// block for read completion
	ocl_finish(OCL_DEFAULT_DEVICE);
#endif

	ocl_wait_for_events(&wait_list);

	ocl_clear_event_wait_list(&wait_list);

	// step (14)
	// add a timer event for the buffer read
	ocl_add_timer("readbuffer", &event);

	// step (15)
	// print results
	for(size_t i= 0; i<ELEMENTS; ++i) {
		fprintf(stderr, "%f\n", h_array[i]);
	} // for
	fprintf(stderr, "\n");

	// step (16)
	// print timer results
	ocl_report_timer("kernel");
	ocl_report_timer("readbuffer");

	double value;
	ocl_read_timer("kernel", OCL_TIMER_QUEUED, &value);
	fprintf(stderr, "Manually read queued value for 'kernel': %lf\n",
		value);
	ocl_read_timer("kernel", OCL_TIMER_INVOCATION, &value);
	fprintf(stderr, "Manually read invocation value for 'kernel': %lf\n",
		value);
	ocl_read_timer("kernel", OCL_TIMER_DURATION, &value);
	fprintf(stderr, "Manually read duration value for 'kernel': %lf\n",
		value);

	fprintf(stderr, "\n");

	// step (17)
	// shutdown the OpenCL layer
	ocl_finalize();

	return 0;
} // main
