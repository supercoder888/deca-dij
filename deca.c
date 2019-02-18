/*
 * deca.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Decision-theoretic file Carver (DeCa) core functionality
 */

#include <unistd.h>
#if defined(_WIN32) || defined(_WIN64)
    // tbd
#else
	#include <dirent.h>
#endif
#include <tsk/libtsk.h>
#include <stdio.h>
#include <string.h>
#include "deca.h"

int deca_init(Deca *c,
		      Deca_bd *bd,
			  char *outputPath,
			  int eflags,
			  int verbose,
			  FILE *stat_output,
			  char *modelfile,
			  int minsize)
{
	int result;
#if defined(_WIN32) || defined(_WIN64)
    // tbd
#else
	DIR *tmp;

    /* Verify that output path is a writable directory */
	if (!(eflags & DECA_ESTIMATOR_FLAG_FS))
	{
    	if (outputPath == NULL) return DECA_OUTPUT_FOLDER_NOT_FOUND;
		tmp=opendir(outputPath);
		if (tmp == NULL) return DECA_OUTPUT_FOLDER_NOT_FOUND;
		closedir(tmp);
    	if (access(outputPath,W_OK) != 0) return DECA_OUTPUT_FOLDER_NOT_WRITABLE;
	
    	/* store outputPath in a member variable */
    	c->outputPath = malloc(strlen(outputPath)+1);
    	strncpy(c->outputPath,outputPath,strlen(outputPath)+1);
	}
#endif
    /* Record address of the given block device structure */
    c->b = bd;

    /* Initialize algorithm sub-modules */
	result = deca_profiler_init(&(c->p),verbose,stat_output); if(result != DECA_OK) return result;
	result = deca_detector_init(&(c->d),modelfile); if(result != DECA_OK) return result;
	result = deca_estimator_init(&(c->e),eflags,bd,&(c->p),&(c->d),minsize); if(result != DECA_OK) return result;
	
	c->nextfileno = 0;
	return DECA_OK;
}

/**
 * Simple linear carving algorithm looking for JPEG header and footer signatures
 * @param c - Deca carver object initialised with deca_init
 * @return deca error code (<0) or the number of the last tested block (>=0)
 */

long deca_linear(Deca* c, long start, long l)
{
	int   buflen;
	char *buf;            /* Buffer for reading block / sector into */
	int   lastbyte;
	int   infile;         /* State of the application =1 processing a file, looking for the end of it */
	                      /*                          =0 looking for the beginning of the file */

	char  *nextfilename;  /* Full path to the next carved file in the output directory */
	FILE *fout;           /* Handle to the output file */

	long filesize;        /* length of the current file being carved (in clusters) */
	long blocksprocessed;
	long bound;

	int lastblocksize;
	int buf_contains_jpeg_header;


	buflen = c->b->bs;
    buf = (char *)malloc(buflen);
    nextfilename = (char *)malloc(strlen(c->outputPath)+256);

    c->totalsize = 0;
    blocksprocessed = 0;

	/* Simple linear file carving algorithm: start at the beginning and examine every carvable block
	 * anything between matching JPEG header and footer signatures is extracted as a carved file.
	 */
    infile = 0;

    bound = l;
	DECA_BD_SKIP_BACK(c->b,start);               /* Skip to the first carvable sector / cluster */
	DECA_BD_READ_BLOCK(c->b,buf,buflen);   /* Read next block */
	DECA_BD_MARK_BLOCK(c->b);
	blocksprocessed++;

	lastbyte = 0;

    while (c->b->btc)
	{
		if (infile == 0)
		{
			if (deca_detector_tst_jpeg_header(&(c->d),buf,buflen) == 1)  /* If the read block appears to be the beginning of a JPG file, */
			{
				int blockswritten;

				/* Print the (beginning of) profiling information if profiler is armed */
				if (deca_profiler_printtime(&(c->p))>0)
				{
					/* Carving statistics output (CSV format):
					 * carv-start-time,file-number,carv-start-sect,carv-stop-time,carv-stop-sect,file-length,total-carved-clust,total-clust-processed
					 */
					DECA_PROFILER_PRINTF(c->p,",%d,%llu,",c->nextfileno,c->b->blk);
				}

				/* Create new file in the output directory and start writing into it */
				//sprintf(nextfilename,"%s/%ld_%d.jpg",c->outputPath,c->b->blk,c->nextfileno);
				sprintf(nextfilename,"%s/%llu.jpg",c->outputPath,c->b->blk);
				c->nextfileno++;
				fout = fopen(nextfilename,"wb");
				if (fout == NULL)
				{
					free(nextfilename);
					free(buf);
					return DECA_FAIL;
				}
				filesize = 1;
				c->totalsize += 1;

				/* If the read block contains also the footer signature, the entire file is in one block */
				lastblocksize = deca_detector_tst_jpeg_footer(&(c->d),buf,buflen,0);
				if (lastblocksize > 0 )
				{
					blockswritten = fwrite(buf,lastblocksize,1,fout);
					if (blockswritten != 1)
					{
						free(nextfilename);
						free(buf);
						return DECA_FAIL;
					}

					/* Print the ending of the profiling information if profiler is armed */
					if (deca_profiler_printtime(&(c->p))>0)
					{
						/* Carving statistics output (CSV format):
						 * carv-start-time,file-number,carv-start-sect,carv-stop-time,carv-stop-sect,file-length,total-carved-clust,total-clust-processed
						 */
						DECA_PROFILER_PRINTF(c->p,",%llu,%ld,%ld,%ld\n",c->b->blk,filesize,(long)c->totalsize,blocksprocessed);
					}

					/* so close output file */
					if (fclose(fout) != 0)
					{
						free(nextfilename);
						free(buf);
						return DECA_FAIL;
					}
					infile = 0;
					bound = l;
				}
				else
				{
				   blockswritten = fwrite(buf,buflen,1,fout);
				   if (blockswritten != 1)
			       {
					  free(nextfilename);
					  free(buf);
					  return DECA_FAIL;
				   }
				   /* Switch carver into in-file state */
                   infile = 1;
                   lastbyte = buf[buflen-1]; // Store last byte of the current block for matching footer signature in the next file block
				}
			}
			else
			{
				if ((bound--)==0) break;
			}
		}
		else /* in-file state */
		{
			/* If the next read block contains relevant (i.e. JPG data) */
			lastblocksize = deca_detector_tst_jpeg_footer(&(c->d),buf,buflen,lastbyte);

			if (lastblocksize == 0)
			{
			   /* Write it into the output file */
			   int blockswritten = fwrite(buf,buflen,1,fout);
			   lastbyte = buf[buflen-1];
			   if (blockswritten != 1)
			   {
				   free(nextfilename);
				   free(buf);
				   return DECA_FAIL;
			   }
			   filesize +=1;
			   c->totalsize +=1;
			}
			else
			{
				int blockswritten = fwrite(buf,lastblocksize,1,fout);
				if (blockswritten != 1)
				{
				   free(nextfilename);
				   free(buf);
				   return DECA_FAIL;
			    }
				/* Print the ending of the profiling information if profiler is armed */
				if (deca_profiler_printtime(&(c->p))>0)
				{
					/* Carving statistics output (CSV format):
					 * carv-start-time,file-number,carv-start-sect,carv-stop-time,carv-stop-sect,file-length,total-carved-clust,total-clust-processed
					 */
					DECA_PROFILER_PRINTF(c->p,",%llu,%ld,%ld,%ld\n",c->b->blk,filesize,(long)c->totalsize,blocksprocessed);
				}
				/* close output file */
				if (fclose(fout) != 0)
				{
					free(nextfilename);
					free(buf);
					return DECA_FAIL;
				}
				infile = 0;
				bound = l;
			}
		}
		DECA_BD_SKIP(c->b,1);               /* Skip to the next carvable sector / cluster */
		DECA_BD_READ_BLOCK(c->b,buf,buflen);   /* Read next block */
		DECA_BD_MARK_BLOCK(c->b);
		blocksprocessed++;
	}

	free(nextfilename);
	free(buf);

#ifdef CARVE_NONJPEG
	fclose(nonjpg);
#endif


	return c->b->blk;
}

/**
 * Decision-theoretic carving algorithm sampling for clusters looking like JPEG data, then searching backward
 * and forward for JPEG header and footer signatures
 * @param c - Deca carver object initialised with deca_init
 * @return deca error code
 */
int deca_deca(Deca *c)
{
	int   buflen;
	char *buf;            /* Buffer for reading block / sector into */
	int   buf_contains_jpeg_data;
	int   lastbyte;

	char  *nextfilename;  /* Full path to the next carved file in the output directory */
	FILE *fout;           /* Handle to the output file */

	long filesize;        /* length of the current file being carved (in clusters) */
	long totalsize;       /* total number of cluster carved */
	long blocksprocessed;
	long svmtested;
	long sigtested;
    long giveups;
    long fpblocks;
    long fphelper;

	long prevblk;

	char *carvings;        /* Buffer space for carved data */

	buflen = c->b->bs;
	nextfilename = (char *)malloc(strlen(c->outputPath)+256);

	totalsize = 0;
	blocksprocessed = 0;
	sigtested = 0;
	svmtested = 0;
	giveups = 0;
	fpblocks = 0;

	carvings = malloc(buflen);


    /*
     * This function implements "sampling" version of Deca algorithm.  The assumption is that statistical model
     * of the jpeg data distribution on the disk has already been pre-compiled and we are given fixed offsets
     * from the given cluster that is most likely to contain the information we need.
     */

	prevblk = c->b->blk;
	DECA_BD_SKIP(c->b,c->e.missOffset-1);

	/* Now we keep sampling until we reach the end of the drive / partition until we wrap-around. */
	while (c->b->wrap_around == 0)
	{

		/* We place working buffer at the end of allocated carving space */
		buf = carvings;

		/* Step 1. now read the chosen block */
		DECA_BD_READ_BLOCK(c->b,buf,buflen);   /* Read next block */
		//DECA_BD_MARK_BLOCK(c->b);              /* Mark it so that we do not read it again */
		blocksprocessed++;
		svmtested++;

		/* Step 2. Check if the buffer contains JPEG-like data */
		buf_contains_jpeg_data = deca_detector_tst_jpeg_data(&(c->d),buf,buflen);

		if (buf_contains_jpeg_data == 1)
		{

			/* If the buffer DOES seem to be part of JPEG,
			 * let's look for header and footer signatures and carve it out
			 */
		    long ret = deca_linear(c,c->e.missOffset-1,c->e.missOffset);
		    if (ret < 0) return ret;

		}
		/* our initial jpeg test did not succeed - we need to jump and test another block */
		DECA_BD_SKIP(c->b,c->e.missOffset-1);
	}
	free(nextfilename);
	free(carvings);
	c->b->blk=c->b->size-1;


	printf("\nDeCa Algorithm statistics:\n"
			       "   Total blocks on the drive:    %llu\n"
			       "   Blocks processed:             %ld\n"
			       "   Blocks tested with SVM:       %ld\n"
			       "   Blocks tested for signature:  %ld\n"
			       "   Give-ups (JPEG mis-detects):  %ld\n"
			       "   False-positive blocks (tot):  %ld",c->b->size,blocksprocessed,svmtested,sigtested,giveups,fpblocks);

	return c->b->blk;
}

int deca_run(Deca *c, int decaflags)
{
	int result;
	switch (decaflags)
	{
	case DECA_ALGORITHM_LINEAR:
		return deca_linear(c,0,c->b->btc);
		break;
	case DECA_ALGORITHM_DECA:    // 20% of the drive are processed using sampling
		result=deca_deca(c);
		//if (result != 0)
			return result;
		//return deca_linear(c);
		break;
	}
	return DECA_UNKNOWN_ALGORITHM;
}

int deca_statistics(Deca *c)
{
	return deca_profiler_output_summary(&(c->p));
}

int deca_close(Deca *c)
{
	int result;
	result = deca_profiler_close(&(c->p)); if (result != DECA_OK) return result;
	result = deca_estimator_close(&(c->e)); if (result != DECA_OK) return result;
	result = deca_detector_close(&(c->d)); if (result != DECA_OK) return result;
	if (c->outputPath != NULL) free(c->outputPath);
	return DECA_OK;
}


