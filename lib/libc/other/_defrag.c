#include <lib.h>
#include <unistd.h>
#include <defrag.h>



PUBLIC int defrag(const char* path)
{
    message m;
    int nfrags;
    /* CREATE MESSAGE */
    m.m1_i1 = strlen(path) + 1;
    m.m1_p1 = (char* ) path;

    m.m1_p2 = (char* ) &nfrags;
    _syscall(VFS_PROC_NR, DEFRAG, &m);
    return nfrags;
}

