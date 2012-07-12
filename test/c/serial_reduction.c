/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#include <stdio.h>
#include "ocl.h"

const size_t ELEMENTS = 1024;

int main(int argc, char ** argv) {

	/*-------------------------------------------------------------------------*
	 * Variables
	 *-------------------------------------------------------------------------*/

	size_t i;
	size_t global_offset = 0;
	size_t global_size = 1;
	size_t local_size = 1;

	float * h_array = (float *)malloc(ELEMENTS*sizeof(float));

	/*-------------------------------------------------------------------------*
	 * OpenCL memory objects
	 *-------------------------------------------------------------------------*/

	cl_mem d_elements;
	cl_mem d_array;
	cl_mem d_acc;
	ocl_event_t event;

	/*-------------------------------------------------------------------------*
	 * Initialize host data
	 *-------------------------------------------------------------------------*/

	for(i=0; i<ELEMENTS; ++i) {
		h_array[i] = 1.0;
	} // for

	/*-------------------------------------------------------------------------*
	 * Initialize the OpenCL layer
	 *-------------------------------------------------------------------------*/

	ocl_init();

	/*-------------------------------------------------------------------------*
	 * Assemble program source
	 *-------------------------------------------------------------------------*/

	char * performance_source = NULL;
	ocl_add_from_file("T_U.cl", &performance_source, 0);
	ocl_add_from_file("T_RS.cl", &performance_source, 0);

	/*-------------------------------------------------------------------------*
	 * Add programs and compile
	 *-------------------------------------------------------------------------*/

	ocl_add_program(OCL_DEFAULT_DEVICE, "program",
		performance_source, NULL);

	/*-------------------------------------------------------------------------*
	 * Add kernels
	 *-------------------------------------------------------------------------*/

	ocl_add_kernel(OCL_DEFAULT_DEVICE, "program", "reduce_serial", "reduce");

	/*-------------------------------------------------------------------------*
	 * Create device data
	 *-------------------------------------------------------------------------*/

	// create device-side array
	ocl_create_buffer(OCL_DEFAULT_DEVICE, ELEMENTS*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_array, &d_array);

	// create device-side element count
	int elements = ELEMENTS;
	ocl_create_buffer(OCL_DEFAULT_DEVICE, sizeof(int),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &elements, &d_elements);

	// create device-side accumulation
	float acc = 0.0;
	ocl_create_buffer(OCL_DEFAULT_DEVICE, sizeof(float),
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &acc, &d_acc);

	/*-------------------------------------------------------------------------*
	 * Set kernel arguments
	 *-------------------------------------------------------------------------*/

	ocl_set_kernel_arg("program", "reduce", 0, sizeof(cl_mem), &d_elements);
	ocl_set_kernel_arg("program", "reduce", 1, sizeof(cl_mem), &d_array);
	ocl_set_kernel_arg("program", "reduce", 2, sizeof(cl_mem), &d_acc);

	/*-------------------------------------------------------------------------*
	 * Execute main reduction
	 *-------------------------------------------------------------------------*/

	// initialize event for timings
	ocl_initialize_event(&event);

	/*-------------------------------------------------------------------------*
	 * Execute the serial reduction
	 *-------------------------------------------------------------------------*/

	ocl_enqueue_kernel_ndrange(OCL_DEFAULT_DEVICE, "program", "reduce",
		1, &global_offset, &global_size, &local_size, &event);

	// block for execution completion
	ocl_finish(OCL_DEFAULT_DEVICE);

	/*-------------------------------------------------------------------------*
	 * Read accumulation from the device
	 *-------------------------------------------------------------------------*/

	ocl_enqueue_read_buffer(OCL_DEFAULT_DEVICE, d_acc, 1, global_offset,
		sizeof(float), &acc, &event);

	// block for read completion
	ocl_finish(OCL_DEFAULT_DEVICE);

	/*-------------------------------------------------------------------------*
	 * Print the results
	 *-------------------------------------------------------------------------*/

	printf("acc: %f\n\n", acc);

	/*-------------------------------------------------------------------------*
	 * Cleanup
	 *-------------------------------------------------------------------------*/

	free(performance_source);
	free(h_array);

	/*-------------------------------------------------------------------------*
	 * Shutdown the OpenCL layer
	 *-------------------------------------------------------------------------*/

	ocl_finalize();

	return 0;
} // main
