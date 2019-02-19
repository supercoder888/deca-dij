/*
 * detector.h
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 * API to the detector of information type contained in the given cluster.
 *  Used to decide if the given cluster is relevant / irrelevant during
 *  probing and carving steps of Decision-theoretic file Carver (DeCa)
 */

#ifndef DETECTOR_H_
#define DETECTOR_H_
//#include <magic.h>
//#include <linear.h>
//#include <libsvm/svm.h>

/**
 * Deca_detector data structure. The user must create an instance of Deca_detector and
 * initialze it using deca_detector_init() function before use.
 */
typedef struct deca_detector_struct
{
	//magic_t cookie;
#ifdef DECA_SVM_JPEG_DETECTION
#ifdef DECA_LIBLINEAR
	struct model *m;   // LIBLINEAR
#define LOAD_MODEL load_model
#define NODE struct feature_node
#define PREDICT predict
#else
	struct svm_model *m;   // LIBSVM
#define LOAD_MODEL svm_load_model
#define NODE struct svm_node
#define PREDICT svm_predict
#endif
#endif
   char _placeholder;
} Deca_detector;

/**
 * Deca_detector initializer. The user is responsible for creating an instance
 * of Deca_detector type and initializing it with this function before use.
 * @param *detector - pointer to the Deca_detector structure to initialize
 * @param modelfile - path to the file containing JPEG liblinear model (same place as executable,
 *                    same name as executable with extension .model
 * @return deca error code
 */
int deca_detector_init(Deca_detector *detector, char *modelfile);

/**
 * Deca_detector desctuctor. Releases any resources allocated during initialisation
 * @param *detector - pointer to the Deca_detector structure initialised with deca_detector_init
 * @return deca error code
 */
int deca_detector_close(Deca_detector *detector);

/**
 * Test the data in the given array for presence of JPEG header (i.e. beginning of file) signature(s)
 * @param d - Deca_detector object initialised with Deca_detector_init
 * @param buf - data to test
 * @param len - length of the data block
 * @return 1 if a match is found, 0 otherwise
 */
int deca_detector_tst_jpeg_header(Deca_detector *d, char *buf, int len);

/**
 * Test the data in the given array for presence of JPEG footer (i,e, end of file) signature(s)
 * @param d - Deca_detector object initialised with Deca_detector_init
 * @param buf - data to test
 * @param len - length of the data block
 * @param lastbyte - last byte of the previous block (used for matching footer signatures crossing block boundary - very ad-hoc'ish will need changing later)
 * @return 0 if a match is not found, length of JPEG data (incluiding footer) from the beginning of the buffer otherwise.
 */
int deca_detector_tst_jpeg_footer(Deca_detector *d, char *buf, int len, int lastbyte);

/**
 * Tests the data in the given array to see if it (statistically) looks like JPEG data
 * @param d - Deca_detector object initialised with Deca_detector_init
 * @param buf - data to test
 * @param len - length of the data block
 * @return 1 if the given data seems to be JPEG data, 0 otherwise
 */
int deca_detector_tst_jpeg_data(Deca_detector *d, char *buf, int len);


#endif /* DETECTOR_H_ */
