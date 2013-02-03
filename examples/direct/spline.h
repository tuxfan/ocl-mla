#ifndef SPLINE_dihHINCLUDED
#define SPLINE_dihHINCLUDED


/*
 ** see (A1) and (A2) of TREESPH: A UNIFICATION OF SPH WITH THE 
 ** HIERARCHICAL TREE METHOD by Lars HernqSPLINE_uist and Neal Katz.
 ** APJ SSPLINE_upplemant Series 70:416-446, 1989
 ** 
 */
#define SPLINE(r2,twoh,a,b)\
{\
	double SPLINE_r,SPLINE_u,SPLINE_dih,SPLINE_dir;\
	SPLINE_r = sqrt(r2);\
	if (SPLINE_r < twoh) {\
		SPLINE_dih = 2.0/twoh;\
		SPLINE_u = SPLINE_r*SPLINE_dih;\
		if (SPLINE_u < 1.0) {\
			a = SPLINE_dih*(7.0/5.0 - 2.0/3.0*SPLINE_u*SPLINE_u + 3.0/10.0*SPLINE_u*SPLINE_u*SPLINE_u*SPLINE_u\
					 - 1.0/10.0*SPLINE_u*SPLINE_u*SPLINE_u*SPLINE_u*SPLINE_u);\
			b = SPLINE_dih*SPLINE_dih*SPLINE_dih*(4.0/3.0 - 6.0/5.0*SPLINE_u*SPLINE_u + 1.0/2.0*SPLINE_u*SPLINE_u*SPLINE_u);\
			}\
		else {\
			SPLINE_dir = 1.0/SPLINE_r;\
			a = -1.0/15.0*SPLINE_dir + SPLINE_dih*(8.0/5.0 - 4.0/3.0*SPLINE_u*SPLINE_u + SPLINE_u*SPLINE_u*SPLINE_u\
			              - 3.0/10.0*SPLINE_u*SPLINE_u*SPLINE_u*SPLINE_u + 1.0/30.0*SPLINE_u*SPLINE_u*SPLINE_u*SPLINE_u*SPLINE_u);\
			b = -1.0/15.0*SPLINE_dir*SPLINE_dir*SPLINE_dir + SPLINE_dih*SPLINE_dih*SPLINE_dih*(8.0/3.0 - 3.0*SPLINE_u + 6.0/5.0*SPLINE_u*SPLINE_u - 1.0/6.0*SPLINE_u*SPLINE_u*SPLINE_u);\
			}\
		}\
	else {\
		a = 1.0/SPLINE_r;\
		b = a*a*a;\
		}\
	}

#endif
