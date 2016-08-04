import sys
import D3
import traceback
import os

def run():
    print "script start...."
    lm = 0
    sf = sys.app_path + "\\test.py"
    while True:
        D3.Sleep(0)
        if not os.path.exists(sf): continue
        
        s = None
        s = os.stat(sf)
        
        if s.st_mtime > lm:
            ctx = {}
            try:
                lm = s.st_mtime
                eval(compile(open(sf, 'rb').read(), "test.py", 'exec'), ctx)
            except:
                print traceback.format_exc()
            ctx.clear()