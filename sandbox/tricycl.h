#ifndef tricycl_h
#define tricycl_h

#include <stdlib.h>
#include <stdint.h>

size_t tricycl_iterations(size_t elements);

int32_t tricycl_solve_sp(size_t system_size, size_t num_systems,
	const float * sub, const float * diag, const float * sup,
	const float * rhs, const float * x);

#endif // tricycl_h
