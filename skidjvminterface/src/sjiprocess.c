#include "../includes/skidjvminterface.h"
#include <Psapi.h>
#include <TlHelp32.h>

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