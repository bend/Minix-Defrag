#include <nfrags.h>
#include <defrag.h>
#include <stdio.h>
                                                                                                                                                 
int
main (int argc, const char * argv[])
{
        int result;
        result = nfrags(argv[1]);
        printf("%d fragments\n", result);
                                                                                                                                                 
}                                                    
