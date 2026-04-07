#ifndef SKIDJVMINTERFACE_H
#define SKIDJVMINTERFACE_H

#include <Windows.h>

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
typedef const void* jclass;
typedef const void* jobject;
typedef const void* jmethodID;

#define SJSuccess				         0
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

#define Jvm17 0x17

#ifdef NULL
    #undef NULL
#endif

#define NULL 0

#define NEW_STRUCT_LIST(x) \
typedef struct _##x##List { \
    x data;\
    DWORD size;\
}##x##List

typedef struct _VMStructEntry {
    PCHAR typeName;           
    PCHAR fieldName;
    PCHAR typeString;
    DWORD  isStatic;
    DWORD offset;
    PVOID address;
}VMStructEntry, *PVMStructEntry;

typedef struct _ExportSymbol {
    PCHAR name;         
    DWORD ordinal;            
    DWORD address;           
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
    jclass (*findClass)(PCHAR);
}IJVMINTERFACE, *PIJVMINTERFACE;

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

/* Memory Api */
VOID        ApiSetTargetHandle(HANDLE Proc);
SJStatus    ApiWritemem(PVOID Buffer, PVOID Addres, size_t Size);
SJStatus    ApiReadmem(PVOID Buffer, PVOID Addres, size_t Size);

/* Hotspot Api */
SJStatus ApiNewHotspotContext(In_ PJvmProccess Proc, 
							  Out_ PHotspotContext Context);

/* Jvm Interface Api */
SJStatus ApiNewJvmInterface(In_ PHotspotContext Context, 
                            Out_ PIJVMINTERFACE* Interface);

/* Macros for deployment */
#define ExpandedBodyReturnStatus \
	SJStatus Status = SJSuccess; \
_return:\
return Status;\

#endif