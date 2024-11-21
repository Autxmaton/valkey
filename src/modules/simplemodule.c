/* Simple example Redis module */
#include "../valkeymodule.h"
#include <string.h>

/* SIMPLE.GREET <name> - Responds with a greeting to <name> */
int SimpleGreet_ValkeyCommand(ValkeyModuleCtx *ctx, ValkeyModuleString **argv, int argc) {
    if (argc != 2) return ValkeyModule_WrongArity(ctx);

    size_t len;
    const char *name = ValkeyModule_StringPtrLen(argv[1], &len);
    char *greeting = ValkeyModule_Alloc(len + 7);

    if(greeting == NULL) {
        return ValkeyModule_ReplyWithError(ctx, "OOM");
    }
    snprintf(greeting, len + 7, "Hello %s", name);
    ValkeyModule_ReplyWithSimpleString(ctx, greeting);
    return VALKEYMODULE_OK;
}

/* Module initialization */
int ValkeyModule_OnLoad(ValkeyModuleCtx *ctx, ValkeyModuleString **argv, int argc) {
    VALKEYMODULE_NOT_USED(argv);
    VALKEYMODULE_NOT_USED(argc);
    if (ValkeyModule_Init(ctx, "simple", 1, VALKEYMODULE_APIVER_1) == VALKEYMODULE_ERR)
        return VALKEYMODULE_ERR;

    /* Create the command */
    if (ValkeyModule_CreateCommand(ctx, "simple.greet",
        SimpleGreet_ValkeyCommand, "readonly", 1, 1, 1) == VALKEYMODULE_ERR)
        return VALKEYMODULE_ERR;

    return VALKEYMODULE_OK;
}