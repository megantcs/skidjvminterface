#include "../includes/skidjvminterface.h"

static HotspotContext CopyHotspotContext(HotspotContext src) {
    HotspotContext copy = src;
    if (src._vmStructs.data != NULL && src._vmStructs.size > 0) {
        copy._vmStructs.data = malloc(src._vmStructs.size * sizeof(VMStructEntry));
        if (copy._vmStructs.data != NULL) {
            memcpy(copy._vmStructs.data,
                src._vmStructs.data,
                src._vmStructs.size * sizeof(VMStructEntry));
        }
    }
    return copy;
}

static void FreeHotspotContextCopy(HotspotContext* ctx) {
    if (ctx->_vmStructs.data != NULL) {
        free(ctx->_vmStructs.data);
        ctx->_vmStructs.data = NULL;
        ctx->_vmStructs.size = 0;
    }
}

SJStatus ApiNewJvmInterface(In_ PHotspotContext Context,
    Out_ PIJVMINTERFACE* Interface)
{
    return ApiNewJvmInterfaceFor17J(Context, Interface);
}