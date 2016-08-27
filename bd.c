/*
 * detector.c
 *
 *  Created on: 20 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  Detector of information type contained in the given cluster.
 *  Used to decide if the given cluster is relevant / irrelevant during
 *  probing and carving steps of Decision-theoretic file Carver (DeCa).
 *
 *  This file is essentially a wrapper around The Sleuthkit library by B. Carrier
 */

#include <tsk/libtsk.h>
#include <string.h>
#include "deca.h"
#include "bd.h"

/* Function that marks unallocated blocks in bd->map with 0s
 * (a callback for tsk_fs_block_walk()
 */
TSK_WALK_RET_ENUM deca_bd_unallocated_cb(TSK_FS_BLOCK *blk, void *context)
{
	Deca_bd *b = (Deca_bd*)context;
    if (blk->addr >= b->size) return TSK_WALK_ERROR;
	b->map[blk->addr] = 0;
	b->blk = blk->addr; /* Set first block to probe during carving */
	b->btc++;
	return TSK_WALK_CONT;
}

ssize_t deca_bd_img_read_block(void *context, uint64_t blk, char* buf, size_t len)
{
	TSK_IMG_INFO *i = (TSK_IMG_INFO*)context;
	return tsk_img_read(i,blk * (i->sector_size), buf, len);
}

int deca_bd_init(Deca_bd *b, int imgcnt, char **imgpaths, int partNo, int flags)
{
	uint8_t result;

	b->i = NULL;
	b->v = NULL;
	b->p = NULL;
	b->f = NULL;
	b->map = NULL;
	b->wrap_around = 0;

	/* Get access to the disk image or block device */
	if ((imgpaths == NULL) || (imgcnt == 0)) return DECA_IMAGE_NOT_FOUND;
	b->i = tsk_img_open(imgcnt,
	  		            (const TSK_TCHAR * const*) imgpaths,
	  		            TSK_IMG_TYPE_DETECT,
	       		        0);
	if (b->i == NULL) return DECA_IMAGE_NOT_FOUND;

	/* Now analyze the flags and set appropriate read function pointer and parameters  */
	switch (flags)
	{
	case DECA_BD_DISK:
		b->skip = deca_bd_goto_skip;
		b->skip_back = deca_bd_goto_back;
		b->read = deca_bd_img_read_block;
		b->context = (void*)b->i;
		b->bs = b->i->sector_size;
		b->size = b->i->size / b->bs;
		b->btc = b->size;
		b->map = (char *)malloc(b->size);
		if (b->map == NULL)
		{
			deca_bd_close(b);
			return DECA_FAIL;
		}
		b->map = memset(b->map,0,b->size);
		b->blk = 0; /* First block to probe */
		break;

	case DECA_BD_VOLUME:
	case (DECA_BD_VOLUME|DECA_BD_FS):
		/* Open volume and get handle to the specified partition */
		b->v = tsk_vs_open(b->i, 0, TSK_VS_TYPE_DETECT);
		if (b->v == NULL)
		{
		    deca_bd_close(b);
			return DECA_DISK_NOT_PARTITIONED;
		}
		b->p = tsk_vs_part_get(b->v, partNo);
		if (b->v == NULL)
		{
		    deca_bd_close(b);
			return DECA_VOLUME_NOT_FOUND;
		}

		if (flags & DECA_BD_FS)  /* If DECA_BD_FS flag is set, carve unallocated space */
		{
			b->f = tsk_fs_open_vol(b->p,TSK_FS_TYPE_DETECT);
			if (b->v == NULL)
			{
			    deca_bd_close(b);
				return DECA_FS_NOT_PRESENT;
			}
			b->read = (ssize_t (*)(void *, uint64_t, char *, size_t))tsk_fs_read_block;
			b->skip = deca_bd_skip;
			b->skip_back = deca_bd_skip_back;
			b->context = b->f;
			b->bs = b->f->block_size;
			b->size = b->f->block_count;
			b->map = (char *)malloc(b->size);
			if (b->map == NULL)
			{
				deca_bd_close(b);
				return DECA_FAIL;
			}
			b->map = memset(b->map,1,b->size);
			b->btc = 0;
			/* Now walk only through unallocated blocks and mark them all in the map */
			result = tsk_fs_block_walk(
					    b->f,
					    b->f->first_block,
					    b->f->last_block,
					    TSK_FS_BLOCK_WALK_FLAG_AONLY  | TSK_FS_BLOCK_WALK_FLAG_UNALLOC,
					    (TSK_FS_BLOCK_WALK_CB)deca_bd_unallocated_cb,
					    (void *)b);
			if (result == 1)
			{
				deca_bd_close(b);
				return DECA_FAIL;
			}
		}
		else   /* Treat partition as a raw disk for carving */
		{
			b->read = (ssize_t (*)(void *, uint64_t, char *, size_t))tsk_vs_part_read_block;
			b->skip = deca_bd_goto_skip;
			b->skip_back = deca_bd_goto_back;
			b->context = (void*)b->p;
			b->bs = b->v->block_size;
			b->size = b->p->len;
			b->btc = b->size;
			b->map = (char *)malloc(b->size);
			if (b->map == NULL)
			{
				deca_bd_close(b);
				return DECA_FAIL;
			}
			b->map = memset(b->map,0,b->size);
			b->blk = 0;  /* First block to probe */
		}
		break;

	default:
	    deca_bd_close(b);
		return DECA_WRONG_BD_FLAGS;
	}

    return DECA_OK;
}


int deca_bd_skip(Deca_bd *b, uint64_t offset)
{
	if ((b->btc) <= 0)  //If there are no carvable blocks, this function will loop forever.
	{
		return DECA_FAIL;
	}

	while (offset>0)
	{
		b->blk = (b->blk+1) % b->size;
		if (b->map[b->blk]==0)
		{
			offset--;
		}
	}
	return DECA_OK;
}

int deca_bd_goto(Deca_bd *bd, uint64_t blk)
{
	bd->blk = blk % bd->size;
	return DECA_OK;
}

int deca_bd_skip_back(Deca_bd *bd, uint64_t offset)
{
	int skipped;
	for (skipped=0;

		(offset > 0) && (bd->blk > 0) && (bd->map[bd->blk-1] == 0);

		(offset--),(bd->blk--),skipped++);
	return skipped;
}

int deca_bd_goto_skip(Deca_bd *bd, uint64_t blk)
{
	uint64_t newpos = bd->blk+blk;
	if (newpos > bd->size)
	{
 	   bd->blk = newpos % bd->size;
 	   bd->wrap_around = 1;
	}
	else
	   bd->blk = newpos;
	return DECA_OK;
}

int deca_bd_goto_back(Deca_bd *bd, uint64_t blk)
{
    if (blk <= bd->blk)
    {
	   bd->blk = bd->blk-blk;
	   return blk;
    }
    else
    {
       uint64_t oldblk = bd->blk;
       bd->blk = 0;
	   return oldblk;
    }
}

/**
 * part_act() callback function from mmls.cpp  Prints partition layout info during part_walk()
 *
 */
static TSK_WALK_RET_ENUM
part_act(TSK_VS_INFO * vs, const TSK_VS_PART_INFO *part, void *ptr)
{
	FILE *f = (FILE *)ptr;
    if (part->flags & TSK_VS_PART_FLAG_META)
        fprintf(f,"%.2" PRIuPNUM ":  Meta    ", part->addr);

    /* Neither table or slot were given */
    else if ((part->table_num == -1) && (part->slot_num == -1))
        fprintf(f,"%.2" PRIuPNUM ":  -----   ", part->addr);

    /* Table was not given, but slot was */
    else if ((part->table_num == -1) && (part->slot_num != -1))
        fprintf(f,"%.2" PRIuPNUM ":  %.2" PRIu8 "      ",
            part->addr, part->slot_num);

    /* The Table was given, but slot wasn't */
    else if ((part->table_num != -1) && (part->slot_num == -1))
        fprintf(f,"%.2" PRIuPNUM ":  -----   ", part->addr);

    /* Both table and slot were given */
    else if ((part->table_num != -1) && (part->slot_num != -1))
        fprintf(f,"%.2" PRIuPNUM ":  %.2d:%.2d   ",
            part->addr, part->table_num, part->slot_num);


        /* Print the layout */
        fprintf(f,"%.10" PRIuDADDR "   %.10" PRIuDADDR "   %.10" PRIuDADDR
            "   %s\n", part->start,
            (TSK_DADDR_T) (part->start + part->len - 1), part->len,
            part->desc);

    return TSK_WALK_CONT;
}


int deca_bd_list_partitions(Deca_bd *b, FILE *s)
{
	if (tsk_vs_part_walk(b->v,
			             0,
			             b->v->part_count - 1,
	                     TSK_VS_PART_FLAG_ALL,
	                     part_act, (void *)s))
	{
       deca_bd_close(b);
	   return DECA_FAIL;
	}
	return DECA_OK;
}


int deca_bd_close(Deca_bd *b)
{
   if (b->map != NULL) { free(b->map); }
   if (b->f != NULL) { tsk_fs_close(b->f); }
   if (b->v != NULL) { tsk_vs_close(b->v); }
   if (b->i != NULL) { tsk_img_close(b->i); }
   return DECA_OK;
}

void deca_bd_mark_blocks(Deca_bd *bd, uint64_t start, uint64_t nblocks)
{
	int i;
	for (i=start; (i<(bd->size)) && (nblocks >0); i++, nblocks--)
	{
		bd->map[i]=1;
		bd->btc--;
	}
	return;
}
