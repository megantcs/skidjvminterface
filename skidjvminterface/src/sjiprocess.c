#include "../includes/skidjvminterface.h"
#include <Psapi.h>
#include <TlHelp32.h>
#include "stdio.h"

SJStatus ApiFindFirstProcessByTitle(Out_ PJvmProccess Proc, In_ PCHAR ProcName, In_ PCHAR FindTitle)
{
	SJStatus Status = SJSuccess;
	HANDLE hSnapshot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe;
	DWORD foundPid = 0;
	BOOL found = FALSE;

	SJCheckOutParam(Proc == NULL);
	SJCheckInParam(ProcName == NULL);
	SJCheckInParam(FindTitle == NULL);

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	SJCheckStatusEx(hSnapshot == INVALID_HANDLE_VALUE, SJUnhandledError);

	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe)) {
		CloseHandle(hSnapshot);
		Status = SJUnhandledError;
		goto _return;
	}

	do {
		if (_stricmp(pe.szExeFile, ProcName) == 0) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
			if (hProcess) {
				HWND hWnd = NULL;
				char title[256] = { 0 };

				do {
					hWnd = FindWindowEx(NULL, hWnd, NULL, NULL);
					if (hWnd) {
						DWORD processId = 0;
						GetWindowThreadProcessId(hWnd, &processId);
						if (processId == pe.th32ProcessID) {
							if (GetWindowTextA(hWnd, title, sizeof(title))) {
								if (strstr(title, FindTitle) != NULL) {
									foundPid = pe.th32ProcessID;
									found = TRUE;
									break;
								}
							}
						}
					}
				} while (hWnd && !found);

				CloseHandle(hProcess);
				if (found) break;
			}
		}
	} while (Process32Next(hSnapshot, &pe));

	CloseHandle(hSnapshot);

	SJCheckStatusEx(!found, SJStatusNotFound);

	Status = ApiNewJvmProcessByPid(Proc, foundPid);
	SJCheckStatus;

_return:
	return Status;
}

DWORD ApiGetPidByName(PWCHAR name)
{
	DWORD pid = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}

	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(hSnapshot, &pe)) {
		do {
			if (_wcsicmp(pe.szExeFile, name) == 0) {
				pid = pe.th32ProcessID;
				break;
			}
		} while (Process32NextW(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);
	return pid;
}

SJStatus ApiGetModuleAddress(Out_ PVOID* Module, In_ HANDLE Process, In_ PCHAR ModuleName)
{
	HMODULE mods[128] = { 0 };
	DWORD count = 0;

	if (!EnumProcessModulesEx(Process, mods, sizeof(mods), &count, LIST_MODULES_64BIT)) {
		return SJErrorEnumProcessModules;
	}

	int module_count = count / sizeof(HMODULE);

	for (int i = 0; i < module_count; ++i) {
		if (!mods[i]) continue;
		char name[255];
		GetModuleBaseNameA(Process, mods[i], name, 255);
		if (strcmp(name, ModuleName) == 0) {
			*Module = mods[i];
			return SJSuccess;
		}
	}
	return SJStatusNotFound;
}

SJStatus ApiNewJvmProcessByPid(Out_ PJvmProccess Proc, In_ DWORD PID)
{
	SJStatus Status = SJSuccess;

	SJCheckOutParam(Proc == NULL);
	SJCheckInParam(PID == 0);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	SJCheckStatusEx(!hProcess, SJErrorOpenProces);

	PVOID hJvmDll = {0};
	JvmProccess Out = {0};

	Status = ApiGetModuleAddress(&hJvmDll, hProcess, "jvm.dll");
	SJCheckStatus;

	Out.hProccess = hProcess;
	Out.hJvmDll = hJvmDll;

	*Proc = Out;
_return:
	return Status;
}

ExportSymbolList InternalGetExportFunctionList(PJvmProccess proc) {
	ExportSymbolList result = { NULL, 0 };

	if (!proc || !proc->hJvmDll) {
		return result;
	}

	IMAGE_DOS_HEADER dosHeader;
	if (!ApiReadmem(&dosHeader, proc->hJvmDll, sizeof(dosHeader))) {
		return result;
	}

	if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
		return result;
	}

	IMAGE_NT_HEADERS64 ntHeaders;
	uint64_t ntHeaderPos = (uint64_t)proc->hJvmDll + dosHeader.e_lfanew;
	if (!ApiReadmem(&ntHeaders, (void*)ntHeaderPos, sizeof(ntHeaders))) {
		return result;
	}

	if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
		return result;
	}

	IMAGE_DATA_DIRECTORY idd = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (idd.VirtualAddress == 0 || idd.Size == 0) {
		return result;
	}

	uint64_t exportDirAddr = (uint64_t)proc->hJvmDll + idd.VirtualAddress;
	IMAGE_EXPORT_DIRECTORY exportDirectory;
	if (!ApiReadmem(&exportDirectory, (void*)exportDirAddr, sizeof(exportDirectory))) {
		return result;
	}

	uint64_t namesAddr = (uint64_t)proc->hJvmDll + exportDirectory.AddressOfNames;
	uint64_t ordinalsAddr = (uint64_t)proc->hJvmDll + exportDirectory.AddressOfNameOrdinals;
	uint64_t functionsAddr = (uint64_t)proc->hJvmDll + exportDirectory.AddressOfFunctions;

	DWORD* names = (DWORD*)malloc(exportDirectory.NumberOfNames * sizeof(DWORD));
	WORD* ordinals = (WORD*)malloc(exportDirectory.NumberOfNames * sizeof(WORD));
	DWORD* functions = (DWORD*)malloc(exportDirectory.NumberOfFunctions * sizeof(DWORD));

	if (!names || !ordinals || !functions) {
		free(names);
		free(ordinals);
		free(functions);
		return result;
	}

	if (!ApiReadmem(names, (void*)namesAddr, exportDirectory.NumberOfNames * sizeof(DWORD)) ||
		!ApiReadmem(ordinals, (void*)ordinalsAddr, exportDirectory.NumberOfNames * sizeof(WORD)) ||
		!ApiReadmem(functions, (void*)functionsAddr, exportDirectory.NumberOfFunctions * sizeof(DWORD))) {
		free(names);
		free(ordinals);
		free(functions);
		return result;
	}

	result.data = (PExportSymbol)malloc(exportDirectory.NumberOfNames * sizeof(ExportSymbol));
	if (!result.data) {
		free(names);
		free(ordinals);
		free(functions);
		return result;
	}

	for (DWORD i = 0; i < exportDirectory.NumberOfNames; i++) {
		char exportName[256] = { 0 };
		uint64_t nameAddr = (uint64_t)proc->hJvmDll + names[i];

		if (ApiReadmem(exportName, (void*)nameAddr, sizeof(exportName) - 1)) {
			size_t nameLen = strlen(exportName);
			result.data[i].name = (char*)malloc(nameLen + 1);
			if (result.data[i].name) {
				memcpy(result.data[i].name, exportName, nameLen + 1);
			}
			else {
				result.data[i].name = NULL;
			}
			result.data[i].ordinal = ordinals[i] + exportDirectory.Base;
			result.data[i].rva = functions[ordinals[i]];
			result.data[i].address = (uint64_t)proc->hJvmDll + functions[ordinals[i]];
		}
		else {
			result.data[i].name = NULL;
			result.data[i].ordinal = 0;
			result.data[i].rva = 0;
			result.data[i].address = 0;
		}
	}
	result.size = exportDirectory.NumberOfNames;

	free(names);
	free(ordinals);
	free(functions);

	return result;
}

SJStatus ApiGetExportSymbolsByProcess(Out_ ExportSymbolList* OutExportSymbolList, In_ JvmProccess Proc)
{
	ApiSetTargetHandle(Proc.hProccess);

	ExportSymbolList Out = InternalGetExportFunctionList(&Proc);
	if (Out.size == 0) {
		return SJStatusNotFound;
	}

	*OutExportSymbolList = Out;
	return SJSuccess;
}

BOOL ApiFreeExportFunctionList(In_ ExportSymbolList* list) {
	if (!list || !list->data) {
		return FALSE;
	}

	for (DWORD i = 0; i < list->size; i++) {
		if (list->data[i].name) {
			free(list->data[i].name);
		}
	}

	free(list->data);
	list->data = NULL;
	list->size = 0;

	return TRUE;
}