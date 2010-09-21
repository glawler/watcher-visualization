#!/usr/bin/env python

# Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
# 
# This file is part of WATCHER.
# 
#     WATCHER is free software: you can redistribute it and/or modify
#     it under the terms of the GNU Affero General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
# 
#     WATCHER is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU Affero General Public License for more details.
# 
#     You should have received a copy of the GNU Affero General Public License
#     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
# 
#

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


def doLoop(iface, serverAddr):
    prevBytesTx=getBytesTx(iface)
    while 1:
        time.sleep(1)
        currBytesTx=getBytesTx(iface)
        bw=(currBytesTx-prevBytesTx)/8.0
        print 'Sending ', str(bw), 'bytes transfered to the watcher'
        try:
            retCode=subprocess.call(['sendDataPointMessage', '-s', str(serverAddr), '-g', 'Bandwidth', '-d', str(bw)])
        except OSError:
            print 'Caught exception when trying to run watchergraphtest, is it in your $PATH?'
            print 'If not, type \'export PATH=$PATH:/path/to/dir/with/sendDataPointMessage\' in this shell'
            sys.exit(1)
        prevBytesTx=currBytesTx

if __name__ == "__main__":
    import sys
    if len(sys.argv)!=3:
        print 'Usage:', sys.argv[0], ' network_interface watcher_server_addr'
        listIfaces()
        sys.exit(1)
    doLoop(sys.argv[1], sys.argv[2]) 


