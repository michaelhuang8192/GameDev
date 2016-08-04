#include "D3D.h"
#include <stdio.h>
#include <Windows.h>

static BOOL WINAPI __HookMainLoop(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	if( *(((unsigned long*)&lpMsg) - 1) == D3__PEEKMESSAGE_CALL_RET ) D3_Mainloop();
	return PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

void D3_Hook_Mainloop_Setup()
{
	DWORD oldprot;
	VirtualProtect(D3__PEEKMESSAGE, 0x04, PAGE_READWRITE, &oldprot);
	*D3__PEEKMESSAGE = (unsigned long)&__HookMainLoop;
	VirtualProtect(D3__PEEKMESSAGE, 0x04, oldprot, &oldprot);
}

static unsigned long __old_OnServerPacket = 0;
static int __cdecl __HookOnServerPacket(void *a, int b, void *c)
{
	D3_OnServerPacket(a, b, c);
	return ((int (__cdecl*)(void*, int, void*))__old_OnServerPacket)(a, b, c);
}


void D3_Hook_OnServerPacket_Setup()
{
	unsigned long tmp = *D3__SERVER_PACKET_HANDLER();
	if( !tmp || tmp == (unsigned long)&__HookOnServerPacket ) return;
	__old_OnServerPacket = tmp;
	*D3__SERVER_PACKET_HANDLER() = (unsigned long)&__HookOnServerPacket;
	printf("Setup Handler - OnServerPacket\n");
}


unsigned int D3_NameToHid(const char *s)
{
	unsigned int hid = 0;
	if(!s) return hid;
	for(; *s; s++) hid = (hid << 5) + hid + (unsigned int)(int)*s;
	return hid;
}

