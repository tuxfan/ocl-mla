#include <ocl.h>
#include <tricycl.h>

typedef float real_t;
const char * COMPILE_OPTS = "-Dreal_t=float";

#define SQR(x) (x)*(x)
#define ABS(x, y) (x)-(y) > 0 ? (x)-(y) : (y)-(x)

int main(int argc, char ** argv) {

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <elements> <systems>\n", argv[0]);
		exit(1);
	} // if

	size_t elements = atoi(argv[1]);
	int32_t systems = atoi(argv[2]);

	real_t * sub = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * diag = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * sup = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * rhs = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * x = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t * analytic = (real_t *)malloc(systems*elements*sizeof(real_t));
	real_t x0 = 0.0;
	real_t x1 = 1.0;
	real_t h = (x1-x0)/(real_t)(elements-1);
	real_t hinv2 = 1.0/SQR(h);

	size_t i, s;

	for(s=0; s<systems; ++s) {
		for(i=0; i<elements; ++i) {
			sub[s*elements + i] = hinv2*(-1.0);	
			diag[s*elements + i] = hinv2*(2.0);	
			sup[s*elements + i] = hinv2*(-1.0);	

			analytic[s*elements + i] = 1.0;
			rhs[s*elements + i] = (i==0 || i==(elements-1)) ? 1.0 : 0.0;

			x[s*elements + i] = 0.0;
		} // for

		sub[s*elements] = 0.0;
		sup[s*elements + elements-1] = 0.0;
	} // for

	// initialize OpenCL layer
	ocl_init();

	tricycl_solve_sp(elements, systems, sub, diag, sup, rhs, x);

	// shutdown OpenCL layer
	ocl_finalize();

	return 0;
} // main
