#include "D3D.h"
#include <stdio.h>
#include <Windows.h>
#include <map>
#include <string>


void dump_scene()
{
	struct SNO
	{
		SNO *next;
		unsigned int id;
		struct DATA
		{
			unsigned int type;
			unsigned int id;
			const char name[128];
		} *data;
	};

	FILE *fp = fopen("c:\\scene_sno.txt", "w");
	D3_NAVCELL_SNO_MGR_PTR ctx = 0;
	D3_SIMPLE_LIST_PTR lst = (D3_SIMPLE_LIST_PTR)(*(char**)0x1895990 + 0x21 * 0x70 + 0x10);
	for(int i = lst->sz - 1; i >= 0; i--) {
		SNO *next = (SNO*)lst->ptr[i];
		while(next) {
			//printf("%06X - %s\n", next->data->id, next->data->name);
			((void (__thiscall*)(void*, unsigned int))0x00AA3C90)(&ctx, next->data->id);
			fprintf(fp, "[0x%06X, \"%s\"],\n", ctx->sno_id, ctx->full_name);
			((void (__thiscall*)(void*))0x00AA3CB0)(&ctx);
			next = next->next;
		}
	}
	fclose(fp);
}

void screen2gamepos()
{
	unsigned long out[8];
	
	D3_GAME_DATA_PTR gd = D3_GameData();
	
	void *player = D3_GetPlayer( D3_GetCurACDHid() );
	if(!player) return;

	((void (__cdecl*)(void*,long,unsigned int*,unsigned long,unsigned long*))0xB2DFF0)(player, -1, &gd->x, 0x777C, out);

	//printf("<%d, %d> <%0.2f, %0.2f, %0.2f>\n", gd->x, gd->y, *(float*)&out[2], *(float*)&out[3], *(float*)&out[4]);

	struct {
		float x;
		float y;
		float z;
		unsigned int world_id;
	} pk = {*(float*)&out[2], *(float*)&out[3], 0, 0x772E0000};

	if( !D3_GetNavCellIdx(&pk, out) ) return;
	D3_SCENE_PTR scene = D3_GetScene(out[0]);
	if(!scene) return;

	D3_NAVCELL_SNO_MGR_PTR ctx = 0;
	((void (__thiscall*)(void*, unsigned int))0x00AA3C90)(&ctx, scene->_17C->sno_id);
	if(!ctx) return;
	unsigned short flag = ctx->mgr.nav_lst[ out[1] ].flag;
	printf("<%06X, %04X> %s\n", ctx->sno_id, flag, ctx->full_name);
	((void (__thiscall*)(void*))0x00AA3CB0)(&ctx);
}


__declspec(naked) const char* __stdcall get_string(int tid, int code)
{
	__asm {
		push ebp;
		mov ebp, esp;
		push edi;
		mov edi, [ebp + 0x08];
		push 0;
		push esp;
		push [ebp + 0x0C];
		mov eax, 0xE7DA20;
		call eax;
		add esp, 0x0C;
		pop edi;
		mov esp, ebp;
		pop ebp;
		retn 0x08;
	}
}

std::map<unsigned int, const char*> _nmap;

void dump_item_affix()
{
	FILE *fp = fopen("c:\\item_affix.txt", "w");
	D3_ITEM_AFFIX_DESC_PTR lst = D3_GetItemAffixDescTable();
	for(int i = 0; i < D3_GetItemAffixDescTableSize() + D3_GetItemAffixDescExTableSize(); i++) {
		D3_ITEM_AFFIX_DESC_PTR ptr = lst + i;

		fprintf(fp, "[0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, \"%s\", \"%s\"],\n",
			ptr->hid, ptr->attr_id_0, ptr->attr_id_1, ptr->attr_id_2,
			ptr->_10, ptr->_14, ptr->call_has_affix, ptr->call_format,
			ptr->_20, _nmap.find(ptr->hid)->second, get_string(0xc918, ptr->hid)
			);
	}
	fclose(fp);
}



unsigned int __cdecl _name_to_hid(const char *s)
{
	unsigned int hid = ((unsigned int (__cdecl*)(const char*))0x1019420)(s);

	_nmap[hid] = s;
	//printf("%08X - %s\n", hid, s);

	return hid;
}


void hook_name_hash_setup()
{
	DWORD prot;

	VirtualProtect((void*)0xE7C7D4, 5, PAGE_EXECUTE_READWRITE, &prot);
	*(unsigned long*)(0xE7C7D4 + 1) = (unsigned long)&_name_to_hid - 0xE7C7D4 - 5;
	VirtualProtect((void*)0xE7C7D4, 5, prot, &prot);
}

static void *g_desc = 0;
static int g_sz = 0;

unsigned char __cdecl hook_proto_name(void *ebp, void *retip, char *name, void *type)
{
	printf("proto: %08X, %s\n", type, name);

	char buf[128];
	strcpy(buf, name);
	char *b = buf;
	for(; *b; b++) { if(*b == '/') *b = '_'; }
	char path[512];
	strcpy(path, "c:\\d3_proto\\");
	strcat(path, buf);
	printf("> %s\n", path);
	FILE *fp = fopen(path, "wb");
	fwrite(g_desc, 1, g_sz, fp);
	fclose(fp);

	g_desc = 0;
	g_sz = 0;

	return *(unsigned char*)0x1939CB0;
}

void __cdecl hook_proto_desc(void *ret_ip, void *esp, void *ebp, void *ret_ip_2, void *desc, int sz)
{
	printf("desc: %08X, %08X\n", desc, sz);
	g_desc = desc;
	g_sz = sz;
}

__declspec(naked) void hook_proto_desc_asm()
{
	__asm {
		call hook_proto_desc;
		push dword ptr [esp];
		mov dword ptr [esp + 0x04], 0x013B3BE9;
		retn;
	}
}

void hook_proto_setup()
{
	unsigned char buf[5] = {0xE8};
	DWORD prot;
	unsigned long ip;

	ip = 0x11303D3;
	VirtualProtect((void*)ip, 5, PAGE_EXECUTE_READWRITE, &prot);
	*(unsigned long*)(buf + 1) = (unsigned long)hook_proto_name - ip - 0x05;
	memcpy((void*)ip, buf, 5);
	VirtualProtect((void*)ip, 5, prot, &prot);

	ip = 0x0115BFB5;
	VirtualProtect((void*)ip, 5, PAGE_EXECUTE_READWRITE, &prot);
	*(unsigned long*)(buf + 1) = (unsigned long)hook_proto_desc_asm - ip - 0x05;
	memcpy((void*)ip, buf, 5);
	VirtualProtect((void*)ip, 5, prot, &prot);

}

FILE *g_fp;
void __cdecl hook_ui(char *o, void *ret, char *name)
{
	((void (__thiscall*)(void*,void*))0x926A10)(o, name);

	fprintf(g_fp, "[0x%016llX, \"%s\"],\n", *(unsigned long long*)o, o + 8);
}

__declspec(naked) void hook_ui_asm(char *name)
{
	__asm {
		push ecx;
		call hook_ui;
		pop eax;
		retn 4;
	}
}



void hook_ui_setup()
{
	DWORD prot;
	unsigned long ip;

	g_fp = fopen("c:\\ui_name.txt", "w");
	ip = 0x00926AAE;
	VirtualProtect((void*)ip, 4, PAGE_EXECUTE_READWRITE, &prot);
	*(unsigned long*)ip = (unsigned long)&hook_ui_asm - ip - 0x04;
	VirtualProtect((void*)ip, 4, prot, &prot);
}

/*
void dump_item_attr()
{
	FILE *fp = fopen("c:\\item_attr.txt", "w");
	D3_ATTR_DESC_PTR lst = D3_GetAttrDescTable();
	for(int i = 0; i < D3_GetAttrDescTableSIZE(); i++) {
		D3_ATTR_DESC_PTR ptr = lst + i;

		fprintf(fp, "[0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, \"%s\", 0x%08X, 0x%08X],\n",
			ptr->attr_id, ptr->default_val,
			ptr->_04[0], ptr->_04[1], ptr->_04[2], ptr->_04[3], ptr->_04[4],
			ptr->attr_name, ptr->obj, ptr->flag
			);
	}
	fclose(fp);
}*/

void dump_ui_handler()
{
	FILE *fp = fopen("c:\\ui_handler.txt", "w");
	D3_UI_HANDLER_DESC_PTR lst = D3_GetUIHandlerDescTable();
	for(int i = 0; i < D3_GetUIHandlerDescTableSize(); i++, lst++) {
		fprintf(fp, "[0x%08X, 0x%08X, \"%s\"],\n",
			lst->hid, lst->handler, lst->name
			);
	}
	fclose(fp);
}

void sleep_hook()
{

	long *p = (long*)0x1801264;

	printf("%d - %d\n", p[0], p[1]);

}
