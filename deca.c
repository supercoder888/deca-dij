/*
 * deca.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Decision-theoretic file Carver (DeCa) core functionality
 */

#include <unistd.h>
#include <dirent.h>
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
	DIR *tmp;

    /* Verify that output path is a writable directory */
	if (outputPath == NULL) return DECA_OUTPUT_FOLDER_NOT_FOUND;
	tmp=opendir(outputPath);
	if (tmp == NULL) return DECA_OUTPUT_FOLDER_NOT_FOUND;
	closedir(tmp);
    if (access(outputPath,W_OK) != 0) return DECA_OUTPUT_FOLDER_NOT_WRITABLE;

    /* store outputPath in a member variable */
    c->outputPath = malloc(strlen(outputPath)+1);
    strncpy(c->outputPath,outputPath,strlen(outputPath)+1);

    /* Record address of the given block device structure */
    c->b = bd;

    /* Initialize algorithm sub-modules */
	result = deca_detector_init(&(c->d),modelfile); if(result != DECA_OK) return result;
	result = deca_profiler_init(&(c->p),verbose,stat_output); if(result != DECA_OK) return result;
	result = deca_estimator_init(&(c->e),eflags,bd,&(c->p),&(c->d),minsize); if(result != DECA_OK) return result;

	c->nextfileno = 0;
	return DECA_OK;
}

/**
 * Simple linear carving algorithm looking for JPEG header and footer signatures
 * @param c - Deca carver object initialised with deca_init
 * @return deca error code
 */

int deca_linear(Deca* c)
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

	int buf_contains_jpeg_data;
	int buf_contains_jpeg_header;
	int buf_contains_jpeg_footer;

	buflen = c->b->bs;
    buf = (char *)malloc(buflen);
    nextfilename = (char *)malloc(strlen(c->outputPath)+256);

    c->totalsize = 0;
    blocksprocessed = 0;

#ifdef CARVE_NONJPEG
    FILE *nonjpg = fopen("nonjpg.bin","wb");
    int nonjpgcnt = 0;
#endif

	/* Simple linear file carving algorithm: start at the beginning and examine every carvable block
	 * anything between matching JPEG header and footer signatures is extracted as a carved file.
	 */
    infile = 0;

	DECA_BD_SKIP(c->b,1);               /* Skip to the next carvable sector / cluster */
	DECA_BD_READ_BLOCK(c->b,buf,buflen);   /* Read next block */
	DECA_BD_MARK_BLOCK(c->b);
	blocksprocessed++;

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
				blockswritten = fwrite(buf,buflen,1,fout);
				if (blockswritten != 1)
				{
					free(nextfilename);
					free(buf);
					return DECA_FAIL;
				}
				filesize = 1;
				c->totalsize += 1;

				/* If the read block contains also the footer signature, the entire file is in one block */
				if (deca_detector_tst_jpeg_footer(&(c->d),buf,buflen,0) == 1)
				{
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
				}
				else
				{
				   /* Switch carver into in-file state */
                   infile = 1;
                   lastbyte = buf[buflen-1]; // Store last byte of the current block for matching footer signature in the next file block
				}
			}
#ifdef CARVE_NONJPEG
			else  /* Save samples of non-jpeg data */
			{
				if((nonjpgcnt++)<9000)
					fwrite(buf,buflen,1,nonjpg);
			}
#endif
		}
		else /* in-file state */
		{
			/* If the next read block contains relevant (i.e. JPG data) */
			//buf_contains_jpeg_data = deca_detector_tst_jpeg_data(&(c->d),buf,buflen);
			buf_contains_jpeg_data = 1;
			buf_contains_jpeg_footer = deca_detector_tst_jpeg_footer(&(c->d),buf,buflen,lastbyte);
			buf_contains_jpeg_header = deca_detector_tst_jpeg_header(&(c->d),buf,buflen);

			if (buf_contains_jpeg_data && (!buf_contains_jpeg_header))
			{
			   /* Write it into the output file */
			   int blockswritten = fwrite(buf,buflen,1,fout);
			   if (blockswritten != 1)
			   {
				   free(nextfilename);
				   free(buf);
				   return DECA_FAIL;
			   }
			   filesize +=1;
			   c->totalsize +=1;
			}

			/* If the read block contains footer signature of a JPG file, or NON-JPG data, or header */
			if (buf_contains_jpeg_footer || (!buf_contains_jpeg_data)|| buf_contains_jpeg_header)
			{
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

			}

			lastbyte = buf[buflen-1]; // Store last byte of the current block for matching footer signature in the next file block

			if (buf_contains_jpeg_header)
			{
				buf_contains_jpeg_header=0;
				infile = 0;
				continue;
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


	return DECA_OK;
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

	carvings = malloc((c->e.missOffset*2+1)*buflen);


    /*
     * This function implements "sampling" version of Deca algorithm.  The assumption is that statistical model
     * of the jpeg data distribution on the disk has already been pre-compiled and we are given fixed offsets
     * from the given cluster that is most likely to contain the information we need.
     */

    /*
     * First we skip 1 cluster ahead (this helps in the case of recovering unallocated data,
     * because after Deca_bd initialisation in "unallocated" mode, blk points
     * to the last unallocated block in the partition)
     */
	prevblk = c->b->blk;
	DECA_BD_SKIP(c->b,c->e.missOffset);

	/* Now we keep sampling until we reach the end of the drive / partition until we wrap-around. */
	while (c->b->wrap_around == 0)
	{

		/* We place working buffer at the end of allocated carving space */
		buf = carvings+(c->e.missOffset-1)*buflen;

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

			fphelper = blocksprocessed;

			/*
			 * Step 2.1 first we read blocks backwards from the current block
			 * and check for header signature. The read data fills the carving space from end to front,
			 * until we either find the the header or run out of space.
			 */

			int blockswritten;
			int jpeg_header_found = deca_detector_tst_jpeg_header(&(c->d),buf,buflen);
			int buf_contains_jpeg_footer;  /* =0 not found, !=0 contains length of JPEG data up to and including the footer */
			long probed_blk = c->b->blk;  /* Save current location on the disk */
			int back_blk_count;
			int verbosity_level = c->p.verbose;
			int skipped;
			clock_t starttime = clock()-(c->p.s);

			filesize = 1; /* We have one block in our would-be file */

			if (jpeg_header_found == 0)
			{
					/* Step back a number of blocks in the carvings[] space and on the disk and read from disk to memory */
					skipped = DECA_BD_SKIP_BACK(c->b,c->e.missOffset);
					if (skipped == 0) break;

					buf -= buflen * skipped;
					DECA_BD_READ_BLOCK(c->b,buf,buflen*skipped);
					blocksprocessed += skipped;
					sigtested += skipped;

					/* Now check all of them for JPEG signature, going backwards */
					back_blk_count=skipped;
					buf = buf+(buflen*(skipped-1));
					do
					{
						jpeg_header_found = deca_detector_tst_jpeg_header(&(c->d),buf,buflen);
						/*if (back_blk_count % 64 == 0)
						{
							buf_contains_jpeg_data = deca_detector_tst_jpeg_data(&(c->d),buf,buflen);
							svmtested++;
						}*/

						back_blk_count--;
						buf -= buflen;
						filesize++;
					}
					while ((back_blk_count>0) && (jpeg_header_found == 0) && (buf_contains_jpeg_data == 1) ); /* Not sure we need the last condition */

					buf += buflen;
					if ((buf_contains_jpeg_data == 0) && (jpeg_header_found == 0))
					{
						filesize--;
						buf += buflen;
					}
					if ( (jpeg_header_found == 1) || (buf_contains_jpeg_data == 0) ) break;

				/* Mark blocks processed so-far */
				// deca_bd_mark_blocks(c->b,probed_blk-(filesize-1),filesize-1);

				/*
				 * If we found JPEG signature - good, we start writing it into the file and
				 * continue looking for the foore signature.
				 *
				 * If we did not find otherwise we simply wasted time,
				 * so we should print a warning about it. :-)
				 */
				if (jpeg_header_found == 0)
				{
					//fprintf(stderr,"\nDECA_DECA: Could not find header after probing at %d!!!\n",probed_blk);
					/*
					 * Go back to the probed_block and skip recommended number of blocks assuming we have NOT FOUND a file
					 */
					giveups++;
					fpblocks += blocksprocessed-fphelper;
					c->b->blk = probed_blk;
					prevblk = c->b->blk;
					DECA_BD_SKIP(c->b,c->e.missOffset);
					continue;
				}
			}

			/*
			 * At this point we have found the beginning of JPEG file. buf points to it.
			 * filesize*buflen is the number of butes after *buf that need to be written into the
			 * newly created carved file.
			 */

			/* First check if the footer signature is already in the carvings[] */
			buf_contains_jpeg_footer = deca_detector_tst_jpeg_footer(&(c->d),buf,buflen*filesize,0);

			/* Start writing carved file & look for the footer signature */
			if (verbosity_level>0)
			{
				/* Carving statistics output (CSV format):
				 * carv-start-time,file-number,carv-start-sect,carv-stop-time,carv-stop-sect,file-length,total-carved-clust,total-clust-processed
				 */
				DECA_PROFILER_PRINTF(c->p,"%ld,%d,%ld,",starttime,c->nextfileno,probed_blk-(filesize-1));
			}

			/* Create new file in the output directory and start writing into it */
			//sprintf(nextfilename,"%s/%ld_%d.jpg",c->outputPath,probed_blk-(filesize-1),c->nextfileno);
			sprintf(nextfilename,"%s/%llu.jpg",c->outputPath,c->b->blk);
			c->nextfileno++;
			fout = fopen(nextfilename,"wb");
			if (fout == NULL)
			{
				free(nextfilename);
				free(carvings);
				return DECA_FAIL;
			}

			if (buf_contains_jpeg_footer != 0)
			{
				//fprintf(stderr,"\nDECA_DECA: Found JPEG Footer!!!\n");
				/* The entire file is in the buffer, so we simply save it using buf_contains_jpeg_footer as the file size */
				blockswritten =fwrite(buf,buf_contains_jpeg_footer,1,fout);
				totalsize += (buf_contains_jpeg_footer/512)+1;
			}
			else
			{
				/* Otherwise write out entire buffer until the end of carvings[] space */
				blockswritten = fwrite(buf,buflen*filesize,1,fout);
				totalsize += filesize;
			}
			if (blockswritten != 1)
			{
				free(nextfilename);
				free(carvings);
				return DECA_FAIL;
			}

			/* If we have found the entire file, close the output file */
			if (buf_contains_jpeg_footer!=0)
			{
				/* Print the ending of the profiling information if profiler is armed */
				if (deca_profiler_printtime(&(c->p))>0)
				{
					/* Carving statistics output (CSV format):
					 * carv-start-time,file-number,carv-start-sect,carv-stop-time,carv-stop-sect,file-length,total-carved-clust,total-clust-processed
					 */
					DECA_PROFILER_PRINTF(c->p,",%llu,%ld,%ld,%ld\n",c->b->blk,filesize,totalsize,blocksprocessed);
				}

				/* so close output file */
				if (fclose(fout) != 0)
				{
					free(nextfilename);
					free(carvings);
					return DECA_FAIL;
				}

				/* set blk to the last probel blk */
				c->b->blk = probed_blk;
			}
			else
			{
				/*
				 * ...otherwise (we did not find footer signature),
				 * use linear carving to search for the end signature
				 */

				/* Go back to the probed block */
				c->b->blk = probed_blk;

				/* Restore buf to its usual address at the end of carvigns[] space */
				buf = carvings+(DECA_MAX_CARVED_SIZE-1)*buflen;

				/* set up lastbyte value properly */
				lastbyte = buf[buflen-1];

				do {
					DECA_BD_SKIP(c->b,1);
					DECA_BD_READ_BLOCK(c->b,buf,buflen);
					blocksprocessed++;
					sigtested++;
					totalsize++;

					//buf_contains_jpeg_data = deca_detector_tst_jpeg_data(&(c->d),buf,buflen);
					buf_contains_jpeg_footer = deca_detector_tst_jpeg_footer(&(c->d),buf,buflen,lastbyte);

					if (((buf_contains_jpeg_data == 1)||(buf_contains_jpeg_footer!=0)) && (probed_blk < c->b->blk))
					{
						int blockswritten;
						//DECA_BD_MARK_BLOCK(c->b);
						/* Write it into the output file */
						if (buf_contains_jpeg_footer != 0)
						{
							blockswritten = fwrite(buf,buf_contains_jpeg_footer,1,fout);
						}
						else
						{
							blockswritten = fwrite(buf,buflen,1,fout);
						}
						if (blockswritten != 1)
						{
							free(nextfilename);
							free(carvings);
							return DECA_FAIL;
						}
						filesize +=1;
						totalsize +=1;
					}

					/* If the read block contains footer signature of a JPG file, or NON-JPG data */
					if (buf_contains_jpeg_footer != 0 || (buf_contains_jpeg_data == 0) || (probed_blk > c->b->blk))
					{
						/* Print the ending of the profiling information if profiler is armed */
						if (deca_profiler_printtime(&(c->p))>0)
						{
							/* Carving statistics output (CSV format):
							 * carv-start-time,file-number,carv-start-sect,carv-stop-time,carv-stop-sect,file-length,total-carved-clust,total-clust-processed
							 */
							DECA_PROFILER_PRINTF(c->p,",%llu,%ld,%ld,%ld\n",c->b->blk,filesize,totalsize,blocksprocessed);
						}
						/* close output file */
						if (fclose(fout) != 0)
						{
							free(nextfilename);
							free(carvings);
							return DECA_FAIL;
						}
					}
				} while ((buf_contains_jpeg_footer == 0) && (buf_contains_jpeg_data == 1));
			}
			/*
			 * File has been carved, so now we can skip a prescribed number of blocks assuming that we are
			 * at the end of a successfully carved file.
			 */
			prevblk = c->b->blk;
			DECA_BD_SKIP(c->b,c->e.hitOffset);
		}
		else  /* our initial jpeg test did not succeed - we need to jump and test another block */
		{
			prevblk = c->b->blk;
			DECA_BD_SKIP(c->b,c->e.missOffset);
		}
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

	return DECA_OK;
}

int deca_run(Deca *c, int decaflags)
{
	int result;
	switch (decaflags)
	{
	case DECA_ALGORITHM_LINEAR:
		return deca_linear(c);
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


