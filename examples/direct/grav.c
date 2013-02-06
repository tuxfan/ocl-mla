/*
 ** Plug in routines for General Softening!
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kd.h"
#include "grav.h"
#include "spline.h"

void diaGrav(PARTICLE *p,int bs,int iSoftType)
{
    int i,j;
	double ax,ay,az,dPot,dx,dy,dz;
	double dir,dir3,d2,twoh,h2;

	for (i=1;i<bs;++i) {
		ax = 0.0;
		ay = 0.0;
		az = 0.0;
		dPot = 0.0;

#if defined(ENABLE_OMP)
#pragma omp parallel for
#endif
		for (j=i;j<bs;++j) {
			dx = p[j].r[0] - p[i-1].r[0];
			dy = p[j].r[1] - p[i-1].r[1];
			dz = p[j].r[2] - p[i-1].r[2];
			d2 = dx*dx + dy*dy + dz*dz;
			twoh = p[i-1].fSoft + p[j].fSoft;
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
				break;
				}

			dPot -= p[j].fMass*dir;
			ax += p[j].fMass*dx*dir3;
			ay += p[j].fMass*dy*dir3;
			az += p[j].fMass*dz*dir3;

			p[j].dPot -= p[i-1].fMass*dir;
			p[j].a[0] -= p[i-1].fMass*dx*dir3;
			p[j].a[1] -= p[i-1].fMass*dy*dir3;
			p[j].a[2] -= p[i-1].fMass*dz*dir3;
			}
		p[i-1].a[0] += ax;
		p[i-1].a[1] += ay;
		p[i-1].a[2] += az;
		p[i-1].dPot += dPot;
		}
	}


void blkGrav(PARTICLE *p,int ps,PARTICLE *q,int qs,int iSoftType)
{
    int i,j;
	double ax,ay,az,dPot,dx,dy,dz;
	double dir,dir3,d2,twoh,h2;

	for (i=0;i<ps;++i) {
		ax = 0.0;
		ay = 0.0;
		az = 0.0;
		dPot = 0.0;
		for (j=0;j<qs;++j) {
			dx = q[j].r[0] - p[i].r[0];
			dy = q[j].r[1] - p[i].r[1];
			dz = q[j].r[2] - p[i].r[2];
			d2 = dx*dx + dy*dy + dz*dz;
			twoh = q[j].fSoft + p[i].fSoft;
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
				break;
				}
			dPot -= q[j].fMass*dir;
			q[j].dPot -= p[i].fMass*dir;
			ax += q[j].fMass*dx*dir3;
			ay += q[j].fMass*dy*dir3;
			az += q[j].fMass*dz*dir3;
			q[j].a[0] -= p[i].fMass*dx*dir3;
			q[j].a[1] -= p[i].fMass*dy*dir3;
			q[j].a[2] -= p[i].fMass*dz*dir3;
			}
		p[i].a[0] += ax;
		p[i].a[1] += ay;
		p[i].a[2] += az;
		p[i].dPot += dPot;
		}
	}


void umkGrav(PARTICLE *p,int ps,PARTICLE *q,int qs,int iSoftType)
{
    int i,j;
	double ax,ay,az,dPot,dx,dy,dz;
	double dir,dir3,d2,twoh,h2;

	for (i=0;i<ps;++i) {
		ax = 0.0;
		ay = 0.0;
		az = 0.0;
		dPot = 0.0;
		for (j=0;j<qs;++j) {
			dx = q[j].r[0] - p[i].r[0];
			dy = q[j].r[1] - p[i].r[1];
			dz = q[j].r[2] - p[i].r[2];
			d2 = dx*dx + dy*dy + dz*dz;
			twoh = q[j].fSoft + p[i].fSoft;
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
				break;
				}
			dPot -= q[j].fMass*dir;
			ax += q[j].fMass*dx*dir3;
			ay += q[j].fMass*dy*dir3;
			az += q[j].fMass*dz*dir3;
			}
		p[i].a[0] += ax;
		p[i].a[1] += ay;
		p[i].a[2] += az;
		p[i].dPot += dPot;
		}
	}
