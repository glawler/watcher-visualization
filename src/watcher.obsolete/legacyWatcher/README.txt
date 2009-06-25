
#  Copyright (C) 2005  McAfee Inc.
#  All rights reserved.

This file documents the simulator and live network clustering daemon.

The Disclaimer:

This code has been tested, but not exhaustively.  It probably
has some memory leaks.

How to build:

This was developed on FreeBSD and Fedora Core 2 Linux.  Its
dependencies are not very exotic, it should be straight forward to
compile it under any modern Unix.
It will require X, GL, GLUT, libxml2, and a Sparta modified 
version of libidmef (included, in the file
 ../libidmef-0.7.3-beta-McAfee20050325.tar.gz) to build.  It
expects libxml2 and libidmef to be in /usr/[lib|include] or
/usr/local/[lib|include].  You may need to edit the Makefile if
X and GL are not in /usr/X11R6/[lib|include].

There are three lines in the make file to enable and disable the
graphics code.  watcher must be built with graphics enabled.  
The watcher binary may then be copied elsewhere, and everything
else rebuilt without graphics.

The makefile will generate several executables.  The executables are
named after the clustering algorithms they implement.  There are 
also infrastructure daemons, the "live" executables, which are the name
of the clustering algorithm with 'live' prepended.

Simulation executables take two arguments: the first is
the config file to use (see the files *.conf).  The second is the
duration to simulate, in seconds.

Infrastructure daemons take a single argument: the config file.
When using the script startIDSComs leave the '.conf' suffix
off the config file name; the script adds the suffix.

The binary names are:

Clustering	Simulation	Live
Algorithm	Executable	Executable
------------------------------------------------
interim		interim		liveinterim
interim2	interim2	liveinterim2
BFT		bft		livebft
graphcluster	graphcluster	livegraphcluster
amroute		amroute		liveamroute

The current (as of 2007.4.23) recommended algorithm is interim2.
The interim and cluster_gm algorithms will also work,
the rest of the algorithms will only function in the simulator,
so while the livenetwork binaries are built they will not work.
(they fail to populate some livenetwork data structures.)

watcher is the watcher, a diagnostics program for when the 
infrastructure daemon binaries are running in an environment like
TEAlab.  watcher takes a single argument, the config file.  It
operates by connecting to the infrastructure daemon's detector
API, and using some diagnostics calls.

Testing:

I run liveinterim2, demodetector and demoaggrigator overnight regularly
on a cluster of machines using a set of ipfw firewall rules to duplicate
the functionality of TEAlab (FreeBSD does not have ipchains).  
The test network does not implement mobility, but does implement packet
loss, where transmitted packets are dropped with a probability of 5%.  

In that environment, the only oddity is that every once in a while
the hierarchy does not converge, but right now I believe that is
a startup problem, where a TCP port is "stuck" and one or more daemons
or detectors do not startup correctly.

Running the simulation:

If you specify a duration, the simulator will not bother with any
graphics and will instead simply run full speed until the end of
the duration, emitting all the log information on stdout and stderr.

If you do not specify a duration, it will instead put up a window
with a diagram of the manet, and take one step each time you press
the space bar.  (Or 50 steps if you press j, or 1000 steps if you
press k, or one second if your press t.)  Pressing escape will exit.
Look in the file main.c for all the single-letter commands it has.

The diagram can be made larger by pressing w, or smaller by pressing
q.  It can be panned around using the arrow keys.  e/r, d/f, and
c/v rotate the diagram around the X,Y, and Z axes.  It doesn't make
much sense though.  (That is left over from the SGI demo code which
the UI code was originally based on.)

As you press space, or whatever, the diagram will be updated as
nodes move, and neighbors are found and lost.  Red edges are
link layer neighbors, blue edges are hierarchy relationships.
Note that they are tapered lines, the narrow end is the clusterhead.
The red edges are also actually tapered lines, its just that there
are two of them, one in each direction.   It can indicate
asymmetrical relationships.

The level of a node is indicated by the number of circles.  A level
0 is one circle, a level 1 is two circles, etc.  The root node is
drawn in green, normal connected nodes are drawn in red, and
disconnected nodes (IE: they do not have a path to a root node)
are drawn in green, since they are their own root node.  

All the node numbers are just the least significant octet of
the IP address.  The simulator does not typically use IP addresses,
so the nodes are numbered from 0 to n.  The simulator can use
IP addresses, if they are specified.

Running the live network code:

The live network binaries have the same names as the simulation
binaries, with the word live prepended.  They take a single argument,
the config file, and then generate log files on stdout and stderr.

In the livenetwork case, an additional config file may also be
needed.  Especially for running the watcher.  The config file may
refer to a locations file, which lists node addresses and XY
coordinates.  Look at the config file live.conf, and the locations
file live.locations for an example.

The hierarchy may be viewed using the watcher program.  The watcher
program takes the same arguments and config file as the live network
binaries.  It then uses the IP addresses listed in the locations 
file (described below) to connect to each of the live network binaries,
and grab debugging data.  Thus the IP addresses in the locations
file are important.

The debugging data is actually extra commands and messages on the 
TCP socket used by client programs, as opposed to a separate 
TCP socket.

The watcher also displays the current bandwidth a node is using.
Each node has a graph beside it.  There are 8 bars, four which
grow up drawn in red, and four which grow down drawn in green.
The red bars are transmission, and are (left to right) broadcast
packets, broadcast bytes, unicast packets, unicast bytes.  The
green bars are reception, and are the same as the transmission
bars.  Typically the broadcast received packets are the highest
numbers.

There are two example clients.  The first one is named testapi,
and can be used to transmit random messages (from stdin) unicast
through the infrastructure.  testapi is the older and less developed
one.  

The second example client is demodetector and demoaggregator.
They are written in the style we are expecting detectors and AACs
to use.  demodetector generates messages once every two seconds. 
Those messages are then accumulated and forwarded up the hierarchy
by demoaggregator, eventually to the ROOT node.

The script file runcoke will execute the infrastructure daemon,
demodetector, wait a specified time period, kill everything off,
then gather up all the log files.  It has hard-coded addresses
for a Sparta test bed however.  runcoke takes the name of the config
file as an argument.

There is also a script file named plotdemodetector, which will take
the detector stdout from the root node, and plot messages which were
not delivered.  To determine which node was the root node, look at
watcher's display, or grep all the detector stdout log files for
the string ROOT, the root node will have lines containing 
"position COORDINATOR_ROOT, active".  Give that filename as an argument
to plotdemodetector, and that will generate demodetector.ps, which
will have marks for each message which was not delivered. 

The Config files:

The config file is a text file containing name, value pairs.
The example config files should be pretty straight forward.
(and contains comments) The config files used for the report
experiment are named manet*.deg*.mob*.conf, where the *'s are
replaced with the size, degree, and mobility speed of the trial
respectively.

Nodes may be positioned randomly
(mobilityinitposition:random), or a location file may be
specified (mobilityinitposition:file, mobilityinitfile:filename)
which assigns specific coordinates.  The file twna.conf uses
the file twna.locations, and creates the topology used for
the OLSR detector experiments.  

The location file has two types of records in it: physical
locations "node n at x y" and IP addresses
"node n addr 1.2.3.4".  The IP addresses will be used in
the simulation of they are specified.  The watcher will
then use them to connect to all the infrastructure daemons.

The node numbers in the locations file MUST start at 0.  When
the simulator starts up, the node numbers are the index into
its array of nodes.  If the locations file does not contain
IP addresses, the nodes in the display will be labeled using
the node numbers.  Otherwise they will be labeled with the
least significant octet of the IP address.

The script file wnte2location will take a wnte config file
(the wireless network topology emulator, http://www.iponair.de/,
click on "Downloads") and convert it to a locations file.

The OLSR detector experiments used the output from the
simulator as the snapshots.  However, those snapshots were
created with a previous version which called the random number
generator slightly differently.  So the snapshots can not
be replicated with the current version.

There is a new config file called a posweights file.  It may
be used to assign which nodes may be the root node, or ban
nodes from being clusterheads.  It actually uses the same
data structures as the idsCommunications API.  The file
live.posweight is an example, and the file live.conf shows
how to specify the posweight file.

The Client API:

The client API is described in idsCommunications.h.  The
programs demodetector, demoaggregator, and testapi are
example clients.  

Most of the API is implemented, however not all of it is.
The missing parts are:
* All addressing modes except DIRECT, CHILDRENOF NODE_LOCAL,
and PARENTSOF NODE_LOCAL.
* Complete assigned hierarchy positions.  Root nodes may
be assigned, and nodes may be banned from coordinator 
positions, but the general case weighting does not work.

Some History:   (to explain some of the design oddities)

This software has had a remarkably long and varied life.  It
started out as a quick and dirty program to visualize what a
clustering algorithm we were working on "looked" like.  We wanted
a extremely simple drawing api, and GL was it.  Thus the SGI 
copyright statements in a couple of the source files, it started
out life as a modified GL demo.

That original algorithm eventually became the interim algorithm.  
Which despite its original purpose of just being a place holder
has also not gone away.

That first version was then massively modified, and turned into
a simple discrete event simulator.  Complete with an extremely
simple mobility model, and RF model.  The simulator was then
extended once or twice more, until it had the idea of protocol
modules.  (observe how flood.cpp, and data.cpp work.)

Those protocol modules worked so well that several more protocols
and clustering algorithms were implemented using them.  That 
functionality then supported the simulation work for IDMANET.

That caused the nomenclature to fail.  Originally the program
was named "manet", but now there were 4 versions implementing
different clustering algorithms, as well as several testing versions
(for example the NOP clustering algorithm, which doesn't actually
do anything, but is a handy skeleton to start from.).  So it
was dubbed "the simulator".  The files were not renamed, to 
retain the CVS log data.

The protocol modules achieved "critical mass", where clustering
algorithms, and detectors could be built on top of them extremely
easily.  Which made this software even more tempting to continue
to employ.

Next, the simulator code was replaced with a different set of
support functions which implemented the simulator API using an
actual network and UDP packets.  That extension was a bit bumpy,
a real network needs separate transmit and receive queues, but
the simulator API only really had an event list, as well as how
packets were passed between protocol modules by simply
transmitting them again with a propagation delay of 0.  (That
is why there are two separate packet type events.)

At the same time as the live network code was added, an API
was added to allow separate detector modules to connect to
the clustering algorithm and send and receive packets using
it.  A debugging message was piggy-backed onto that detector
API, and the watcher program was created.  It executed the 
global-view display code from the simulation using information
gathered from all the separate processes running the live
network code.  That allowed nearly as good a visualization
of the live network as was possible in the simulator.  (It
assumes that a single monitoring machine has connectivity
to all the manet node machines, thus it will not work in a
real manet.)

So now, a clustering algorithm could be implemented, tested
in the simulator, and then re-linked and run on a live network
with nearly zero effort.  Infortunately, the nomenclature
failed again, "the simulator" was now no longer a simulator.

So, if you see comments referring to "simulator land", that is
the API offered to clustering algorithms, and is not just
referring to simulation.


Simulation Implementation Details:

This is pretty much a from first principles discrete event
simulator.  There are two types of events: packets and timers.
The event queue is in the manet structure (described below),
is a priority queue with an unstable sort.  Thus, events at
the same time may or may not occur in the order in which they
were scheduled.

The simulation clock is an integer, and the variable is defined
to be milliseconds.

The base DES code is in des.[ch].  

The various functions of the simulated network stack are implemented
in "modules".  The modules do not map directly to the ISO-OSI
model, TCP/IP, or even to a typical network stack (the greatest
difference being that there is no strict application -> UDP -> IP
ordering).

The modules pass packets between themselves using the packet
type field, packet encapsulation, and stuffing de-capsulated packets
back into the event list.

For example, a node can just send a packet into the ether using
packetSend(),  but thats physical layer, the packet will be received,
Maybe.  If a node wishes to get retransmits and an ack, it may
send the packet using dataSend(), which is part of the data module,
and implements reliability.  dataSend() has the same arguments
as packetSend(), as well as some extra ones specifying how to
route and ack the packet.  (not all of which are implemented)

The data module will then take that packet, encapsulate it, and
send it to other instances of the data module using packetSend().
Those other data modules will then receive the packet, send
ACK packets back to the sending node, decapsulate the packet
and pass it back to whatever originally called dataSend(), by 
making it appear as if the encapsulated packet had just been
received.

One final detail is that the encapsulation is done by appending
headers and rewriting the type field in the outermost generic header
(So the appended header has an original-packet-type field), instead
of prepending headers.  Decapsulation then consists of copying the
original packet type field out of the appended header back into the
outermost header, and just forgetting about the appended bytes.  
The next module to get the packet may do the same thing again.

The nodes are arranged in an 800 unit square window.  The units
started out as pixels, and didn't have any relationship to
physical reality.  With the addition of radio propagation and
mobility, the units are meters.  (though light nanoseconds, and
millifurlongs were also considered.)

There is a function named manetInit(), defined in des.c.  It
creates a manet, and takes a config struct as an argument.  It
then calls Init functions found elsewhere.  For example
nodeMobilityInit(), found in mobility.[ch], and nodeInit(),
found in simulation.[ch] and banerjee.[ch].  

The mobility code is compartmentalized in mobility.c.  There
is a private pointer in the node structure which the mobility
code can use for its private data.  A node's mobility model is
defined by calling one of the mobility functions on it.  That
function then reschedules itself and mobility takes care of
itself.

The clustering algorithm is currently being compartmentalized by
having separate .o files for each algorithm.  So the interim
algorithm is in simulation.c (old naming scheme, needs to be
called interim_alg.c or something), and a start at the graphcluster
algorithm is in banerjee.c.  Then two binaries are generated,
one for each algorithm.

The simulation is executed by calling takeStep(manet) repeatedly.
That will advance the DES clock by one tick.  Where a tick is
one unique instant.  The instants are not necessarily equally
spaced.  Mobility will do events every 1000 ticks.  So if the
only thing happening is events then the instants will be 1000 ticks
apart.  But, if some packets are transmitted, a packet's
propagation time is 1 tick.  So the next instant after a packet
is transmitted will be one tick later.

takeStep is called in main(), which is in main.c.  main.c is
actually an SGI GL demo program, which handles all the GL 
initialization for visualizing the manet.

A node will get nodeInit() called on it by manetInit().  It can
then schedule events with timerSet(), or send and receive packets
with packetMalloc, packetDup, packetCopy, and packetSend.  

When a node receives packets, takestep() will call nodeGotPacket(),
with the node as an argument.

The manet will be drawn with manetDraw() (called by main())
after each call to takeStep().  There are a number of metrics
in a timerSet() callback which is attached to node 0, and breaks
the abstraction rules to get a global view.

There are a couple of little utility things, like a metric 
structure, described in metric.h, for generating means, standard
deviations, etc.  There is a set of neighbor list functions,
and recently seen packet list functions in node.c.  There
are a couple of graphics functions in graphics.[ch].  

Todo:
Redo nodeGotPacket as a callback type thing (like how timerSet 
works).
Integrate libxml2 and libidmef into the makefile.
Update the clustering algorithm.  (including honoring the 
hierarchy position weight data.)
Implement message disposition functions.
Update TODO list.

$Id: README.txt,v 1.12 2007/04/30 19:47:05 sherman Exp $
