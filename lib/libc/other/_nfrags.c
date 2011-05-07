#include <lib.h>
#include <unistd.h>
#include <nfrags.h>



PUBLIC int nfrags(const char* path )
{
    message m;
    int nfrags;
    /* CREATE MESSAGE */
    m.m1_i1 = strlen(path) + 1;
    m.m1_p1 = (char* ) path;
    _syscall(VFS_PROC_NR, NFRAGS, &m);
    return nfrags
}

