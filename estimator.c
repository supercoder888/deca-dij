/*
 * estomator.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Estimator of relevant statistical properties of the given disk (image)
 *  for Decision-theoretic file carver (DeCa)
 */

#include <tsk/libtsk.h>
#include <math.h>
#include "deca.h"
#include "bd.h"
#include "estimator.h"
#include "profiler.h"
#include "detector.h"

int deca_estimator_init(Deca_estimator *e,
		                int flags,
						Deca_bd *bd,
						Deca_profiler *p,
						Deca_detector *d,
						int minsize)
{

	/* Default values */
	e->hitOffset = (minsize > DECA_STEP_BACK_BLOCKS+1) ? minsize : DECA_STEP_BACK_BLOCKS+1;
	e->missOffset = e->hitOffset;

	if (flags & DECA_ESTIMATOR_FLAG_FS)
	{
	  /*
	   * Do estimation of statistical properties
	   *
	   *	 1. Estimate seek time using the given Deca_db object. Time 10000 sequential reads. Time
       */

	   int i,j,newpos;
	   long dist;
       char *buf = (char *)malloc(sizeof(char)*
        		                   512*
								   (DECA_LINEAR_START+
								       (DECA_LINEAR_INCREMENT*
								    	  (DECA_MAX_LINEAR_TESTS-1))));

       long nblocks; // Number of blocks to read;
       int lintest;

       long jmptest;

	   long t0; // variable use to hold initial and final time for an interval;
	   long t1;

       // for calculating linear regressions
       double *x = (double *)malloc(sizeof(double)*(DECA_MAX_LINEAR_TESTS > DECA_MAX_JUMP_TESTS ? DECA_MAX_LINEAR_TESTS : DECA_MAX_JUMP_TESTS));
       double *y = (double *)malloc(sizeof(double)*(DECA_MAX_LINEAR_TESTS > DECA_MAX_JUMP_TESTS ? DECA_MAX_LINEAR_TESTS : DECA_MAX_JUMP_TESTS));
       double sx,sy,sxx,sxy,syy,blin,alin,bjmp,ajmp;

       long distblk; //distance (in 512 sectors) at which linear scanning and seek-the-test strategies consume the same time.

       /* Initialize random number generator */
       srand(time(NULL));

       /* 1.1 Measuring linear reading speed */

       if (p->verbose > 0)
       {
           DECA_PROFILER_PRINTF(*p,"Calculating time of sequentially reading 100000 512-byte blocks and checking each block for JPEG signature.\n"
        			                "Nblocks, Start time, Finish time (in process timer ticks):\n"
           							"-------------------------------------------------\n");
       }

       /* Go somewhere far */
       deca_bd_goto(bd,(bd->size)/2);
       DECA_BD_READ_BLOCK(bd,buf,512);

       nblocks = DECA_LINEAR_START;
       for(lintest=0;
    	   lintest<DECA_MAX_LINEAR_TESTS;
    	   lintest++, nblocks += DECA_LINEAR_INCREMENT)
       {

        	t0 = deca_profiler_gettime(p);
        	for (j=0; j<100; j++)
        	{
        		DECA_BD_READ_BLOCK(bd,buf,512*nblocks);
        		for (i=0; i<nblocks; i++)
        		{
        			deca_detector_tst_jpeg_header(d,&buf[i*512],512);
        		}
        	}
            t1 = deca_profiler_gettime(p);
            x[lintest]=nblocks*100;
            y[lintest]=((double)(t1-t0))/1000000.0;
       }

       /* Estimate Tproc using first and last computed values */

       if (p->verbose > 0)
       {
    	   fprintf(p->o,"\nCalcluating approcimation of linear scanning time...\n---\nx[]:\n");

    	   for (i=0; i<DECA_MAX_LINEAR_TESTS; i++)
           {
              fprintf(p->o,"%e,",x[i]);
           }
    	   fprintf(p->o,"\ny[]:\n");
    	   for (i=0; i<DECA_MAX_LINEAR_TESTS; i++)
           {
              fprintf(p->o,"%e,",y[i]);
           }
    	   fprintf(p->o,"\n");
       }

       // slope calculation
       blin = (y[DECA_MAX_LINEAR_TESTS-1]-y[0])/(x[DECA_MAX_LINEAR_TESTS-1]-x[0]);
       // intercept calculation
       alin = y[0]-blin*x[0];

       if (p->verbose>0)
       {
            DECA_PROFILER_PRINTF(*p,"\nRegression parameters\nSlope (blin):%e\nIntercept (alin):%e\n\n",blin,alin);
       }

       /* 1.2 Measure 1000 random accessed around drive */
       if (p->verbose > 0)
       {
    	   DECA_PROFILER_PRINTF(*p,"\nCalculating time of 1000 random block accesses. For each block a LIBLINEAR test for JPEG data is included into the timing.\n"
        			"Distance jumped (+/-), start time, finish time (in process timer ticks):\n"
        			"--------------------------------------------------\n");
       }

       /* Go to the beginning of disk */
       deca_bd_goto(bd,0);
       DECA_BD_READ_BLOCK(bd,buf,512);

       jmptest=0;

       /* Read 1000 blocks while randmoly jumping around */
       for (i=0; i<DECA_MAX_JUMP_TESTS; i++)
       {
/*           newpos= (rand()) %
            		(DECA_LINEAR_START+
            	       (DECA_LINEAR_INCREMENT*
            	    	  (DECA_MAX_LINEAR_TESTS-1)));
*/
            newpos = labs(rand()) % bd->size;
    	    dist=newpos - (bd->blk);

        	t0 = deca_profiler_gettime(p);
        	deca_bd_goto(bd,newpos);
        	DECA_BD_READ_BLOCK(bd,buf,512);
        	deca_detector_tst_jpeg_data(d,buf,512);
        	t1 = deca_profiler_gettime(p);
//        	if ((dist > 0) && ((t1-t0)>0))
        	{
        	    x[jmptest]=(double)dist;
        	    y[jmptest]=((double)(t1-t0))/1000000.0;
        	    jmptest++;
        	}
        }

       /* Do linear regression calculation for jump test */
       sx=0.0; sy=0.0; sxx=0.0; syy=0.0; sxy=0.0;

       if (p->verbose>0)
       {
    	   fprintf(p->o,"\nCalcluating regression for the jump test...\n---\nx[]:\n");

    	   for (i=0; i<jmptest; i++)
           {
           	   fprintf(p->o,"%e,",x[i]);
           }
    	   fprintf(p->o,"\ny[]:\n");
    	   for (i=0; i<jmptest; i++)
           {
    		   fprintf(p->o,"%e,",y[i]);
           }
    	   fprintf(p->o,"\n");
       }
       for (i=0; i<jmptest; i++)
       {
    	   sx += fabs(x[i]);
    	   sy += y[i];
    	   sxx += x[i]*x[i];
    	   syy += y[i]*y[i];
    	   sxy += fabs(x[i])*y[i];
       }

       // slope calculation
       bjmp = (jmptest*sxy-sx*sy)/(jmptest*sxx-sx*sx);
       // intercept calculation
       ajmp = 1.0/jmptest*sy-bjmp*(1.0/jmptest)*sx;

       if (p->verbose > 0)
       {
           DECA_PROFILER_PRINTF(*p,"\nRegression parameters\nSlope (bjmp):%e\nIntercept (ajmp):%e\n\n",bjmp,ajmp);
       }

       distblk=(long)(-(ajmp-alin)/(bjmp-blin));

	   if (p->verbose > 0)
	   {
		   fprintf(p->o,"\nNumber of 512-byte sectors at which both processing techniques are equal: %ld\n\n",distblk);
	   }

       free(buf);
       free(x);
       free(y);
       exit(0);
	}

	return DECA_OK;
}

int deca_estimator_close(Deca_estimator *estimator)
{
    return DECA_OK;
}
