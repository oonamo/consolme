#include "../consoleme_internal.h"
#include <consolme/consolme.h>
#include <consolme/extensions/cmyreflection.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char **items;
    size_t count;
    size_t capacity;
} StringList;

typedef struct
{
    char *prefix;
    const FieldInfo *fields;
    size_t field_count;
} QueueItem;

static void PushStr(StringList *list, char *str)
{
    if (list->count >= list->capacity)
    {
        list->capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->items = realloc(list->items, sizeof(char *) * list->capacity);
    }
    list->items[list->count++] = str;
}

void Console_GenerateReflectionCompletion(ConsoleCtx *ctx, const char *basename,
                                          const FieldInfo *metadata,
                                          size_t field_count,
                                          ConsoleReflectionCfg cfg)
{
    StringList results = {0};

    size_t q_capacity = 32;
    QueueItem *queue = malloc(q_capacity * sizeof(QueueItem));
    size_t q_head = 0;
    size_t q_tail = 0;

    if (basename != NULL) { queue[q_tail].prefix = Console_StrDup(basename); }
    else { queue[q_tail].prefix = NULL; }

    queue[q_tail].fields = metadata;
    queue[q_tail].field_count = field_count;
    q_tail++;

    while (q_head < q_tail)
    {
        QueueItem current = queue[q_head++];

        for (size_t i = 0; i < current.field_count; i++)
        {
            const FieldInfo *field = &current.fields[i];
            char *path = NULL;

            if (current.prefix == NULL)
            {
                size_t len = strlen(field->name) + 1;
                path = malloc(len);
                snprintf(path, len, "%s", field->name);
            }
            else
            {
                size_t len =
                    strlen(current.prefix) + 1 + strlen(field->name) + 1;
                path = malloc(len);
                snprintf(path, len, "%s.%s", current.prefix, field->name);
            }

            StructMetaData child_meta = {0};
            if (get_struct_metadata(field->type, &child_meta))
            {
                if (q_tail >= q_capacity)
                {
                    q_capacity *= 2;
                    queue = realloc(queue, q_capacity * sizeof(QueueItem));
                }

                queue[q_tail].prefix = path;
                queue[q_tail].fields = child_meta.fields;
                queue[q_tail].field_count = child_meta.count;
                q_tail++;
            }
            else { PushStr(&results, path); }
        }

        free(current.prefix);
    }

    free(queue);

    if (results.count > 0)
    {
        if (cfg.enable_setter && cfg.setter_cmd != NULL)
        {
            Console_RegisterDynamicCommand(ctx, cfg.setter_cmd,
                                           (const char **)results.items,
                                           results.count);
        }
        if (cfg.enable_getter && cfg.getter_cmd != NULL)
        {
            Console_RegisterDynamicCommand(ctx, cfg.getter_cmd,
                                           (const char **)results.items,
                                           results.count);
        }
    }

    for (size_t i = 0; i < results.count; i++) { free(results.items[i]); }

    free(results.items);
}
