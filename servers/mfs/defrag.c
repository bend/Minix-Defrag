#include "fs.h"
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>
/*===========================================================================*
 *				search_free_region              	     *
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
  bit_t i, b;                   /* bit iteration + bit found */
  int current_region_size;
  current_region_size=0;

  if (sp->s_rd_only) panic("can't allocate bit on read-only filesys");

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
    for (wptr = &bp->b_bitmap[word]; wptr < wlim && current_region_size<region_size; wptr++) {
      /* Does this word contain a free bit? */
      if (*wptr == (bitchunk_t) ~0)  {
        current_region_size=0;
        continue;
      }

      /* Increment the free bits. */
      k = (bitchunk_t) conv2(sp->s_native, (int) *wptr);
      for (i = 0; i<FS_BITCHUNK_BITS && current_region_size<region_size ; i++) { 
        if ( (k & (1 << i)) == 0){
          b = ((bit_t) block * FS_BITS_PER_BLOCK(sp->s_block_size))
            + (wptr - &bp->b_bitmap[0]) * FS_BITCHUNK_BITS
            + i;
          current_region_size++;
        }
        else{
          b = ((bit_t) block * FS_BITS_PER_BLOCK(sp->s_block_size))
            + (wptr - &bp->b_bitmap[0]) * FS_BITCHUNK_BITS
            + i;
          current_region_size=0;
        }
      }

    }
    put_block(bp, MAP_BLOCK);
    if (++block >= (unsigned int) bit_blocks) /* last block, wrap around */
      block = 0;
    word = 0;
  } while (--bcount > 0 && current_region_size<region_size);

  if (current_region_size>=region_size) {
    /* Don't allocate bits beyond the end of the map. */
    if (b >= map_bits) { return(NO_BIT);}
    b=b-region_size+1; /* go back to first bit of zone */
    return(b);
  }
  return(NO_BIT);		/* no bit could be allocated */
}

PUBLIC int fs_defrag()
{
  register int r;               /* return value */
  register struct inode *rip;   /* target inode */
  int nblocks;                  /* number of blocks used by the file*/
  int pos,i, scale, block_count;/* position in file, iteration variable, block-zone conversion, block counter */
  long zone;                    /* zone mnipulated*/
  int nfrags;                   /* number of fragments */
  zone_t first_zone;            /* first zone of the new file */
  bit_t first_bit, current_bit ;/* first bit of new file in zonemap + iteration variable */ 
  block_t block_number,         /*iteration variables for block copy*/
    previous_block_number, first_block, src_block;
  struct buf *bp_src, *bp_dst;  /* block pointers for block copy*/

  previous_block_number = -1;
  nblocks = 0;
  pos = 0;
  r=0;

  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL)
    return(EINVAL);

  nfrags = nb_frags(rip);
  if(nfrags == 1) {		/* No need to defrag, file is not fragmented */
    r = sys_safecopyto(fs_m_in.m_source, (cp_grant_id_t) fs_m_in.REQ_GRANT, (vir_bytes) 0, (vir_bytes) &nfrags,
        (size_t) sizeof(int), D);
    return r;
  }
  /* set scale for block-zone conversion */
  scale = rip->i_sp->s_log_zone_size;
  /* count number of blocks */
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size) {  
    block_number = read_map(rip,pos);
    if (block_number!=previous_block_number) {
      nblocks++;
    }
    previous_block_number=block_number;
  }

  first_bit = search_free_region(rip->i_sp, ZMAP,0 ,nblocks>>scale); 
  if (first_bit == NO_BIT) {
    nfrags=-1; /* set negative return value to notify of error*/
    r = sys_safecopyto(fs_m_in.m_source, (cp_grant_id_t) fs_m_in.REQ_GRANT, (vir_bytes) 0, (vir_bytes) &nfrags,
        (size_t) sizeof(int), D);
    return ENOSPC;
  }
  for(i=0; i<nblocks; i++) {
    alloc_this_bit(rip->i_sp, ZMAP, first_bit+i);
  }

  /* see alloc_zone */
  first_zone = ( (zone_t) (rip->i_sp->s_firstdatazone - 1) + (zone_t) first_bit);
  first_block = first_zone<<scale;

  /* allocate all zones */
  current_bit = first_bit;

  block_count=0;
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size,block_count++) {
    src_block = read_map(rip,pos);
    /* read current block */
    bp_src = get_block(rip->i_dev, src_block, 1);  /* defined in cache.c */
    bp_dst = get_block(rip->i_dev, first_block+block_count, 1);  
    /* write block in current zone */
    memcpy(bp_dst->b_data, bp_src->b_data, (size_t) bp_src->b_bytes);
    bp_dst->b_dirt = DIRTY;

    /*free cached blocks*/
    put_block(bp_src,PARTIAL_DATA_BLOCK);
    put_block(bp_dst,PARTIAL_DATA_BLOCK);

  }
  /*modify inode*/
  zone = (pos/rip->i_sp->s_block_size) >> scale;

  /* free old zones */ 
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size) { 
    write_map(rip, pos, 0, WMAP_FREE); 
  }
  /* attach zones to inode */
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size) { 
    block_number = (pos/rip->i_sp->s_block_size)+first_block;
    zone = block_number >> scale;
    write_map(rip, pos, zone, 0);
  }
  /* mark inode as dirty and give it back */
  rip->i_dirt=DIRTY;
  put_inode(rip);		/* release the inode */
  /* return number of fragments of original file */
  r = sys_safecopyto(fs_m_in.m_source, (cp_grant_id_t) fs_m_in.REQ_GRANT, (vir_bytes) 0, (vir_bytes) &nfrags,
      (size_t) sizeof(int), D);
  return(r);
}



