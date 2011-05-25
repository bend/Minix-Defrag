#include <lib.h>
#include <unistd.h>
#include <nfrags.h>



PUBLIC int nfrags(const char* path )
{
  message m;			/* the message sent */
  int nfrags;			/* here will be stored the return value of nfrags */
  /* CREATE MESSAGE */
  m.m1_i1 = strlen(path) + 1;
  m.m1_p1 = (char* ) path;
  /* we pass the pointer to nfrag in the message after a cast to char*.
   * In do_nfrags, this pointer will be passed as aggument when calling req_nfrags
   * req_nfrags will do a vircopy of the returned value to nfrags
   */ 
  m.m1_p2 = (char* ) &nfrags;
  _syscall(VFS_PROC_NR, NFRAGS, &m);
  return (nfrags);
}

