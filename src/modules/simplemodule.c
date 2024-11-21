/* Counter module example - demonstrates a more complex but understandable Redis module */
#include "../valkeymodule.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>


/* COUNTER.INCR <key> [amount] - Increment counter by amount (default 1) */
int CounterIncr_ValkeyCommand(ValkeyModuleCtx *ctx, ValkeyModuleString **argv, int argc) {
    if (argc != 2 && argc != 3) return ValkeyModule_WrongArity(ctx);
    
    long long increment = 1;
    if (argc == 3) {
        if (ValkeyModule_StringToLongLong(argv[2], &increment) != VALKEYMODULE_OK) {
            return ValkeyModule_ReplyWithError(ctx, "ERR invalid increment amount");
        }
    }

    ValkeyModuleKey *key = ValkeyModule_OpenKey(ctx, argv[1], VALKEYMODULE_READ | VALKEYMODULE_WRITE);
    if (key == NULL) {
        return ValkeyModule_ReplyWithError(ctx, "ERR failed to open key");
    }

    long long value = 0;
    int type = ValkeyModule_KeyType(key);
    if (type != VALKEYMODULE_KEYTYPE_EMPTY && type != VALKEYMODULE_KEYTYPE_STRING) {
        ValkeyModule_CloseKey(key);
        return ValkeyModule_ReplyWithError(ctx, VALKEYMODULE_ERRORMSG_WRONGTYPE);
    }

    // Get existing value if key exists
    if (type != VALKEYMODULE_KEYTYPE_EMPTY) {
        size_t len;
        const char *str = ValkeyModule_StringDMA(key, &len, VALKEYMODULE_READ);
        if (!str || len == 0) {
            ValkeyModule_CloseKey(key);
            return ValkeyModule_ReplyWithError(ctx, "ERR failed to read value");
        }
        value = atoll(str);
    }

    // Calculate new value and check for overflow
    if ((increment > 0 && value > LLONG_MAX - increment) || 
        (increment < 0 && value < LLONG_MIN - increment)) {
        ValkeyModule_CloseKey(key);
        return ValkeyModule_ReplyWithError(ctx, "ERR increment would overflow");
    }
    value += increment;

    // Convert to string and store
    ValkeyModuleString *buf = ValkeyModule_CreateStringFromLongLong(ctx, value);
    if (ValkeyModule_StringSet(key, buf) != VALKEYMODULE_OK) {
        ValkeyModule_CloseKey(key);
        return ValkeyModule_ReplyWithError(ctx, "ERR failed to store value");
    }

    ValkeyModule_CloseKey(key);
    return ValkeyModule_ReplyWithLongLong(ctx, value);
}

/* COUNTER.GET <key> - Get current counter value */
int CounterGet_ValkeyCommand(ValkeyModuleCtx *ctx, ValkeyModuleString **argv, int argc) {
    if (argc != 2) return ValkeyModule_WrongArity(ctx);

    ValkeyModuleKey *key = ValkeyModule_OpenKey(ctx, argv[1], VALKEYMODULE_READ);
    if (key == NULL) {
        return ValkeyModule_ReplyWithError(ctx, "ERR failed to open key");
    }

    int type = ValkeyModule_KeyType(key);
    if (type == VALKEYMODULE_KEYTYPE_EMPTY) {
        ValkeyModule_CloseKey(key);
        return ValkeyModule_ReplyWithLongLong(ctx, 0);
    }
    if (type != VALKEYMODULE_KEYTYPE_STRING) {
        ValkeyModule_CloseKey(key);
        return ValkeyModule_ReplyWithError(ctx, VALKEYMODULE_ERRORMSG_WRONGTYPE);
    }

    size_t len;
    const char *str = ValkeyModule_StringDMA(key, &len, VALKEYMODULE_READ);
    if (!str || len == 0) {
        ValkeyModule_CloseKey(key);
        return ValkeyModule_ReplyWithError(ctx, "ERR failed to read value");
    }

    long long value = atoll(str);
    ValkeyModule_CloseKey(key);
    return ValkeyModule_ReplyWithLongLong(ctx, value);
}

/* Module initialization */
int ValkeyModule_OnLoad(ValkeyModuleCtx *ctx, ValkeyModuleString **argv, int argc) {
    VALKEYMODULE_NOT_USED(argv);
    VALKEYMODULE_NOT_USED(argc);
    if (ValkeyModule_Init(ctx, "counter", 1, VALKEYMODULE_APIVER_1) == VALKEYMODULE_ERR)
        return VALKEYMODULE_ERR;

    /* Create the commands */
    if (ValkeyModule_CreateCommand(ctx, "counter.incr",
        CounterIncr_ValkeyCommand, "write deny-oom", 1, 1, 1) == VALKEYMODULE_ERR)
        return VALKEYMODULE_ERR;
        
    if (ValkeyModule_CreateCommand(ctx, "counter.get",
        CounterGet_ValkeyCommand, "readonly", 1, 1, 1) == VALKEYMODULE_ERR)
        return VALKEYMODULE_ERR;

    return VALKEYMODULE_OK;
}