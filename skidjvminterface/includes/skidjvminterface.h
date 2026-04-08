#ifndef SKIDJVMINTERFACE_H
#define SKIDJVMINTERFACE_H

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdint.h>
#include <assert.h>

#define In_
#define Out_

typedef unsigned char JvmVersion;
typedef short		  SJStatus;

typedef unsigned char jboolean;
typedef unsigned char jbyte;
typedef unsigned short jchar;
typedef short jshort;
typedef int jint;
typedef long jlong;
typedef float jfloat;
typedef double jdouble;
typedef unsigned short jfieldID;
typedef void* jclass;
typedef void* jobject;
typedef void* jmethodID;

#define SJSuccess				         1          // Success Status 
#define SJUnhandledError		         0x0001B    // An unhandled error. for example, an exception.
#define SJInvalidState  		         0x000CB    // Not a valid state. no pointers are set. or not valid data
#define SJInvalidInParamaters	         0x0002B    // Invalid input parameters 
#define SJInvalidOutParamaters           0x0003B    // Invalid output parameters 
#define SJErrorOpenProces                0x0004B    // Could not be opened for any reason
#define SJStatusNotFound                 0x0005B    // Status error: When something is not found
#define SJErrorEnumProcessModules        0x0006B    // Could not list all modules for some reason

/* convenient macros for checking 
    states, clearly linked to specific names. */
#define SJCheckStatus if(Status != SJSuccess) { goto _return; }
#define SJCheckStatusEx(x, flag) if((x)) { Status = flag; goto _return; }
#define SJCheckOutParam(x) SJCheckStatusEx(x, SJInvalidOutParamaters) 
#define SJCheckInParam(x) SJCheckStatusEx(x, SJInvalidInParamaters) 

/* Macro for converting a pointer 
    to an integer type and back to void* */
#define PTRMATH(a) (void *) ((uintptr_t) a)

/* By default, NULL is set to (void*)0. */
#ifdef NULL
    #undef NULL
#endif

#define NULL 0
/*  -----   */

/* macro for creating typed 
    arrays using a structure */
#define NEW_STRUCT_LIST(x) \
typedef struct _##x##List { \
    x* data;\
    DWORD size;\
}##x##List

typedef struct _VMStructEntry {
    PCHAR typeName;
    PCHAR fieldName;
    PCHAR typeString;

    int32_t  isStatic;
    uint64_t offset;

    PVOID address;
}VMStructEntry, *PVMStructEntry;

typedef struct _ExportSymbol {
    PCHAR name;
    DWORD ordinal;
    DWORD rva;
    uint64_t address;
}ExportSymbol, *PExportSymbol;

/* Arrays */
NEW_STRUCT_LIST(VMStructEntry);     // VMStructEntryList
NEW_STRUCT_LIST(ExportSymbol);      // ExportSymbolList

typedef struct _JvmProccess {
	HANDLE hProccess;
	PVOID  hJvmDll;
	JvmVersion version;
}JvmProccess, *PJvmProccess;

typedef struct _HotspotContext {
    VMStructEntryList VMStructs;
    PJvmProccess      Proc;
}HotspotContext, *PHotspotContext;

/* an interface in which methods are created. 
        a java native interface mirror. */
typedef struct _IJVMINTERFACE {
    jclass(*findClass)(PCHAR);
    jfieldID(*findField)(jclass clazz, PCHAR fieldName, PCHAR signature);
    jint(*getArrayLen)(jobject oop);
    jobject(*getObjectField)(jobject obj, jfieldID fieldID);
    jboolean(*isInstanceOf)(jobject obj, jclass clazz);
    jclass(*getObjectClass)(jobject obj);

    jobject(*getFieldObject)(jobject obj, jfieldID fieldID);
    jint(*getFieldInt)(jobject obj, jfieldID fieldID);
    jfloat(*getFieldFloat)(jobject obj, jfieldID fieldID);
    jdouble(*getFieldDouble)(jobject obj, jfieldID fieldID);
    jboolean(*getFieldBoolean)(jobject obj, jfieldID fieldID);
    jbyte(*getFieldByte)(jobject obj, jfieldID fieldID);
    jchar(*getFieldChar)(jobject obj, jfieldID fieldID);
    jshort(*getFieldShort)(jobject obj, jfieldID fieldID);
    jlong(*getFieldLong)(jobject obj, jfieldID fieldID);

    jobject(*getStaticFieldObject)(jclass clazz, jfieldID fieldID);
    jint(*getStaticFieldInt)(jclass clazz, jfieldID fieldID);
    jfloat(*getStaticFieldFloat)(jclass clazz, jfieldID fieldID);
    jdouble(*getStaticFieldDouble)(jclass clazz, jfieldID fieldID);
    jboolean(*getStaticFieldBoolean)(jclass clazz, jfieldID fieldID);
    jbyte(*getStaticFieldByte)(jclass clazz, jfieldID fieldID);
    jchar(*getStaticFieldChar)(jclass clazz, jfieldID fieldID);
    jshort(*getStaticFieldShort)(jclass clazz, jfieldID fieldID);
    jlong(*getStaticFieldLong)(jclass clazz, jfieldID fieldID);

    void(*setFieldObject)(jobject obj, jfieldID fieldID, jobject value);
    void(*setFieldInt)(jobject obj, jfieldID fieldID, jint value);
    void(*setFieldFloat)(jobject obj, jfieldID fieldID, jfloat value);
    void(*setFieldDouble)(jobject obj, jfieldID fieldID, jdouble value);
    void(*setFieldBoolean)(jobject obj, jfieldID fieldID, jboolean value);
    void(*setFieldByte)(jobject obj, jfieldID fieldID, jbyte value);
    void(*setFieldChar)(jobject obj, jfieldID fieldID, jchar value);
    void(*setFieldShort)(jobject obj, jfieldID fieldID, jshort value);
    void(*setFieldLong)(jobject obj, jfieldID fieldID, jlong value);

    void(*setStaticFieldObject)(jclass clazz, jfieldID fieldID, jobject value);
    void(*setStaticFieldInt)(jclass clazz, jfieldID fieldID, jint value);
    void(*setStaticFieldFloat)(jclass clazz, jfieldID fieldID, jfloat value);
    void(*setStaticFieldDouble)(jclass clazz, jfieldID fieldID, jdouble value);
    void(*setStaticFieldBoolean)(jclass clazz, jfieldID fieldID, jboolean value);
    void(*setStaticFieldByte)(jclass clazz, jfieldID fieldID, jbyte value);
    void(*setStaticFieldChar)(jclass clazz, jfieldID fieldID, jchar value);
    void(*setStaticFieldShort)(jclass clazz, jfieldID fieldID, jshort value);
    void(*setStaticFieldLong)(jclass clazz, jfieldID fieldID, jlong value);

    void(*setObjectField)(jobject obj, jfieldID fieldID, jobject value);
    void(*getObjectArrayElement)(jobject oop, jobject* array, int start, int end);
    void(*monitorEnter)(jobject obj);
    void(*monitorExit)(jobject obj);
    jboolean(*isSameObject)(jobject obj1, jobject obj2);
    jclass(*getSuperclass)(jclass clazz);
    jboolean(*isAssignableFrom)(jclass sub, jclass super);
    jobject(*allocObject)(jclass clazz);
} IJVMINTERFACE, * PIJVMINTERFACE;
/* For 17 Jvm */
typedef struct {
    uint16_t _flags;
} AccessFlags;

static inline void access_flags_init(AccessFlags* af, uint16_t flags) {
    af->_flags = flags;
}

static inline void access_flags_init_default(AccessFlags* af) {
    af->_flags = 0;
}

static inline boolean access_is_public(const AccessFlags* af) {
    return (af->_flags & 0x0001) != 0;
}

static inline boolean access_is_private(const AccessFlags* af) {
    return (af->_flags & 0x0002) != 0;
}

static inline boolean access_is_protected(const AccessFlags* af) {
    return (af->_flags & 0x0004) != 0;
}

static inline boolean access_is_static(const AccessFlags* af) {
    return (af->_flags & 0x0008) != 0;
}

static inline boolean access_is_final(const AccessFlags* af) {
    return (af->_flags & 0x0010) != 0;
}

static inline boolean access_is_volatile(const AccessFlags* af) {
    return (af->_flags & 0x0040) != 0;
}

static inline boolean access_is_transient(const AccessFlags* af) {
    return (af->_flags & 0x0080) != 0;
}

static inline uint32_t access_flag_mask(int p) {
    return (uint32_t)1 << p;
}


typedef struct {
    uint32_t phar;
    uint16_t _length;
    char _body[255];
} Symbol;
typedef struct {
    uint16_t _shorts[6];
} FieldInfo17;

enum FieldOffset {
    AccessFlagsOffset = 0,
    NameIndexOffset = 1,
    SignatureIndexOffset = 2,
    InitvalIndexOffset = 3,
    LowPackedOffset = 4,
    HighPackedOffset = 5,
    FieldSlots = 6
};
/* --------------------------------------------------- */

/* forFor 21 Jvm */



typedef enum {
    FF_INITIALIZED,
    FF_INJECTED,
    FF_GENERIC,
    FF_STABLE,
    FF_CONTENDED
} FieldFlagsBitPosition;

static const uint32_t FIELD_FLAGS_OPTIONAL_ITEM_BIT_MASK =
(1u << FF_INITIALIZED) |
(1u << FF_GENERIC) |
(1u << FF_CONTENDED);

typedef struct {
    uint32_t _flags;
} FieldFlags;

static inline void field_flags_init(FieldFlags* ff, uint32_t flags) {
    ff->_flags = flags;
}

static inline void field_flags_init_default(FieldFlags* ff) {
    ff->_flags = 0;
}


static inline boolean field_flags_test_flag(const FieldFlags* ff, FieldFlagsBitPosition pos) {
    return (ff->_flags & (1u << pos)) != 0;
}

static inline void field_flags_update_flag(FieldFlags* ff, FieldFlagsBitPosition pos, boolean z) {
    if (z) {
        ff->_flags |= (1u << pos);
    }
    else {
        ff->_flags &= ~(1u << pos);
    }
}

static inline uint32_t field_flags_as_uint(const FieldFlags* ff) {
    return ff->_flags;
}

static inline boolean field_flags_has_any_optionals(const FieldFlags* ff) {
    return (ff->_flags & FIELD_FLAGS_OPTIONAL_ITEM_BIT_MASK) != 0;
}

// Геттеры
static inline boolean field_flags_is_initialized(const FieldFlags* ff) {
    return field_flags_test_flag(ff, FF_INITIALIZED);
}

static inline boolean field_flags_is_injected(const FieldFlags* ff) {
    return field_flags_test_flag(ff, FF_INJECTED);
}

static inline boolean field_flags_is_generic(const FieldFlags* ff) {
    return field_flags_test_flag(ff, FF_GENERIC);
}

static inline boolean field_flags_is_stable(const FieldFlags* ff) {
    return field_flags_test_flag(ff, FF_STABLE);
}

static inline boolean field_flags_is_contended(const FieldFlags* ff) {
    return field_flags_test_flag(ff, FF_CONTENDED);
}

// Сеттеры
static inline void field_flags_set_initialized(FieldFlags* ff, boolean z) {
    field_flags_update_flag(ff, FF_INITIALIZED, z);
}

static inline void field_flags_set_injected(FieldFlags* ff, boolean z) {
    field_flags_update_flag(ff, FF_INJECTED, z);
}

static inline void field_flags_set_generic(FieldFlags* ff, boolean z) {
    field_flags_update_flag(ff, FF_GENERIC, z);
}

static inline void field_flags_set_stable(FieldFlags* ff, boolean z) {
    field_flags_update_flag(ff, FF_STABLE, z);
}

static inline void field_flags_set_contended(FieldFlags* ff, boolean z) {
    field_flags_update_flag(ff, FF_CONTENDED, z);
}

static inline void field_flags_mark_initialized(FieldFlags* ff) {
    field_flags_set_initialized(ff, 1);
}

static inline void field_flags_mark_injected(FieldFlags* ff) {
    field_flags_set_injected(ff, 1);
}

static inline void field_flags_mark_generic(FieldFlags* ff) {
    field_flags_set_generic(ff, 1);
}

static inline void field_flags_mark_stable(FieldFlags* ff) {
    field_flags_set_stable(ff, 1);
}

static inline void field_flags_mark_contended(FieldFlags* ff) {
    field_flags_set_contended(ff, 1);
}

typedef struct 
{
    uint32_t _index;
    uint16_t _name_index;
    uint16_t _signature_index;
    uint32_t _offset;
    AccessFlags _access_flags;
    FieldFlags _field_flags;
    uint16_t _initializer_index;
    uint16_t _generic_signature_index;
    uint16_t _contention_group;


}FieldInfo20 ;

static inline void field_info20_init_default(FieldInfo20* fi) {
    fi->_index = 0;
    fi->_name_index = 0;
    fi->_signature_index = 0;
    fi->_offset = 0;
    access_flags_init_default(&fi->_access_flags);
    field_flags_init_default(&fi->_field_flags);
    fi->_initializer_index = 0;
    fi->_generic_signature_index = 0;
    fi->_contention_group = 0;
}

static inline uint16_t field_info20_name_index(const FieldInfo20* fi) {
    return fi->_name_index;
}

static inline uint16_t field_info20_signature_index(const FieldInfo20* fi) {
    return fi->_signature_index;
}

static inline int field_info20_offset(const FieldInfo20* fi) {
    return (int)fi->_offset;
}

/* Process Api */
SJStatus ApiFindFirstProcessByTitle(Out_ PJvmProccess Proc, 
                                    In_ PCHAR ProcName, 
                                    In_ PCHAR FindTitle);

DWORD ApiGetPidByName(PWCHAR Name);

SJStatus ApiGetModuleAddress(Out_ PVOID* Module, 
                            In_ HANDLE Process, 
                            In_ PCHAR ModuleName);

SJStatus ApiNewJvmProcessByPid(Out_ PJvmProccess Proc, 
                               In_ DWORD PID);

JvmVersion ApiGetJvmVersionFromModule(In_ HANDLE hProcess, 
                                      In_ PVOID hJvmDll);

SJStatus ApiGetExportSymbolsByProcess(Out_ ExportSymbolList* ExportSymbol, 
                                      In_ JvmProccess Proc);

BOOL ApiFreeExportFunctionList(In_ ExportSymbolList* list);

/* Memory Api */
VOID        ApiSetTargetHandle(HANDLE Proc);
SJStatus    ApiWritemem(PVOID Buffer, PVOID Addres, size_t Size);
SJStatus    ApiReadmem(PVOID Buffer, PVOID Addres, size_t Size);

/* Hotspot Api */
SJStatus ApiNewHotspotContext(In_ JvmProccess Proc, 
							  Out_ PHotspotContext Context);

SJStatus ApiNewVmStructsEntry(In_ JvmProccess Proc,
                              In_ ExportSymbolList SymbolList, 
                              Out_ VMStructEntryList* OutList);

PVMStructEntry ApiFindStructure(VMStructEntryList* list, PCHAR typeName, PCHAR fieldName);

/* Jvm Interface Api */
SJStatus ApiNewJvmInterface(In_ PHotspotContext Context,
                            Out_ PIJVMINTERFACE* Interface);

SJStatus ApiNewJvmInterfaceFor17J(In_ PHotspotContext Context,
                                  Out_ PIJVMINTERFACE* Interface);

SJStatus ApiNewJvmInterfaceFor21J(In_ PHotspotContext Context, 
                                  Out_ PIJVMINTERFACE* Interface);
/* Macros for deployment */
#define ExpandedBodyReturnStatus \
	SJStatus Status = SJSuccess; \
_return:\
return Status;\

#endif
