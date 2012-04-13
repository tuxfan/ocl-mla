/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#include <stdio.h>
#include "ocl.h"

#define SIMD_SIZE 4

//const size_t ELEMENTS = 1024*1024*SIMD_SIZE;
const size_t ELEMENTS = 4*1024*SIMD_SIZE;

int main(int argc, char ** argv) {

	/*-------------------------------------------------------------------------*
	 * Variables
	 *-------------------------------------------------------------------------*/

	size_t i;
	size_t global_offset = 0;
	size_t global_size = ELEMENTS/SIMD_SIZE;
	size_t max_work_group_size = 0;
	size_t offset = 0;

	float * h_array = (float *)malloc(ELEMENTS*sizeof(float));
	float * h_acc_wg = NULL;

	/*-------------------------------------------------------------------------*
	 * OpenCL memory objects
	 *-------------------------------------------------------------------------*/

	cl_mem a_elements;
	cl_mem p_array;
	cl_mem p_acc_wg;
	cl_mem a_acc_wg;
	cl_mem a_acc;
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
	 * Get maximum workgroup size for the performance device
	 *-------------------------------------------------------------------------*/

	ocl_max_work_group_size(OCL_PERFORMANCE_DEVICE, &max_work_group_size);

	/*-------------------------------------------------------------------------*
	 * Assemble program source
	 *-------------------------------------------------------------------------*/

	char * performance_source = NULL;
	ocl_add_from_file("T_U.cl", &performance_source, 0);
	ocl_add_from_file("T_RDP.cl", &performance_source, 0);

	char * auxiliary_source = NULL;
	ocl_add_from_file("T_U.cl", &auxiliary_source, 0);
	ocl_add_from_file("T_RS.cl", &auxiliary_source, 0);

	/*-------------------------------------------------------------------------*
	 * Add programs and compile
	 *-------------------------------------------------------------------------*/

	ocl_add_program(OCL_PERFORMANCE_DEVICE, "program", performance_source,
		NULL);
	ocl_add_program(OCL_AUXILIARY_DEVICE, "aux", auxiliary_source, NULL);

	/*-------------------------------------------------------------------------*
	 * Add kernels
	 *-------------------------------------------------------------------------*/

	ocl_add_kernel(OCL_PERFORMANCE_DEVICE, "program", "reduce_data_parallel",
		"reduce");
	ocl_add_kernel(OCL_AUXILIARY_DEVICE, "aux", "reduce_serial", "reduce");

	/*-------------------------------------------------------------------------*
	 * Get hints on executing data parallel kernel
	 *-------------------------------------------------------------------------*/

	size_t work_group_size = 0, work_group_elements = 0, single_elements = 0;
	const size_t work_group_single = 1;

	// get a tighter bound on the max work group size
	ocl_kernel_hint("program", "reduce", &max_work_group_size);

	ocl_ndrange_hints(global_size, max_work_group_size, &work_group_size,
		&work_group_elements, &single_elements);

	// check size
	if(single_elements > 0) {
		error("Size must be a multiple of 2!!! size = %d\n", (int)global_size);
		exit(1);
	} // if

	size_t work_groups = work_group_elements/work_group_size;
	h_acc_wg = (float *)malloc(work_groups*sizeof(float));

	/*-------------------------------------------------------------------------*
	 * Create device data
	 *-------------------------------------------------------------------------*/

	// create device-side array
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, ELEMENTS*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_array, &p_array);

	// create device-side work group accumulation array
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, work_groups*sizeof(float),
		CL_MEM_READ_WRITE, NULL, &p_acc_wg);

	// create device-side element count
	ocl_create_buffer(OCL_AUXILIARY_DEVICE, sizeof(int),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &work_groups, &a_elements);

	// create device-side work group accumulation array
	ocl_create_buffer(OCL_AUXILIARY_DEVICE, work_groups*sizeof(float),
		CL_MEM_READ_WRITE, NULL, &a_acc_wg);

	// create device-side accumulation
	ocl_create_buffer(OCL_AUXILIARY_DEVICE, sizeof(float),
		CL_MEM_WRITE_ONLY, NULL, &a_acc);

	/*-------------------------------------------------------------------------*
	 * Set kernel arguments
	 *-------------------------------------------------------------------------*/

	ocl_set_kernel_arg("program", "reduce", 0, sizeof(cl_mem), &p_array);
	ocl_set_kernel_arg("program", "reduce", 1,
		work_group_size*sizeof(float), NULL);
	ocl_set_kernel_arg("program", "reduce", 2, sizeof(cl_mem), &p_acc_wg);

	ocl_set_kernel_arg("aux", "reduce", 0, sizeof(cl_mem), &a_elements);
	ocl_set_kernel_arg("aux", "reduce", 1, sizeof(cl_mem), &a_acc_wg);
	ocl_set_kernel_arg("aux", "reduce", 2, sizeof(cl_mem), &a_acc);

	/*-------------------------------------------------------------------------*
	 * Execute main reduction
	 *-------------------------------------------------------------------------*/

	// initialize event for timings
	ocl_initialize_event(&event);

	// invoke kernel using preferred work group size
	if(work_group_elements > 0) {
		ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, "program",
			"reduce", 1, &global_offset, &work_group_elements,
			&work_group_size, &event);
	} // if

	// block for kernel completion
	ocl_finish(OCL_PERFORMANCE_DEVICE);

	// add a timer event for the kernel invocation
	ocl_add_timer("kernel", &event);

	/*-------------------------------------------------------------------------*
	 * Move data from performance device to auxiliary device
	 *-------------------------------------------------------------------------*/

	ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, p_acc_wg,
		OCL_SYNCHRONOUS, offset, work_groups*sizeof(float),
		h_acc_wg, &event);	

	ocl_finish(OCL_PERFORMANCE_DEVICE);

	ocl_enqueue_write_buffer(OCL_AUXILIARY_DEVICE, a_acc_wg,
		OCL_SYNCHRONOUS, offset, work_groups*sizeof(float),
		h_acc_wg, &event);	

	ocl_finish(OCL_AUXILIARY_DEVICE);

	/*-------------------------------------------------------------------------*
	 * Execute the serial reduction
	 *-------------------------------------------------------------------------*/

	global_offset = 0;
	global_size = 1;
	ocl_enqueue_kernel_ndrange(OCL_AUXILIARY_DEVICE, "aux",
		"reduce", 1, &global_offset, &global_size,
		&work_group_single, &event);

	/*-------------------------------------------------------------------------*
	 * Read the final accumulation from the auxiliary device
	 *-------------------------------------------------------------------------*/

	float acc = 0.0;
	ocl_enqueue_read_buffer(OCL_AUXILIARY_DEVICE, a_acc, 1, offset,
		sizeof(float), &acc, &event);

	// block for read completion
	ocl_finish(OCL_AUXILIARY_DEVICE);

	// add a timer event for the buffer read
	ocl_add_timer("readbuffer", &event);

	/*-------------------------------------------------------------------------*
	 * Print the results
	 *-------------------------------------------------------------------------*/

	printf("acc: %f\n\n", acc);

	// print timer results
	ocl_report_timer("kernel");
	ocl_report_timer("readbuffer");

	/*-------------------------------------------------------------------------*
	 * Cleanup
	 *-------------------------------------------------------------------------*/

	free(h_acc_wg);
	free(performance_source);
	free(auxiliary_source);
	free(h_array);

	/*-------------------------------------------------------------------------*
	 * Shutdown the OpenCL layer
	 *-------------------------------------------------------------------------*/

	ocl_finalize();

	return 0;
} // main
