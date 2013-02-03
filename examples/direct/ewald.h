#ifndef EWALD_HINCLUDED
#define EWALD_HINCLUDED

#include "kd.h"

void diaEwald(PARTICLE *,int,int,double);
void blkEwald(PARTICLE *,int,PARTICLE *,int,int,double);
void umkEwald(PARTICLE *,int,PARTICLE *,int,int,double);

#endif
