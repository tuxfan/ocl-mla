#include <math.h>
#include "kd.h"
#include "ewald.h"
#include "spline.h"


#define NREP	3
#define NHUP	4

#define M_PI 3.14159265359

void EwaldInteract(double dRel[3],double twoh,int iSoftType,double L,
				   double *pphi,double dAccel[3])
{
	double alpha,k5,k6,k7;
	double d2,d,h2,dir,dir3;
	double phi,fx,fy,fz,x,y,z,da,eda,dot;
	double lx,ly,lz,t;
	int nx,ny,nz,n2,ih;
	/*
	 ** I will use these static structures here, although I do not 
	 ** condone this style. It is used here to simplify the interface,
	 ** and could present problems in some more modular designs.
	 */
	static int nTable=0;
	static struct {
		double k2exp;
		double k3exp;
		double lx;
		double ly;
		double lz;
		} hTable[200];

	alpha = 2.0/L;
	if (!nTable) {
		/*
		 ** Calculate the h-loop table!
		 */
		for (nx=-NHUP;nx<=NHUP;++nx) {
			for (ny=-NHUP;ny<=NHUP;++ny) {
				for (nz=-NHUP;nz<=NHUP;++nz) {
					n2 = nx*nx + ny*ny + nz*nz;
					if (n2 > 8) continue;
					if (!n2) continue;
					t = exp(-M_PI*M_PI/(alpha*alpha*L*L)*n2)/n2;
					hTable[nTable].k2exp = 1.0/(M_PI*L)*t; 
					hTable[nTable].k3exp = 2.0/(L*L)*t;
					hTable[nTable].lx = nx;
					hTable[nTable].ly = ny;
					hTable[nTable].lz = nz;
					++nTable;
					}
				}
			}
		}
	k5 = 2.0*M_PI/L;
    k6 = 2.0/sqrt(M_PI);
	k7 = 2.6*L;
	k7 *= k7;
	phi = M_PI/(alpha*alpha*L*L*L);
	fx = 0.0;
	fy = 0.0;
	fz = 0.0;
	for (nx=-NREP;nx<=NREP;++nx) {
		x = dRel[0] - nx*L;
		for (ny=-NREP;ny<=NREP;++ny) {
			y = dRel[1] - ny*L;
			for (nz=-NREP;nz<=NREP;++nz) {
				z = dRel[2] - nz*L;
				d2 = x*x + y*y + z*z;
				if (d2 > k7) continue;
				d = sqrt(d2);
				da = alpha*d;
				eda = erfc(da);
				switch (iSoftType) {
				case SOFT_PLUM:
					dir = 1.0/sqrt(d2 + 0.25*twoh*twoh);
					dir3 = dir*dir*dir;
					break;
				case SOFT_UNI:
					h2 = 0.25*twoh*twoh;
					if (d2 < h2) {
						dir3 = 2.0/(twoh*h2);
						dir = 0.5*dir3*(3.0*h2 - d2);
						}
					else {
						dir = 1.0/sqrt(d2);
						dir3 = dir*dir*dir;
						}
					break;
				default:
				case SOFT_SPLINE:
					SPLINE(d2,twoh,dir,dir3);
					}
				phi -= dir*eda;
				dir3 *= (eda + k6*da*exp(-da*da));
				fx += x*dir3;
				fy += y*dir3;
				fz += z*dir3;
				}
			}
		}
	for (ih=0;ih<nTable;++ih) {
		lx = hTable[ih].lx;
		ly = hTable[ih].ly;
		lz = hTable[ih].lz;
		dot = k5*(lx*dRel[0] + ly*dRel[1] + lz*dRel[2]);
		phi -= hTable[ih].k2exp*cos(dot);
		dir3 = hTable[ih].k3exp*sin(dot);
		fx += lx*dir3;
		fy += ly*dir3;
		fz += lz*dir3;
		}
	*pphi = phi;
	dAccel[0] = fx;
	dAccel[1] = fy;
	dAccel[2] = fz;
	}


void diaEwald(PARTICLE *p,int bs,int iSoftType,double L)
{
    int i,j;
	double ax,ay,az,dPot,phi,f[3],d[3],twoh;

	for (i=1;i<bs;++i) {
		ax = 0.0;
		ay = 0.0;
		az = 0.0;
		dPot = 0.0;
		for (j=i;j<bs;++j) {
			d[0] = p[j].r[0] - p[i-1].r[0];
			d[1] = p[j].r[1] - p[i-1].r[1];
			d[2] = p[j].r[2] - p[i-1].r[2];
			twoh = p[j].fSoft + p[i-1].fSoft;
			EwaldInteract(d,twoh,iSoftType,L,&phi,f);
			dPot += p[j].fMass*phi;
			p[j].dPot += p[i-1].fMass*phi;
			ax += p[j].fMass*f[0];
			ay += p[j].fMass*f[1];
			az += p[j].fMass*f[2];
			p[j].a[0] -= p[i-1].fMass*f[0];
			p[j].a[1] -= p[i-1].fMass*f[1];
			p[j].a[2] -= p[i-1].fMass*f[2];
			}
		p[i-1].a[0] += ax;
		p[i-1].a[1] += ay;
		p[i-1].a[2] += az;
		p[i-1].dPot += dPot;
		}
	}


void blkEwald(PARTICLE *p,int ps,PARTICLE *q,int qs,int iSoftType,double L)
{
    int i,j;
	double ax,ay,az,dPot,phi,f[3],d[3],twoh;

	for (i=0;i<ps;++i) {
		ax = 0.0;
		ay = 0.0;
		az = 0.0;
		dPot = 0.0;
		for (j=0;j<qs;++j) {
			d[0] = q[j].r[0] - p[i].r[0];
			d[1] = q[j].r[1] - p[i].r[1];
			d[2] = q[j].r[2] - p[i].r[2];
			twoh = q[j].fSoft + p[i].fSoft;
			EwaldInteract(d,twoh,iSoftType,L,&phi,f);
			dPot += q[j].fMass*phi;
			q[j].dPot += p[i].fMass*phi;
			ax += q[j].fMass*f[0];
			ay += q[j].fMass*f[1];
			az += q[j].fMass*f[2];
			q[j].a[0] -= p[i].fMass*f[0];
			q[j].a[1] -= p[i].fMass*f[1];
			q[j].a[2] -= p[i].fMass*f[2];
			}
		p[i].a[0] += ax;
		p[i].a[1] += ay;
		p[i].a[2] += az;
		p[i].dPot += dPot;
		}
	}


void umkEwald(PARTICLE *p,int ps,PARTICLE *q,int qs,int iSoftType,double L)
{
    int i,j;
	double ax,ay,az,dPot,phi,f[3],d[3],twoh;

	for (i=0;i<ps;++i) {
		ax = 0.0;
		ay = 0.0;
		az = 0.0;
		dPot = 0.0;
		for (j=0;j<qs;++j) {
			d[0] = q[j].r[0] - p[i].r[0];
			d[1] = q[j].r[1] - p[i].r[1];
			d[2] = q[j].r[2] - p[i].r[2];
			twoh = q[j].fSoft + p[i].fSoft;
			EwaldInteract(d,twoh,iSoftType,L,&phi,f);
			dPot += q[j].fMass*phi;
			ax += q[j].fMass*f[0];
			ay += q[j].fMass*f[1];
			az += q[j].fMass*f[2];
			}
		p[i].a[0] += ax;
		p[i].a[1] += ay;
		p[i].a[2] += az;
		p[i].dPot += dPot;
		}
	}


































