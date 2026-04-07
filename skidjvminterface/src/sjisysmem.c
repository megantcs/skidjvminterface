#include "../includes/skidjvminterface.h"

HANDLE ActivityTargetHandle = NULL;
BOOL   SingleInitialize     = FALSE;

typedef NTSTATUS(NTAPI* pZwReadVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
typedef NTSTATUS(NTAPI* pZwWriteVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);

static pZwReadVirtualMemory ZwReadVirtualMemory = NULL;
static pZwWriteVirtualMemory ZwWriteVirtualMemory = NULL;

#define CHECK_ACTIVITY_HANDLE if(ActivityTargetHandle == NULL) return SJInvalidState;

BOOL InternalWritemem(PVOID buffer, PVOID addres, size_t size)
{
	if (!ZwWriteVirtualMemory) return FALSE;
	NTSTATUS status = ZwWriteVirtualMemory(ActivityTargetHandle, addres, buffer, size, NULL);
	return (status == 0);
}

BOOL InternalReadmem(PVOID* buffer, PVOID addres, size_t size)
{
	if (!ZwReadVirtualMemory) return FALSE;
	SIZE_T bytesRead = 0;
	NTSTATUS status = ZwReadVirtualMemory(ActivityTargetHandle, addres, buffer, size, &bytesRead);
	return (status == 0 && bytesRead == size);
}

VOID ApiSetTargetHandle(HANDLE Proc)
{
	ActivityTargetHandle = Proc;

	if (!SingleInitialize) {
		HMODULE ntdll = GetModuleHandleA("ntdll.dll");
		if (!ntdll) ntdll = LoadLibraryA("ntdll.dll");
		

		if (!ntdll) {
			abort();
		}

		ZwReadVirtualMemory = (pZwReadVirtualMemory)GetProcAddress(ntdll, "NtReadVirtualMemory");
		ZwWriteVirtualMemory = (pZwWriteVirtualMemory)GetProcAddress(ntdll, "NtWriteVirtualMemory");
		SingleInitialize = TRUE;
	}
}

SJStatus ApiWritemem(PVOID Buffer, PVOID Addres, size_t Size)
{
	CHECK_ACTIVITY_HANDLE
    if(!InternalWritemem(Buffer, Addres, Size)) {
		return SJUnhandledError;
	}
	return SJSuccess;
}

SJStatus ApiReadmem(PVOID Buffer, PVOID Addres, size_t Size)
{
	CHECK_ACTIVITY_HANDLE
	if (!InternalReadmem(Buffer, Addres, Size)) {
		return SJUnhandledError;
	}

	return SJSuccess;
}