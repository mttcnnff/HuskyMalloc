#include <stdio.h>
#include <string.h>

#include "freelist.h"

int
main(int _ac, char* _av[])
{
	freelist* list = freelist_make();
    freelist_free(list);
    return 0;
}
