#include "fs.h"
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>


PUBLIC int fs_nfrags()
{
  register int r;              /* return value */
  register struct inode *rip;  /* target inode */
  off_t pos;
bit_t first_bit;
int nblocks;
  block_t block_number, previous_block_number;
  int nfrags, scale;


  nfrags=0;


  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL) /* get inode */
	return(EINVAL);
  scale = rip->i_sp->s_log_zone_size;
   
  printf("FS_NFRAGS_OK\n");
  for (pos=0; pos<rip->i_size; pos++){                                  /* count number of fragments */
    block_number = read_map(rip,pos);
    if (block_number!=previous_block_number)
      printf("currentblock = %d\n", block_number>>scale);
    if (block_number-previous_block_number>1 || block_number-previous_block_number<0 ){
      
      nfrags++;
    }
    previous_block_number=block_number;
  }

  printf("will print map\n");
  print_map(rip->i_sp, ZMAP);

  for (pos=0; pos<rip->i_size; pos++){                                  /* count number of fragments */
    block_number = read_map(rip,pos);
    if (block_number!=previous_block_number){
      nblocks++;
    }
    previous_block_number=block_number;
  }
  
  printf("need %d blocks region", nblocks);
  first_bit = search_free_region(rip->i_sp, ZMAP,0 ,nblocks>>scale);





  printf("number of fragments = %d\n", nfrags);
  
  put_inode(rip);		/* release the inode */
  /*works fine  */
  r = sys_safecopyto(fs_m_in.m_source, (cp_grant_id_t) fs_m_in.REQ_GRANT, (vir_bytes) 0, (vir_bytes) &nfrags,
  		(size_t) sizeof(int), D);
  
  return(r);
}
