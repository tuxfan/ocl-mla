#include <ocl.h>
#include <sandbox_strings.h>

const size_t ELEMENTS = 8;

int main(int argc, char ** argv) {

	/*-------------------------------------------------------------------------*
	 * Initialize OpenCL layer
	 *-------------------------------------------------------------------------*/

	ocl_init();

	/*-------------------------------------------------------------------------*
	 * Create program
	 *-------------------------------------------------------------------------*/

	ocl_add_program(OCL_DEFAULT_DEVICE, "stream",
		stream_reduction_PPSTR, NULL);

	/*-------------------------------------------------------------------------*
	 * Create kernel
	 *-------------------------------------------------------------------------*/

	ocl_add_kernel(OCL_DEFAULT_DEVICE, "stream", "phz_reduce", "reduce");

	/*-------------------------------------------------------------------------*
	 * Create buffers
	 *-------------------------------------------------------------------------*/

	unsigned * values = (unsigned *)malloc(ELEMENTS*sizeof(unsigned));
	//unsigned * count = (unsigned *)malloc(ELEMENTS*sizeof(unsigned));

	for(size_t i=0; i<ELEMENTS; ++i) {
		values[i] = 0;
	} // for

	ocl_create_buffer(OCL_DEFAULT_DEVICE, "values", ELEMENTS*sizeof(unsigned),
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, values);
	ocl_create_buffer(OCL_DEFAULT_DEVICE, "count", ELEMENTS*sizeof(unsigned),
		CL_MEM_READ_WRITE, NULL);


	/*-------------------------------------------------------------------------*
	 * Set kernel arguments
	 *-------------------------------------------------------------------------*/

	ocl_set_kernel_arg_buffer("stream", "reduce", "values", 0);
	ocl_set_kernel_arg_buffer("stream", "reduce", "count", 1);

	/*-------------------------------------------------------------------------*
	 * Enqueue kernel
	 *-------------------------------------------------------------------------*/

	size_t global_offset = 0;
	size_t global_size = ELEMENTS;
	size_t local_size = ELEMENTS;

	ocl_enqueue_kernel_ndrange(OCL_DEFAULT_DEVICE, "stream", "reduce",
		1, &global_offset, &global_size, &local_size, NULL);

	ocl_finish(OCL_DEFAULT_DEVICE);

	ocl_enqueue_read_buffer(OCL_DEFAULT_DEVICE, "values", 1, 0,
		ELEMENTS*sizeof(unsigned), values, NULL);

	for(size_t i=0; i<ELEMENTS; ++i) {
		printf("%d\n", values[i]);
	} // for

	/*-------------------------------------------------------------------------*
	 * Finalize OpenCL layer
	 *-------------------------------------------------------------------------*/

	ocl_finalize();

	return 0;
} // main
