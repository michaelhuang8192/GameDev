#ifndef __D2LDR__
#define __D2LDR__


typedef struct _CodeCtx{
	char dll_fnz[MAX_PATH];
	unsigned long code_entry;
	unsigned long code_addr;
	unsigned char code_str[ 32 ];
	int code_len;

} CodeCtx, *pCodeCtx;


#endif
