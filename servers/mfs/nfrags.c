#include "fs.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>

PUBLIC int fs_nfrags()
{
  register int r;              /* return value */
  register struct inode *rip;  /* target inode */
  int i;

  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL)
	return(EINVAL);
  
  printf("FS_NFRAGS_OK\n");
  for(i=0; i++; i<V2_NR_TZONES){
	  printf("zone number %d = %u", i, rip->i_zone[i]);
  }
  put_inode(rip);		/* release the inode */
  return(r);
}
