#include "../includes/skidjvminterface.h"

static VMStructEntryList ShadowParseVMStructs(PJvmProccess proc, ExportSymbolList* exports) {
    VMStructEntryList result = { NULL, 0 };

    if (!proc || !exports || !exports->data) {
        return result;
    }

    uint64_t structsSymbolRVA = 0;
    for (DWORD i = 0; i < exports->size; i++) {
        if (exports->data[i].name && strcmp(exports->data[i].name, "gHotSpotVMStructs") == 0) {
            structsSymbolRVA = exports->data[i].rva;
            break;
        }
    }

    if (structsSymbolRVA == 0) {
        return result;
    }

    void* structsSymbolAddr = (void*)((uint64_t)proc->hJvmDll + structsSymbolRVA);

    uint64_t structArrayPtr = 0;
    if (!ApiReadmem(&structArrayPtr, structsSymbolAddr, sizeof(structArrayPtr))) {
        return result;
    }

    if (!structArrayPtr) {
        return result;
    }

    const int MAX_STRUCTURES = 1000;
    VMStructEntry* tempStructs = (VMStructEntry*)malloc(MAX_STRUCTURES * sizeof(VMStructEntry));
    if (!tempStructs) {
        return result;
    }

    DWORD structCount = 0;

    for (int i = 0; i < MAX_STRUCTURES; i++) {
        uint64_t entryAddress = structArrayPtr + i * sizeof(VMStructEntry);

        if (!ApiReadmem(&tempStructs[structCount], (void*)entryAddress, sizeof(VMStructEntry))) {
            break;
        }

        if (!tempStructs[structCount].typeName) {
            break;
        }

        char typeName[256] = { 0 };
        char fieldName[256] = { 0 };
        char typeString[256] = { 0 };

        BOOL stringsRead = TRUE;
        stringsRead &= ApiReadmem(typeName, tempStructs[structCount].typeName, sizeof(typeName) - 1);
        stringsRead &= ApiReadmem(fieldName, tempStructs[structCount].fieldName, sizeof(fieldName) - 1);

        if (tempStructs[structCount].typeString) {
            stringsRead &= ApiReadmem(typeString, tempStructs[structCount].typeString, sizeof(typeString) - 1);
        }

        if (!stringsRead) {
            continue;
        }

        tempStructs[structCount].typeName = _strdup(typeName);
        tempStructs[structCount].fieldName = _strdup(fieldName);
        tempStructs[structCount].typeString = tempStructs[structCount].typeString ? _strdup(typeString) : NULL;

        structCount++;
    }

    if (structCount == 0) {
        free(tempStructs);
        return result;
    }

    result.data = (PVMStructEntry)malloc(structCount * sizeof(VMStructEntry));
    if (!result.data) {
        for (DWORD i = 0; i < structCount; i++) {
            free(tempStructs[i].typeName);
            free(tempStructs[i].fieldName);
            if (tempStructs[i].typeString) free(tempStructs[i].typeString);
        }
        free(tempStructs);
        return result;
    }

    memcpy(result.data, tempStructs, structCount * sizeof(VMStructEntry));
    result.size = structCount;

    free(tempStructs);

    return result;
}

SJStatus ApiNewVmStructsEntry(In_ JvmProccess Proc, In_ ExportSymbolList SymbolList, Out_ VMStructEntryList* OutList) {
    if (!OutList) {
        return SJInvalidOutParamaters;
    }

    VMStructEntryList list = ShadowParseVMStructs(&Proc, &SymbolList);
    if (list.size == 0 || !list.data) {
        return SJStatusNotFound;
    }

    *OutList = list;
    return SJSuccess;
}

PVMStructEntry ApiFindStructure(VMStructEntryList* list, PCHAR typeName, PCHAR fieldName)
{
    for (DWORD i = 0; i < list->size; ++i) {
        VMStructEntry entry = list->data[i];
        if (strcmp(entry.typeName, typeName) == 0 && strcmp(entry.fieldName, fieldName) == 0) {
            return &list->data[i];
        }
    }
    return NULL;
}

SJStatus ApiNewHotspotContext(In_ JvmProccess Proc, Out_ PHotspotContext Context) {
    SJStatus Status = SJSuccess;
    HotspotContext Out = { 0 };
    ExportSymbolList symbols = { 0 };
    VMStructEntryList vmStructs = { 0 };

    SJCheckOutParam(Context == NULL);

    Status = ApiGetExportSymbolsByProcess(&symbols, Proc);
    SJCheckStatus;

    Status = ApiNewVmStructsEntry(Proc, symbols, &vmStructs);
    SJCheckStatus;

    Out.Proc = &Proc;
    Out.VMStructs = vmStructs;

    *Context = Out;

_return:
    if (symbols.data) {
        ApiFreeExportFunctionList(&symbols);
    }

    if (Status != SJSuccess && vmStructs.data) {
        for (DWORD i = 0; i < vmStructs.size; i++) {
            if (vmStructs.data[i].typeName) free((void*)vmStructs.data[i].typeName);
            if (vmStructs.data[i].fieldName) free((void*)vmStructs.data[i].fieldName);
            if (vmStructs.data[i].typeString) free((void*)vmStructs.data[i].typeString);
        }
        free(vmStructs.data);
    }

    return Status;
}