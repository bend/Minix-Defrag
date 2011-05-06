#include <defrag.h>
#include "fs.h"


PUBLIC int do_defrag()
{
  int r;
  struct vnode *vp;

  if (fetch_name(m_in.name1, m_in.name1_length, M1) != OK) return(err_code);
  if ((vp = eat_path(PATH_NOFLAGS, fp)) == NULL) return(err_code);
  r = req_defrag(vp->v_fs_e, vp->v_inode_nr, who_e);
  put_vnode(vp);
  return r;
}
