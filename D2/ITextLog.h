#pragma once

class ITextLog
{
public:
	virtual void WriteTo(const char* text) = 0;
};