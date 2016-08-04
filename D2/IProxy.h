#pragma once

class IPacket;
class IModule;

class IProxy
{
public:
    //Public
	virtual void __stdcall RelayDataToServer(const IPacket* packet, const IModule* owner) = 0;
	virtual void __stdcall RelayDataToClient(const IPacket* packet, const IModule* owner) = 0;
	virtual int __stdcall GetClientSocket() = 0;
	virtual int __stdcall GetServerSocket() = 0;
	virtual IPacket* __stdcall CreatePacket(const void* data, int size) const = 0;
	virtual IProxy* __stdcall GetPeer() = 0;

    //Internal use
    virtual void RemoveModule(IModule* module) = 0;
	virtual ~IProxy() {}
};