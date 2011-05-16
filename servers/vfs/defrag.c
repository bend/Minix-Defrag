#include <defrag.h>
#include <minix/com.h>
#include <minix/u64.h>
#include <string.h>
#include "fs.h"
#include "param.h"
#include <minix/vfsif.h>
#include "vnode.h"
#include "vmnt.h"


PUBLIC int do_defrag()
{
  int r;
  struct vnode *vp;
  cp_grant_id_t grant_id;

  /* initialise r */
  r=OK;

  /* See if file exists */
  if (fetch_name(m_in.name1, m_in.name1_length, M1) != OK) return(err_code);
  if ((vp = eat_path(PATH_NOFLAGS, fp)) == NULL) return(err_code);
  /* Check file type  */
  if( (vp->v_mode & I_TYPE) != I_REGULAR) 
  	r = EPERM;
  if(vp->v_ref_count > 1) r = EBUSY;  /* resource busy if file opened */
  /* If error, free the inode. */
  if (r != OK) {
  	put_vnode(vp);
    	return(r);
  }
  /* send request to file system 									*/
  /* message is sent to the right process, which will have the global variable fs_dev set correctly	*/
  r = req_defrag(vp->v_fs_e, vp->v_inode_nr, who_e, (int*) m_in.name2);
  put_vnode(vp); /* free vnode */
  return r;
}
