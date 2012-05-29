/*----------------------------------------------------------------------------*
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include <ocl.h>
#include <sandbox_strings.h>

	typedef double real_t;
	const char * COMPILE_OPTS = "-Dreal_t=double";
//	typedef float real_t;
//	const char * COMPILE_OPTS = "-Dreal_t=float";

const real_t PI = 3.14159;
const size_t k = 1;

#define SQR(x) (x)*(x)
#define ABS(x, y) (x)-(y) > 0 ? (x)-(y) : (y)-(x)

int main(int argc, char ** argv) {

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <elements> <systems>\n", argv[0]);
		exit(1);
	} // if

	size_t elements = atoi(argv[1]);
	int32_t systems = atoi(argv[2]);

	real_t * h_sub = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * h_diag = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * h_sup = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * h_rhs = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * h_x = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * analytic = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t x0 = 0.0;
	real_t x1 = 1.0;
	real_t h = (x1-x0)/(real_t)(elements-1);
	real_t hinv2 = 1.0/SQR(h);
	int32_t iterations = 0;

	cl_mem d_sub;
	cl_mem d_diag;
	cl_mem d_sup;
	cl_mem d_rhs;
	cl_mem d_x;

	size_t i, s;

	for(s=0; s<systems; ++s) {
		for(i=0; i<elements; ++i) {
			h_sub[s*elements + i] = hinv2*(-1.0);	
			h_diag[s*elements + i] = hinv2*(2.0);	
			h_sup[s*elements + i] = hinv2*(-1.0);	

			analytic[s*elements + i] = 1.0;
			h_rhs[s*elements + i] = (i==0 || i==(elements-1)) ? 1.0 : 0.0;

			h_x[s*elements + i] = 0.0;
		} // for

		h_sub[s*elements] = 0.0;
		h_sup[s*elements + elements-1] = 0.0;
	} // for

	iterations = elements/2;
	int32_t cnt = 1;

	do {
		++cnt;
		iterations/=2;
	} while (iterations > 1);

	iterations = cnt-1;

	ocl_init();

	ocl_add_program(OCL_PERFORMANCE_DEVICE, "tridiag", pcr_kernels_PPSTR,
		COMPILE_OPTS);
	ocl_add_kernel(OCL_PERFORMANCE_DEVICE, "tridiag",
		"pcr_branch_free_kernel", "solve");

	ocl_kernel_hints_t kernel_hints;
	ocl_kernel_hints(OCL_PERFORMANCE_DEVICE, "tridiag", "solve", &kernel_hints);

	if(elements > kernel_hints.max_work_group_size) {
		fprintf(stderr, "Error: too many elements for device!!!");
		exit(1);
	} // if

	size_t global_offset = 0;
	size_t local_size;
	size_t work_group_indeces;
	size_t single_indeces;

	ocl_ndrange_hints(elements, kernel_hints.max_work_group_size,
		0.5, 0.5, &local_size, &work_group_indeces, &single_indeces);

	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, systems*elements*sizeof(real_t),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_sub, &d_sub);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, systems*elements*sizeof(real_t),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_diag, &d_diag);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, systems*elements*sizeof(real_t),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_sup, &d_sup);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, systems*elements*sizeof(real_t),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_rhs, &d_rhs);
	ocl_create_buffer(OCL_PERFORMANCE_DEVICE, systems*elements*sizeof(real_t),
		CL_MEM_WRITE_ONLY, NULL, &d_x);

	ocl_set_kernel_arg("tridiag", "solve", 0, sizeof(cl_mem), &d_sub);
	ocl_set_kernel_arg("tridiag", "solve", 1, sizeof(cl_mem), &d_diag);
	ocl_set_kernel_arg("tridiag", "solve", 2, sizeof(cl_mem), &d_sup);
	ocl_set_kernel_arg("tridiag", "solve", 3, sizeof(cl_mem), &d_rhs);
	ocl_set_kernel_arg("tridiag", "solve", 4, sizeof(cl_mem), &d_x);
	ocl_set_kernel_arg("tridiag", "solve", 5,
		(elements+1)*5*sizeof(real_t), NULL);
	ocl_set_kernel_arg("tridiag", "solve", 6, sizeof(int32_t), &elements);
	ocl_set_kernel_arg("tridiag", "solve", 7, sizeof(int32_t), &systems);
	ocl_set_kernel_arg("tridiag", "solve", 8, sizeof(int32_t), &iterations);

	ocl_event_t event;

	ocl_initialize_event(&event);

	size_t global_size = systems*elements;

	ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, "tridiag", "solve",
		1, &global_offset, &global_size, &local_size, &event);

	ocl_finish(OCL_PERFORMANCE_DEVICE);

	ocl_add_timer("solve", &event);

	ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d_x, 1, global_offset,
		systems*elements*sizeof(real_t), h_x, &event);

	ocl_add_timer("read", &event);

	double rms = 0.0;
	for(i = 0; i<systems*elements; ++i) {
		real_t abs = ABS(analytic[i], h_x[i]);
		rms += SQR(abs);
	} // for

	rms /= (double)elements;
	rms = sqrt(rms);

	fprintf(stdout, "rms: %e\n", rms);

	ocl_report_timer("solve");
	ocl_report_timer("read");

	ocl_finalize();

	free(h_sub);
	free(h_diag);
	free(h_sup);
	free(h_rhs);
	free(h_x);

	return 0;
} // main
