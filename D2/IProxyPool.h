#pragma once

class TcpProxy;

class IProxyPool
{
public:
	virtual void AddProxy(TcpProxy* proxy) = 0;
	virtual void RemoveProxy(TcpProxy* proxy) = 0;
	virtual void ClearProxies() = 0;
};