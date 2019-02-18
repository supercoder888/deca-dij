/*
 * profiler.h
 *
 *  Created on: 20 Jun 2015
  *      Author: Pavel Gladyshev
 *
 *  API to profiling / statistics collection functionality used to measure
 *  performance of Decision-theoretic file Carver (DeCa).
 *
 *  Inclusion of the profiling functionality into DeCa code is controlled
 *  using DECA_PROFILE define
 */

#ifndef PROFILER_H_
#define PROFILER_H_

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
   //tbd
#else
	#include <sys/time.h>
#endif

#include <signal.h>
#include <time.h>

/**
 * Deca_detector data structure. The user must create an instance of Deca_detector and
 * initialze it using deca_detector_init() function before use.
 */
typedef struct deca_profiler_struct
{
	int verbose;  //!< verbose - level of detail to output (=0 no output; =1 normal output)
	FILE *o;      //!< o - output stream to print statistics into (when verbose >0)
	long s;    //!< s - starting time of the profiler
} Deca_profiler;

/**
 * Deca_profiler initializer. The user is responsible for creating an instance
 * of Deca_profiler type and initializing it with this function before use.
 * @param *profiler - pointer to the Deca_detector structure to initialize
 * @param verbose - level of output (=0 no output, =1 normal output)
 * @param output - stream (file handle / stdout) where the statistics is to be printed
 * @return deca error code
 */
int deca_profiler_init(Deca_profiler *profiler, int verbose, FILE *output);

/**
 * Prints time passed since the beginning of profiling
 * @param p - Deca_profiler objecte initialised with deca_profiler_init()
 * @return verbosity level (=0 no statistical output, =1 normal output)
 */
long deca_profiler_gettime(Deca_profiler *p);

/**
 * Returns time passed since the beginning of profiling
 * @param p - Deca_profiler objecte initialised with deca_profiler_init()
 * @return verbosity level (=0 no statistical output, =1 normal output)
 */
int deca_profiler_printtime(Deca_profiler *p);

#define DECA_PROFILER_PRINTF(p,...) (fprintf((p).o, __VA_ARGS__))

/**
 * Print profiling statistics to the specified output stream.
 * Works only if DECA_PROFILE is defined.
 * @param profiler - Deca_profiler structure initialized with deca_profiler_init
 * @return deca error code
 */
int deca_profiler_output_summary(Deca_profiler *profiler);

/**
 * Deca_profiles descructor. Releases any resources allocated during initisatin and subsequent use
 * Works only if DECA_PROFILE is defined.
 * @param profiler - Deca_profiler structure initialized with deca_profiler_init()
 * @return deca error code
 */
int deca_profiler_close(Deca_profiler *profiler);



#endif /* PROFILER_H_ */
