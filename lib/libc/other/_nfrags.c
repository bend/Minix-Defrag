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
    /* on passe le pointeur vers nfrags dans le message apres l'avoir casté en (char *) 
    dans do_nfrags ce pointeur sera passé en argument à l'appel req_nfrags, qui fera un vircopy vers la case de ce pointeur de la valeur calculée par fs_nfrags
    */  
    m.m1_p2 = (char* ) &nfrags;
    _syscall(VFS_PROC_NR, NFRAGS, &m);
    return nfrags;
}

