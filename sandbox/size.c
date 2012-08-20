#include <ocl.h>
#include <sandbox_strings.h>

int main(int argc, char ** argv) {

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <elements> <systems>\n", argv[0]);
		exit(1);
	} // if

	size_t elements = atoi(argv[1]);
//	int32_t systems = atoi(argv[2]);

	ocl_init();

	ocl_add_program(OCL_DEFAULT_DEVICE, "tridiag", pcr_kernels_PPSTR,
		"-Dreal_t=float");

	ocl_add_kernel(OCL_DEFAULT_DEVICE, "tridiag",
		"pcr_branch_free_kernel", "solve");

	float * h_a = (float *)malloc(elements*sizeof(float));
	cl_mem d_a;

	ocl_create_buffer_raw(OCL_DEFAULT_DEVICE, elements*sizeof(float),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_a, &d_a);

	ocl_set_kernel_arg("tridiag", "solve", 0, sizeof(cl_mem), &d_a);
	ocl_set_kernel_arg("tridiag", "solve", 5,
		(elements+1)*5*sizeof(float), NULL);

	ocl_kernel_work_group_info_t info;
	ocl_kernel_work_group_info(OCL_DEFAULT_DEVICE,
		"tridiag", "solve", &info);

	printf("global work size: (%d, %d, %d)\n",
		(int)info.global_work_size[0], (int)info.global_work_size[1],
		(int)info.global_work_size[2]);
	printf("work group size: %d\n", (int)info.work_group_size);
	printf("compiler work group size: (%d, %d, %d)\n",
		(int)info.compile_work_group_size[0],
		(int)info.compile_work_group_size[1],
		(int)info.compile_work_group_size[2]);
	printf("local mem size: %d\n", (int)info.local_mem_size);
	printf("preferred multiple: %d\n", (int)info.preferred_multiple);
	printf("private mem size: %d\n", (int)info.private_mem_size);

	ocl_finalize();

	return 0;
} // main
