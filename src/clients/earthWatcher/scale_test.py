#!/usr/bin/env python

# Script to generate a set of mobile nodes and move them around randomly by sending GPSMessage(s)

import os
import sys
import random
import time

total_nodes=10 # number of nodes to create
refresh_rate=200 # milliseconds -- how often to move a node
server_name='localhost'
pos_increment=0.0005 # how far to move the node during each update

net_addr = '10.0.0.' # network address prefix -- used as label

# base coordinates for test
base_lat = 34.0
base_lon = -118.0

class Node(object):
    """Represents an individual mobile node."""
    def __init__(self, id):
        self.id = id
        self.addr = net_addr + str(id)
        self.lat = base_lat
        self.lon = base_lon

    def adjust(self, rand_val):
        if rand_val <= 0.25:
            self.lat += pos_increment
        elif rand_val <= 0.50:
            self.lat -= pos_increment
        elif rand_val <= 0.75:
            self.lon += pos_increment
        else:
            self.lon -= pos_increment
        return self

    def update(self):
        #print './sendGPSMessage -n %s -x %f -y %f -s %s' % (self.addr, self.lat, self.lon, server_name)
        os.system('./sendGPSMessage -n %s -x %f -y %f -s %s' % (self.addr, self.lat, self.lon, server_name))


nodes = [Node(x) for x in xrange(1, total_nodes+1)]

while True:
    # select a node to update at random
    nodes[random.randint(0, total_nodes-1)].adjust(random.random()).update()
    time.sleep(refresh_rate / 1000.0)

sys.exit(0)
