#include <nfrags.h>
#include <minix/com.h>
#include <minix/u64.h>
#include <string.h>
#include "fs.h"
#include "param.h"
#include <minix/vfsif.h>
#include "vnode.h"
#include "vmnt.h"


PUBLIC int do_nfrags()
{
  int r;
  struct vnode *vp;
  cp_grant_id_t grant_id;

  /* initialise r */
  r=OK;

  /* See if file exists */
  if( fetch_name(m_in.name1, m_in.name1_length, M1) != OK) return(err_code);
  if( (vp = eat_path(PATH_NOFLAGS, fp)) == NULL) return(err_code);
  /* Check file type  */
  if( (vp->v_mode & I_TYPE) != I_REGULAR ) 
      r = EPERM;
  /* If error, return the inode. */
  if (r != OK) {
	  put_vnode(vp);
	  return(r);
  }
  /* send request to file system */
  
  /* m_in.name2 has pointer int * to  nfrags created in libc function */
  r = req_nfrags(vp->v_fs_e, vp->v_inode_nr, who_e, (int *) m_in.name2);
  put_vnode(vp);
  return r;
}
