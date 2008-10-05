#!/usr/bin/env python

#********************************************************************
#                  netClient                              
#    
#  Author:                       
#      Jose .C Renteria            
#      jrenteri@arl.army.mil        
#      US Army Research Laboratory      
#      Adelphi, MD     
#                                                                 
#      Copyright @ 2008 US Army Research Laboratory 
#      All Rights Reserved                                         
#      See Copyright.txt or http://www.arl.hpc.mil/ice for details 
#                                                                  
#      This software is distributed WITHOUT ANY WARRANTY; without  
#      even the implied warranty of MERCHANTABILITY or FITNESS     
#      FOR A PARTICULAR PURPOSE.  See the above copyright notice   
#      for more information.                                       
#                                                                  
# *******************************************************************

import time
import socket
import Pyro.core

def get_dev_stats():
    netDic= {}
    DEVFILE = open ( '/proc/net/dev' )
    lines = DEVFILE.readlines()
    for line in lines:
        if ":" in line:
            ethdev, alldata = line.split(":")
            dataArray = map(int, alldata.split())        
            netDic[ethdev.strip()] = dataArray
    DEVFILE.close()
    return netDic

def get_dev_diff (dev, lastStat, newStat, time_elapsed):
    RKps = (newStat[dev][0] - lastStat[dev][0])/ 1024 / time_elapsed
    TKps = (newStat[dev][8] - lastStat[dev][8])/ 1024 / time_elapsed
    return  RKps, TKps

    

# you have to change the URI below to match your own host/port.
netstat = Pyro.core.getProxyForURI("PYROLOC://glory:7766/welstats")

device = 'eth0'
netInfo_last = {}
netInfo_current = {}
sleep_time =5 #sleep interval in seconds
n = 0

hostname = socket.gethostname().split(".")[0]

# litle trick to parse out node id: assuming synat t30n#d
tmp = hostname +" 0"
try:
    head, tail = tmp.split("n")
    node_id, tail2  = map(int, tail.split("d"))
except ValueError:
    node_id =1

time_last = time.time()
netInfo_last = get_dev_stats()
time.sleep(sleep_time)

while 1:

    time_current = time.time()
    netInfo_current = get_dev_stats()
    elasped_time = time_current - time_last
    rec_rate, trans_rate = get_dev_diff(device,netInfo_last,
                                        netInfo_current,
                                        elasped_time )
   
    netstat.sendData(hostname, node_id,  n, rec_rate, trans_rate)
    time.sleep(sleep_time)
    n = n+1
    time_last = time_current
    netInfo_last = netInfo_current
    
