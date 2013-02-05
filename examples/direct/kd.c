#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include "kd.h"
#include "grav.h"
#include "ewald.h"
#include "tipsydefs.h"


void kdTime(KD kd,int *puSecond,int *puMicro)
{
	struct rusage ru;

	getrusage(0,&ru);
	*puMicro = ru.ru_utime.tv_usec - kd->uMicro;
	*puSecond = ru.ru_utime.tv_sec - kd->uSecond;
	if (*puMicro < 0) {
		*puMicro += 1000000;
		*puSecond -= 1;
		}
	kd->uSecond = ru.ru_utime.tv_sec;
	kd->uMicro = ru.ru_utime.tv_usec;
	}


int kdInit(KD *pkd,double G,float *fPeriod,float *fCenter,
		   int iChkptInterval,char *pszChkptName)
{
	KD kd;
	int j;

	kd = (KD)malloc(sizeof(struct kdContext));
	assert(kd != NULL);
	kd->G = G;
	kd->pszChkptName = (char *)malloc(strlen(pszChkptName)+1);
	assert(kd->pszChkptName != NULL);
	strcpy(kd->pszChkptName,pszChkptName);
	kd->iChkptInterval = iChkptInterval;
	for (j=0;j<3;++j) {
		kd->fPeriod[j] = fPeriod[j];
		kd->fCenter[j] = fCenter[j];
		}
	*pkd = kd;
	return(1);
	}


void kdFinish(KD kd)
{
	free(kd->pszChkptName);
	free(kd->p);
	free(kd);
	}


int kdReadTipsy(KD kd,FILE *fp,int bGas,int bDark,int bStar)
{
	int i,j,nCnt;
	struct dump h;
	struct gas_particle gp;
	struct dark_particle dp;
	struct star_particle sp;

	fread(&h,sizeof(struct dump),1,fp);
	kd->nParticles = h.nbodies;
	kd->nDark = h.ndark;
	kd->nGas = h.nsph;
	kd->nStar = h.nstar;
	kd->fTime = h.time;
	kd->nActive = 0;
	if (bDark) kd->nActive += kd->nDark;
	if (bGas) kd->nActive += kd->nGas;
	if (bStar) kd->nActive += kd->nStar;
	kd->nMark = kd->nActive;
	kd->bDark = bDark;
	kd->bGas = bGas;
	kd->bStar = bStar;
	/*
	 ** Allocate particles.
	 */
	kd->p = (PARTICLE *)malloc(kd->nActive*sizeof(PARTICLE));
	assert(kd->p != NULL);

	printf("%d active particles\n", kd->nActive);

#if defined(ENABLE_OCL)
	/*
	 ** Allocate device buffers.
	 */
	kd->h_p = (position_t *)malloc(kd->nActive*sizeof(position_t));
	kd->h_a = (acceleration_t *)malloc(kd->nActive*sizeof(acceleration_t));

	/*
	 ** Allocate device buffers.
	 */
	ocl_create_buffer_raw(OCL_PERFORMANCE_DEVICE,
		kd->nActive*sizeof(position_t), CL_MEM_READ_ONLY, NULL, &kd->d_p);

	ocl_create_buffer_raw(OCL_PERFORMANCE_DEVICE,
		kd->nActive*sizeof(acceleration_t), CL_MEM_WRITE_ONLY, NULL, &kd->d_a);

	size_t max_work_group_size = 0;
	ocl_max_work_group_size(OCL_PERFORMANCE_DEVICE, &max_work_group_size);

#define SIMD_SIZE 4

	//double d_gravity = kd->G;
	float d_gravity = kd->G;
	ocl_set_kernel_arg("direct", "grav", 0, sizeof(cl_mem), &kd->d_p);
	ocl_set_kernel_arg("direct", "grav", 1, sizeof(double), &kd->d_a);
	ocl_set_kernel_arg("direct", "grav", 2, sizeof(float), &d_gravity);
	ocl_set_kernel_arg("direct", "grav", 3,
		kd->local_size*SIMD_SIZE*sizeof(float), NULL);
#endif

#define EPS 1.0e-3

	/*
	 ** Read Stuff!
	 */
	nCnt = 0;
	for (i=0;i<h.nsph;++i) {
		fread(&gp,sizeof(struct gas_particle),1,fp);
		if (bGas) {
			kd->p[nCnt].fMass = gp.mass;
			kd->p[nCnt].fSoft = gp.hsmooth == 0 ? EPS : gp.hsmooth;
			kd->p[nCnt].iOrder = nCnt;
			kd->p[nCnt].iMark = 1;
			for (j=0;j<3;++j) kd->p[nCnt].r[j] = gp.pos[j];

#if defined(ENABLE_OCL)
			kd->h_p[nCnt].x = dp.pos[0]; // x position
			kd->h_p[nCnt].y = dp.pos[1]; // y position
			kd->h_p[nCnt].z = dp.pos[2]; // z position
			kd->h_p[nCnt].m = dp.mass; // mass

			kd->h_a[nCnt].x = 0.0; // x acceleration
			kd->h_a[nCnt].y = 0.0; // y acceleration
			kd->h_a[nCnt].z = 0.0; // z acceleration
			kd->h_a[nCnt].p = 0.0; // potential
#endif

			++nCnt;
			}
		}
	for (i=0;i<h.ndark;++i) {
		fread(&dp,sizeof(struct dark_particle),1,fp);
		if (bDark) {
			kd->p[nCnt].fMass = dp.mass;
			kd->p[nCnt].fSoft = dp.eps == 0 ? EPS : dp.eps;
			kd->p[nCnt].iOrder = nCnt;
			kd->p[nCnt].iMark = 1;
			for (j=0;j<3;++j) kd->p[nCnt].r[j] = dp.pos[j];

#if defined(ENABLE_OCL)
			kd->h_p[nCnt].x = dp.pos[0]; // x position
			kd->h_p[nCnt].y = dp.pos[1]; // y position
			kd->h_p[nCnt].z = dp.pos[2]; // z position
			kd->h_p[nCnt].m = dp.mass; // mass

			kd->h_a[nCnt].x = 0.0; // x acceleration
			kd->h_a[nCnt].y = 0.0; // y acceleration
			kd->h_a[nCnt].z = 0.0; // z acceleration
			kd->h_a[nCnt].p = 0.0; // potential
#endif

			++nCnt;
			}
		}
	for (i=0;i<h.nstar;++i) {
		fread(&sp,sizeof(struct star_particle),1,fp);
		if (bStar) {
			kd->p[nCnt].fMass = sp.mass;
			kd->p[nCnt].fSoft = sp.eps == 0 ? EPS : sp.eps;
			kd->p[nCnt].iOrder = nCnt;
			kd->p[nCnt].iMark = 1;
			for (j=0;j<3;++j) kd->p[nCnt].r[j] = sp.pos[j];

#if defined(ENABLE_OCL)
			kd->h_p[nCnt].x = dp.pos[0]; // x position
			kd->h_p[nCnt].y = dp.pos[1]; // y position
			kd->h_p[nCnt].z = dp.pos[2]; // z position
			kd->h_p[nCnt].m = dp.mass; // mass

			kd->h_a[nCnt].x = 0.0; // x acceleration
			kd->h_a[nCnt].y = 0.0; // y acceleration
			kd->h_a[nCnt].z = 0.0; // z acceleration
			kd->h_a[nCnt].p = 0.0; // potential
#endif

			++nCnt;
			}
		}
	return(kd->nParticles);
	}


void kdSetSoft(KD kd,float fSoft)
{
	int i;
	
	for (i=0;i<kd->nActive;++i) {
		kd->p[i].fSoft = fSoft;
		}
	}


void kdInMark(KD kd,char *pszFile)
{
	FILE *fp;
	char ach[80];
	int i,iCnt,iDum;

	fp = fopen(pszFile,"r");
	if (!fp) {
		fprintf(stderr,"Could not open mark array, %s\n",pszFile);
		exit(1);
		}
	fgets(ach,80,fp);	/* ignore the array header! */
	iCnt = 0;
	for (i=0;i<kd->nGas;++i) {
		if (kd->bGas) {
			fscanf(fp,"%d",&kd->p[iCnt++].iMark);
			}
		else fscanf(fp,"%d",&iDum);
		}
	for (i=0;i<kd->nDark;++i) {
		if (kd->bDark) {
			fscanf(fp,"%d",&kd->p[iCnt++].iMark);
			}
		else fscanf(fp,"%d",&iDum);
		}
	for (i=0;i<kd->nStar;++i) {
		if (kd->bStar) {
			fscanf(fp,"%d",&kd->p[iCnt++].iMark);
			}
		else fscanf(fp,"%d",&iDum);
		}
	fclose(fp);
	}


void kdMarkOrder(KD kd)
{
	PARTICLE *p,t;
	int i,j;

	p = kd->p;
	i = 0;
	j = kd->nActive-1;
	while (i < j) {
		while (p[i].iMark) if (++i > j) goto done;
		while (!p[j].iMark) if (i > --j) goto done;
		t = p[i];
		p[i] = p[j];
		p[j] = t; 
		}
 done:
	kd->nMark = i;
	}


int cmpParticles(const void *v1,const void *v2)
{
	PARTICLE *p1=(PARTICLE *)v1,*p2=(PARTICLE *)v2;
	
	return(p1->iOrder - p2->iOrder);
	}


void kdOrder(KD kd)
{
	qsort(kd->p,kd->nActive,sizeof(PARTICLE),cmpParticles);
	}


/*
 ** A simple driver. No support for check-pointing here!
 */
void kdGravSimple(KD kd,int iSoftType,int bPeriodic)
{
	PARTICLE *p;
	int i,n;

	p = kd->p;
	n = kd->nActive;
	for (i=0;i<n;++i) {
		p[i].a[0] = 0.0;
		p[i].a[1] = 0.0;
		p[i].a[2] = 0.0;
		p[i].dPot = 0.0;
		}
	/*
	 ** Process one huge diagonal Block!
	 */
	if (bPeriodic) {
		diaEwald(p,n,iSoftType,kd->fPeriod[0]);
		}
	else {
		diaGrav(p,n,iSoftType);
		}
	for (i=0;i<n;++i) {
		p[i].a[0] *= kd->G;
		p[i].a[1] *= kd->G;
		p[i].a[2] *= kd->G;
		p[i].dPot *= kd->G;
		}
	}


void WriteChkpt(KD kd,int lBlock,int kBlock)
{
 	FILE *fp;
	struct rusage ru;
	struct chkptHeader h;
	int j;
	
	h.nParticles = kd->nParticles;
	h.nGas = kd->nGas;
	h.nDark = kd->nDark;
	h.nStar = kd->nStar;
	h.bGas = kd->bGas;
	h.bDark = kd->bDark;
	h.bStar = kd->bStar;
	h.nActive = kd->nActive;
	h.nMark = kd->nMark;
	h.G = kd->G;
	for (j=0;j<3;++j) {
		h.fPeriod[j] = kd->fPeriod[j];
		h.fCenter[j] = kd->fCenter[j];
		}
	h.iSoftType = kd->iSoftType;
	h.bPeriodic = kd->bPeriodic;
	h.iBlockSize = kd->iBlockSize;
	/*
	 ** Store time used up to now.
	 */
	getrusage(0,&ru);
	h.uMicro = ru.ru_utime.tv_usec - kd->uMicro;
	h.uSecond = ru.ru_utime.tv_sec - kd->uSecond;
	if (h.uMicro < 0) {
		h.uMicro += 1000000;
		h.uSecond -= 1;
		}
	h.lBlock = lBlock;
	h.kBlock = kBlock;
	fp = fopen(kd->pszChkptName,"wb");
	assert(fp != NULL);
	fwrite(&h,sizeof(struct chkptHeader),1,fp);
	fwrite(kd->p,sizeof(PARTICLE),kd->nActive,fp);
	fclose(fp);
	}


void ReadChkpt(KD kd,int *plBlock,int *pkBlock)
{
	FILE *fp;
	struct rusage ru;
	struct chkptHeader h;
	int j;

	fp = fopen(kd->pszChkptName,"rb");
	if (fp == NULL) {
		fprintf(stderr,"Sorry the check point file %s could not be opened\n",
				kd->pszChkptName);
		kdFinish(kd);
		exit(1);
		}
	fread(&h,sizeof(struct chkptHeader),1,fp);
	kd->nParticles = h.nParticles;
	kd->nGas = h.nGas;
	kd->nDark = h.nDark;
	kd->nStar = h.nStar;
	kd->bGas = h.bGas;
	kd->bDark = h.bDark;
	kd->bStar = h.bStar;
	kd->nActive = h.nActive;
	kd->nMark = h.nMark;
	kd->G = h.G;
	for (j=0;j<3;++j) {
		kd->fPeriod[j] = h.fPeriod[j];
		kd->fCenter[j] = h.fCenter[j];
		}
	kd->iSoftType = h.iSoftType;
	kd->bPeriodic = h.bPeriodic;
	kd->iBlockSize = h.iBlockSize;
	/*
	 ** Fix up CPU time.
	 */
	getrusage(0,&ru);
	kd->uMicro = ru.ru_utime.tv_usec - h.uMicro;
	kd->uSecond = ru.ru_utime.tv_sec - h.uSecond;
	if (kd->uMicro < 0) {
		kd->uMicro += 1000000;
		kd->uSecond -= 1;
		}
	/*
	 ** Allocate particles.
	 */
	kd->p = (PARTICLE *)malloc(kd->nActive*sizeof(PARTICLE));
	assert(kd->p != NULL);
	fread(kd->p,sizeof(PARTICLE),kd->nActive,fp);
	*plBlock = h.lBlock;
	*pkBlock = h.kBlock;
	fclose(fp);
	}


void print_particle(const position_t * p, const acceleration_t * a) {
	printf("mass: %f\n", p->m);
	printf("position: (%f, %f, %f)\n", p->x, p->y, p->z);
	printf("acceleration: (%f, %f, %f)\n", a->x, a->y, a->z);
	printf("potential energy: %f\n", a->p);
} // print_particle


/*
 ** Driver routine for direct. With support for check points!
 */
void Grav(KD kd,int lBlock,int kBlock,int iSoftType,int bPeriodic,
		  int bVerbose)
{
	PARTICLE *p,*q;
#if defined(ENABLE_OCL)
	position_t * h_p = kd->h_p;
	acceleration_t * h_a = kd->h_a;
#endif
	int k,l,n,nk,nb,nbk,bs,rs,rsk,iStride,bc,i;

// FIXME: bVerbose
bVerbose = 0;

	p = kd->p;
	n = kd->nMark;
	q = &kd->p[n];
	nk = kd->nActive - kd->nMark;
	bs = kd->iBlockSize;
	nb = n/bs;
	rs = n%bs;
	nbk = nk/bs;
	rsk = nk%bs;
	iStride = 16384/bs;
	if (bPeriodic) iStride = iStride/100;
	if (!iStride) iStride = 1;
	if (lBlock || kBlock) {
		l = lBlock;
		k = kBlock;
		bc = 1;
		if (k < nb) goto Restart1;
		else {
			k -= nb;
			goto Restart2;
			}
		}
	for (i=0;i<n;++i) {
		p[i].a[0] = 0.0;
		p[i].a[1] = 0.0;
		p[i].a[2] = 0.0;
		p[i].dPot = 0.0;

#if defined(ENABLE_OCL)
		h_a[i].x = 0.0;
		h_a[i].y = 0.0;
		h_a[i].z = 0.0;
		h_a[i].p = 0.0;
#endif
		}

#if defined(ENABLE_OCL)
	ocl_event_t event;
	ocl_initialize_event(&event);

	ocl_host_initialize_timer("aggregate");
	ocl_host_start_timer("aggregate");

	// write particle data to device
	ocl_enqueue_write_buffer_raw(OCL_PERFORMANCE_DEVICE, kd->d_p,
		OCL_SYNCHRONOUS, 0, n*sizeof(position_t), h_p, &event);

	ocl_add_timer("write", &event);

	kd->global_offset = 0;
	kd->global_size = n;

	// update particles
	ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, "direct",
		"grav", 1, &kd->global_offset, &kd->global_size,
		&kd->local_size, &event);

	ocl_finish(OCL_PERFORMANCE_DEVICE);

	ocl_add_timer("execute", &event);

	// read particle data from device
	ocl_enqueue_read_buffer_raw(OCL_PERFORMANCE_DEVICE, kd->d_a,
		OCL_SYNCHRONOUS, 0, n*sizeof(acceleration_t), h_a, &event);

	ocl_add_timer("read", &event);
	ocl_host_stop_timer("aggregate");
#if 0
	printf("post\n");
	for(k=0; k<4; ++k) {
		print_particle(&h_p[k], &h_a[k]);
	} // for
#endif

	ocl_report_timer("write");
	ocl_report_timer("execute");
	ocl_report_timer("read");
	ocl_host_report_timer("aggregate");

	exit(1);
#endif

	/*
	 ** First do all the diagonal blocks.
	 */
	bc = 1;

	for (k=0;k<nb;++k,++bc) {
		if (bVerbose) {
			if (!(bc%iStride)) {
				printf("Block:(%d,%d)\n",k,k);
				fflush(stdout);
				bc = 0;
				}
			}
		if (bPeriodic) {
			diaEwald(&p[k*bs],bs,iSoftType,kd->fPeriod[0]);
			}
		else {
			diaGrav(&p[k*bs],bs,iSoftType);
			}
		} // for

	if (bVerbose && rs) {
		printf("Block:(%d,%d)\n",k,k);
		fflush(stdout);
		}
	if (bPeriodic) {
		diaEwald(&p[k*bs],rs,iSoftType,kd->fPeriod[0]);
		}
	else {
		diaGrav(&p[k*bs],rs,iSoftType);
		}
	/*
	 ** Now do the off-diagonal blocks.
	 */
	bc = 1;
	for (l=1;l<nb;++l) {
		for (k=l;k<nb;++k,++bc) {
			if (bc == kd->iChkptInterval) {
				WriteChkpt(kd,l,k);
				if (bVerbose) {
					printf("Check point (%d,%d) written\n",l-1,k);
					fflush(stdout);
					}
				bc = 1;
				}
		Restart1:
			if (bVerbose) {
				if (!(bc%iStride)) {
					printf("Block:(%d,%d)\n",l-1,k);
					fflush(stdout);
					}
				}
			if (bPeriodic) {
				blkEwald(&p[(l-1)*bs],bs,&p[k*bs],bs,iSoftType,kd->fPeriod[0]);
				}
			else {
				blkGrav(&p[(l-1)*bs],bs,&p[k*bs],bs,iSoftType);
				}
			}
		}
	bc = 1;
	for (l=0;l<nb;++l,++bc) {
		if (!rs) break;
		if (bPeriodic) {
			blkEwald(&p[l*bs],bs,&p[nb*bs],rs,iSoftType,kd->fPeriod[0]);
			}	
		else {
			blkGrav(&p[l*bs],bs,&p[nb*bs],rs,iSoftType);
			}
		}
	/*
	 ** We are now done the mutual interactions now we need to do the
	 ** interactions due to the unmarked particles on the marked ones.
	 */
	bc = 1;
	if (nk) {
		if (bVerbose) {
			printf("Now doing unmarked particle interactions\n");
			fflush(stdout);
			}
		for (l=0;l<nb;++l) {
			for (k=0;k<nbk;++k,++bc) {
				if (bc == kd->iChkptInterval) {
					WriteChkpt(kd,l,k+nb);
					if (bVerbose) {
						printf("Check point (%d,%d) written\n",l,k+nb);
						fflush(stdout);
						}
					bc = 1;
					}
			Restart2:
				if (bVerbose) {
					if (!(bc%iStride)) {
						printf("Block:(%d,%d)\n",l,k+nb);
						fflush(stdout);
						}
					}
				if (bPeriodic) {
					umkEwald(&p[l*bs],bs,&q[k*bs],bs,iSoftType,kd->fPeriod[0]);
					}
				else {
					umkGrav(&p[l*bs],bs,&q[k*bs],bs,iSoftType);
					}
				}
			if (bPeriodic) {
				umkEwald(&p[l*bs],bs,&q[k*bs],rsk,iSoftType,kd->fPeriod[0]);
				}
			else {
				umkGrav(&p[l*bs],bs,&q[k*bs],rsk,iSoftType);
				}
			}
		for (k=0;k<nbk;++k,++bc) {
			if (bPeriodic) {
				umkEwald(&p[l*bs],rs,&q[k*bs],bs,iSoftType,kd->fPeriod[0]);
				}
			else {
				umkGrav(&p[l*bs],rs,&q[k*bs],bs,iSoftType);
				}
			}
		if (bPeriodic) {
			umkEwald(&p[l*bs],rs,&q[k*bs],rsk,iSoftType,kd->fPeriod[0]);
			}
		else {
			umkGrav(&p[l*bs],rs,&q[k*bs],rsk,iSoftType);
			}
		}

#if 0
	printf("post normal\n");
	for(i=0; i<4; ++i) {
		printf("%f %f %f\n", p[i].a[0], p[i].a[1], p[i].a[2]);
		printf("%f\n", p[i].dPot);
	} // for
#endif

#if defined(ENABLE_OCL)
	double rms = 0.0;
#endif

	for (i=0;i<n;++i) {
		p[i].a[0] *= kd->G;
		p[i].a[1] *= kd->G;
		p[i].a[2] *= kd->G;
		p[i].dPot *= kd->G;
#if defined(ENABLE_OCL)
		const double dx = p[i].a[0] - h_a[i].x;
		const double dy = p[i].a[1] - h_a[i].y;
		const double dz = p[i].a[2] - h_a[i].z;
		//const double dp = p[i].dPot - h_a[i].p;
		//rms += dx*dx + dy*dy + dz*dz + dp*dp;
		rms += dx*dx + dy*dy + dz*dz;
#endif
		}
	
#if defined(ENABLE_OCL)
	rms /= n;
	rms = sqrt(rms);
	printf("rms: %e\n", rms);
#endif
	}


void kdGrav(KD kd,int iBlockSize,int iSoftType,int bPeriodic,
			int bVerbose)
{
	kd->iSoftType = iSoftType;
	kd->bPeriodic = bPeriodic;
	kd->iBlockSize = iBlockSize;
	Grav(kd,0,0,iSoftType,bPeriodic,bVerbose);
	}


void kdRestart(KD kd,int bVerbose)
{
	int l,k;

	ReadChkpt(kd,&l,&k);
	Grav(kd,l,k,kd->iSoftType,kd->bPeriodic,bVerbose);
	}


void kdOutAccel(KD kd,char *pszFile)
{
	FILE *fp;
	int i,iCnt;

	fp = fopen(pszFile,"w");
	assert(fp != NULL);
	fprintf(fp,"%d\n",kd->nParticles);
	iCnt = 0;
	for (i=0;i<kd->nGas;++i) {
		if (kd->bGas) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[0]);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nDark;++i) {
		if (kd->bDark) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[0]);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nStar;++i) {
		if (kd->bStar) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[0]);
		else fprintf(fp,"0\n");
		}
	iCnt = 0;
	for (i=0;i<kd->nGas;++i) {
		if (kd->bGas) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[1]);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nDark;++i) {
		if (kd->bDark) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[1]);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nStar;++i) {
		if (kd->bStar) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[1]);
		else fprintf(fp,"0\n");
		}
	iCnt = 0;
	for (i=0;i<kd->nGas;++i) {
		if (kd->bGas) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[2]);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nDark;++i) {
		if (kd->bDark) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[2]);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nStar;++i) {
		if (kd->bStar) fprintf(fp,"%.17g\n",kd->p[iCnt++].a[2]);
		else fprintf(fp,"0\n");
		}
	fclose(fp);
	}


void kdOutPot(KD kd,char *pszFile)
{
	FILE *fp;
	int i,iCnt;

	fp = fopen(pszFile,"w");
	assert(fp != NULL);
	fprintf(fp,"%d\n",kd->nParticles);
	iCnt = 0;
	for (i=0;i<kd->nGas;++i) {
		if (kd->bGas) fprintf(fp,"%.17g\n",kd->p[iCnt++].dPot);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nDark;++i) {
		if (kd->bDark) fprintf(fp,"%.17g\n",kd->p[iCnt++].dPot);
		else fprintf(fp,"0\n");
		}
	for (i=0;i<kd->nStar;++i) {
		if (kd->bStar) fprintf(fp,"%.17g\n",kd->p[iCnt++].dPot);
		else fprintf(fp,"0\n");
		}
	fclose(fp);
	}
