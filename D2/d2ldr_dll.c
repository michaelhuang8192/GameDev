#include <stdio.h>
#include <windows.h>

static HMODULE g_d2client_handle;
static HINSTANCE g_cur_inst;

void ldr_start();
void ldr_end();
int __stdcall recv_packet(char *buf, int len);
int __stdcall send_packet(int len, int type, const char *buf);
int (__stdcall *_recv_packet)(char *buf, int len);
int (__stdcall *_send_packet)(int len, int type, const char *buf);
int send_packet_hook(int len, int type, const char *buf);
void print_packet(int server, const char *buf, int len);

int __stdcall send_data_to_server(const char *buf, int len);
int __stdcall send_data_to_client(const char *buf, int len);
extern int __stdcall relay_data_to_client(const char *buf, int len, int max_len);
extern int __stdcall module_update();
extern int __stdcall module_init(const char *plugin_dir);
extern int __stdcall relay_data_to_server(const char *buf, int len, int max_len);

typedef struct _GamePacket {
	struct _GamePacket *prev;
	struct _GamePacket *next;
	int len;
	char data[1];
} GamePacket, *pGamePacket;

static GamePacket g_server_data_head = {&g_server_data_head, &g_server_data_head, 0};
static GamePacket g_client_data_head = {&g_client_data_head, &g_client_data_head, 0};
static int g_in_game = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

	switch(fdwReason) {

	case DLL_PROCESS_ATTACH:
		{
			g_cur_inst = hinstDLL;
			ldr_start();
		}
		break;

	case DLL_PROCESS_DETACH:
		{
			ldr_end();
		}
		break;

	}

	return 1;
}

static void ldr_start()
{
	char dll_fnz[MAX_PATH];
	DWORD oldprot;
	unsigned long addr;

	AllocConsole();
	freopen("CONOUT$", "wb", stdout);

	printf("Dll Attached...\n");
	g_d2client_handle = LoadLibrary("D2Client.dll");
	//_recv_packet = D2Net.10008;
	//_send_packet = D2Net.10024;
	addr = (unsigned long)g_d2client_handle + 0xCEB38;
	VirtualProtect((LPVOID)addr, 8, PAGE_READWRITE, &oldprot);
	*(unsigned long*)&_recv_packet = *(unsigned long*)addr;
	*(unsigned long*)addr = (unsigned long)&recv_packet;
	*(unsigned long*)&_send_packet = *(unsigned long*)(addr + 4);
	*(unsigned long*)(addr + 4) = (unsigned long)&send_packet;
	VirtualProtect((LPVOID)addr, 8, oldprot, &oldprot);

	GetModuleFileName(g_cur_inst, dll_fnz, sizeof(dll_fnz));
	strcpy(strrchr(dll_fnz, '\\') + 1, "plugins");
	module_init(dll_fnz);
}

static void ldr_end()
{
	FreeLibrary(g_d2client_handle);
}

static void flush_server_data()
{
	pGamePacket cur = g_server_data_head.next;

	while(g_server_data_head.next != &g_server_data_head) {
		cur = g_server_data_head.next;
		g_server_data_head.next = cur->next;
		cur->next->prev = &g_server_data_head;
		send_packet_hook(cur->len, 0, cur->data);
		free(cur);
	}
}

static void gamepacket_clear(pGamePacket head)
{
	pGamePacket tmp;
	pGamePacket cur = head->next;
	while(cur != head) {
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	head->prev = head->next = head;
}

static int recv_user_packet(char *buf, int len)
{
	int n;
	pGamePacket cur_pkg = g_client_data_head.next;
	if(cur_pkg == &g_client_data_head) return -1;

	g_client_data_head.next = cur_pkg->next;
	cur_pkg->next->prev = &g_client_data_head;
	n = min(len, cur_pkg->len);
	memcpy(buf, cur_pkg->data, n);
	free(cur_pkg);

	return n;
}

static int __stdcall recv_packet(char *buf, int len)
{
	int n;
	int again = 1;
	while(again) {
		again = (n=recv_user_packet(buf, len)) < 0 && (n=_recv_packet(buf, len)) > 0 && (n=relay_data_to_client(buf, n, len)) <= 0;
		flush_server_data();
	}

	if(n <= 0) {
		module_update();
		flush_server_data();
		if(n < 0) n = recv_user_packet(buf, len);
	}

	return n;
}

static int __stdcall send_packet(int len, int type, const char *buf)
{
	int n = len;
	int dont_send = n && (n=relay_data_to_server(buf, n, len)) <= 0;
	flush_server_data();
	if(dont_send) return len;

	n = send_packet_hook(n, type, buf);
	return n > 0 ? len : n;
}

static int send_packet_hook(int len, int type, const char *buf)
{
	if(len > 0) {
		if(buf[0] == 0x67 || buf[0] == 0x68) {
			g_in_game = 1;
		} else if(buf[0] == 0x69) {
			g_in_game = 0;
			gamepacket_clear(&g_server_data_head);
			gamepacket_clear(&g_client_data_head);
		}
	}

	return _send_packet(len, type, buf);
}

int __stdcall send_data_to_server(const char *buf, int len)
{
	pGamePacket pkg;
	if(!g_in_game) return 0;
	
	pkg = (pGamePacket)malloc( sizeof(GamePacket) + len - 1 );
	memcpy(pkg->data, buf, len);
	pkg->len = len;

	pkg->prev = g_server_data_head.prev;
	pkg->next = &g_server_data_head;
	pkg->prev->next = pkg;
	g_server_data_head.prev = pkg;

	return len;
}

int __stdcall send_data_to_client(const char *buf, int len)
{
	pGamePacket pkg;
	if(!g_in_game) return 0;

	pkg = (pGamePacket)malloc( sizeof(GamePacket) + len - 1 );
	memcpy(pkg->data, buf, len);
	pkg->len = len;

	pkg->prev = g_client_data_head.prev;
	pkg->next = &g_client_data_head;
	pkg->prev->next = pkg;
	g_client_data_head.prev = pkg;

	return len;
}

static void print_packet(int server, const char *buf, int len)
{
	int i;
	printf("%s[%02d] - ", server ? "SEND": "RECV", len);
	for(i = 0; i < len; i++) printf("%02X ", (unsigned char)buf[i]);
	printf("\n");
}