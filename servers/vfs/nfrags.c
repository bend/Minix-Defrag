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
  int nfrags;
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
      printf("result not ok, returning vnode\n");
	  put_vnode(vp);
	  return(r);
  }
  /* send request to file system */
  printf("sending request to fs\n");
  nfrags = req_nfrags(vp->v_fs_e, vp->v_inode_nr, who_e, &nfrags);
  printf("number of frags received from request: %d\n", nfrags);
  put_vnode(vp);
  return r;
}
