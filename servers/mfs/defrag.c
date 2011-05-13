#include "fs.h"
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>
/*===========================================================================*
 *				search_free_region				     *
 *===========================================================================*/
PUBLIC bit_t search_free_region(sp, map, origin, region_size)
struct super_block *sp;		/* the filesystem to allocate from */
int map;			/* IMAP (inode map) or ZMAP (zone map) */
bit_t origin;			/* number of bit to start searching at */
{
/* Allocate a bit from a bit map and return its bit number. */

  block_t start_block;		/* first bit block */
  block_t block;
  bit_t map_bits;		/* how many bits are there in the bit map? */
  short bit_blocks;		/* how many blocks are there in the bit map? */
  unsigned word, bcount;
  struct buf *bp;
  bitchunk_t *wptr, *wlim, k;
  bit_t i, b;
  int current_region_size;
  current_region_size=0;

  if (sp->s_rd_only)
	panic("can't allocate bit on read-only filesys");

  start_block = START_BLOCK + sp->s_imap_blocks;
  map_bits = (bit_t) (sp->s_zones - (sp->s_firstdatazone - 1));
  bit_blocks = sp->s_zmap_blocks;
  

  /* Figure out where to start the bit search (depends on 'origin'). */
  if (origin >= map_bits) origin = 0;	/* for robustness */

  /* Locate the starting place. */
  block = (block_t) (origin / FS_BITS_PER_BLOCK(sp->s_block_size));
  word = (origin % FS_BITS_PER_BLOCK(sp->s_block_size)) / FS_BITCHUNK_BITS;

  /* Iterate over all blocks plus one, because we start in the middle. */
  bcount = bit_blocks + 1;
  do {
    /* fs_dev global variable is set correctly at mount time for this process  */
	bp = get_block(sp->s_dev, start_block + block, NORMAL);
	wlim = &bp->b_bitmap[FS_BITMAP_CHUNKS(sp->s_block_size)];

	/* Iterate over the words in block. */
	for (wptr = &bp->b_bitmap[word]; wptr < wlim; wptr++) {

		/* Does this word contain a free bit? */
		if (*wptr == (bitchunk_t) ~0)  {
            current_region_size=0;
            continue;
        }

		/* Increment the free bits. */
		k = (bitchunk_t) conv2(sp->s_native, (int) *wptr);
		for (i = 0; i<FS_BITCHUNK_BITS && current_region_size<region_size ; ++i) { 
            if ( (k & (1 << i)) == 0){
              current_region_size++;

            }
            else{
                printf("region ended at size %d  \n", current_region_size);
                current_region_size=0;
            }
        }

		/* Bit number from the start of the bit map. */
		b = ((bit_t) block * FS_BITS_PER_BLOCK(sp->s_block_size))
		    + (wptr - &bp->b_bitmap[0]) * FS_BITCHUNK_BITS
		    + i;

		/* Don't allocate bits beyond the end of the map. */
		if (b >= map_bits) { printf("no region found because beyond end of map")  ;break;}

        b=b-region_size; /* go back to first bit of zone */


		/* Allocate and return bit number. */
        /*
		k |= 1 << i;
		*wptr = (bitchunk_t) conv2(sp->s_native, (int) k);
		bp->b_dirt = DIRTY;
		put_block(bp, MAP_BLOCK);
        */
		return(b);
	}
	put_block(bp, MAP_BLOCK);
	if (++block >= (unsigned int) bit_blocks) /* last block, wrap around */
		block = 0;
	word = 0;
  } while (--bcount > 0);
  return(NO_BIT);		/* no bit could be allocated */
}

PUBLIC int fs_defrag()
{
  register int r;              /* return value */
  register struct inode *rip;  /* target inode */
  int nblocks;
  int pos,i, scale, block_count;
  long zone;
  zone_t current_zone, previous_zone;
  bit_t first_bit; 
  block_t block_number, previous_block_number, first_block, src_block;
  struct buf *bp_src, *bp_dst;
  nblocks = 1;
  pos = 0;

  r=0;

  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL)
	return(EINVAL);

  /* set scale for block-zone conversion */
  scale = rip->i_sp->s_log_zone_size;

  for (pos=0; pos<rip->i_size; pos++){                                  /* count number of fragments */
    block_number = read_map(rip,pos);
    if (block_number!=previous_block_number){
      nblocks++;
    }
    previous_block_number=block_number;
  }

  /*
  print_map(rip->i_sp, ZMAP);
  */

  printf("need %d blocks region", nblocks);
  first_bit = search_free_region(rip->i_sp, ZMAP,0 ,nblocks); 

  for(i=0; i<nblocks; i++) {
    alloc_bit(rip->i_dev, ZMAP, first_bit+i);
  }

  /* see alloc_zone */
  first_block = ( (zone_t) (rip->i_sp->s_firstdatazone - 1) + (zone_t) first_bit);
  printf("first bits : %d\n", first_bit);
  /*
  first_bit = search_free_region(rip->i_sp, ZMAP,first_bit ,nblocks); 
  printf("FS_DEFRAG_OK %d\n", first_bit);
  */
  /*
  allouer toutes les  zones
  */
  block_count=0;
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size,block_count++){
    src_block = read_map(rip,pos);
    printf(" copy count %d src %d to dst %d\n", block_count, src_block, first_block+block_count);
    /* read current block */
    bp_src = get_block(rip->i_dev, src_block, 1);  /* defined in cache.c */
    bp_dst = get_block(rip->i_dev, first_block+block_count, 1);  
    /* write block in current zone */
    printf("memset \n");
    memset(bp_dst->b_data, bp_src->b_data, (size_t) bp_src->b_bytes);
    bp_dst->b_dirt = DIRTY;

    /*free cached blocks*/
    printf("put block \n");
    put_block(bp_src,PARTIAL_DATA_BLOCK);
    put_block(bp_dst,PARTIAL_DATA_BLOCK);

   }
   previous_block_number=block_number;
    
  }
  /*modify inode*/
  zone = (pos/rip->i_sp->s_block_size) >> scale;

  printf("modify inode\n");
  for (i=0; i<V2_NR_TZONES; i++) {
    printf("zone number[%d] = %d\n", i, rip->i_zone[i]);
  }
  printf("----------------------------------\n");
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size){ 
    write_map(rip, pos, NULL, WMAP_FREE); 
  }
  printf("did clear inode\n");
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size){ 
    block_number = (pos/rip->i_sp->s_block_size)+first_block;
    zone = block_number >> scale;
    write_map(rip, pos, zone, 0);
  }
  rip->i_dirt=DIRTY;
  printf("zones after :\n");
  for (i=0; i<V2_NR_TZONES; i++) {
    printf("zone number[%d] = %d\n", i, rip->i_zone[i]);
  }
  printf("release inode\n");
  put_inode(rip);		/* release the inode */
  return(r);
}


PUBLIC void print_map(sp, map)
struct super_block *sp;		/* the filesystem to allocate from */
int map;			/* IMAP (inode map) or ZMAP (zone map) */
{
/* Allocate a bit from a bit map and return its bit number. */

  block_t start_block;		/* first bit block */
  block_t block;
  bit_t map_bits;		/* how many bits are there in the bit map? */
  short bit_blocks;		/* how many blocks are there in the bit map? */
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
  if (origin >= map_bits) origin = 0;	/* for robustness */
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

