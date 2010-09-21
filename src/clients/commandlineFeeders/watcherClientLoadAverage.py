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

def doMain(server_address):
    while 1:
        la=getLoadAverage()
        print 'Sending load average:', la, ' to the watcher'
        try:
            retCode=subprocess.call(['sendDataPointMessage', '-s', str(server_address), '-g', 'Load Average', '-d', str(la)])
        except OSError:
            print 'Caught exception when trying to run watchergraphtest, is it in your $PATH?'
            print 'If not, type \'export PATH=$PATH:/path/to/dir/with/sendDataPointMessage\' in this shell'
            sys.exit(1)
        time.sleep(1)

if __name__ == "__main__":
    import sys
    if len(sys.argv)!=2:
        print 'Usage:', sys.argv[0], ' watcher_server_addr'
        sys.exit(1)
    doMain(sys.argv[1])


