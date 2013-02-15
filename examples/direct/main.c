#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <ctype.h>
#include "kd.h"

#if defined(ENABLE_OCL)
	#include <example_strings.h>
#endif

void usage(void)
{
	fprintf(stderr,"USAGE:\n");
	fprintf(stderr,"direct [-e <fSoft>] [-uniform] [-plummer] [-spline]\n");
	fprintf(stderr,"     [-p <xyzPeriod>] [-G <fGravConst>] [-b <iBlockSize>]\n");
	fprintf(stderr,"     [-dgs] [-do <MarkName>] [-o <FileName>]\n");
	fprintf(stderr,"     [-i <iChkptInterval>] [-restart] [-v] [-t]\n");
	fprintf(stderr,"Reads tipsy binary format from stdin\n");
	fprintf(stderr,"For more information see man page, direct(1)\n");
	exit(1);
	}

int main(int argc,char **argv)
{
	KD kd;
	int i,iBlockSize,bVerbose,bSoft,bTestGrav;
	int bGas,bDark,bStar,iChkptInterval,bRestart;
	int iSoftType,bMark;
	int j,bPeriodic;
	int sec,usec;
	double G;
	float fSoft,fPeriod[3],fCenter[3];
	char ach[80],achFile[80],achCheck[80],achMark[80],*p;
	FILE *fp;

	strcpy(ach,"direct");
	bSoft = 0;
	bMark = 0;
	bVerbose = 0;
	bTestGrav = 0;
	bGas = 1;
	bDark = 1;
	bStar = 1;
	bRestart = 0;
	iChkptInterval = 0;	/* No Check points default */
	G = 1.0;
	iBlockSize = 256;
	bPeriodic = 0;
	fSoft = 0.0;
	for (j=0;j<3;++j) {
		fCenter[j] = 0.0;
		fPeriod[j] = FLT_MAX;
		}
	/*
	 ** Set up driver routines for uniform-density sphere softening.
	 */
	iSoftType = SOFT_SPLINE;
	i = 1;
	while (i < argc) {
	    if (!strcmp(argv[i],"-e")) {
			++i;
			if (i >= argc) usage();
			fSoft = atof(argv[i]);
			bSoft = 1;
			++i;
			}
		else if (!strcmp(argv[i],"-uniform")) {
			++i;
			iSoftType = SOFT_UNI;
			}
		else if (!strcmp(argv[i],"-plummer")) {
			++i;
			iSoftType = SOFT_PLUM;
			}
		else if (!strcmp(argv[i],"-spline")) {
			++i;
			iSoftType = SOFT_SPLINE;
			}
		else if (!strcmp(argv[i],"-G")) {
			++i;
			if (i >= argc) usage();
			G = atof(argv[i]);
			++i;
			}
		else if (!strcmp(argv[i],"-v")) {
			++i;
			bVerbose = 1;
			}
		else if (!strcmp(argv[i],"-t")) {
			++i;
			bTestGrav = 1;
			}
		else if (!strcmp(argv[i],"-o")) {
			++i;			
			if (i >= argc) usage();
			strcpy(ach,argv[i]);
			++i;
			}
		else if (!strcmp(argv[i],"-p")) {
			++i;
			bPeriodic = 1;
			fPeriod[0] = atof(argv[i]);
			fPeriod[1] = atof(argv[i]);
			fPeriod[2] = atof(argv[i]);
			++i;
			}
		else if (!strcmp(argv[i],"-do")) {
			++i;			
			if (i >= argc) usage();
			strcpy(achMark,argv[i]);
			bMark = 1;
			++i;
			}
		else if (!strcmp(argv[i],"-b")) {
			++i;
			if (i >= argc) usage();
			iBlockSize = atoi(argv[i]);
			++i;
			}
		else if (!strcmp(argv[i],"-i")) {
			++i;
			if (i >= argc) usage();
			iChkptInterval = atoi(argv[i]);
			++i;
			}
		else if (!strcmp(argv[i],"-restart")) {
			++i;
			bRestart = 1;
			}
		else if (*argv[i] == '-') {
			p = argv[i];
			++p;
			if (*p == 'd' || *p == 'g' || *p == 's') {
				bDark = 0;
				bGas = 0;
				bStar = 0;
				}
			else usage();
			while (isalpha(*p)) {
				switch (*p) {
				case 'd':
					bDark = 1;
					break;
				case 'g':
					bGas = 1;
					break;
				case 's':
					bStar = 1;
					break;
				default:
					usage();
					}
				++p;
				}
			++i;
			}
		else usage();
		} // while
	if (bVerbose) {
		printf("DIRECT v1.2: Joachim Stadel, Jan 1995\n");
		fflush(stdout);
		} // if
	strcpy(achCheck,ach);
	strcat(achCheck,".chk");
	kdInit(&kd,G,fPeriod,fCenter,iChkptInterval,achCheck);

#if defined(ENABLE_OCL)
	ocl_init();

	char * source = NULL;
	ocl_add_from_string(utils_PPSTR, &source, 0);
	ocl_add_from_string(kernels_PPSTR, &source, 0);

/* ATI */
	char * compile_options =
//		"-cl-fast-relaxed-math -DBLSZ=64 -DUSE_RSQRT -DUNROLL_LOOPS -DUNROLL_FACTOR=8 -DUSE_CONDITIONAL";
		"-DBLSZ=64 -DUSE_LOCAL_MEMORY";
//		"-DBLSZ=64 -DUSE_RSQRT";
//		"-DBLSZ=64 -DUSE_RSQRT -DUNROLL_LOOPS -DUNROLL_FACTOR=8";
//		"-DBLSZ=128 -DUSE_RSQRT";
//		"-DBLSZ=128 -DUSE_RSQRT";
//		"-DBLSZ=128 -DUSE_RSQRT -DUNROLL_LOOPS -DUNROLL_FACTOR=16";
//		"-cl-fast-relaxed-math -DBLSZ=64";
//		"-DBLSZ=64 -DUSE_RSQRT -DUNROLL_LOOPS -DUNROLL_FACTOR=32";
//		"-DBLSZ=64 -DUSE_RSQRT -DUSE_CONDITIONAL";
//		"-DBLSZ=64 -DUSE_RSQRT -DUNROLL_LOOPS -DUNROLL_FACTOR=32 -DUSE_CONDITIONAL";
/* NVIDIA */
//	char * compile_options =
//		"-DBLSZ=1024";
//		"-DBLSZ=1024 -DUSE_RSQRT -DUSE_CONDITIONAL";
//		"-cl-mad-enable -cl-fast-relaxed-math -DBLSZ=1024 -DUSE_RSQRT -DUNROLL_LOOPS -DUNROLL_FACTOR=32 -DUSE_CONDITIONAL";

	kd->local_size = 1024;
//	kd->local_size = 256;
//	kd->local_size = 128;
//	kd->local_size = 64;

	ocl_add_program(OCL_PERFORMANCE_DEVICE, "direct", source,
		compile_options);
	free(source);

	ocl_add_kernel(OCL_PERFORMANCE_DEVICE, "direct", "grav", "grav");
#endif
	
	if (bRestart) {
		fprintf(stderr,"Restarting from check point file\n");
		fprintf(stderr,"All command line arguments ignored other than:\n");
		fprintf(stderr,"-o <FileName>, -i <iChkptInterval> and -v\n");
		kdRestart(kd,bVerbose);
		}
	else {
		/*
		 ** First see if there is a check point file already present!
		 */
		fp = fopen(achCheck,"rb");
		if (iChkptInterval && fp != NULL) {
			fclose(fp);
			fprintf(stderr,"The check point file %s exists. You can:\n",
					achCheck);
			fprintf(stderr,"    1) run direct -o %s -restart\n",ach);
			fprintf(stderr,"    2) run direct with different output name\n");
			fprintf(stderr,"    3) remove %s\n",achCheck);
			kdFinish(kd);
			exit(1);
			}
		kdReadTipsy(kd,stdin,bGas,bDark,bStar);

		if (bSoft) kdSetSoft(kd,fSoft);
		if (bMark) {
			kdInMark(kd,achMark);
			kdMarkOrder(kd);
			}
		kdTime(kd,&sec,&usec);
		if (bTestGrav) {
			kdGravSimple(kd,iSoftType,bPeriodic);
			}
		else {
			kdGrav(kd,iBlockSize,iSoftType,bPeriodic,bVerbose);
			}
		} // if

	if (bVerbose) {
		kdTime(kd,&sec,&usec);
		printf("GRAV CPU Time:%d.%06d\n",sec,usec);
		fflush(stdout);
		}

	if (bMark) kdOrder(kd);
	strcpy(achFile,ach);
	strcat(achFile,".acc");
	kdOutAccel(kd,achFile);
	strcpy(achFile,ach);
	strcat(achFile,".pot");
	kdOutPot(kd,achFile);
	/*
	 ** Succesful completion remove the check pointfile if one was
	 ** specified (with -i)
	 */
	if (iChkptInterval) {
		fp = fopen(achCheck,"rb");
		if (fp != NULL) {
			fclose(fp);
			sprintf(ach,"rm %s",achCheck);
			system(ach);
			}
		}
	kdFinish(kd);

#if defined(ENABLE_OCL)
	ocl_finalize();
#endif

	return 0;
	}
