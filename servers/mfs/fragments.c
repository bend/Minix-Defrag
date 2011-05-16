#include "fs.h"
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>

PUBLIC void print_map(sp, map)
struct super_block *sp;     /* the filesystem to allocate from */
int map;            /* IMAP (inode map) or ZMAP (zone map) */
{
    /* Allocate a bit from a bit map and return its bit number. */

    block_t start_block;      /* first bit block */
    block_t block;
    bit_t map_bits;       /* how many bits are there in the bit map? */
    short bit_blocks;     /* how many blocks are there in the bit map? */
    unsigned word, bcount;
    struct buf *bp;
    bitchunk_t *wptr, *wlim, k;
    bit_t i, b;
    int origin;
    int count_bits;
    int current_region_size;
    current_region_size=0;
    origin=0;
    start_block = START_BLOCK + sp->s_imap_blocks;
    map_bits = (bit_t) (sp->s_zones - (sp->s_firstdatazone - 1));
    bit_blocks = sp->s_zmap_blocks;
    count_bits=0;

    /* Figure out where to start the bit search (depends on 'origin'). */
    if (origin >= map_bits) origin = 0;   /* for robustness */
    /* Locate the starting place. */
    block = (block_t) (origin / FS_BITS_PER_BLOCK(sp->s_block_size));
    word = (origin % FS_BITS_PER_BLOCK(sp->s_block_size)) / FS_BITCHUNK_BITS;

    /* Iterate over all blocks plus one, because we start in the middle. */
    bcount = bit_blocks + 1;
    do {
	bp = get_block(sp->s_dev, start_block + block, NORMAL);
	wlim = &bp->b_bitmap[FS_BITMAP_CHUNKS(sp->s_block_size)];

	/* Iterate over the words in block. */
	for (wptr = &bp->b_bitmap[word]; wptr < wlim; wptr++) {
	    if(count_bits > map_bits)
		return;

	    /* Increment the free bits. */
	    k = (bitchunk_t) conv2(sp->s_native, (int) *wptr);
	    for (i = 0; i<FS_BITCHUNK_BITS ; ++i) { 
		count_bits++;
		if ( (k & (1 << i)) == 0){
		    printf("_ ");
		}
		else{
		    printf("* ");
		}
	    }
	}
	put_block(bp, MAP_BLOCK);
    } while (--bcount > 0);
    printf("\n");
}


PUBLIC int nb_frags(rip)
register struct inode *rip;     /* inode pointer */
{
  int nfrags;                   /* number of fragments of the file*/
  off_t pos;                    /* position in the file (for iteration)*/
  block_t block_number,         /* iteration variables*/
    previous_block_number;

  nfrags = 0;
  /* count number of fragments */
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size){                               
    block_number = read_map(rip,pos);
    if (block_number!=previous_block_number){
      if (block_number-previous_block_number>1 || block_number-previous_block_number<0) {
        nfrags++;
      }
    }
    previous_block_number=block_number;
  }
  return(nfrags);
}
