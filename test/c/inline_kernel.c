/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#include <stdio.h>
#include "ocl.h"

const char * kernel = "__kernel void test(__global float * data) {\n\tsize_t gid = get_global_id(0);\n\tdata[gid] = (float)gid;\n}";

int main(int argc, char ** argv) {

	size_t i;

	const int32_t ELEMENTS = 32;
	size_t global_offset = 0;
	size_t global_size = ELEMENTS;
	size_t local_size = 1;
	size_t offset = 0;
	size_t bytes;

	float h_array[ELEMENTS];

	cl_mem d_array;
	ocl_event_t event;
	ocl_event_wait_list_t wait_list;

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
	ocl_create_buffer_raw(OCL_DEFAULT_DEVICE, bytes,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_array, &d_array);

	// step (5)
	// add program and build
	ocl_add_program(OCL_DEFAULT_DEVICE, "program", kernel, "");

	// step (6)
	// add kernel
	ocl_add_kernel(OCL_DEFAULT_DEVICE, "program", "test", "my test");

	// step (7)
	// set kenerl argument
	ocl_set_kernel_arg("program", "my test", 0, sizeof(cl_mem), &d_array);

	// step (8)
	// initialize event for timings
	ocl_initialize_event(&event);
	ocl_initialize_event_wait_list(&wait_list, NULL, 0);

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
	ocl_enqueue_read_buffer_raw(OCL_DEFAULT_DEVICE, d_array, 1, offset,
		ELEMENTS*sizeof(float), h_array, &event);

	ocl_add_event_to_wait_list(&wait_list, &event);

#if 0
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
		fprintf(stdout, "%f\n", h_array[i]);
	} // for
	fprintf(stderr, "\n");

	// step (16)
	// print timer results
	ocl_report_timer("kernel");
	ocl_report_timer("readbuffer");

	// step (17)
	// shutdown the OpenCL layer
	ocl_finalize();

	return 0;
} // main
