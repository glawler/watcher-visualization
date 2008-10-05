#!/usr/bin/env python

#********************************************************************
#                  netServer                              
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

import Pyro.core

class WELStats(Pyro.core.ObjBase):
        def __init__(self):
	    Pyro.core.ObjBase.__init__(self)
	    self.netData= {}
	    
        def sendData (self, host, node_id, step, rec_rate, trans_rate):
	    if self.netData.has_key(host):
		    data = self.netData[host]
		    avg_rr = self.computeAvg(data[1], data[3], rec_rate)
		    avg_tr = self.computeAvg(data[2], data[3], trans_rate)
		    sample_cnt =  data[3]+1
		    newdata = data[0], avg_rr, avg_tr, sample_cnt, rec_rate,  trans_rate
		    self.netData[host] = newdata
	    else:
	            # data[0] is node_id
		    # data[1] is avg R_rate
		    # data[2] is avg T_rate
		    # data[3] is  number of samples
		    # data[4] is the current R_rate
		    # data[5] is the current T_rate
		    
		    data = node_id, rec_rate, trans_rate , 1, rec_rate,  trans_rate
		    self.netData[host] = data
	    
        def getData(self):
	    # sort data
	    return self.netData
    
        def computeAvg(self, avg, num_samples, new_sample):
	    new_average = ((avg * num_samples) + new_sample) / (num_samples + 1)
	    return new_average


Pyro.core.initServer()
daemon=Pyro.core.Daemon()
uri=daemon.connect(WELStats(),"welstats")

print "The daemon runs on port:",daemon.port
print "The object's uri is:",uri

daemon.requestLoop()

