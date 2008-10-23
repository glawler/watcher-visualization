#!/usr/bin/env python

import time
import re
import subprocess
import sys

def getLoadAverage():
    # sample output ' 09:04:26 up 6 days, 22:32, 16 users,  load average: 0.08, 0.08, 0.04'
    output=subprocess.Popen(['uptime'], stdout=subprocess.PIPE).communicate()[0]
    la=re.search('load average: ([^,]*),', output)
    if la:
        return float(la.group(1))
    return -1.0

def doMain():
    while 1:
        la=getLoadAverage()
        print 'Sending load average:', la, ' to the watcher'
        try:
            retCode=subprocess.call(['watchergraphtest', '-g', 'Load Average', '-d', str(la)])
        except OSError:
            print 'Caught exception when trying to run watchergraphtest, is it in your $PATH?'
            print 'If not, type \'export PATH=$PATH:/path/to/dir/with/watchergraphtest/in/it\' in this shell'
            sys.exit(1)
        time.sleep(1)

if __name__ == "__main__":
    doMain()


