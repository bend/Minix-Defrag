#include <nfrags.h>
#include <defrag.h>
#include <stdio.h>

int
main (int argc, const char * argv[])
{
        int result;
        result = defrag(argv[1]);
        if (result<1)
                return 1;
        if (result==1)
                printf("did not defragment existing defragmented file\n");
        else
                printf("defragmented file composed of %d fragments\n", result);
        return 0;

}     
