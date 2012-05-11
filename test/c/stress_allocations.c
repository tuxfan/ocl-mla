/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#include <stdio.h>
#include "ocl.h"

const size_t ITERATIONS = 50000;

// must be power of 2
const size_t ELEMENTS = 1024;

// message frequency
const size_t OUTPUT = 10000;

const char * kernel =
"__kernel void test(__global float * d0, __global float * d1,\n"
"	__global float * d2, __global float * d3) {\n"
"	size_t gid = get_global_id(0);\n"
"	d0[gid] = (float)gid;\n"
"	d1[gid] = (float)(gid*2);\n"
"	d2[gid] = (float)(gid*3);\n"
"	d3[gid] = (float)(gid*4);\n"
"}";

int main(int argc, char ** argv) {
	size_t i, e;
	size_t global_offset = 0;
	size_t global_size = ELEMENTS;
	size_t local_size;
	size_t work_group_elements;
	size_t single_elements;

	float * h0_array = (float *)malloc(ELEMENTS*sizeof(float));	
	float * h1_array = (float *)malloc(ELEMENTS*sizeof(float));	
	float * h2_array = (float *)malloc(ELEMENTS*sizeof(float));	
	float * h3_array = (float *)malloc(ELEMENTS*sizeof(float));	

	cl_mem d0_array;
	cl_mem d1_array;
	cl_mem d2_array;
	cl_mem d3_array;
	ocl_event_t event;
	ocl_event_wait_list_t wait_list;
	ocl_kernel_hints_t hints;

	ocl_init();

	ocl_add_program(OCL_PERFORMANCE_DEVICE, "program", kernel, NULL);

	ocl_add_kernel(OCL_PERFORMANCE_DEVICE, "program", "test", "test");

	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, ELEMENTS*sizeof(float),
		CL_MEM_READ_ONLY, NULL, &d0_array);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, ELEMENTS*sizeof(float),
		CL_MEM_READ_ONLY, NULL, &d1_array);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, ELEMENTS*sizeof(float),
		CL_MEM_READ_ONLY, NULL, &d2_array);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, ELEMENTS*sizeof(float),
		CL_MEM_READ_ONLY, NULL, &d3_array);

	ocl_set_kernel_arg("program", "test", 0, sizeof(cl_mem), &d0_array);
	ocl_set_kernel_arg("program", "test", 1, sizeof(cl_mem), &d1_array);
	ocl_set_kernel_arg("program", "test", 2, sizeof(cl_mem), &d2_array);
	ocl_set_kernel_arg("program", "test", 3, sizeof(cl_mem), &d3_array);

	ocl_kernel_hints("program", "test", &hints);
	ocl_ndrange_hints(global_size, hints.max_work_group_size, 0.5, 0.5,
		&local_size, &work_group_elements, &single_elements);

	for(i=0; i<ITERATIONS; ++i) {
		if(i%OUTPUT == 0) {
			printf("iteration: %d allocations: %d\n",
				(int)i, (int)num_allocations());
		} // if

		// initialize data
		for(e=0; e<ELEMENTS; ++e) {
			h0_array[e] = 0.0;
			h1_array[e] = 0.0;
			h2_array[e] = 0.0;
			h3_array[e] = 0.0;
		} // for

		ocl_initialize_event(&event);
		ocl_initialize_event_wait_list(&wait_list, NULL, 0);

		// write
		ocl_enqueue_write_buffer(OCL_PERFORMANCE_DEVICE, d0_array,
			0, 0, ELEMENTS*sizeof(float), h0_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_enqueue_write_buffer(OCL_PERFORMANCE_DEVICE, d1_array,
			0, 0, ELEMENTS*sizeof(float), h1_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_enqueue_write_buffer(OCL_PERFORMANCE_DEVICE, d2_array,
			0, 0, ELEMENTS*sizeof(float), h2_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_enqueue_write_buffer(OCL_PERFORMANCE_DEVICE, d3_array,
			0, 0, ELEMENTS*sizeof(float), h3_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_wait_for_events(&wait_list);
		ocl_clear_event_wait_list(&wait_list);

		// kernel
		ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, "program",
			"test", 1, &global_offset, &global_size, &local_size, &event);

		ocl_finish(OCL_PERFORMANCE_DEVICE);

		// read
		ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d0_array,
			0, 0, ELEMENTS*sizeof(float), h0_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d1_array,
			0, 0, ELEMENTS*sizeof(float), h1_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d2_array,
			0, 0, ELEMENTS*sizeof(float), h2_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d3_array,
			0, 0, ELEMENTS*sizeof(float), h3_array, &event);
		ocl_add_event_to_wait_list(&wait_list, &event);

		ocl_wait_for_events(&wait_list);
		ocl_clear_event_wait_list(&wait_list);

		for(e=0; e<ELEMENTS; ++e) {
			if(h0_array[e] != (float)e ||
				h1_array[e] != (float)(e*2) ||
				h2_array[e] != (float)(e*3) ||
				h3_array[e] != (float)(e*4)) {
				fprintf(stderr, "Bad value!");
				exit(1);
			} // if
		} // for
	} // for

	ocl_finalize();

	free(h0_array);
	free(h1_array);
	free(h2_array);
	free(h3_array);

	return 0;
} // main
