/*
 * estimator.h
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  API to the estimator of statistical properties of the given disk (image)
 *  for Decision-theoretic file carver (DeCa)
 */

#ifndef ESTIMATOR_H_
#define ESTIMATOR_H_

#include "bd.h"
#include "profiler.h"

/**
 * Flags specifying initialisation mode of Deca_estimator
 */
enum DECA_ESTIMATOR_FLAGS_ENUM{
	DECA_ESTIMATOR_FLAG_DEFAULT = 0,    //!< DECA_ESTIMATOR_DEFAULT - use default values for the distribution of files in unallocated space.
	DECA_ESTIMATOR_FLAG_FS = 1,          //!< DECA_ESTIMATOR_FS - use filesystem data to estimate distribution of files in unallocated space.
    DECA_ESTIMATOR_FLAG_SEEK = 2        //!< DECA_ESTIMATOR_SEEK - try to determine overall seektime curve for the disk / image
};

enum DECA_ESTIMATOR_PARAMETERS{
	DECA_MAX_LINEAR_TESTS=4,          //!< DECA_MAX_LINEAR_TESTS - number of points (for linear regressions) used to estimate sequential read & signature test time
	DECA_LINEAR_START=1,           //!< DECA_LINEAR_START - number of blocks to read for the first point.
	DECA_LINEAR_INCREMENT=100000,       //!< DECA_LINEAR_INCREMENT - increase in the number of blocks to read with each point.
	DECA_MAX_JUMP_TESTS=1000         //!< DECA_MAX_JUMP_TESTS - number of points for random seek and JPEG test
};


/**
 * Deca_estimator data structure. estimator_init() function before use.
 */
typedef struct deca_estimator_struct
{
	int hitOffset;  //!< number of clusters to skip for the next probe following a successfully found and carved file.
	                //!< given that files tend to appear in clumps, this number is small (based on our experiments
	                //!< it is set to 4 by default)

	int missOffset; //!< Offset to the next cluster to probe following an unsuccessful probe.
	                //!< This is dependent on the particular image. In out tests the default of 3000 worked well.

	double seqSpeed;//!< Number of CPU process ticks for accessing 1 block of data. Calculated as average
					//!< After sequentially reading 100000 blocks.

} Deca_estimator;

/**
 * Deca_estimator initializer. The user is responsible for creating an instance
 * of Deca_estimator type and initializing it with this function before use.
 * @param *estimator - pointer to the Deca_detector structure to initialize
 * @param flags - estimator initialisation mode (DECA_ESTIMATOR_FLAGS_ENUM)
 * @param bd - structre representing block device / image
 * @param p - structure representing profiler object
 * @param d - structure representing detector object
 * @param minsize - minimal file size in 512 sectors. used as jump offset after both hits and misses.
 * @return deca error code
 */
int deca_estimator_init(Deca_estimator *estimator,
		                int flags,
						Deca_bd *bd,
						Deca_profiler *p,
						Deca_detector *d,
						int minsize);

/**
 * Deca_estimator destructor. Releases any resources allocated during initialisation.
 * @param *estimator - pointer to the Deca_detector structure previously initialized with deca_estimator_init
 * @return deca error code
 */
int deca_estimator_close(Deca_estimator *estimator);

#endif /* ESTIMATOR_H_ */

