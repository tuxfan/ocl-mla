/*----------------------------------------------------------------------------*
 * Direct n-body simulation kernels
 *----------------------------------------------------------------------------*/

#ifndef BLSZ
#define BLSZ 64
#endif

#ifndef UNROLL_FACTOR
#define UNROLL_FACTOR 32
#endif

#ifndef EPS2
#define EPS2 .000001f
#endif

#define SQR(x) (x)*(x)
#define CUBE(x) (x)*(x)*(x)

typedef struct position_ {
	float4 b;
} position_t;

typedef struct acceleration_ {
	float4 a;
} acceleration_t;

/*----------------------------------------------------------------------------*
 * This is based on the CUDA GEMS N-Body example.
 * As per Mike Warren's suggestion, 'bi' and 'bj' store position and mass,
 * while 'a' stores acceleration and potential.
 *----------------------------------------------------------------------------*/

STATIC inline float4 interactPlummer(float4 bi, float4 bj, float4 a) {
	float4 r;

#if !defined(USE_CONDITIONAL)
	float4 rref;
	const float4 one = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float4 zero = { 0.0f, 0.0f, 0.0f, 0.0f };

	rref.x = bi.x - bi.x;
	rref.y = bi.y - bi.y;
	rref.z = bi.z - bi.z;
#endif

	r.x = bj.x - bi.x;
	r.y = bj.y - bi.y;
	r.z = bj.z - bi.z;

#if !defined(USE_CONDITIONAL)
	float4 sel = select(zero, one, isnotequal(rref, r));
#endif

	float distSqr = SQR(r.x) + SQR(r.y) + SQR(r.z) + EPS2;
#if defined(USE_RSQRT)
	float dir = rsqrt(distSqr);
#else
	float dir = 1.0f/sqrt(distSqr);
#endif
	float invDistCube = CUBE(dir);

	float s = bj.w * invDistCube;

	a.x += r.x * s; // x acceleration
	a.y += r.y * s; // y acceleration
	a.z += r.z * s; // z acceleration

	// potential energy
#if !defined(USE_CONDITIONAL)
	a.w -= bj.w * dir * sel.x;
#else
	if(distSqr != EPS2) {
		a.w -= bj.w * dir;
	} // if
#endif

	return a;
} // interactPlummer

/*----------------------------------------------------------------------------*
 * Compute all N interactions for a work-group of work-group-size bodies.
 *----------------------------------------------------------------------------*/

__kernel void grav(__global position_t * p, __global acceleration_t * a,
	float gravity, __local float4 * b) {
	size_t blid = get_group_id(0);
	size_t blks = get_num_groups(0);
	size_t thid = get_local_id(0);
	size_t glsz = get_global_size(0);

	size_t offset = blid*BLSZ + thid;
	float4 bi = p[offset].b;
	float4 _a = { 0.0f, 0.0f, 0.0f, 0.0f };
	float4 g = { gravity, gravity, gravity, gravity };

	size_t i, j, tile;

	for(i=0, tile=0; i<glsz; i+=BLSZ, ++tile) {
		size_t idx = tile*BLSZ + thid;

#if defined(USE_LOCAL_MEMORY)
		b[thid] = p[idx].b;
		barrier(CLK_LOCAL_MEM_FENCE);
#endif

		// update tile
#if defined(UNROLL_LOOPS)
#pragma unroll UNROLL_FACTOR
#endif
		for(j=0; j<BLSZ; ++j) {
#if defined(USE_LOCAL_MEMORY)
			_a = interactPlummer(bi, b[j], _a);
#else
			_a = interactPlummer(bi, p[idx].b, _a);
#endif
		} // for

#if defined(USE_LOCAL_MEMORY)
		barrier(CLK_LOCAL_MEM_FENCE);
#endif
	} // for

	a[offset].a = _a*g;
} // Grav

/*
 * vim: set syntax=c : set ts=3 :
 */
