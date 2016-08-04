from _D3 import *
import _struct as struct

def UI2Hid(ui_name):
    if not ui_name: return 0
    s = 0xCBF29CE484222325
    for c in ui_name.lower():
        s ^= ord(c)
        s = ((((s >> 32) * 0x1B3 + (s & 0xFFFFFFFF) * 0x100) << 32) + (s & 0xFFFFFFFF) * 0x1B3) & 0xFFFFFFFFFFFFFFFF
    return s

def UI_IsHidden(hid):
    ui = GetUI(hid)
    if ui and ui.visible: return False
    return True

def Wait(func, func_arg, timeout=None):
    cts = GetTickCount()
    while not func(func_arg):
        if timeout != None:
            nts = GetTickCount()
            if nts - cts >= timeout: return False
            cts = nts
        Sleep(0)
        
    return True

def ReadMemoryEx(ptr, fmt):
    if not ptr: return None
    sz = struct.calcsize(fmt)
    mem = ReadMemory(ptr, sz)
    if not mem: return None
    return struct.unpack(fmt, mem)

