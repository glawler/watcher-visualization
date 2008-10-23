#!/usr/bin/env python

import time
import re
import subprocess

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
        retCode=subprocess.call(['watchergraphtest', '-g', 'Load Average', '-d', str(la)])
        time.sleep(1)

if __name__ == "__main__":
    doMain()


