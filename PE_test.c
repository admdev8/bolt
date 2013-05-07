#include <stdio.h>

#include "stuff.h"
#include "PE.h"
#include "porg_utils.h"

int main(int argc, char* argv[])
{
    PE_info info;
    obj* i;

    if (argc!=2)
        die ("use: %s filename.(exe|dll)\n", argv[0]);

    PE_get_info (argv[1], 0, &info);
    printf ("PE32_plus=%d\n", info.PE32_plus);
    printf ("original_base=0x%x\n", info.original_base);
    printf ("OEP=0x%x\n", info.OEP);
    printf ("timestamp=0x%x\n", info.timestamp);
    printf ("machine=0x%x\n", info.machine);
    printf ("size=0x%x\n", info.size);
    printf ("internal_name=%s\n", info.internal_name);
    printf ("exports:\n");
    for (i=info.exports; i; i=cdr(i))
        printf ("0x%x, %s\n", obj_get_as_REG(car(car(i))), obj_get_as_cstring(cdr(car(i))));
    printf ("exports_ordinals\n");
    for (i=info.exports_ordinals; i; i=cdr(i))
        printf ("0x%x, %d\n", obj_get_as_REG(car(car(i))), obj_get_as_tetrabyte(cdr(car(i))));

    return 0;
};

