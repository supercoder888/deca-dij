/*
 * detector.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Detector of information type contained in the given cluster.
 *  Used to decide if the given cluster is relevant / irrelevant during
 *  probing and carving steps of Decision-theoretic file Carver (DeCa)
 */

#include <magic.h>
#include <linear.h>
#include <libsvm/svm.h>
#include <string.h>
#include "deca.h"
#include "detector.h"

int deca_detector_init(Deca_detector *d, char *modelfile)
{

#ifdef DECA_SVM_JPEG_DETECTION
	/* Load appropriate model file */
	d->m = LOAD_MODEL(modelfile);
	if (d->m == NULL)
	{
		return DECA_MODEL_FILE_NOT_FOUND;
	}

#else
	// Initialise libmagic
	/*
	d->cookie = magic_open(MAGIC_MIME_TYPE);
	if (d->cookie == NULL)
	{
		return DECA_FAIL;
	}
	if(magic_load(d->cookie, NULL) != 0)
	{
		return DECA_FAIL;
	}
	*/
#endif

	return DECA_OK;
}

int deca_detector_tst_jpeg_header(Deca_detector *d, char *buf, int len)
{
   if ((buf[0]=='\xff')&&
	   (buf[1]=='\xd8')&&
	   (buf[2]=='\xff'))
   {
	   if (buf[3]=='\xe1')
	   {
		   return 1;
	   }
	   else
	   {
		   if ((buf[3]=='\xe0')&&(buf[4]=='\x00')&&(buf[5]=='\x10'))
		   {
			   return 1;
		   }
		   else
		   {
			   return 0;
		   }
	   }
   }
   else
   {
	   return 0;
   }
}

int deca_detector_tst_jpeg_footer(Deca_detector *d, char *buf, int len, int lastbyte)
{
	int i;
	for(i=len-1;i>0;i--)
	{
		if(buf[i]=='\xd9')
		{
			if(buf[i-1]=='\xff')
			{
				return i+1;
			}
		}
	}
	if ((buf[1]=='\xd9') && (lastbyte == '\xff'))
	{
		return 1;
	}
	else
	{
	    return 0;
	}
}

int deca_detector_tst_jpeg_data(Deca_detector *d, char *buf, int len)
{

    int i;
    int f[256];
    int esc_count;

#ifdef DECA_SVM_JPEG_DETECTION
    int j;
    double h[256];
    NODE features[1024];  /* Input data is a vector of byte value frequencies */

	//if (deca_detector_tst_jpeg_header(d,buf,len)) return 1;

    /* Initialise frequency array */
    for (i=0; i<256; i++)
    {
    	h[i] = 0.0;
    }

    /* Calculate frequencies of byte values in the buffer */
    for (i=0; i<len; i++)
    {
    	h[(unsigned char)(buf[i])] += 1.0;
    }
    /* Implement scaling here */
    for (i=0; i<256; i++)
    {
    	h[i] = h[i] / (len);
    }
    /* Prepare array of features */
    for (i=0,j=0; i< 256; i++)
    {
        if (h[i] != 0)
        {
        	features[j].index = i+1;
        	features[j].value = h[i];
        	j++;
        }
    }
    features[j].index = -1;
    /* Get prediction on svm model. Return "true" if the prediction is in (0.. +1] or
     * "false" if the prediction is in [-1..0] */
    return (PREDICT(d->m,features))>0;

#else

    /* Initialise frequency array */
    for(i=0; i<256; i++)
    {
    	f[i]=0;
    }

    /* reset counter of reset and escape sequences */
    esc_count=0;

    /* Calculate frequencies of byte values in the buffer */
    for (i=0; i<len-1; i++)
    {
       if ( (f[(unsigned char)(buf[i])] += 1) > 50 ) return 0;
       if (((unsigned char)buf[i])==0xff)
       {
    	   if ( (((unsigned char)buf[i+1])==0) || ((((unsigned char)buf[i+1]) & 0xF8)==0xD0) )
    	   {
    		   esc_count++;
    	   }
    	   else
    	   {
    		   if (((unsigned char)buf[i+1])==0xff)
    		   {
    			   return 0;
    		   }
    	   }
       }
       //if (esc_count != 0) return 1;
    }

    return (esc_count != 0);

	/* JPEG detection using Libmagic */
	/*char *descr = magic_buffer(d->cookie,buf,len);
	if (descr == NULL)
	{
		return 0;
	}
	if (strcmp(descr,"application/octet-stream")==0)
	{
		return 1;
	}
	else
	{
		return 0;
	}*/

#endif
}

int deca_detector_close(Deca_detector *d)
{
	//magic_close(d->cookie);
	return DECA_OK;
}
