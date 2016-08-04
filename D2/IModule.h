#pragma once

class IProxy;
class IPacket;

enum ModuleKind {
	RealmModule,
	ChatModule,
	GameModule
	};

class IModule
{
public:
	virtual void __stdcall Destroy() = 0;
	virtual void __stdcall OnRelayDataToServer(IPacket* packet, const IModule* owner) = 0;
	virtual void __stdcall OnRelayDataToClient(IPacket* packet, const IModule* owner) = 0;
	virtual void __stdcall Update() = 0;
	virtual ~IModule(){}
};