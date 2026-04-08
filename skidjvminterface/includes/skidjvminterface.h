#ifndef SKIDJVMINTERFACE_H
#define SKIDJVMINTERFACE_H

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdint.h>

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

#define SJSuccess				         1
#define SJUnhandledError		         0x0001B
#define SJInvalidState  		         0x000CB
#define SJInvalidInParamaters	         0x0002B
#define SJInvalidOutParamaters           0x0003B
#define SJErrorOpenProces                0x0004B
#define SJStatusNotFound                 0x0005B
#define SJErrorEnumProcessModules        0x0006B

#define SJCheckStatus if(Status != SJSuccess) { goto _return; }
#define SJCheckStatusEx(x, flag) if((x)) { Status = flag; goto _return; }
#define SJCheckOutParam(x) SJCheckStatusEx(x, SJInvalidOutParamaters) 
#define SJCheckInParam(x) SJCheckStatusEx(x, SJInvalidInParamaters) 

#define Jvm17 17
#define PTRMATH(a) (void *) ((uintptr_t) a)

#ifdef NULL
    #undef NULL
#endif

#define NULL 0

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

NEW_STRUCT_LIST(VMStructEntry);
NEW_STRUCT_LIST(ExportSymbol);

typedef struct _JvmProccess {
	HANDLE hProccess;
	PVOID  hJvmDll;
	JvmVersion version;
}JvmProccess, *PJvmProccess;

typedef struct _HotspotContext {
    VMStructEntryList _vmStructs;
    PJvmProccess      _proc;
}HotspotContext, *PHotspotContext;

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

typedef struct {
    int major;
    int minor;
    int update;

    unsigned int use_compressed_oops;
    unsigned int compressed_oops_base;
    unsigned int compressed_oops_shift;

    unsigned int use_compressed_class_pointers;
    unsigned int compressed_class_pointers_base;
    unsigned int compressed_class_pointers_shift;

    unsigned int collectedheap;
    unsigned int classloaderdatagraph_head;
    unsigned int symboltable;
    unsigned int systemcl;

    unsigned int fieldinfo_stream;
    unsigned int fields;
    unsigned int fields_count;
    unsigned int methods;
    unsigned int klass_constants;

    unsigned int klass_super;
    unsigned int klass_javamirror;
    unsigned int klass_name;
    unsigned int klass_next_link;
    unsigned int klass_super_check_offset;
    unsigned int klass_secondary_supers;

    unsigned int symbol_length;
    unsigned int symbol_body;
    unsigned int cp_base;

    unsigned int cld_class_loader;
    unsigned int cld_has_class_mirror_holder;
    unsigned int cld_klasses;
    unsigned int cld_next;

    unsigned int local_interfaces;
} JVMVersion;

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

/* Macros for deployment */
#define ExpandedBodyReturnStatus \
	SJStatus Status = SJSuccess; \
_return:\
return Status;\

#endif
