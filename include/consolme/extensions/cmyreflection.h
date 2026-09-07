#ifndef _CONSOLME_CMR
#define _CONSOLME_CMR

#include "consolme/consolme.h"
#include <cmyreflection.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    bool enable_setter;
    const char *setter_cmd;

    bool enable_getter;
    const char *getter_cmd;
} ConsoleReflectionCfg;

typedef bool (*ReflectionTypeHandler)(const FieldInfo *leaf, void *target,
                                      int argc, char **argv,
                                      char *response_msg);

bool Console_ReflectionSet(void *base_instance, const FieldInfo *base_meta,
                           size_t base_count, const char *path, int argc,
                           char **argv, ReflectionTypeHandler custom_handler,
                           char *response_msg);

bool Console_ReflectionGet(void *base_instance, const FieldInfo *base_meta,
                           size_t base_count, const char *path, int argc,
                           char **argv, ReflectionTypeHandler custom_handler,
                           char *response_msg);

void Console_GenerateReflectionCompletion(ConsoleCtx *ctx, const char *basename,
                                          const FieldInfo *metadata,
                                          size_t field_count,
                                          ConsoleReflectionCfg cfg);

#define CONSOLME_ARG_INT(idx) atoi(argv[idx])
#define CONSOLME_ARG_FLOAT(idx) (float)atof(argv[idx])
#define CONSOLME_ARG_STR(idx) (argv[idx])

#define DEFINE_COLOR_SETTER(func_name)                                         \
    static bool func_name(void *target, int argc, char **argv, char *msg)      \
    {                                                                          \
        if (argc < 3)                                                          \
        {                                                                      \
            snprintf(msg, MAX_INPUT_CHARS,                                     \
                     "Error: Color requires 3 or more values");                \
            return false;                                                      \
        }                                                                      \
        Color *val = (Color *)target;                                          \
        val->r = (unsigned char)CONSOLME_ARG_INT(0);                           \
        val->g = (unsigned char)CONSOLME_ARG_INT(1);                           \
        val->b = (unsigned char)CONSOLME_ARG_INT(2);                           \
        val->a = argc >= 4 ? (unsigned char)CONSOLME_ARG_INT(3) : 255;         \
        snprintf(msg, MAX_INPUT_CHARS, "Set to {R:%d, G:%d, B:%d, A:%d}");     \
    }

#define DEFINE_FLOAT2_SETTER(func_name, struct_type, f1, f2)                   \
    static bool func_name(void *target, int argc, char **argv, char *msg)      \
    {                                                                          \
        if (argc < 2)                                                          \
        {                                                                      \
            snprintf(msg, MAX_INPUT_CHARS,                                     \
                     "Error: " #struct_type " requires 2 values");             \
            return false;                                                      \
        }                                                                      \
        struct_type *value = (struct_type *)target;                            \
        val->f1 = CONSOLME_ARG_FLOAT(0);                                       \
        val->f2 = CONSOLME_ARG_FLOAT(1);                                       \
        snprintf(msg, MAX_INPUT_CHARS, #struct_type " set to [%.2f, %.2f]",    \
                 val->f1, val->f2);                                            \
    }

#define DEFINE_FLOAT4_SETTER(func_name, struct_type, f1, f2, f3, f4)           \
    static bool func_name(void *target, int argc, char **argv, char *msg)      \
    {                                                                          \
        if (argc < 2)                                                          \
        {                                                                      \
            snprintf(msg, MAX_INPUT_CHARS,                                     \
                     "Error: " #struct_type " requires 2 values");             \
            return false;                                                      \
        }                                                                      \
        struct_type *value = (struct_type *)target;                            \
        val->f1 = CONSOLME_ARG_FLOAT(0);                                       \
        val->f2 = CONSOLME_ARG_FLOAT(1);                                       \
        val->f3 = CONSOLME_ARG_FLOAT(2);                                       \
        val->f4 = CONSOLME_ARG_FLOAT(3);                                       \
        snprintf(msg, MAX_INPUT_CHARS,                                         \
                 #struct_type " set to [%.2f, %.2f, %.2f, %2.f]", val->f1,     \
                 val->f2, val->f3, val->f4);                                   \
    }

#endif // _CONSOLME_CMR
