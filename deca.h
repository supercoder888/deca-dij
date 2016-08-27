/*
 * deca.h
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  API to Decision-theoretic file Carver (DeCa) functionality
 */

#ifndef DECA_H_
#define DECA_H_

#include <stdio.h>
#include <tsk/libtsk.h>
#include "detector.h"
#include "estimator.h"
#include "profiler.h"
#include "bd.h"

/**
 * Deca error codes (returned by functions):
 */
enum DECA_ERROR_CODE_ENUM {
	DECA_OK =0, //!< DECA_OK - successful completion of an operation
	DECA_FAIL =-1, //!< DECA_FAIL - something went wrong
	DECA_IMAGE_NOT_FOUND =-2, //!< DECA_IMAGE_NOT_FOUND
	DECA_OUTPUT_FOLDER_NOT_FOUND =-3, //!< DECA_OUTPUT_FOLDER_NOT_FOUND
	DECA_OUTPUT_FOLDER_NOT_WRITABLE = -4, //!< DECA_OUTPUT_FOLDER_NOT_WRITABLE
	DECA_VOLUME_NOT_FOUND = -5, //!< DECA_VOLUME_NOT_FOUND
	DECA_FS_NOT_PRESENT = -6, //!< DECA_FS_NOT_PRESENT
	DECA_WRONG_BD_FLAGS = -7, //!< DECA_BD_WRONG_FLAGS
	DECA_DISK_NOT_PARTITIONED = -8,
	DECA_UNKNOWN_ALGORITHM = -9,
	DECA_MODEL_FILE_NOT_FOUND = -10
};

/**
 * Type of carving algorithm to use
 */
enum DECA_ALGORITHM_ENUM {
	DECA_ALGORITHM_LINEAR,  //!< DECA_ALGORITHM_LINEAR - simple linear carving (default)
	DECA_ALGORITHM_DECA//!< DECA_ALGORITHM_DECA - sampling-based carving
};

/**
 * DECA_MAX CARVED_SIZE of a file is limited by 131072 blocks for DECA algorithm
 * (this is in line with size carved file size restriction imposed by foremost carver)
 *
 * Having maximum number of blocks a power of 2 means that we can step back in the carvings[]
 * in chunks that are an arbitrary power of two in size (DECA_STEP_BACK_BLOCKS).
 */
enum DECA_ALGORITHM_LIMITS {
	DECA_MAX_CARVED_SIZE=131072,
	DECA_STEP_BACK_BLOCKS=1024 // MUST BE A POWER OF TWO !!! (1,2,4,8,16,32,64,128,256, etc.)
};

/**
 * Deca data structure. The user must create an instance of Deca type and
 * initialze it using deca_init() function before use.
 */
typedef struct deca_struct
{
	Deca_bd        *b;
	Deca_detector   d;
	Deca_estimator  e;
	Deca_profiler   p;
	char *outputPath;
	int  nextfileno;
	int totalsize;
} Deca;

/**
 * Deca carver initializer. The user is responsible for creating an instance
 * of Deca type and initializing it with this function before use.
 * @param *carver - pointer to the Deca structure to initialize
 * @param bd - Deca block device object - used to access block devices and images
 * @param outputFolder - path to the directory where carved files will be stored.
 * @param eflags - estimator initialization mode flags (DECA_ESTIMATOR_FLAGS_ENUM)
 * @param verbose - level of profiling output (=0 no output, =1 normal output)
 * @param stat_output - output stream for printing statistics into (when verbose >0)
 * @param modelfie - location of JPEG model file for linear SVM detector
 * "param minsize - minimal size of JPEG file in sectors (used as a testing step)
 * @return deca error code
 */
int deca_init(Deca *carver,
		      Deca_bd *bd,
			  char *outputFolder,
			  int eflags,
			  int verbose,
			  FILE *stat_output,
			  char *modelfile,
			  int minsize);

/**
 * Run carver until completion
 * @param carver - Deca carver structure initialized with Deca_init
 * @param decaflags - type and parameters of the carving algorithm to run
 * @return deca error code
 */
int deca_run(Deca *carver, int decaflags);

/**
 * Print profiling statistics to the specified output stream.
 * Works only if DECA_PROFILE is defined.
 * @param carver - Deca carver structure initialized with Deca_init
 * @return deca error code
 */
int deca_statistics(Deca *carver);

/**
 * Close deca carver and release all associated resources
 * @param carver - Deca carver structure initialized with Deca_init
 * @return
 */
int deca_close(Deca *carver);

#endif /* DECA_H_ */
