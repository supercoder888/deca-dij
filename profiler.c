/*
 * profiler.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Profiling / statistics collection fucntions used to measure
 *  performance of Decision-theoretic file Carver (DeCa).
 *
 *  Inclusion of the profiling functionality into DeCa code is controlled
 *  using DECA_PROFILE define
 */

#include "profiler.h"
#include "deca.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

int deca_profiler_init(Deca_profiler *p, int verbose, FILE *output)
{
#if defined(_WIN32) || defined(_WIN64)
   //tbd
#else
	struct timespec ts;
#endif

	p->verbose = verbose;
	if (p->verbose >0)
	{
		p->o = output;
		if (output == NULL)
		{
			return DECA_FAIL; /* No proper output stream is specified */
		}

#if defined(_WIN32) || defined(_WIN64)
    //tbd
#else  // MAC or LINUX
	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        ts.tv_sec = mts.tv_sec;
        ts.tv_nsec = mts.tv_nsec;
	#else
        clock_gettime(CLOCK_REALTIME, &ts);
	#endif

		p->s = ts.tv_sec*1000000000+ts.tv_nsec;
#endif
		if (p->s == -1)
		{
			return DECA_FAIL;
		}
		if (p->verbose >=2)
		{
			fprintf(p->o,
					"\nStart time (time):%ld\n"
					"\nCarving statistics:\n\n"
					"carv-start-time,carv-file-no,carv-start-blk,carv-stop-time,carv-stop-blk,file-len-in-blocks,total-carved-blks,total-blks-processed\n"
					"---------------------------------------------------------------------------------------------------------------------------------\n",p->s);
		}
	}
	return DECA_OK;

}

long deca_profiler_gettime(Deca_profiler *p)
{
#if defined(_WIN32) || defined(_WIN64)
    // tbd
#else
	struct timespec ts;
#endif

#if defined(_WIN32) || defined(_WIN64)
    // tbd
   return 0;
#else  // Unix or Mac
	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	    clock_serv_t cclock;
	    mach_timespec_t mts;
	    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	    clock_get_time(cclock, &mts);
	    mach_port_deallocate(mach_task_self(), cclock);
	    ts.tv_sec = mts.tv_sec;
	    ts.tv_nsec = mts.tv_nsec;
	#else
    	clock_gettime(CLOCK_REALTIME, &ts);
	#endif
	return ts.tv_sec*1000000000+ts.tv_nsec;
#endif
}

int deca_profiler_printtime(Deca_profiler *p)
{
#if defined(_WIN32) || defined(_WIN64)
    // tbd
#else
	struct timespec ts;
#endif

	long tcur;
	if (p->verbose > 0)
	{
#if defined(_WIN32) || defined(_WIN64)
   // tbd
#else // Unix or Mac
	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        ts.tv_sec = mts.tv_sec;
        ts.tv_nsec = mts.tv_nsec;
	#else
        clock_gettime(CLOCK_REALTIME, &ts);
	#endif
		tcur = ts.tv_sec*1000000000+ts.tv_nsec;
		fprintf(p->o,"%ld",tcur-(p->s));
#endif
	}
	return p->verbose;
}

int deca_profiler_output_summary(Deca_profiler *p)
{
#if defined(_WIN32) || defined(_WIN64)
    // tbd
#else
	struct timespec ts;
#endif
	long tcur;
	if (p->verbose >= 2)
	{
#if defined(_WIN32) || defined(_WIN64)
   // tbd
#else  // Unix or Mac
	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        ts.tv_sec = mts.tv_sec;
        ts.tv_nsec = mts.tv_nsec;
	#else
        clock_gettime(CLOCK_REALTIME, &ts);
	#endif
	    tcur = ts.tv_sec*1000000000+ts.tv_nsec;
		fprintf (p->o,"\nTotal processing time (nanoseconds):%ld\n",
					  tcur-(p->s));
#endif
	}
	return DECA_OK;
}

int deca_profiler_close(Deca_profiler *p)
{
	return DECA_OK;
}
