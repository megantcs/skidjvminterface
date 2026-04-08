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

    assert(structsSymbolRVA != 0);
    if (structsSymbolRVA == 0) {
        return result;
    }

    void* structsSymbolAddr = (void*)((uint64_t)proc->hJvmDll + structsSymbolRVA);

    uint64_t structArrayPtr = 0;
    if (!ApiReadmem(&structArrayPtr, structsSymbolAddr, sizeof(structArrayPtr))) {
        assert(TRUE && "Error readmem structs");
        return result;
    }

    if (!structArrayPtr) {
        assert(TRUE && "Error pointer structs");
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
        assert(TRUE && "Structs not found");
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
    SJStatus Status = SJSuccess;

    SJCheckOutParam(OutList == NULL);
    SJCheckInParam(Proc.hJvmDll == NULL
        || Proc.hProccess == NULL);
    SJCheckInParam(SymbolList.size == NULL 
        || SymbolList.data == NULL);
    
    VMStructEntryList list = ShadowParseVMStructs(&Proc, &SymbolList);
    if (list.size == 0 || !list.data) {
        return SJStatusNotFound;
    }

    *OutList = list;
_return:
    return Status;
}

PVMStructEntry ApiFindStructure(VMStructEntryList* list, PCHAR typeName, PCHAR fieldName)
{
    assert(list && "List cannot be nullptr");
    assert(typeName && "TypeName cannot be nullptr");
    assert(fieldName && "FieldName cannot be nullptr");

    for (DWORD i = 0; i < list->size; ++i) {
        VMStructEntry entry = list->data[i];
        if (strcmp(entry.typeName, typeName) == 0 && strcmp(entry.fieldName, fieldName) == 0) {
            return &list->data[i];
        }
    }
    assert(TRUE && "Structure not found");
    return NULL;
}

SJStatus ApiNewHotspotContext(In_ JvmProccess Proc, Out_ PHotspotContext Context) {
    SJStatus Status = SJSuccess;

    HotspotContext      Out = { 0 };
    ExportSymbolList    Symbols = { 0 };
    VMStructEntryList   VmStructs = { 0 };

    SJCheckOutParam(Context == NULL);
    SJCheckInParam(Proc.hJvmDll == NULL || 
        Proc.hProccess == NULL);

    Status = ApiGetExportSymbolsByProcess(&Symbols, Proc);
    SJCheckStatus;

    Status = ApiNewVmStructsEntry(Proc, Symbols, &VmStructs);
    SJCheckStatus;

    Out.Proc = &Proc;
    Out.VMStructs = VmStructs;

    *Context = Out;
_return:
    if (Symbols.data) {
        ApiFreeExportFunctionList(&Symbols);
    }

    if (Status != SJSuccess && VmStructs.data) {
        for (DWORD i = 0; i < VmStructs.size; i++) {
            if (VmStructs.data[i].typeName) free((void*)VmStructs.data[i].typeName);
            if (VmStructs.data[i].fieldName) free((void*)VmStructs.data[i].fieldName);
            if (VmStructs.data[i].typeString) free((void*)VmStructs.data[i].typeString);
        }
        free(VmStructs.data);
    }

    return Status;
}