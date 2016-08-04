#include "D3D.h"
#include <stdio.h>
#include <python.h>
#include <Windows.h>

HINSTANCE g_inst;
char g_app_path[4096];
void *g_main_fiber = 0;
void *g_worker_fiber = 0;
unsigned int g_worker_sleep_ts = 0;
unsigned int g_worker_sleep_ms = 0;
int g_worker_quit = 0;


unsigned int worker_sleep(unsigned int ms);

VOID CALLBACK worker_proc(PVOID lpParameter)
{
	//extern void dump_item_affix(); dump_item_affix();
	//extern void dump_item_attr(); dump_item_attr();
	//extern void dump_ui_handler(); dump_ui_handler();

	PyRun_SimpleString("from D3 import main; main.run();");

	g_worker_quit = 1;
	printf("Worker Done..\n");
	worker_sleep(0);
}

unsigned int worker_sleep(unsigned int ms)
{
	//printf("switch to main fiber\n");
	//extern void sleep_hook(); sleep_hook();

	g_worker_sleep_ts = GetTickCount();
	g_worker_sleep_ms = ms;
	SwitchToFiber(g_main_fiber);
	unsigned int rest = GetTickCount() - g_worker_sleep_ts;
	return rest > g_worker_sleep_ms ? rest - g_worker_sleep_ms : 0;
}

void D3_Mainloop()
{
	if(g_worker_quit) return;
	if(!g_main_fiber) {
		g_main_fiber = ConvertThreadToFiber(0);
		g_worker_fiber = CreateFiber(0, worker_proc, 0);
		printf("<Parent:%08X>, <Worker:%08X>, <ThreadId:%08X>, <Dll:%08X>\n", g_main_fiber, g_worker_fiber, GetCurrentThreadId(), GetModuleHandle(0));
	}
	if(GetTickCount() - g_worker_sleep_ts >= g_worker_sleep_ms) {
		SwitchToFiber(g_worker_fiber);
	}
	D3_Hook_OnServerPacket_Setup();
}

int D3_OnServerPacket(void *packet, int type, void *global_38)
{
	static int seq = 0;
	unsigned int pid = ((unsigned int*)packet)[1];
	D3_PACKET_TYPE_PTR t = &D3_ServerPacketTypeTable[ pid ];

	if(pid == 0x53 || pid == 0xa2 || pid == 0x13b) return 1;
	if(pid != 0x55 && pid != 0x45) return 1;
	printf("[%05X][%x:len(%x):type(%d)] %s\n", seq++, pid, t->sz, type, t->name);

	return 1;
}

void init_py()
{
	PyObject *t;

	char buf[4096];
	strcpy(buf, g_app_path);
	strcat(buf, ";");
	strcat(buf, g_app_path);
	strcat(buf, "\\lib");

	Py_DontWriteBytecodeFlag++;
	Py_NoSiteFlag++;
	Py_Initialize();
	PySys_SetPath(buf);

	t = PyString_FromString(g_app_path);
	PySys_SetObject("app_path", t);
	Py_DECREF(t);

	t = PyFile_FromString("CONOUT$", "wb");
	PyFile_SetBufSize(t, 0);
	PySys_SetObject("stdout", t);
	Py_DECREF(t);
	t = PyFile_FromString("CONOUT$", "wb");
	PyFile_SetBufSize(t, 0);
	PySys_SetObject("stderr", t);
	Py_DECREF(t);

	extern void init_D3D_module();
	init_D3D_module();
}

void close_py()
{
	Py_Finalize();
}

void init()
{
	GetModuleFileNameA(g_inst, g_app_path, sizeof(g_app_path));
	char *s = strrchr(g_app_path, '\\');
	if(s) *s = 0x00;

	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	init_py();

	D3_Hook_Mainloop_Setup();

	//extern void hook_name_hash_setup(); hook_name_hash_setup();
	//extern void hook_proto_setup(); hook_proto_setup();
	//extern void hook_ui_setup(); hook_ui_setup();

}

void close()
{
	close_py();

}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch(fdwReason)
	{ 
		case DLL_PROCESS_ATTACH: { g_inst = hinstDLL; init(); } break;
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
		case DLL_PROCESS_DETACH: { close(); }  break;
	}

	return 1;
}
