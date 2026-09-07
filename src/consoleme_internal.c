#include "consoleme_internal.h"

#include <stdlib.h>
#include <string.h>

char *Console_StrDup(const char *src)
{
    if (!src) return NULL;
    size_t len = strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (dst) memcpy(dst, src, len);
    return dst;
}
