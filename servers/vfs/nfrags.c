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

  if (fetch_name(m_in.name1, m_in.name1_length, M1) != OK) return(err_code);
  if ((vp = eat_path(PATH_NOFLAGS, fp)) == NULL) return(err_code);
  r = req_nfrags(vp->v_fs_e, vp->v_inode_nr, who_e);
  put_vnode(vp);
  return r;
}
