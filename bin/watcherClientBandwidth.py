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
    print 'Error: did not find interface', iface, ' in list of network devices'
    listIfaces()
    sys.exit(1)

def listIfaces():
    print 'Interfaces on this machine are:'
    with open('/proc/net/dev') as f:
        for line in f:
            found=re.search('(\w*):\d*', line)
            if found:
                print found.group(1)


def doLoop(iface):
    prevBytesTx=getBytesTx(iface)
    while 1:
        time.sleep(1)
        currBytesTx=getBytesTx(iface)
        bw=(currBytesTx-prevBytesTx)/8.0
        print 'Sending ', str(bw), 'bytes transfered to the watcher'
        try:
            retCode=subprocess.call(['watchergraphtest', '-g', 'Bandwidth', '-d', str(bw)])
        except OSError:
            print 'Caught exception when trying to run watchergraphtest, is it in your $PATH?'
            print 'If not, type \'export PATH=$PATH:/path/to/dir/with/watchergraphtest/in/it\' in this shell'
            sys.exit(1)
        prevBytesTx=currBytesTx

if __name__ == "__main__":
    import sys
    if len(sys.argv)!=2:
        print 'Usage:', sys.argv[0], ' network_interface'
        listIfaces()
        sys.exit(1)
    doLoop(sys.argv[1])


