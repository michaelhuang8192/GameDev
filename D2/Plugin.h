#pragma once
#include "windows.h"
#include "IModule.h"

typedef void(__stdcall *DestroyPlugin)(void* Info);
typedef IModule*(__stdcall *ModuleCreator)(IProxy* proxy, ModuleKind Kind);

typedef struct _PluginInfo {
	const char* Name;
	const char* Author;
	int SDKVersion;
	DestroyPlugin Destroy;
	ModuleCreator Create;

} PluginInfo, *pPluginInfo;

typedef struct _PluginCtx {
	pPluginInfo pi;
	IModule *mod;
	HMODULE dll;
} PluginCtx, *pPluginCtx;

typedef PluginInfo* (__stdcall *InitPlugin)(void *);
