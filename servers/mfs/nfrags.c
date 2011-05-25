#include "fs.h"
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>


PUBLIC int fs_nfrags()
{
  register int r;              	/* return value */
  register struct inode *rip;  	/* target inode */
  int nfrags;			/* number of fragments of the file */

  if ((rip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL) /* get inode */
	return(EINVAL);
  nfrags = nb_frags(rip);	/* count number of fragments */
  put_inode(rip);		/* release the inode */
  r = sys_safecopyto(fs_m_in.m_source, (cp_grant_id_t) fs_m_in.REQ_GRANT, (vir_bytes) 0, (vir_bytes) &nfrags,
  		(size_t) sizeof(int), D);
  return(r);
}
