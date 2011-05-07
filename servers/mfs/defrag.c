#include "fs.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>

PUBLIC int fs_defrag()
{
  register int r;              /* return value */
  register struct inode *rip;  /* target inode */

  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL)
	return(EINVAL);
  
  printf("FS_DEFRAG_OK\n");
  put_inode(rip);		/* release the inode */
  return(r);
}