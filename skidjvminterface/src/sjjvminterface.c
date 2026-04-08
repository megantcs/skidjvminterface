#include "../includes/skidjvminterface.h"

static HotspotContext CopyHotspotContext(HotspotContext src) {
    HotspotContext copy = src;
    if (src.VMStructs.data != NULL && src.VMStructs.size > 0) {
        copy.VMStructs.data = malloc(src.VMStructs.size * sizeof(VMStructEntry));
        if (copy.VMStructs.data != NULL) {
            memcpy(copy.VMStructs.data,
                src.VMStructs.data,
                src.VMStructs.size * sizeof(VMStructEntry));
        }
    }
    return copy;
}

static void FreeHotspotContextCopy(HotspotContext* ctx) {
    if (ctx->VMStructs.data != NULL) {
        free(ctx->VMStructs.data);
        ctx->VMStructs.data = NULL;
        ctx->VMStructs.size = 0;
    }
}

SJStatus ApiNewJvmInterface(In_ PHotspotContext Context,
    Out_ PIJVMINTERFACE* Interface)
{
    if (Context == NULL || Context->Proc == NULL || Context->VMStructs.size == 0 || Interface == NULL)
        return SJInvalidInParamaters;

    if (Context->Proc->version == 17) {
        return ApiNewJvmInterfaceFor17J(Context, Interface);
    }
<<<<<<< HEAD
    if (Context->Proc->version == 21) {
        return ApiNewJvmInterfaceFor21J(Context, Interface);
    }
=======
>>>>>>> 01e1a52d8fb32884195b12d1f4e32380b4729336
    return SJStatusNotFound;
}