#include <stdio.h>

#include "stuff.h"
#include "PE.h"
#include "porg_utils.h"

void add_symbol (address a, char *name, void *dummy)
{
    printf ("add_symbol (0x%x, %s)\n", (unsigned)a, name);
};

int main(int argc, char* argv[])
{
    PE_info info;

    if (argc!=2)
        die ("use: %s filename.(exe|dll)\n", argv[0]);

    PE_get_info (argv[1], 0, &info, add_symbol, NULL);
    printf ("PE32_plus=%d\n", (unsigned)info.PE32_plus);
    printf ("original_base=0x%x\n", (unsigned)info.original_base);
    printf ("OEP=0x%x\n", (unsigned)info.OEP);
    printf ("timestamp=0x%x\n", (unsigned)info.timestamp);
    printf ("machine=0x%x\n", (unsigned)info.machine);
    printf ("size=0x%x\n", (unsigned)info.size);
    printf ("internal_name=%s\n", info.internal_name);
    return 0;
};

