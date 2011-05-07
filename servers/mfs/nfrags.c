#include "fs.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>

PUBLIC int fs_nfrags()
{
  register int r;              /* return value */
  register struct inode *rip;  /* target inode */
  off_t pos;
  int nfrags;
  block_t block_number, previous_block_number;


  nfrags=0;


  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL) /* get inode */
	return(EINVAL);
  
  printf("FS_NFRAGS_OK\n");
  for (pos=0; pos<rip->i_size; pos++){                                  /* count number of fragments */
    block_number = read_map(rip,pos);
    if (block_number-previous_block_number>1 || block_number-previous_block_number<0 ){
      printf("currentblock = %d, previous = %d, so we increment nfrags\n", block_number, previous_block_number);
      nfrags++;
    }
    previous_block_number=block_number;
  }
  printf("number of fragments = %d\n", nfrags);





  
  
  put_inode(rip);		/* release the inode */
  return(r);
}
