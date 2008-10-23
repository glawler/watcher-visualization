#!/usr/bin/env python

from __future__ import with_statement
import subprocess
import time
import re

def getBytesTx(iface):
    # sample output of /proc/net/dev:
    # face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    #    lo:745123180 8766258    0    0    0     0          0         0 745123180 8766258    0    0    0     0       0          0
    #  eth0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    #  eth1:1500615070 11444980    0    0    0     0          0      2447 3629337877 10060250    0    0    0     0       0          0

    with open('/proc/net/dev') as f:
        for line in f:
            if iface in line:
                data=line.replace(':', ' ').split()
                return int(data[1])
    return 0

def doLoop(iface):
    prevBytesTx=getBytesTx(iface)
    while 1:
        time.sleep(1)
        currBytesTx=getBytesTx(iface)
        bw=(currBytesTx-prevBytesTx)/8.0
        print 'Sending ', str(bw), 'bytes transfered to the watcher'
        retCode=subprocess.call(['watchergraphtest', '-g', 'Bandwidth', '-d', str(bw)])
        prevBytesTx=currBytesTx

if __name__ == "__main__":
    doLoop('eth1')


