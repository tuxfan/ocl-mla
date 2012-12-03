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

	char * source = NULL;
	ocl_add_from_string(phaze_PPSTR, &source, 0);
	ocl_add_from_string(stream_reduction_PPSTR, &source, 0);

	ocl_add_program(OCL_DEFAULT_DEVICE, "stream",
		source, NULL);

	free(source);

	/*-------------------------------------------------------------------------*
	 * Create kernel
	 *-------------------------------------------------------------------------*/

	ocl_add_kernel(OCL_DEFAULT_DEVICE, "stream", "reduce", "reduce");

	/*-------------------------------------------------------------------------*
	 * Create buffers
	 *-------------------------------------------------------------------------*/

	unsigned * value = (unsigned *)malloc(ELEMENTS*sizeof(unsigned));
	unsigned * offset = (unsigned *)malloc(ELEMENTS*sizeof(unsigned));

	value[0] = 1;
	value[1] = 0;
	value[2] = 1;
	value[3] = 2;
	value[4] = 0;
	value[5] = 3;
	value[6] = 0;
	value[7] = 1;

	ocl_create_buffer(OCL_DEFAULT_DEVICE, "value", ELEMENTS*sizeof(unsigned),
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, value);
	ocl_create_buffer(OCL_DEFAULT_DEVICE, "offset", ELEMENTS*sizeof(unsigned),
		CL_MEM_READ_WRITE, NULL);


	/*-------------------------------------------------------------------------*
	 * Set kernel arguments
	 *-------------------------------------------------------------------------*/

	ocl_set_kernel_arg_buffer("stream", "reduce", "value", 0);
	ocl_set_kernel_arg_buffer("stream", "reduce", "offset", 1);

	/*-------------------------------------------------------------------------*
	 * Enqueue kernel
	 *-------------------------------------------------------------------------*/

	size_t global_offset = 0;
	size_t global_size = ELEMENTS;
	size_t local_size = ELEMENTS;

	ocl_enqueue_kernel_ndrange(OCL_DEFAULT_DEVICE, "stream", "reduce",
		1, &global_offset, &global_size, &local_size, NULL);

	ocl_finish(OCL_DEFAULT_DEVICE);

	ocl_enqueue_read_buffer(OCL_DEFAULT_DEVICE, "offset", 1, 0,
		ELEMENTS*sizeof(unsigned), offset, NULL);

	for(size_t i=0; i<ELEMENTS; ++i) {
		printf("%d\n", offset[i]);
	} // for

	/*-------------------------------------------------------------------------*
	 * Finalize OpenCL layer
	 *-------------------------------------------------------------------------*/

	ocl_finalize();

	return 0;
} // main
