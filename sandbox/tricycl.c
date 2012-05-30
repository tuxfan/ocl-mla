#include <tricycl.h>
#include <ocl.h>
#include <sandbox_strings.h>

size_t tricycl_iterations(size_t elements) {
	size_t iterations = elements/2;
	size_t cnt = 0;

	do {
		++cnt;
		iterations/=2;
	} while(iterations > 1); // do-while

	return cnt;
} // iterations

int32_t tricycl_solve_sp(size_t system_size, size_t num_systems,
	float * sub, float * diag, float * sup, float * rhs, float * x) {
	int32_t ierr = 0;

	/*-------------------------------------------------------------------------*
	 * add program and kernel
	 *-------------------------------------------------------------------------*/

	ocl_add_program(OCL_PERFORMANCE_DEVICE, "tridiag", pcr_kernels_PPSTR,
		"-Dreal_t=float");
	ocl_add_kernel(OCL_PERFORMANCE_DEVICE, "tridiag",
		"pcr_branch_free_kernel", "solve_interface");
	ocl_add_kernel(OCL_PERFORMANCE_DEVICE, "tridiag",
		"pcr_branch_free_kernel", "solve");

	/*-------------------------------------------------------------------------*
	 * compute sizing information for sub-systems
	 *-------------------------------------------------------------------------*/

	// get device information
	ocl_device_info_t device_info;
	ocl_device_info(OCL_PERFORMANCE_DEVICE, &device_info);

	ocl_kernel_hints_t kernel_hints;
	ocl_kernel_hints(OCL_PERFORMANCE_DEVICE, "tridiag", "solve",
		&kernel_hints);

// test
//kernel_hints.max_work_group_size = 16;

	size_t sub_size = kernel_hints.max_work_group_size;
	size_t sub_local_memory = (sub_size+1)*5*sizeof(float);
	size_t sub_systems = 0;
	size_t interface_systems = 1;

	// compute sizes
	if(system_size > kernel_hints.max_work_group_size) {
		while(system_size%sub_size != 0 && sub_size > 1 &&
			sub_local_memory > device_info.local_mem_size) {
			sub_size /= 2;
			sub_local_memory = (sub_size+1)*5*sizeof(float);
		} // while

		if(sub_size == 1) {
			message("System size must be evenly divisible by "
				"a valid work group size");
			exit(1);
		} // if

		sub_systems = system_size/sub_size;
	} // if

	/*-------------------------------------------------------------------------*
	 * compute sizing information for interface system
	 *-------------------------------------------------------------------------*/

	size_t interface_size = 2*sub_systems*num_systems;
	size_t interface_local_memory = (interface_size+1)*5*sizeof(float);

	if(interface_local_memory > device_info.local_mem_size) {
		message("Interface system is too large for device! Unrecoverable!");
		exit(1);
	} // if

	/*-------------------------------------------------------------------------*
	 * setup the interface system
	 *-------------------------------------------------------------------------*/

	float * ia = (float *)malloc(interface_size*sizeof(float));
	float * ib = (float *)malloc(interface_size*sizeof(float));
	float * ic = (float *)malloc(interface_size*sizeof(float));
	float * id = (float *)malloc(interface_size*sizeof(float));
	float * ix = (float *)malloc(interface_size*sizeof(float));

	for(size_t s=0; s<num_systems; ++s) {
		const size_t soff = s*system_size; // input matrix offset
		const size_t lsoff = s*2*sub_systems; // interface matrix offset

		for(size_t r=0; r<sub_systems; ++r) {
			const size_t roff = soff + r*sub_size;
			const size_t lroff = lsoff + 2*r; // interface matrix sub offset

			// set initial in-place temporaries
			ia[lroff+1] = sub[roff+1]; // tmp0
			ib[lroff+1] = diag[roff+1]; // tmp1
			ic[lroff+1] = sup[roff+sub_size-1];
			id[lroff+1] = rhs[roff+1]; // tmp3

			// eliminate interface sub-diagonal
			for(size_t i=2; i<sub_size; ++i) {
				const float ratio = -1.0*sub[roff+i]/ib[lroff+1];

				ia[lroff+1] = ratio*ia[lroff+1];
				ib[lroff+1] = ratio*sup[roff+i-1] + diag[roff+i];
				// ic is static
				id[lroff+1] = ratio*id[lroff+1] + rhs[roff+i];
			} // for

			// set initial in-place temporaries
			ia[lroff] = sub[roff];
			ib[lroff] = diag[roff+sub_size-2];
			ic[lroff] = sup[roff+sub_size-2];
			id[lroff] = rhs[roff+sub_size-2];

			// eliminate interface super-diagonal
			for(size_t i=sub_size-3; i != 0; --i) {
				const float ratio = -1.0*sup[roff+i]/ib[lroff];

				// ia is static
				ib[lroff] = ratio*sub[roff+i+1] + diag[roff+i];
				ic[lroff] = ratio*ic[lroff];
				id[lroff] = ratio*id[lroff] + rhs[roff+i-1];
			} // for
		} // for
	} // for

#if 0
	for(size_t s=0; s<interface_size; ++s) {
		printf("%f %f %f %f\n", ia[s], ib[s], ic[s], id[s]);
	} // for
#endif

	const size_t offset_zero = 0;
	size_t global_size;
	size_t local_size;

/*
	size_t single_indeces;
	ocl_ndrange_hints(system_size, kernel_hints.max_work_group_size,
		0.5, 0.5, &local_size, &global_size, &single_indeces);
*/

	global_size = interface_size;
	local_size = interface_size;
	size_t interface_iterations = tricycl_iterations(interface_size);

	printf("interface iterations: %d\n", (int)interface_iterations);
	printf("interface size: %d\n", (int)interface_size);

	cl_mem d_ia;
	cl_mem d_ib;
	cl_mem d_ic;
	cl_mem d_id;
	cl_mem d_ix;

	// buffers for interface system
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, interface_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ia, &d_ia);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, interface_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ib, &d_ib);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, interface_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ic, &d_ic);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, interface_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, id, &d_id);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, interface_size*sizeof(float),
		CL_MEM_WRITE_ONLY, NULL, &d_ix);

	// arguments for interface system
	ocl_set_kernel_arg("tridiag", "solve_interface", 0, sizeof(cl_mem), &d_ia);
	ocl_set_kernel_arg("tridiag", "solve_interface", 1, sizeof(cl_mem), &d_ib);
	ocl_set_kernel_arg("tridiag", "solve_interface", 2, sizeof(cl_mem), &d_ic);
	ocl_set_kernel_arg("tridiag", "solve_interface", 3, sizeof(cl_mem), &d_id);
	ocl_set_kernel_arg("tridiag", "solve_interface", 4, sizeof(cl_mem), &d_ix);
	ocl_set_kernel_arg("tridiag", "solve_interface", 5,
		(interface_size+1)*5*sizeof(float), NULL);
	ocl_set_kernel_arg("tridiag", "solve_interface", 6, sizeof(int32_t),
		&interface_size);
	ocl_set_kernel_arg("tridiag", "solve_interface", 7, sizeof(int32_t),
		&interface_systems);
	ocl_set_kernel_arg("tridiag", "solve_interface", 8, sizeof(int32_t),
		&interface_iterations);

	ocl_event_t event;

	ocl_initialize_event(&event);

	ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, "tridiag",
		"solve_interface", 1, &offset_zero, &global_size, &local_size, &event);
	
	ocl_finish(OCL_PERFORMANCE_DEVICE);

	ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d_ix, 1, offset_zero,
		interface_size*sizeof(float), ix, &event);

	ocl_finish(OCL_PERFORMANCE_DEVICE);

#if 1
	for(size_t s=0; s<interface_size; ++s) {
		printf("%f\n", ix[s]);
	} // for
#endif

	/*-------------------------------------------------------------------------*
	 * use interface solutions to decouple original system
	 *-------------------------------------------------------------------------*/

	for(size_t s=0; s<num_systems; ++s) {
		const size_t soff = s*system_size; // input matrix offset
		const size_t lsoff = s*2*sub_systems; // interface matrix offset

		for(size_t r=0; r<sub_systems; ++r) {
			const size_t roff = soff + r*sub_size;
			const size_t lroff = lsoff + 2*r; // interface matrix sub offset

			sub[roff] = 0.0;
			diag[roff] = 1.0;
			sup[roff] = 0.0;
			rhs[roff] = ix[lroff];

			sub[roff+sub_size-1] = 0.0;
			diag[roff+sub_size-1] = 1.0;
			sup[roff+sub_size-1] = 0.0;
			rhs[roff+sub_size-1] = ix[lroff+1];
		} // for
	} // for

	free(ia);
	free(ib);
	free(ic);
	free(id);
	free(ix);

for(size_t s=0; s<num_systems; ++s) {
	for(size_t i=0; i<system_size; ++i) {
		printf("%f %f %f %f\n", sub[s*system_size+i], diag[s*system_size+i],
			sup[s*system_size+i], rhs[s*system_size+i]);
	} // for
} // for

	/*-------------------------------------------------------------------------*
	 * use interface solutions to decouple original system
	 *-------------------------------------------------------------------------*/

	ocl_release_buffer(&d_ia);
	ocl_release_buffer(&d_ib);
	ocl_release_buffer(&d_ic);
	ocl_release_buffer(&d_id);
	ocl_release_buffer(&d_ix);

	cl_mem d_a;
	cl_mem d_b;
	cl_mem d_c;
	cl_mem d_d;
	cl_mem d_x;

	size_t full_size = sub_size*sub_systems*num_systems;

	// buffers for full system
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, full_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sub, &d_a);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, full_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, diag, &d_b);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, full_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sup, &d_c);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, full_size*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, rhs, &d_d);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, full_size*sizeof(float),
		CL_MEM_WRITE_ONLY, NULL, &d_x);

	size_t sub_iterations = tricycl_iterations(sub_size);

	// arguments for interface system
	ocl_set_kernel_arg("tridiag", "solve", 0, sizeof(cl_mem), &d_a);
	ocl_set_kernel_arg("tridiag", "solve", 1, sizeof(cl_mem), &d_b);
	ocl_set_kernel_arg("tridiag", "solve", 2, sizeof(cl_mem), &d_c);
	ocl_set_kernel_arg("tridiag", "solve", 3, sizeof(cl_mem), &d_d);
	ocl_set_kernel_arg("tridiag", "solve", 4, sizeof(cl_mem), &d_x);
	ocl_set_kernel_arg("tridiag", "solve", 5,
		(sub_size+1)*5*sizeof(float), NULL);
	ocl_set_kernel_arg("tridiag", "solve", 6, sizeof(int32_t),
		&sub_size);
	ocl_set_kernel_arg("tridiag", "solve", 7, sizeof(int32_t),
		&sub_systems);
	ocl_set_kernel_arg("tridiag", "solve", 8, sizeof(int32_t),
		&sub_iterations);

	global_size = full_size;
	local_size = sub_size;

	ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, "tridiag",
		"solve", 1, &offset_zero, &global_size, &local_size, &event);
	
	ocl_finish(OCL_PERFORMANCE_DEVICE);

	ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d_x, 1, offset_zero,
		full_size*sizeof(float), x, &event);

	ocl_finish(OCL_PERFORMANCE_DEVICE);

	ocl_release_buffer(&d_a);
	ocl_release_buffer(&d_b);
	ocl_release_buffer(&d_c);
	ocl_release_buffer(&d_d);
	ocl_release_buffer(&d_x);

	return ierr;
} // tricycl_solve_sp
