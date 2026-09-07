#ifndef _CONSOLME_CMR
#define _CONSOLME_CMR

#include "consolme/consolme.h"
#include <cmyreflection.h>
#include <stdint.h>

#define REFLECTION_SET (1 << 0)
#define REFLECTION_GET (1 << 1)

#define REFLECTION_DEFAULT (REFLECTION_SET | REFLECTION_GET)

typedef struct
{
    bool enable_setter;
    const char *setter_cmd;

    bool enable_getter;
    const char *getter_cmd;
} ConsoleReflectionCfg;

void Console_GenerateReflectionCompletion(ConsoleCtx *ctx, const char *basename,
                                          const FieldInfo *metadata,
                                          size_t field_count, ConsoleReflectionCfg cfg);

#endif // _CONSOLME_CMR
