#include <stdio.h>
#include <windows.h>
#include "d2ldr.h"


unsigned char pcode_str[] = {0x68, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD0};

CodeCtx codectx = {
	{0x00},
	0x0040122E,
	0x0040122E,
	{0x00},
	sizeof(pcode_str)
};


void enable_debug();
int get_seg_info(const char *seg_nz, unsigned long *seg_addr, int *seg_sz);
void __stdcall d2ldr_jmp_entry(pCodeCtx p_codectx);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD oldprt;
	char *schr;
	LPVOID src_addr, dst_addr;
	int src_sz;
	char buf[MAX_PATH];
	char exe[MAX_PATH];

	//AllocConsole();
	//freopen("CONOUT$", "wb", stdout);
	//freopen("CONIN$", "rb", stdin);
	//enable_debug();

	GetModuleFileName(NULL, codectx.dll_fnz, sizeof(codectx.dll_fnz));
	memcpy(buf, codectx.dll_fnz, sizeof(buf));
	schr = strrchr(codectx.dll_fnz, '.');
	memcpy(schr + 1, "dll", 4);

	schr = strrchr(buf, '\\');
	if(schr)
		memcpy(schr + 1, "config.ini", 11);
	else
		buf[0] = 0;
	exe[0] = 0;
	GetPrivateProfileString("d2info", "exe", 0, exe, sizeof(exe), buf);
	memcpy(buf, exe, sizeof(buf));
	schr = strrchr(buf, '\\');
	if(schr)
		schr[0] = 0;
	else
		buf[0] = 0;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	//GetWindowThreadProcessId(FindWindow("Diablo II", NULL), &pi.dwProcessId);
	//pi.hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, 1904);
	if( !CreateProcess(NULL, exe, 0, 0, 0, CREATE_SUSPENDED, 0, buf, &si, &pi) ) {
		MessageBox(NULL, "cann't create process!", "", 0);
		return 1;
	}

	//read original code
	ReadProcessMemory(pi.hProcess, (LPVOID)codectx.code_addr, codectx.code_str, codectx.code_len, 0);

	//write data seg
	dst_addr = VirtualAllocEx(pi.hProcess, 0, sizeof(codectx), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(pi.hProcess, dst_addr, &codectx, sizeof(codectx), 0);
	*(unsigned long*)(pcode_str + 1) = (unsigned long)dst_addr;

	//write code seg
	get_seg_info(".d2ldr", (unsigned long*)&src_addr, &src_sz);
	dst_addr = VirtualAllocEx(pi.hProcess, 0, src_sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	*(unsigned long*)(pcode_str + 6) = (unsigned long)&d2ldr_jmp_entry - (unsigned long)src_addr + (unsigned long)dst_addr;
	WriteProcessMemory(pi.hProcess, dst_addr, src_addr, src_sz, 0);
	
	//patch code entry
	VirtualProtectEx(pi.hProcess, (LPVOID)codectx.code_addr, codectx.code_len, PAGE_EXECUTE_READWRITE, &oldprt);
	WriteProcessMemory(pi.hProcess, (LPVOID)codectx.code_addr, pcode_str, codectx.code_len, 0);
	VirtualProtectEx(pi.hProcess, (LPVOID)codectx.code_addr, codectx.code_len, oldprt, &oldprt);

	ResumeThread(pi.hThread);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	//getchar();

	return 0;
}

int get_seg_info(const char *seg_nz, unsigned long *seg_addr, int *seg_sz)
{
	PIMAGE_FILE_HEADER ph;
	PIMAGE_OPTIONAL_HEADER oh;
	PIMAGE_SECTION_HEADER sh, csh;
	int i;
	void *addr = (void*)GetModuleHandle(NULL);

	ph = (PIMAGE_FILE_HEADER)((char*)addr + ((PIMAGE_DOS_HEADER)addr)->e_lfanew + 4);
	oh = (PIMAGE_OPTIONAL_HEADER)((char*)ph + sizeof(IMAGE_FILE_HEADER));
	sh = (PIMAGE_SECTION_HEADER)((char*)oh + ph->SizeOfOptionalHeader);

	for(i = 0; i < ph->NumberOfSections; i++) {
		csh = sh + i;
		if( !strcmp(seg_nz, (const char*)csh->Name) ) {
			*seg_addr = (unsigned long)((char*)addr + csh->VirtualAddress);
			*seg_sz = (int)csh->SizeOfRawData;
			return 1;
		}
	}

	return 0;
}

#pragma code_seg(".d2ldr")

typedef BOOL (WINAPI *VirtualProtect_FuncType)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef BOOL (WINAPI *VirtualFree_FuncType)(LPVOID, SIZE_T, DWORD);
typedef HMODULE (WINAPI *LoadLibrary_FuncType)(LPCTSTR);

void __stdcall d2ldr_jmp_entry(pCodeCtx p_codectx)
{
	DWORD oldprt;
	int i;
	VirtualProtect_FuncType VirtualProtect_Func;
	VirtualFree_FuncType VirtualFree_Func;
	LoadLibrary_FuncType LoadLibrary_Func;

	*(unsigned long*)&VirtualProtect_Func = *(unsigned long*)0x0040910C;
	*(unsigned long*)&VirtualFree_Func = *(unsigned long*)0x004091A8;
	*(unsigned long*)&LoadLibrary_Func = *(unsigned long*)0x004091C4;

	*(((unsigned long*)&p_codectx) - 1) = p_codectx->code_entry;

	VirtualProtect_Func((LPVOID)p_codectx->code_addr, p_codectx->code_len, PAGE_EXECUTE_READWRITE, &oldprt);
	for(i = 0; i < p_codectx->code_len; i++)
		((unsigned char*)p_codectx->code_addr)[i] = p_codectx->code_str[i];
	VirtualProtect_Func((LPVOID)p_codectx->code_addr, p_codectx->code_len, oldprt, &oldprt);

	LoadLibrary_Func(p_codectx->dll_fnz);

	VirtualFree_Func(p_codectx, 0, MEM_RELEASE);
}

#pragma code_seg()


BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid )) {
		printf("LookupPrivilegeValue error: %u\n", GetLastError() ); 
		return FALSE;
	}
	
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL)) { 
		printf("AdjustTokenPrivileges error: %u\n", GetLastError() ); 
		return FALSE;
	} 

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		printf("The token does not have the specified privilege. \n");
		return FALSE;
	}
	
	return TRUE;
}

void enable_debug()
{
	HANDLE hToken;

	if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) {
        if(GetLastError() != ERROR_NO_TOKEN) return;
		if(!ImpersonateSelf(SecurityImpersonation)) return;
		if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) return;
     }

	if(!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
		CloseHandle(hToken);
		return;
	}

}
