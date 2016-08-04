import D3
import time
import sys
from _struct import unpack, calcsize


print "--->>+++"

for i in range(100):
    D3.CallCFunction()
    D3.Sleep(1000)

        
"""
for u in D3.GetACD():
    if u.name.lower().find('leader'.lower()) < 0: continue
    p = u.pos0
    print u.name, p.v0, p.v1, p.v2
    sp = D3.Pos_WorldToScreen(p.v0, p.v1, p.v2)
    print int(sp[0]), int(sp[1])
    D3.UseSkill(0x777C, int(sp[0]), int(sp[1]))
    print "kk"
"""
"""
for i in range(100):
    print D3.UseSkill(0x012ef0, 800, 600)
    D3.Sleep(0)
"""
#print D3.GetCurACD()

"""
for u in D3.GetUI():
    if not u.visible: continue
    #if u.name.lower().find('object'.lower()) < 0: continue
    #if u.name.lower().find('_4'.lower()) < 0: continue
    #print u, u.name, u.GetFlag()
    
print D3.GetUI(16445388802899937593)
"""

"""
sz = 4096
s = D3.ReadMemory(0x27DFAD00, sz)
for i in range(0, sz, 4):
    fw = s[i : i + 4]
    sw = sys.rm[i : i + 4]
    
    if fw != sw:
        fl = unpack('L', fw)[0]
        sl = unpack('L', sw)[0]
        print "%04X: %08X, %08X" % (i, fl, sl)

print "Done"
"""