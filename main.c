/*
 * deca.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Decision-theoretic file Carver (DeCa) command-line interface / main() function.
 */

#include <stdio.h>
#include <string.h>
#include <tsk/libtsk.h>
#include "deca.h"

void notify_error(int errcode);
void print_help();

int main(int argc, char** argv)
{
	/* --- DeCa block device object --- */
	Deca_bd bd;

	/* --- DeCa carver data --- */
	Deca carver;
	int result;

	/* --- Command-line options --- */
	int i;
	char *outputPath = NULL;     /* Path to the output folder */
	int partno = -1;             /* Number of the partition to carve */
	int bdflags = DECA_BD_DISK;  /* Flags for Deca_bd object initialisation */
	int listPart = 0;            /* =1 list partitions and exit */
	int eflags = DECA_ESTIMATOR_FLAG_DEFAULT;
	int decaflags = DECA_ALGORITHM_LINEAR;
	int verbose = 0;
	int minsize = 1024;          /* default minimal size for JPEG photographs is 700Kb */

	char *modelfile = (char *)malloc(strlen(argv[0])+50);

	/* By default assume that JPEG odel file has the same name as executable with extention .model */
	sprintf(modelfile,"%s.model",argv[0]);

	/* Process command line options */
	if (argc == 1)
	{
		print_help(stderr);
		exit(DECA_FAIL);
	}
    for (i = 1; (i < argc) && (argv[i][0] == '-'); i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
        	if (i<(argc-1)) { outputPath=argv[i+1]; i++; }
        }
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
        	print_help(stdout);
        	exit(DECA_OK);
        }
        if (strcmp(argv[i], "-v") == 0)
        {
        	verbose = 1;
        }
        if (strcmp(argv[i], "-vv") == 0)
        {
        	verbose = 2;
        }
        if (strcmp(argv[i], "-p") == 0)
        {
        	if (i<(argc-1)) { bdflags = DECA_BD_VOLUME; partno=atoi(argv[i+1]); i++; }
        }
        if (strcmp(argv[i], "-u") == 0)
        {
        	bdflags |= DECA_BD_FS;
        }
        if (strcmp(argv[i], "-l") == 0)
        {
        	listPart = 1;
        }
        if (strcmp(argv[i], "-e") == 0)
        {
        	eflags |= DECA_ESTIMATOR_FLAG_FS;
        }
        if (strcmp(argv[i], "-s") == 0)
        {
        	eflags |= DECA_ESTIMATOR_FLAG_SEEK;
        }
        if (strcmp(argv[i], "--linear") == 0)
        {
        	decaflags = DECA_ALGORITHM_LINEAR;
        }
        if (strcmp(argv[i], "--deca") == 0)
        {
        	decaflags = DECA_ALGORITHM_DECA;
        }
        if (strcmp(argv[i], "-m") == 0)
        {
        	if (i<(argc-1))
        	{
        		free(modelfile);
        		modelfile = (char *)malloc(strlen(argv[i+1])+50);
        		strcpy(modelfile,argv[i+1]);
        		i++;
        	}
        }
        if (strcmp(argv[i], "--min") == 0)
        {
        	if (i<(argc-1))
        	{
        		minsize = atoi(argv[i+1]);
        		i++;
        	}
        }
    }

    /* All option flags have been processed. Whatever left on the command line
     * are paths to image fragments.
     */
    /* Initialise Deca block device object */

    if (listPart)
    {   // MUST initialize Deca block device with partition No 0 and DECA_BD_VOLUME flag!!!
        result = deca_bd_init(&bd,argc-i,&argv[i],0,DECA_BD_VOLUME);
    }
    else
    {
       result = deca_bd_init(&bd,argc-i,&argv[i],partno,bdflags);
    }

    if (result != DECA_OK)
    {
    	notify_error(result);
    	exit(result);
    }

    /* If partition listing was requested, list them and exit */
    if (listPart)
    {
    	deca_bd_list_partitions(&bd,stdout);
    	exit(DECA_OK);
    }

    /* Initialize DeCa carver */
	result = deca_init(&carver,&bd,outputPath,eflags,verbose,stdout,modelfile,minsize);
	if (result != DECA_OK)
	{
		notify_error(result);
		exit(result);
	}

	/* Run DeCa carver */
	result = deca_run(&carver,decaflags);
	if (result != DECA_OK)
	{
		notify_error(result);
		exit(result);
	}

	/* Output profiler statistics */
    result = deca_statistics(&carver);
    if (result != DECA_OK)
    {
    	notify_error(result);
    	exit(result);
    }
    /* Close DeCa carver and release associated resources */
    result = deca_close(&carver);
    if (result != DECA_OK)
    {
    	notify_error(result);
    	exit(result);
    }

    /* Close Deca block device object */
    deca_bd_close(&bd);

    /* Free objects malloc'ed in main() */
    free(modelfile);

    /* Terminate successfully */
	return 0;
}

void notify_error(int errcode)
{
	switch (errcode)
	{
	case DECA_MODEL_FILE_NOT_FOUND:
			fprintf(stderr,"DECA (%d): could not read file with SVM model of jpeg file\n",errcode); break;
	case DECA_UNKNOWN_ALGORITHM:
		fprintf(stderr,"DECA (%d): Unsupported carving algorithm\n",errcode); break;
	case DECA_IMAGE_NOT_FOUND:
		fprintf(stderr,"DECA (%d): Cannot Read Image File / Source Disk\n",errcode); break;
	case DECA_OUTPUT_FOLDER_NOT_FOUND:
		fprintf(stderr,"DECA (%d): Output Folder Not Found\n",errcode); break;
	case DECA_OUTPUT_FOLDER_NOT_WRITABLE:
		fprintf(stderr,"DECA (%d): Output Folder Is Not Writable\n",errcode); break;
	case DECA_VOLUME_NOT_FOUND:
		fprintf(stderr,"DECA (%d): Specified volume/partition not found on the image / disk \n",errcode); break;
	case DECA_FS_NOT_PRESENT:
		fprintf(stderr,"DECA (%d): Specified volume/partition on the image /disk does not contain a recognizable file system\n",errcode); break;
	case DECA_FAIL:
		fprintf(stderr,"DECA (%d): Error\n",errcode);
	}
	return;
}

void print_help(FILE *stream)
{
	fprintf(stream,"\nUsage: deca <options> <image files>\n"
			"   -l                print the list of available partitions on the given disk/image and exit\n"
			"   -h or --help      print this message and exit\n"
			"   -v                be verbose: print timings while carving\n"
			"   -vv               be extra verbose: print table headers and start/ stop times\n"
			"   -m path           Specify path to the LIBLINEAR jpeg detector model file\n"
			"   -o path           Specify output folder for the carved data (mandatory for carving operations)\n"
			"   -p partition      carve data only from the specified partition\n"
			"   -u                carve data only from unallocated space in the file system on the specified partition (requires -p)\n"
			"   -e                estimate distribution of target files in the unallocated space as distribution of\n"
			"                     active files in the file system (requires -u -p)\n"
			"   -s                (only makes sense when used with -e). makes the carver collect samples for the overall seek time curve\n"
			"                     for the given disk / image\n"
			"   --linear          use simple linear carving algorithm looking for header and footer signatures (default)\n"
			"   --deca            use sampling-based decision-theoretic DeCa algorithm\n"
			"   --min sectors     specifies minimal size of file in 512 sectors "
			"\n"
			"Example 1. Carving files from raw image data in image1.e01 image file:\n"
			"    deca -o /cases/data /images/image1.e01  \n\n"
			"Example 2. Carving files from unallocated space of partition 2 of a raw disk image split into segments\n"
			"    deca -o /cases/data -p 2 -u /images/image.???  \n\n");
}
