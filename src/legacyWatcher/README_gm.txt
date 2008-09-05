#
# $Id: README_gm.txt,v 1.3 2007/01/24 23:48:51 glawler Exp $
#
#  Copyright (C) 2007, SPARTA, Inc.
#  All rights reserved.
#

This README documents the gmcluster and interimgm hierarchy algorithms. 

Introduction
------------
The gmcluster algorithm implements "flat partitioning" amongst nodes in a MANET. 
Flat partitioning is putting nodes in a set of groups such that the node's velocities
(speed and direction) are similar. The idea is that if this information is made available
to the nodes when generating and maintaining a hierarchy, then the hierarchy can be more
robust and stable as links of the hierarchy can be made using links that will exist for 
a longer time. If links are more stable then heartbeat messages can be less frequent, 
CPU cycles and bandwidth are saved as hierarchy links are not brought up and broken
down as frequently. 

The main binary for generating the flat partitioning is gmcluster. The heart of gmcluster
is GPS information and the Link Expiration Time (LET) computation. 

In the gmcluster implementation GPS is handled by two external components: the GPS 
server and the GPS daemon. The GPS server is NRL's MANE component, mane-gpse. Information about 
mane-gpse and running mane-gpse can be found at http://cs.itd.nrl.navy.mil/work/mane/index.php. 
For our purposes it is enough to know that GPS for all nodes in the MANET are multicast 
to all nodes. (At SPARTA, it is multicast over a control channel on the testbed). The 
GPS daemon component (found in ./hierachyClients/gpsDaemon) listens for the multicast GPS 
information, extracts the GPS information, then rebroadcasts it to the localhost as 
a WatcherGPS hierarchy message (type: IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS). 
gmcluster subscribes to these messages and stores the information for localhost
in node specific storage. 

The Link Expiration Time is the estimated length of time that two nodes will 
remain in contact based on instantaneous velocity and antenna radius. The formula
used assumes both nodes have the same antenna radius. It also does not attempt to 
use any historical information other than the last two known points of each node. 
The formula was taken from "Dead Reckoning in Mobile Ad Hoc Networks" by Aarti Agarwal 
(University of Cinninati) and Samir R. Das (SUNY at Stony Brooke) [wcnc-03]. Additional
computation (and algorithm ideas) taken from Kyriakos Manousakis (Telcordia). The 
formula for LET used in gmcluster is:

-(ab+cd) + sqrt( (a^2+c^2) R^2 - (ad-bc)^2 )
--------------------------------------------
                 a^2 + c^2

where:
a = velocity node 1 - velocity node 2, where V = (x2-x1)/(t2-t1), where the xs are two 
	x coordinates of the node and t are the time values when the coordinates were measured. 
b = x2-x1 (x coordinates of the nodes)
c = same as a, but with y coordinates
d = same as b, but with y cooridnates
R = effective radius of node's radios 

Note the formula assumes a Cartesian coordinate system, even though gmcluster uses GPS
information. gmcluster converts GPS to Cartesian using the CEarth C++ library. Also this 
formula can not be used with a more realistic RF model, terrain model, or historical 
velocity input. 

gmcluster internals
------------------
GPS data processing in gmcluster is as follows. When a WatcherGPS hierarchy message
is received, the GPS information is extracted and put on a list. Only localnode GPS 
information is gotten this way. When a hello message goes out, gmcluster puts the newest
GPS information (including a timestamp) for the localnode into the outbound message. When
a hello is received from a neighboring node (be it one-hop or greater), the GPS information
is extracted and put on in a similar list of GPS information about that neighbor. These 
lists are then used to periodically generate a list of neighbors whose links have a high 
enough LET, for the localnode to consider them part of its own group. 

The clustering algorithm is a simple heartbeat based one. Periodically every node generates 
a list of neighbors which have a LET greater than a runtime threshold. The list is sorted by 
ID (IP address). If the localnode has the lowest ID, it periodically sends a 
heartbeat message with a group ID (GID) (the last octet of its IP address). The node that 
broadcasts the heartbeat is called the group leader. If a node (including the group
leader) receive a GID message with a lower GID than it currently has, then it sets its GID
to that one. If it is a group leader, it ceases to send heartbeats as its received a heartbeat
from a node in the group with a lower GID. This has the effect of very quickly setting one 
node in a group to be the group leader. If a node does not hear from it's group leader for 
a configurable number of heartbeat intervals, it sets its GID to the lowest ID from its list of
LET acceptable neighbors and starts over. Heartbeat messages received from LET acceptable 
neighbors are forwarded; Others are silently dropped. This limits the heartbeat flood to 
the maximum diameter of the group. This algorithm works as the acceptable LET computation
done by every node implicitly defines all the groups automatically. Then all that needs
to be done is make sure everyone is aware of the group's ID, so they can claim membership
in that particular group.

gmcluster integration with other hierarchy components
-----------------------------------------------------
gmcluster relies on another component (interim2 or the hello module) to send hellos for it
and do neighbor sensing. There is an API for clustering algorithms to use to interface 
with another hello-sending module. This API can be found hello.h. The API includes callbacks
for hello arrival and hello departure. It allows gmcluster to append arbitrary bits to 
an outbound hello, and have access to a buffer containing (hopefully) the same bits on 
inbound arrival. 

gmcluster exports an API as well. The API answers the question: is this node in my group?
The API can be found in gmcluster.h and consists of one function:

int gmclusterInMyGroup(const manetNode *us, const neighbor *n);

which returns non-zero if the neighbor n is in the localnode's group. 

This API is used by interim2 when choosing clusterheads. It is not currently used by anyone else.

gmcluster and interimgm
-----------------------
The idsCommunications Makefile will build two gmcluster related binaries (if told to do so).
The first, gmcluster simply implements the flat partitioning algorithm and sits on the group ID
information doing nothing. It uses the hello module for neighbor sensing and hello packet data
transmission. The second is a special build of liveinterim2. gmcluster uses liveinterim2 for 
hello packet data transmission and neighbor sensing. Then liveinterim2 uses gmcluster to make 
sure that it only uses neighbors from its group as clusterheads. 

Search for gmcluster in the Makefile and uncomment the appropriate lines to build 
gmcluster and (live)interimgm. gmcluster has dependencies outside of the idsCommunciations 
directory that needs to be build before a make is done in idsCommunications. (I did not add
the build dependencies in the idsCommunications/Makefile as I did not want it to get too
ugly). The following command from ./idsCommunications is usually good enough to build the external
dependencies:

for i in ../protection ../linkInconsistencyDetector ../hierarchyClient/nodeVelocityData .; do make -C $i clean; make -C $i; done

The external dependencies are:
protection - crypto for idsCommunciations. May not be needed, but seemed happier when I put it there.
linkInconsistencyDetector - Builds the logging library gmcluster uses. 
nodeVelocityData - a C++ class library which holds GPS information and does LET computation.

Known Issues
------------
External dependencies in the Makefile are a little annoying. 

The amount of actually used in nodeVelocityData is minimal. It could be folded
directly into gmcluster so the dependency disapears. 

liveinterim2 inter-group clusterheads are a little strange. li2 will only choose clusterheads
based on group when they are not a "group root," the highest clusterhead in a group. Once
they are group root, they choose clusterheads (and root) from outside the group. But when 
a group root is overthrown by a new group root, it continues to remain attached to the 
group-external neighbor. This means over time, that there is more than one node per group
that has a group-external clusterhead. This may be a bug or a feature depending on whether
you want to emphasize hierarchy stability or hierarchy group cohesiveness (floating mini, 
group-centered hierarchies). 

gmcluster and interim.gm configuration
--------------------------------------

Configuration file variables:
The operation of gmcluster can be altered by variables in the standard "live.conf" hierarchy
configuration file. Here they are straight out of the canonical live.conf file in 
./idsCommuncations:
#
# If using gmcluster, the number of positions to consider when
# computing velocity.
# (Note - not yet implemented for anything greater than 2)
gmcluster_positionHistory:30
#
# If using gmcluster, the level to log at, one of: trace, debug,
# info, notice, warn, critical, or fatal.
#
gmcluster_loglevel:debug
#
# If using gmcluster, the length of the time stamp used when logging,
# one of: long, short, or no.
gmcluster_logtimestamp:short
#
# If using gmcluster, the minimum number of seconds  a node must be thought to remain in
# contact with a node before it is considered part of the group.
#
gm_cluster_acceptableLinkExpirationTime:15
#
# If using gmcluster (or interimgm), how often to send a group ID heartbeat in seconds.
#
gmcluster_gidHeartbeatInterval:2
#
# If using gmcluster (or interimgm), how many heartbeat intervals to wait before 
# choosing a new (local) group ID. 
#
gmcluster_gidHeartbeatTimeoutFactor:3

