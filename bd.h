/*
 * block.h
 *
 *  Created on: 21 Jun 2015
 *      Author: Pavel Gladyshev
 *
 *  API to the block device abstraction used by Decision-theoretic file Carver (DeCa)
 *  to access images and block devices.
 *
 *  This file is essentially a wrapper for The Sleuthkit library by B.Carrier
 */

#ifndef BD_H_
#define BD_H_

/**
 * Deca block device flags
 */
enum DECA_BD_FLAG_ENUM {
	DECA_BD_DISK =0,   //!< DECA_BD_DISK - carve blocks from the physical disk/image ignoring volumes and file systems (if present)
	DECA_BD_VOLUME =1, //!< DECA_BD_VOLUME - carve blocks from specified partition/volume ignoring the file system (if present)
	DECA_BD_FS =2     //!< DECA_BD_FS - carve block from unallocated space on the file system
};

/**
 * Deca_bd data structure. The user must create an instance of Deca_bd and
 * initialze it using deca_bd_init() function before use.
 */
typedef struct deca_bd_struct
{
    TSK_IMG_INFO  *i;    //!< Pointer to the sleuthkit disc/image object
    TSK_VS_INFO   *v;    //!< The Sleuthkit volume object
    const TSK_VS_PART_INFO *p;    //!< The Sleuthkit partition object
    TSK_FS_INFO   *f;    //!< The Sleuthkit filesystem object
    char          *map;  //!< Map of blocks/clusters excluded from carving (=1 excluded, =0 included)
    int            bs;   //!< Block size in bytes
    uint64_t       size; //!< Number of blocks on the device
    uint64_t       blk;  //!< Next block for reading
    uint64_t       btc;  //!< Blocks-To-Carve - the number of blocks available for carving
    int			   wrap_around; //!< Indicator that DECA algorithm wrapped around.

    /**
     * Pointer to the function to read a block from physical disk / volume / or file system
     * @param context - pointer to the corresponding disk / volue/ or filesystem object
     * @param addr - block to read
     * @param buf - pointer to the buffer
     * @param len - size of buffer
     * @return -1 on error or the number of butes read.
     */
    ssize_t (*read)(void *context, uint64_t addr, char *buf, size_t len);
    int (*skip) (struct deca_bd_struct *bd, uint64_t offset);
    int (*skip_back) (struct deca_bd_struct *bd, uint64_t offset);

    void *context;


} Deca_bd;

/**
 * Deca_bd initializer. The user is responsible for creating an instance
 * of Deca_bd type and initializing it with this function before use.
 * @param *bd - pointer to the Deca_bd structure to initialize
 * @param imageCount - number of image file fragments
 * @param imagePaths - array of paths to individual image file fragments
 * @param partNo - number of the partition to use (ignored with DECA_BD_DISK)
 * @param flags - mode in which to open the disk
 * @return deca error code
 */
int deca_bd_init(Deca_bd *bd, int imgcnt, char **imgpath, int partNo, int flags);

/**
 * Print the list of partitions in the given disk / image
 * @param *bd - pointer to the Deca_bd structure initialized with deca_bd_init(). MUST BE INITIALIZED WITH DECA_BD_VOLUME flag and partition number 0 !!!
 * @param stream - output stream to print into (e.g. stdout)
 * @return deca error code
 */
int deca_bd_list_partitions(Deca_bd *bd,FILE *stream);

/**
 * Read current block from the disk / image using initialised Deca_bd object
 * @param bd - Deca_bd object initialised with deca_bd_init()
 * @param buf - buffer for the data
 * @param len - length of the buffer in bytes
 * @return number of bytes read or -1 if error
 */
#define DECA_BD_READ_BLOCK(bd,buf,len) ((bd)->read((bd)->context, (bd)->blk, (buf), (len)))

/**
 * Mark current block as not carvable in the map
 * @param bd - Deca_bd object initialised with deca_bd_init()
 * @return always 1
 */
#define DECA_BD_MARK_BLOCK(bd) ((bd)->map[(bd)->blk]=1,(bd)->btc--)

/**
 * Skip given number of blocks, skipping over uncarvable blocks as
 * specified in Deca_bd maps and wrapping around to the beginning
 * of disk if necessary
 * @param bd - Deca block device object initialised with deca_bd_init()
 * @param offset - number of blocks to skip (>0)
 * @return deca error code
 */
int deca_bd_skip(Deca_bd *bd, uint64_t offset);
int deca_bd_goto_skip(Deca_bd *bd, uint64_t offset);

/**
 * Go to the block with specified number the information in Deca_bd map
 * is ignored.
 * @param bd - Deca block device object initialised with deca_bd_init()
 * @param blk - number of the block to go to (>=0)
 * @return deca error code
 */
int deca_bd_goto(Deca_bd *bd, uint64_t blk);

/**
 * Skip back given number of blocks, stopping at the first uncarvable block encountered
 *
 * @param bd - Deca block device object initialised with deca_bd_init()
 * @param offset - number of blocks to skip backwards(>0)
 * @return number of blocks actually skipped
 */
int deca_bd_skip_back(Deca_bd *bd, uint64_t offset);

/**
 * Skip back given number of blocks, stopping at the first uncarvable block encountered
 *
 * @param bd - Deca block device object initialised with deca_bd_init()
 * @param offset - number of blocks to skip backwards(>0)
 * @return number of blocks actually skipped
 */
int deca_bd_goto_back(Deca_bd *bd, uint64_t offset);

/**
 * macros used to invoke bd->skip() and bd->skip_back(),
 * which are set during initialisation to point to either deca_bd_skip()/deca_bd_goto() and
 * deca_bd_skip_back()/deca_bd_goto_back() respectively
 * @param bd - Deca block device object initialised with deca_bd_init()
 * @param blk - number of the block to go to (>=0)
 */
#define DECA_BD_SKIP(bd,offset) ((bd)->skip((bd),(offset)))
#define DECA_BD_SKIP_BACK(bd,offset) ((bd)->skip_back((bd),(offset)))


/**
 * Mark a number of blocks starting from the given block
 * @param bd - Deca block device object initialised with deca_bd_init()
 * @param start_blk - strting block
 * @param nblocks - number of blocks to mark
 * @return deca error code
 */
void deca_bd_mark_blocks(Deca_bd *bd, uint64_t start, uint64_t nblocks);

/**
 * Close a previously initialized Deca_bd object by releasing any allocated resources.
 * @param b - Deca_bd object initialized with deca_bd_init()
 * @return deca error code
 */
int deca_bd_close(Deca_bd *b);


#endif /* BD_H_ */
