#!/usr/bin/perl

####
# tsm.pl
# (Topology Scenario Manager)
#
#	Orchestrates the host's perception of the network topology by using
#	"iptables" to filter out incoming packets from all sources but those
#	deemed to be reachable.
#
#  Usage:
#	tsm [-v] <identity> <scenario file>
#
#	    <identity>
#		The scenario identity that the scenario manager is to 
#		adopt while interpreting the scenario file (see Notes).
#
#	    <scenario file>
#		The file defining the topology scenario (see Notes).
#
#	Options -
#	     -d Debug mode. When specified provides detailed messages beyond those
#			provided by verbose mode providing increased insight into the
#			execution of the script.  The form of debug output is:
#
#				tsm <filename>: <cmd> <phrase>
#
#			where <filename> is the source of the command, <cmd> is the
#			name of the command being executed, and <phrase> is the command
#			line phrase to be interpreted (<phrase> appears as specified in
#			the command line except extra white space has been removed).
#
#			Key system commands that are executed (i.e. iptables commands)
#			are also output during verbose mode. The form of this output is:
#
#				tsm SYSTEM: <command>
#
#			where <command> is the internally generated system command phrase
#			passed for execution.
#
#			Other significant points in the execution, such as entry into
#			special internal subroutines, are displayed as follows:
#
#				tsm DEBUG: <message>
#
#	     -f	Flush old rules. When specified, any rules that exist within
#			iptables from previous executions are flushed on start-up. 
#			By default, any previously existing rules will remain.
#			The default behavior is to keep any old rules (i.e. flush must
#			be explicitly stated to remove pre-existing rules). Only rules
#			within the tsm related chains are flushed, other rules
#			that may be in place within iptables will be undisturbed.
#
#	     -v	Verbose mode. When specified each command line read from the scenario file
#			will be displayed (at the time it is encountered within the
#			scenario file).
#
#	Usage Examples -
#
#		(*nix systems)       ./tsm.pl myhost scenario.tdf
#					   ./tsm.pl -v host5 flap.tdf
#
#		(Windows systems)    perl tsm.pl myhost scenario2.tdf
#
#  Purpose:
#
#	Manipulates the acceptance of IP packets based upon conditions set forth
#	in a scenario file. These conditions include who the packet can be from and
#	the time of day it may arrive. Since packets are filtered prior to IP processing,
#	the host perceives a network topology consisting only of those neighbors
#	whose packets are not being filtered. Packets can still be generated and sent
#	to any host (their replies, however, may not be heard). This asymmetry is useful
#	for emulating wireless environments over broadcast media (i.e. Ethernet) where
#	discovery protocols are active.
#
#  Example:
#
#	Given a physical network of eight hosts connected by an Ethernet:
#
#                         HostB   HostD    HostF    HostH
#                          |       |        |       |
#                        ---------------------------- 
#                        |       |      |        |
#                      HostA   HostC  HostE    HostG
#
#	With TSM running on each host, a scenario file can be defined to establish
#	the following network topology:
#
#                        HostB               HostF
#                        /   \               /   \
#                       /     \             /     \
#                   HostD     HostA --- HostH     HostG
#                       \     /             \     /
#                        \   /               \   /
#                        HostC               HostE
#
#	Subsequent scenario file definitions can alter this topology. For example,
#     the link between HostA and HostH could be dropped for 2 seconds every
#	5 minutes; or an alternate path between HostD and HostG could be established
#	every evening. In the above example topology, asymmetric link definition can 
#	establish uni-directional rings.
#
#  Limitations:
#
#	Operates on Linux-based hosts built and configured with iptables
#	(see http://www.netfilter.org/).
#
#  History:
#	0.0	28 Mar 2003		M. Little
#					Initial version of software.
#
#	1.0	09 Jul 2003		M. Little
#					Changed syntax (dropped -n designator) and syntax parsing.
#					Updated documentation, notably changed <name> to <identity> and
#						added descriptions for the new commands.
#					Expanded command set; adding EXIT, GOTO and USE (this involved
#						adding an inverse stack for file names and handles) as new
#						commands and allowing ACCEPT and DENY as stand alone commands.
#					Expanded "usage" response output to explain syntax elements.
#					Restructured ON command processing (notably added hash jump table).
#					Expanded relative command processing (via ON command) to
#						include all commands other than ON.
#
#	1.1	28 Jul 2003	ML	Expanded documentation note on iptables.
#
#	1.2	 5 Sep 2003	ML	Added the flush option (-f). The default behavior was flush old
#					rules every time; now the default behavior is to keep any old
#					rules and only flush them at start-up if explicitly requested.
#
#	1.3	11 Oct 2004	ML	Added the IPADDRESS name space to the DEFINE command.
#					Expanded the syntax of the WAIT command to accommodate
#						waiting for messages containing specific text. The added
#						syntax is: WAIT FOR <text> MESSAGE.
#					Expanded the command set: adding SEND.
#					Expanded the command syntax descriptions in the header comments,
#						hopefully all command directives are now documented.
#
#	1.4	11 Nov 2004	ML	Expanded command set: adding DROP.
#					Changed verbose mode operation such that it essentially just
#						echoes each line of the scenario file as it is encountered.
#					Added debug mode which performs essentially as the old verbose mode,
#						providing insight into the actual script operation.
#					Added notes on installation requirements and on running 
#						multiple simultaneous scripts.
#					Added iptables initialization warning message to help explain
#						console error message produced by iptables list command 
#						that is used to test for the existence of the tsm chains.
#
#	2.0	15 Nov 2004	ML	Old TDF files may not work with version 2.0, due to change to SEND
#						directive syntax (if your TDF file does not use SEND, it
#						should work just fine with v2.0).
#					Modified SEND directive syntax to generalize to sending either
#						text messages or named text from a name space. Also, you can
#						now specify a port number as well as address to send to.
#					Added the ATTRIBUTE name space. This impacts the SEND and DEFINE
#						directives.
#					Created the name space hash table to generalize name space access.
#						This impacts command processing for directives such as DEFINE
#						and SEND.
#					Changed the udp_messaging subroutine to handle a port attribute.
#					Added an address syntax checking subroutine, but this has yet to be
#						validated and incorporated by those directives using addresses.	
#  Notes:
#
#	Installation
#
#		Requires Perl. We have been using Perl 5.2, but it may work on older versions.
#
#		Requires netfiler/iptables to be installed in the kernel (this pretty much
#		limits the use of the script to Linux systems). The iptables "mac" module
#		needs to be loaded and the "random" patch applied (see www.netfilter.org).
#
#	Mulitple Running Instances
#
#		In general, you may run multiple simultaneous instances of this script on a 
#		single host. The TSM scripting operation will only interfere with each other 
#		when:
#
#			(1) You attempt to filter (i.e. accept and/or deny) the same
#			    mac source address in more than one script. Only the most
#			    recent filter specification will be used.
#
#			(2) You attempt to use WAIT FOR text MESSAGE in more than one
#			    script at the same time.
#
#			(3) You use DROP in more than one script, in which case the
#			    the most recent DROP issued will be in effect (until
#			    one of the scripts executes a new DROP directive).
#
#			(4) You begin a script specifying the flush option (-f), in which
#			    case any iptables actions (ACCEPT, DENY, DROP) defined by 
#			    currently running scripts will be lost.
#
#		We find it useful sometimes to augment the operation of a long running script with
#		short "one shot" specialized scripts that perform a specific action and terminate.
#		For example, you can create a long running script for a base scenario and augment
#		this operation with a speparate script to define the node drop rate. At various
#		points in the long running script you could alter the node drop rate with the
#		short script.
#
#	iptables
#
#		The iptables approach used in this program assumes a default
#		behavior (i.e. rule) of accept all incoming packets. This
#		program will explicitly insert and remove deny packet rules
#		where the basis of denial is a match on origination MAC address.
#
#		Utilizes the "mangle" table and the "prerouting" chain. Attaches
#		a named chain, called "tsmdeny", for the MAC address filter definitions.
#
#		Requires the iptables "mac" module to be loaded and available.
#
#		As if 1.4 also utilizes the "filter" table and the "input", "output",
#		and "forward" chains herein. Attaches new named chains "tsmdropin",
#		"tsmdropout", and "tsmdropthru" to the "input", "output", and "forward"
#		chains respectively.
#
#		As of 1.4 requires the iptables "random" patch to enable packet dropping.
#		This is part of the Patch-O-Matic base repository of netfilter extensions
#		as of Nov 2004.
#
#		See www.netfilter.org for iptables downloads, patches, and documentation.
#
#	Scenario Identity
#
#		The name specified as the scenario identity is utilized to determine which 
#		relative scenario directives are to be executed and which are to be ignored.
#		For example, if the command line was "tsm mikes_host test.tsf" the identity
#		adopted would be "mikes_host". If the scenario file "test.tsf" contained:
#
#			define macaddress testhost1 00:C0:4F:70:C7:A9
#			define macaddress testhost2 00:B2:05:2A:41:77
#			on mikes_host deny testhost1 inbound
#			on jeffs_host deny testhost1 inbound
#			wait for 20 seconds
#			on mikes_host accept testhost1 inbound
#			wait for 20 seconds
#			on jeffs_host accept testhost1 inbound
#			exit		
#
#		Each of the above command lines will be executed with the exception of the 
#		two lines denoting "on jeffs_host".	
#
#
#	Scenario File Format
#
#		The scenario file contains zero or more scenario directives, one per line.
#		Valid directives are as follows:
#
#			# <comment>
#
#				where
#					<comment> is any number of non-line terminating characters.
#
#				The # directive allows annotation of the scenario file with
#				user descriptive narrative and remarks. The character '#', which
#				denotes a comment directive, must be the first character encountered
#				on the line. IMPORTANT: note that comments are not allowed on the same 
#				line with other directives (i.e. "stop   # End of scenario" is NOT VALID), 
#				only on a line by themselves.
#
#
#			accept <mac_address> <flow>
#
#				where
#					<mac_address> is a contiguous string of non-white space
#						characters denoting one of the following:
#
#						    - A mac address in the standard 6-tuple hexadecimal 
#						      representation xx:xx:xx:xx:xx:xx
#
#						    - A previously defined pseudonym from the MACADDRESS 
#						      name space as defined by the DEFINE directive.
#
#					<flow> specifies the directional packet flow to consider.
#						Acceptable flows are:
#							
#							inbound
#
#				The ACCEPT directive adjusts the filtering performed to allow packets
#				received from (i.e. inbound to us) a specific mac layer address. 
#
#				Examples:
#					# Begin accepting packets originating from a specific interface
#					accept 02:9A:33:C0:4F:70 inbound
#					# Alternatively we can define and use a pseudonym
#					define macaddress romeo 02:9A:33:C0:4F:70
#					accept romeo inbound
#
#
#			define <name_space> <name> <value>
#
#				where
#					<name_space> is one of the following:
#						macaddress
#						ipaddress
#						attribute
#
#					<name> is a contiguous string of non-white space characters.
#
#					<value> is a contiguous string of non-white space characters.
#
#				The DEFINE directive provides the ability to establish pseudonyms for
#				specific and possibly complex values such as mac layer addresses. These
#				pseudonyms can be used in lieu of the specific representations for those
#				directives that require values from a given name space.
#
#				Examples:
#					# Define a pseudonym for the ethernet address
#					define macaddress testhost 00:C0:4F:70:C7:A9
#					# Use the pseudonym as the macaddress to accept
#					on myhost accept testhost inbound
#
#					# Define a pseudonym for a specific ip address
#					define ipaddress testhost 192.168.0.12
#					# Use the pseudonym where an ip address is expected
#					send "Rapid Start" to testhost
#
#					# Define an attribute for node location and alert level
#					define attribute location 12,25,0	# x,y,z coordinates
#					define alert_level orange
#
#
#			deny <mac_address> <flow>
#
#				where
#					<mac_address> is a contiguous string of non-white space
#						characters denoting one of the following:
#
#						    - A mac address in the standard 6-tuple hexadecimal 
#						      representation xx:xx:xx:xx:xx:xx
#
#						    - A previously defined pseudonym from the MACADDRESS 
#						      name space as defined by the DEFINE directive.
#
#					<flow> specifies the directional packet flow to consider.
#						Acceptable flows are:
#							
#							inbound
#
#				The DENY directive adjusts the filtering performed to deny packets
#				received from (i.e. inbound to us) a specific mac layer address. 
#
#				Examples:
#					# Explicitly deny packets originating from a specific interface
#					deny 02:9A:33:C0:4F:70 inbound
#					# Alternatively we can define and use a pseudonym
#					define macaddress romeo 02:9A:33:C0:4F:70
#					deny romeo inbound
#
#
#			position <Lat> <Long>
#		
#				where
#					<Lat> is a Latitude (or x location)
#
#					<Long> is a Longitude (or y location)
#
#				The POSITION directive issues a system command to the gpsUpdate program
#				with the Latitude and Longitude as arguments. 
#
#				Examples:
#					# Position tara at location -77.3825 -38.5432
#					on tara position -77.3825 -38.5432
#
#
#			drop <probability> <direction>
#
#				where
#
#					<probability> is an integer between 0 and 100 specifying the
#						chance that a packet passing into, out of, or through 
#						this node will be dropped.
#
#					<direction> is optional and is one of the following
#						keywords: IN, OUT, THROUGH, or ALL; indicating the
#						host packet flow processing point that should be 
#						effected by the packet loss. If unspecified, ALL is 
#						assumed.
#
#				The DROP directive provides a simple means to emulate link noise.
#				You can impact the packet flow at a host in three key places:
#				packets destined for this host (IN), packets outbound from this
#				host (OUT), and packets routed through this host (THROUGH). 
#
#
#			exit
#
#				The EXIT directive indicates to leave a scenario file, but this does
#				not necessarily terminate all scenario file interpretation if leaving 
#				will return to a previous file (by contrast, the 'stop' directive is an 
#				absolute termination). Essentially, this directive will cause the 
#				subsequent command lines of the file to be ignored.
#
#
#			goto <filename>
#
#				where
#					<filename> is the name of a text file with scenario commands.
#
#				The GOTO directive specifies a new file to use for scenario command
#				input. Processing of the current file will terminate.
#
#				Note: If you want processing of the current file to continue once
#					the commands in the referenced file are finished, specify
#					the "use" directive instead of "goto".
#
#					Looping a scenario can be accomplished by redirecting a file
#					to itself. Circular references can lead to non-terminating
#					scenarios.
#
#
#			on <id_name> <command>
#
#				where
#					<id_name> is a non-white space character string denoting a scenario manager
#						identity (matched against the <identity> command line attribute).
#
#					<command> is any valid scenario directive other than 'on'.
#
#				The ON directive specifies the the command indicated should only be
#				executed by the the topology manager associated with <id_name>. 
#
#				Example(s):
#
#					on myhost deny testhost inbound
#					on myhost accept testhost inbound
#					on alpha wait for 3 seconds
#					on delta goto delta.tdf
#
#
#			send <text> message to <ip_address>
#			send <text> message to <ip_address>:<port>
#			send <name> <namespace> to <ip_address>:<port>
#
#				where
#					<text> is a contiguous string of non-whitespace	characters.
#
#					<name> is a contiguous string of non-whitespace characters denoting
#						a defined name in the specified name space (see the DEFINE directive).
#
#					<namespace> is a contiguous string of non-whitespace characters denoting
#						a name space as specified by the DEFINE directive.
#
#					<ip_address> is a contiguous string of non-white space
#						characters denoting one of the following:
#
#						    - An ip address in standard dotted notation (xxx.xxx.xxx.xxx)
#
#						    - A previously defined pseudonym from the IPADDRESS 
#						      name space as defined by the DEFINE directive.
#
#					<port> the UDP port value to use. This is optional when sending a text message.
#						If not specified, an internal default value is used that will correspond
#						to the port value used by the WAIT FOR <text> MESSAGE directive. The port
#						must be specified if sending a value from a name space.
#
#				The SEND directive constructs a text message and attempts a UDP-based transfer to the
#				specified IP address (using the UDP port specified, when provided). The text message
#				constructed will contain either the text provided (when MESSAGE is indicated) or will
#				be the string associated with the name from the name space specified. No guarantees are 
#				made as to acutal data transfer or receipt as such depends on the state and capabilities
#				of the network protocols employed and if a process at the specified address is listening
#				at the time the message is sent.
#
#				Example(s):
#					send go message to 192.168.0.12
#
#					define ipaddress romeo 192.168.0.12
#					send restart message to romeo
#
#					define ipaddress alpha 192.168.0.11
#					define attribute level 5
#					send level attribute to alpha
#					send romeo ipaddress to alpha:2025
#
#
#			stop
#
#				The STOP directive indicates to terminate any and all scenario file
#				interpretation.
#
#
#			use <filename>
#
#				where
#					<filename> is the name of a text file with scenario commands.
#
#				The USE directive specifies that scenario command processing is to
#				be temporarily redirected to incorporated commands from the specified 
#				file. Once those commands are completed, processing will continue
#				with the current file.
#
#				Note: If you want processing to be permenantly redirected to the
#					specified file, specify the "goto" directive instead.
#
#					Circular references can lead to recursive opening of files,
#					which in turn may lead to an excessive number of outstanding 
#					filehandles. The goto directive does not leave behind an open
#					filehandle, while the use directive does.
#
#
#			wait for <number> seconds
#			wait for <text> message
#
#				where
#					<number> is an unsigned integer.
#					<text> is a contiguous string of non-whitespace	characters.
#
#				The WAIT directive suspends the line-by-line reading of the scenario 
#				file until the specified event occurs. When specifying the message 
#				event form of this directive, a UDP-based message containing the exact
#				same text string (case insensitive) must be received before processing 
#				will continue.
#
#				Example(s):
#					# Pause for a short time
#					wait for 20 seconds
#					# Wait for a synchronization message
#					wait for restart message
#
#
#		The following commands and syntax are not implemented but are shown for future consideration:
#			
#			# Wait for an absolute time
#			wait until 15:00
#
#			# Wait for the relative occurrence of an absolute time epoch
#			wait epoch minute/halfhour/hour/quarterhour/tenminute/fiveminute
#
#			# Allow comments to follow directives on the same line
#			wait for 4 seconds   # A comment line on a command line
#
#  Copyright Statment:
#	Copyright Telcordia Technologies, Inc. 2003-2004.
#	Permission to use under license agreement only.
####

use File::Basename;
use Socket;

##
# Initialization
##

# Hash jump table for scenario file command set
# (maps command names to supporting routines)
my %CommandSet = (
	define		=> \&cmd_define,
	stop		=> sub { last },
	on		=> \&cmd_on,
	wait		=> \&cmd_wait,
	accept		=> \&cmd_accept,
	exit		=> \&cmd_exit,
	deny		=> \&cmd_deny,
	drop		=> \&cmd_drop,
	goto		=> \&cmd_goto,
	send		=> \&cmd_send,
	use		=> \&cmd_use,
        shellout        => \&cmd_shellout,
	position	=> \&cmd_position,
        echoout 	=> \&cmd_echoout,
        echotimetologfile  => \&cmd_echotimetologfile,
	);

# Hash jump table for identity relative commands ( on <id_name> <command>)
my %OnCommandSet = (
	define		=> \&cmd_define,
	stop		=> sub { last },
	wait		=> \&cmd_wait,
	accept		=> \&cmd_accept,
	exit		=> \&cmd_exit,
	deny		=> \&cmd_deny,
	drop		=> \&cmd_drop,
	goto		=> \&cmd_goto,
	send		=> \&cmd_send,
	use		=> \&cmd_use,
        shellout        => \&cmd_shellout,
	position	=> \&cmd_position,
        echoout 	=> \&cmd_echoout,
        echotimetologfile  => \&cmd_echotimetologfile,
	);

# Name Space Hash Table(s) 
# (for user defined name-value mappings via 'define' directive)
my %Attributes =();
my %IpAddresses =();
my %MacAddresses =();

# Hash table of name spaces
my %NameSpaces = (
	"ipaddress"	=> \%IpAddresses,
	"macaddress"	=> \%MacAddresses,
	"attribute"	=> \%Attributes,
	);

# (Inverse) Stacks to track command flow through multiple scenario files
my (@filehandles, @filenames);

# UDP messaging
$default_udp_port	=	7099;		# Default port number to use for 
						# sending and receiving text messages
$udp_socket;

##
# Argument Defaults
##
$debug = 1;		# Debug mode is off by default.
$verbose = 0;		# Verbose mode is off by default.
$flush = 0;		# By default, do not flush any pre-existing rules.

##
# Check argument syntax
##

ARG: foreach (@ARGV) {
	# Check for option flags
	if (/^-/) {
		# Create list of flags (ignores empty option lists)
		@flags = split //;
		shift @flags;
		# Check for valid option flags
		OPT: foreach (@flags) {
			if (/^d/) { $debug = 1; next OPT; }
			if (/^v/) { $verbose = 1; next OPT; }
			if (/^f/) { $flush = 1; next OPT; }
			#if (/^m/) { $mobilityinterval= $_; print "MOB rate: $mobilityinterval";next OPT}
			&usage;
			}
		next ARG;
		}
	# If not an option flag, must be an argument
	# Arguments are defined by order of appearance (and complains if too many!)
	unless ($adopted_name) { $adopted_name = $_; next ARG }
	unless ($scenario_filenames[0]) { $scenario_filenames[0] = $_; next ARG }
	$mobilityint= $_; next ARG;
	&usage;
	}
unless ($scenario_filenames[0]) { &usage }
print "MOB int: $mobilityint";
##
# Establish access to scenario file 
##
unless (-T ($scenario_filenames[0]) ) {
	die "Scenario file \'$scenario_filenames[0]\' is not a plain file (or does\'t exist)";
	}
my($scenario_filehandle);
open($scenario_filehandles[0],$scenario_filenames[0])
	or die "Cannot open Scenario file $scenario_filenames[0]";

# $filehandles[0] = $scenario_filehandle;
# $filenames[0] = $scenario_filenames[0];

##
# Establish chains for our own use within iptables
##
use constant CHAIN => "tsmdeny";
        $iptcmd = `iptables -t mangle -L tsmdeny`; 
            if ($iptcmd =~ m/tsmdeny/) {
            print "YES tsmdeny chain already exists\n"}
            else {
		print "NO tsmdeny chain doesn't exist SO create\n";
	        system ("iptables -t mangle -N " . CHAIN);
            }
        $iptcmd1 = `iptables -t mangle -L PREROUTING | grep tsmdeny`;
            if ($iptcmd1 =~ m/tsmdeny/) {
            print "YES already insert into PREROUTING\n"}
            else {
		print "NO tsmdeny in PREROUTING yet SO inserting...\n";
	        system ("iptables -t mangle -I PREROUTING -j " . CHAIN);
            }
	if ($flush) {
		system ("iptables -t mangle -F " . CHAIN);
	}

use constant DROP_IN_CHAIN => "tsmdropin";
use constant DROP_OUT_CHAIN => "tsmdropout";
use constant DROP_THRU_CHAIN => "tsmdropthru";
        $iptcmd = `iptables -t filter -L tsmdropin`; 
            if ($iptcmd =~ m/tsmdropin/) {
            print "YES tsmdropin chain already exists\n"}
            else {
		print "NO tsmdropin chain doesn't exist SO create\n";
	        system ("iptables -t filter -N " . DROP_IN_CHAIN);
                system ("iptables -t filter -I INPUT -p ip -s 127.0.0.1 -d 127.0.0.1 -j ACCEPT");
            }
        $iptcmd1 =  `iptables -t filter -L INPUT | grep tsmdropin`;
            if ($iptcmd1 =~ m/tsmdropin/) {
            print "YES already insert into INPUT\n"}
            else {
		print "NO tsmdropin in INPUT yet SO inserting...\n";
	        system ("iptables -t filter -I INPUT -j " . DROP_IN_CHAIN);
                system ("iptables -t filter -I INPUT -p ip -s 127.0.0.1 -d 127.0.0.1 -j ACCEPT");
            }
        $iptcmd  = `iptables -t filter -L tsmdropout`; 
            if ($iptcmd =~ m/tsmdropout/) {
            print "YES tsmdropout chain already exists\n"}
            else {
		print "NO tsmdropout chain doesn't exist SO create\n";
	        system ("iptables -t filter -N " . DROP_OUT_CHAIN);
                system ("iptables -t filter -I OUTPUT -p ip -s 127.0.0.1 -d 127.0.0.1 -j ACCEPT");
            }
        $iptcmd1 = `iptables -t filter -L OUTPUT | grep tsmdropout`;
            if ($iptcmd1 =~ m/tsmdropout/) {
            print "YES already insert into OUTPUT\n"}
            else {
		print "NO tsmdropin in OUTPUT yet SO inserting...\n";
	        system ("iptables -t filter -I OUTPUT -j " . DROP_OUT_CHAIN);
                system ("iptables -t filter -I OUTPUT -p ip -s 127.0.0.1 -d 127.0.0.1 -j ACCEPT");
            }
        $iptcmd = `iptables -t filter -L tsmdropthru`; 
            if ($iptcmd =~ m/tsmdropthru/) {
            print "YES tsmdropthru chain already exists\n"}
            else {
		print "NO tsmdropthru chain doesn't exist SO create\n";
	        system ("iptables -t filter -N " . DROP_THRU_CHAIN);
            }
        $iptcmd1 = `iptables -t filter -L FORWARD | grep tsmdropthru`;
            if ($iptcmd1 =~ m/tsmdropthru/) {
            print "YES already insert into FORWARD\n"}
            else {
		print "NO tsmdropthru in FORWARD yet SO inserting...\n";
	        system ("iptables -t filter -I FORWARD -j " . DROP_THRU_CHAIN);
            }
	# Chain Pre-exists
	if ($flush) {
		system ("iptables -t filter -F " . DROP_IN_CHAIN);
		system ("iptables -t filter -F " . DROP_OUT_CHAIN);
		system ("iptables -t filter -F " . DROP_THRU_CHAIN);
		#system ("iptables -t filter -X " . DROP_IN_CHAIN);
		#system ("iptables -t filter -X " . DROP_OUT_CHAIN);
		#system ("iptables -t filter -X " . DROP_THRU_CHAIN);
	}
##
# Parse and Execute Commands from Referenced Scenario files
##

while (defined $scenario_filenames[0] && defined $scenario_filehandles[0]) {
   while(&Read_scenario($scenario_filehandles[0])) {
	($keyword,@phrase) = split(" ",$_);	# Parse line into words (leading word is command keyword)
	# Check for being a valid command keyword
	if ($CommandSet{lc $keyword}) { $CommandSet{lc $keyword}->(@phrase) }
	else { warn "Unknown command keyword encountered: '$keyword'\n"; }
   }
   # After all file commands are processed, pop back to previous file (if any)
   shift @scenario_filenames;
   shift @scenario_filehandles;
}

##
# All done
##
print "End of TSM program";
exit; ### End of program ###

############################
#  Supporting Subroutines  #
############################

#############################################################
# usage()
#
#	Print statement showing acceptable command line syntax.
#
   #-m mobility interval to change the topologies
sub usage {
	print <<USAGE;
Usage: tsm [-dfv] <identity> <scenario file> <mobility rate>
 where
   -d display debug messages (optional)
   -f flush any pre-existing tsm rules (optional)
   -v enables verbose mode (optional)
   <identity> is the scenario identity to adopt
   <scenario file> defines the scenario
   <mobility interval> defines the rate
USAGE
	exit;
}
### End of usage ###


#############################################################
# Read_scenario()
#
#	Acquire next meaningful input line from a scenario
#	description file.
#
#	Usage:
#		Read_scenario <file handle>
#
# 	We will skip all comments and empty (zero characters or only white space) lines. 
#	All other lines are translated to lower case and have their trailing new line
#	charater removed before being returned otherwise as found.
#
sub Read_scenario {
my($fh) = @_;
while (<$fh>) {
	if ($verbose) {print "tsm>$_";}
	#if ($debug) {print "<$_>\n";}
	next if (/^#/);			# Skip comment lines
	next if (m/^\s*\n/);		# Empty lines are ignored
	chomp;				# Remove any trailing new line character
	tr/A-Z/a-z/;			# Translate any upper case to lower case
	last;
	}
return $_;
}  ### End of Read_scenario ###

#############################################################
# cmd_define()
#
#	Provide semantics and pragmatics of the "define" command.
#
#	Acceptable syntax:
#
#		define <name_space> <name> <value>
#
#		where
#		  <name_space> must be one of the following:
#
#			attribute
#			ipaddress
#			macaddress
#
#		  <name> is any non-white space string of characters.
#
#		  <value> depends upon the name space:
#
#			attribute
#				<value> specifies an abritray string on non-white
#				space characters (Note: qouting the string to
#				include white space characters will *not* work!).
#
#			ipaddress
#				<value> specifies an ip protocol address in 
#				dotted octet form, as in "192.168.0.12". 
#				However, this syntax is only loosely checked.
#
#			macaddress 
#				<value> specifies a mac layer address string 
#				of six pairs of hexadecimal characters 
#				separated by colons, as in "00:C0:4F:70:C7:A9". 
#				However, this syntax is only loosely checked.
#
#	Examples:
#
#		define macaddress testhost 00:C0:4F:70:C7:A9
#		define ipaddress testhost 192.168.0.12
#		define attribute testhost lab2_pc5
#
#		The above examples define the name "teshost" in three distinct
#		name spaces.
#
#		
# 	
#
sub cmd_define {

if ($debug) {print "tsm <$scenario_filenames[0]>: define @_\n";}

unless ($#_ == 2) {
	warn "Invalid number of directive attributes: DEFINE '@_'\n";
	return;
	}
my($space, $name, $value) = @_;

unless (defined $NameSpaces{$space}) {
	warn "Requested name space is not defined: DEFINE '@_'\n";
	return;
}

$NameSpaces{$space}->{$name} = $value;

}
### End of cmd_define ###


#############################################################
# cmd_on()
#
#	Provide semantics and pragmatics of the "on" command.
#
#	Acceptable syntax:
#
#		on <identity> <command>
#
#		where
#			<identity> is a string specifying the identity to match
#			<command> is a command to execute
#
#	Summary:
#
#		Once simple syntax checking is accomplished, the identity of the
#		command is checked against the adopted identity of the scenario
#		manager. If they match, the command is passed to the appropriate
#		command routine for processing.
#
#	Notes:
#		Valid commands are defined in the hash table %OnCommandSet.
#		These commands are not necessarily those available in the
#		general scenario file command set. Currently, commands allowed are:
#
#			accept 
#			deny
#			position
#
#		on <name> deny <name> inbound
#		on <name> accept <name> inbound
#		on <name> position <Lat> <Long>
# 	
#
sub cmd_on {

if ($debug) {print "tsm <$scenario_filenames[0]>: on @_\n";}

# Basic syntax check
unless ($#_ >= 1) {warn "Syntax error: Missing <identity> and/or <command>: 'ON @_'\n"; return;}

# Grab command elements
my($on_identity, $on_cmd, @on_cmd_phrase) = @_;

# Check if this directive is for us
if ($debug) {print "tsm DEBUG: Our adopted name is [$adopted_name] \n";}
unless ($adopted_name eq $on_identity) { return; }

# Execute allowable commands - complain if command keyword not in allowable command set
if ($OnCommandSet{lc $on_cmd}) { $OnCommandSet{lc $on_cmd}->(@on_cmd_phrase) }
else { warn "Invalid command directive: 'ON @_'\n"; return;}

}
### End of cmd_on ###

#############################################################
# cmd_accept()
#
#	Provide semantics and pragmatics of the "accept" command.
#
#	Acceptable syntax:
#
#		accept <name> inbound
# 	
#
sub cmd_accept {

if ($debug) {print "tsm <$scenario_filenames[0]>: accept @_\n";}

# Basic syntax check
unless ($#_ == 1) {warn "Syntax error: wrong number of keywords: 'ACCEPT @_'\n"; return;}

my($mac_name, $direction) = @_;

if ($direction ne "inbound") {warn "Invalid directional attribute: 'ACCEPT @_'\n"; return;}

&inbound_packet_filter( "accept", $mac_name );

}
### End of cmd_accept ###

# cmd_position()
#
#       Provide semantics and pragmatics of the "position" command.
#
#       Acceptable syntax:
#
#               position <Lat (x)> <Long (y)>
#
#
sub cmd_position {
                                                                                                                             
if ($debug) {print "tsm <$scenario_filenames[0]>: position @_\n";}
                                                                                                                             
# Basic syntax check
unless ($#_ == 1) {warn "Syntax error: wrong number of keywords: 'POSITION @_'\n"; return;}
                                                                                                                             
my($latitude, $longitude) = @_;

@position_cmd = ("/usr/src/twna/tealab/tsm-v4/gpsUpdate", $latitude, $longitude);
        if ($debug) {print "tsm SYSTEM: @position_cmd\n";}
        system( @position_cmd ) == 0
                or warn "position command failed: $? \n";
}
### End of cmd_position ###

#############################################################
# cmd_exit()
#
#	Provide semantics and pragmatics of the "exit" command.
#
#	Acceptable syntax:
#
#		exit
# 	
#
sub cmd_exit {

if ($debug) {print "tsm <$scenario_filenames[0]>: exit @_\n";}

# Basic syntax check
unless ($#_ == -1) {warn "Syntax error: wrong number of keywords: 'EXIT @_'\n"; return;}

# Ignore rest of file input
while (&Read_scenario($scenario_filehandles[0])) {
	if ($debug) {print "tsm DEBUG: Skipping \'$_\'\n";}
	next;
	}

}
### End of cmd_exit ###

#############################################################
# cmd_drop()
#
#	Provide semantics and pragmatics of the "drop" command.
#
#	Acceptable syntax:
#
#		drop <probability> <direction>
#
#		where
#
#			<probability> is the percent chance between
#				0 and 100 that any packet will be droppped.
#
#			<direction> is the packet flow direction to be
#				impacted by the drop action. This can be:
#
#					IN - where only packets desinted for this 
#						host are impacted.
#					OUT - where only packets originated by 
#						this host are impacted.
#					THROUGH - where only packets being forwarded
#						through this host to other desintations
#						are impacted.
#					ALL - where all packets, either orginating at, 
#						destined to, or forwarded by the host 
#						are impacted.
#
#				If unspecified, ALL is assumed.
# 	
#
sub cmd_drop {

if ($debug) {print "tsm <$scenario_filenames[0]>: drop @_\n";}

# Basic syntax check
my($probability, $direction, @chains);
if ($#_ == 0) {

	($probability, $direction) = (@_, "all");

} elsif ($#_ == 1) {

	($probability, $direction) = @_;
	
} else {warn "Syntax error: wrong number of keywords: 'DROP @_'\n"; return;}

for ($probability) {
	/\D/ and do {warn "Must use integer value for drop probability: 'DROP @_'\n"; return;}
}
$probability = int $probability;

unless ( ($probability >= 0) and ($probability <= 100) )
    {warn "Invalid probability value: 'DROP @_'\n"; return;}

if ($direction eq "in")		{@chains = (DROP_IN_CHAIN);
} elsif ($direction eq "out") 	{@chains = (DROP_OUT_CHAIN);
} elsif ($direction eq "through")  {@chains = (DROP_THRU_CHAIN);
} elsif ($direction eq "all") 	{@chains = (DROP_IN_CHAIN, DROP_THRU_CHAIN, DROP_OUT_CHAIN);
} else {warn "Invalid directional attribute: 'DROP @_'\n"; return;}

# Set up chain rule
#	Zero and 100 are special case, creating unique rules
my(@rule);
$interface_name = "eth0"; #this is the wireless lan
if ($probability == 0) {
	@rule = ("-j", "RETURN");
} elsif ($probability == 100) {
	@rule = ("-j", "DROP");
} else {
        if ($direction eq "in")		
            {$interface_opt = "-i";}
        elsif ($direction eq "out")		
            {$interface_opt = "-o";}
	@rule = ("-m", "random", "--average", $probability, "-j", "DROP", $interface_opt, $interface_name);
}

# Replace first rule in affected chains with the new rule
my($chain, @iptable_cmd);
foreach $chain (@chains) {
	#@iptable_cmd = ("iptables", "-t", "filter", "-R", $chain, "1", "-p", "ip", @rule);
        if ($chain eq DROP_OUT_CHAIN){
            $iptcmd11 =  `iptables -t filter -L tsmdropout | grep random`;}
        elsif ($chain eq DROP_IN_CHAIN){
            $iptcmd11 =  `iptables -t filter -L tsmdropin | grep random`;}
        if ($iptcmd11 =~ m/random/) {
	@iptable_cmd = ("iptables", "-t", "filter", "-R", $chain, "1", "-p", "ip", @rule);}
	else{
	@iptable_cmd = ("iptables", "-t", "filter", "-I", $chain, "-p", "ip", @rule);}
	#if ($debug) {print "tsm SYSTEM: @iptable_cmd\n";}
	print "tsm SYSTEM: @iptable_cmd\n";
	system( @iptable_cmd ) == 0
		or warn "iptables command failed: $? \n";
}

}
### End of cmd_drop ###


#############################################################
# cmd_deny()
#
#	Provide semantics and pragmatics of the "deny" command.
#
#	Acceptable syntax:
#
#		deny <name> inbound
# 	
#
sub cmd_deny {

if ($debug) {print "tsm <$scenario_filenames[0]>: deny @_\n";}

# Basic syntax check
unless ($#_ == 1) {warn "Syntax error: wrong number of keywords: 'DENY @_'\n"; return;}

my($mac_name, $direction) = @_;

if ($direction ne "inbound") {warn "Invalid directional attribute: 'DENY @_'\n"; return;}

&inbound_packet_filter( "deny", $mac_name );

}
### End of cmd_deny ###

#############################################################
# inbound_packet_filter()
#
#	Provides supporting pragmatics for the "accept" and "deny"  commands.
#
#	Utilizes iptables commands to establish, or remove, packet filters.
#	Packet filtering is based upon mac-layer addresses.
#
#	Acceptable syntax:
#
#		inbound_packet_filter <action> <address>
#
#		where
#			<action> is either "accept" or "deny".
#			<address> is a name within the MacAddresses hash table
#			    or a mac layer address string of six pairs of
#			    hexadecimal characters separated by colons,
#			    as in "00:C0:4F:70:C7:A9". However, this syntax 
#			    is not checked.#		
#	Notes:
#		This routine assumes an iptables default behavior of accept all.
#		E.g., it installs filters only to deny inbound packets, and removes
#		them to allow inbound packets. If default behavior is to deny all,
#		this routine will need to be modified accordingly (e.g. install filter
#		to accept packets, remove to deny).
#
#
sub inbound_packet_filter {

if ($debug) {print "tsm DEBUG: inbound_packet_filter @_\n";}

my($action, $address) = @_;

my($cmd_action, $cmd_adr);

# Set command for accept or deny
# NOTE: This approach assumes iptables default of accept all behavior.
if ($action eq "accept") {
	$cmd_action = "-D";
} else { $cmd_action = "-A"; }

# Use macaddress pseudonym if defined, otherwise use what is given
if (defined $MacAddresses{$address}) {
	$cmd_adr = $MacAddresses{$address};
} else { $cmd_adr = $address; }

# Establish iptable command
my (@iptable_cmd) = ("iptables", "-t", "mangle", $cmd_action, CHAIN, 
	 		  "-m", "mac", "--mac-source", $cmd_adr,
	 		  "-j", "DROP");

# Execute iptables command
if ($debug) {print "tsm SYSTEM: @iptable_cmd\n";}
system( @iptable_cmd ) == 0
  or warn "iptables command failed: $? \n";

}
### End of inbound_packet_filter ###


#############################################################
# cmd_goto()
#
#	Provide semantics and pragmatics of the "goto" command.
#
#	Acceptable syntax:
#
#		goto <filename>
# 	
#
sub cmd_goto {

if ($debug) {print "tsm <$scenario_filenames[0]>: goto @_\n";}

# Basic syntax check
unless ($#_ == 0) {warn "Syntax error: wrong number of keywords: 'GOTO @_'\n"; return;}
my($newfile) = @_;

unless (-T ($newfile) ) {
	warn "Referenced scenario file \'$newfile\' is not a plain file (or does\'t exist)\n";
	warn "Execution will continue with current file \'$scenario_filenames[0]\'\n";
	return;
	}

#Establish references to the new scenario file
my($newfilehandle);
open($newfilehandle, $newfile)
	or do {
		warn "Cannot open Scenario file $newfile \n";
		warn "Execution will continue with current file \'$scenario_filenames[0]\'\n";
		return;
		};

#Complete by replacing current scenario with new scenario (file names and handles)
$scenario_filenames[0] = $newfile;
$scenario_filehandles[0] = $newfilehandle;

}
### End of cmd_goto ###

#############################################################
# cmd_send()
#
#	Provide semantics and pragmatics of the "use" command.
#
#	Acceptable syntax:
#
#		send <text> message to <ip_address>
#		send <name> <namespace> to <ip_address>
#
#		where
#			<text> is a contiguous string of non-whitespace	characters.
#
#			<ip_address> is a contiguous string of non-white space
#				characters denoting one of the following:
#
#				    - An ip address in standard dotted notation 
#				      (xxx.xxx.xxx.xxx)
#
#				    - An ip address and UDP port number to use in
#				      the form xxx.xxx.xxx.xxx:nnnn where nnnn is the
#				      port number
#
#				    - A previously defined pseudonym from the IPADDRESS 
#				      name space as defined by the DEFINE directive.
#
#
#
sub cmd_send {

if ($debug) {print "tsm <$scenario_filenames[0]>: send @_\n";}

unless ($#_ == 3) {
	warn "Invalid number of directive attributes: 'SEND @phrase'\n";
	return;
	}

my($name, $space, $to, $address) = @_;

unless ($to eq "to") {
	warn "Invalid send operative: SEND '@phrase'\n";
	return;
	}

# Extract message from namespace if necessary
my $message;
if ($space eq "message") { $message = $name }
else {
	unless (defined $NameSpaces{$space}) {
		warn "Requested name space is not defined: DEFINE '@_'\n";
		return;
	}
	$message = $NameSpaces{$space}->{$name};
}

# Grab port number (if any, use default if need be)
my $port;
($address, $port) = split /:/, $address;
if ($port =~ /\D/) {warn "Invalid port number: SEND '@phrase'\n"; return}
unless (defined $port) {$port = $default_udp_port};

# Use ipaddress pseudonym if defined, otherwise use what is given
if (defined $IpAddresses{$address}) {
	$ip_address = $IpAddresses{$address};
} else { $ip_address = $address; }

&udp_messaging( "send", $message, $ip_address, $port);

}
### End of cmd_send ###


#############################################################
# cmd_use()
#
#	Provide semantics and pragmatics of the "use" command.
#
#	Acceptable syntax:
#
#		use <filename>
# 	
#
sub cmd_use {

if ($debug) {print "tsm <$scenario_filenames[0]>: use @_\n";}

# Basic syntax check
unless ($#_ == 0) {warn "Syntax error: wrong number of keywords: 'USE @_'\n"; return;}
my($newfile) = @_;

unless (-T ($newfile) ) {
	warn "Referenced scenario file \'$newfile\' is not a plain file (or does\'t exist)\n";
	warn "Execution will continue with current file \'$scenario_filenames[0]\'\n";
	return;
	}

#Establish access to the new scenario file
my($newfilehandle);
open($newfilehandle, $newfile)
	or do {
		warn "Cannot open Scenario file $newfile \n";
		warn "Execution will continue with current file \'$scenario_filenames[0]\'\n";
		return;
		};
# Complete by saving the old name and handle (on inverse stack) and establishing the new
unshift (@scenario_filenames, $newfile);
unshift (@scenario_filehandles, $newfilehandle);

}
### End of cmd_use ###


#############################################################
#
# cmd_wait()
#
#	Provide semantics and pragmatics of the "wait" command.
#
#	Acceptable syntax:
#
#		wait for <integer value> seconds
#		wait for <text> message
# 	
#
sub cmd_wait {

if ($debug) {print "tsm <$scenario_filenames[0]>: wait @_\n";}

# Syntax check
if ($#_ != 2) {warn "Number of command attributes invalid: 'WAIT @phrase'\n"; return;}

my($op, $value, $event) = @_;

if ($op ne "for") {warn "Invalid wait operative: 'WAIT @_'\n"; return;}
#the syntax can just be wait for n interval
#the interval specified is in mins
if ($event eq "interval") {
    print "wait for t int is true";
    if ($mobilityint >0) {sleep $mobilityint*60; }
} 

# Process event type
elsif ($event eq "seconds") {

	if ($value!~/^[0-9]+$/) {
		warn "Invalid time value '$value'\n";
	} else {
		if ($debug) {print "tsm DEBUG: Sleeping for $value seconds\n";}
		sleep $value;
		if ($debug) {print "tsm DEBUG: Woke up\n";}
	}

}
elsif ($event eq "message") {
	# Dummy address used for receive (i.e. we don't care where the message comes from)
	&udp_messaging( "receive", $value, 0, $default_udp_port);

} else {warn "Invalid wait unit: 'WAIT @_'\n"; return;}

}
### End of cmd_wait ###

#############################################################
# udp_messaging()
#
#	Provides supporting messaging pragmatics to the "send" and "wait"
#	commands.
#
#	Utilizes iptables commands to establish, or remove, packet filters.
#	Packet filtering is based upon mac-layer addresses.
#
#	Acceptable syntax:
#
#		udp_messaging <action> <text> <address> <port>
#
#		where
#			<action> is either "send" or "receive".
#			<text> is a contiguous string of non-whitespace 
#				characters to either send or to match 
#				while receiving.
#			<address> is either a name within the IpAddresses
#				hash table or is an IP address in dotted 
#				notation form (xxx.xxx.xxx.xxx). Address is not
#
#			<port> is the UDP port number to use.
#		
#	Notes:
#		During a receive action, this routine will not return until
#		a message is received that matches character for character 
#		(case insensitive) the text specified. 	
#
sub udp_messaging {

if ($debug) {print "tsm DEBUG: udp_messaging @_\n";}

# Basic syntax check
unless ($#_ == 3) {warn "Internal syntax error: wrong number of keywords: udp_messaging @_\n"; return;}

# Allocate keywords
my($action, $msg, $address, $udp_port) = @_;


print "udp message stuff...\n";

# Check for UDP socket, create if needed
unless (defined $udp_socket) {
	if ($debug) {print "Creating UDP socket...\n";}

	socket($udp_socket, PF_INET, SOCK_DGRAM, getprotobyname("udp"))
		or die "socket: $!";
	print "creating UDP socked on bcast addr\n";
	my $rcvpaddr = sockaddr_in($udp_port, inet_aton("192.168.1.255"));
	bind($udp_socket, $rcvpaddr)
		or die "Couldn't bind to port $udp_port \(bind\( $udp_socket, $rcvpaddr \) : $!\)\n";
	}

# Perform Send or Receive activity
if ($action eq "send") {

	# Use ipaddress pseudonym if defined, otherwise use what is given
	my ($target, $rmsg);
	unless (defined $address) {die "No UDP address specified\n";}
	if (defined $IpAddresses{$address}) {
		$target = $IpAddresses{$address};
	} else { $target = $address; }

	# Execute UDP message send
	my $dstpaddr = sockaddr_in($udp_port, inet_aton($target));
	if ($debug) {
		print "tsm DEBUG: Sending the following message:\n";
		print "$msg \n";
	}
	send($udp_socket, $msg, 0, $dstpaddr)
		or die "send: $!";

} elsif ($action eq "receive") {


	# Listen on port until we receive the message we expect
	while ($msg ne $rmsg) {
		if ($debug) {
			print "tsm DEBUG: Waiting to receive the following message:\n";
			print "$msg \n";
		}
		my $srcpaddr = recv($udp_socket, $rmsg, 255, 0)
			or die "recv: $!";

		my($r_port, $r_ipaddr) = sockaddr_in($srcpaddr);
		$r_ipaddr = inet_ntoa($r_ipaddr);
		if ($debug) {
			print "tsm DEBUG: Received from $r_ipaddr:$r_port the following message:\n";
			print "$rmsg \n";
		}
	}
        $rmsg = "";
	
} else {warn "Invalid sub-routine action requested: udp_messaging '$action'\n"; return;}


}
### End of udp_messaging ###


#############################################################
# address_syntax()
#
#	Provides a simplified syntax checking mechanism for
#	validating address formats.
#
#	Acceptable syntax:
#
#		address_syntax <type> <address>
#
#		where
#			<type> is one of the following:
#				ip
#				mac
#
#			<address> is the address string to validate.
#
#		returns
#			0 if address violates acceptable syntactic form.
#			1 if address appears to be of a valid form.
#
#	Note: The syntax checking determines if the appropriate number
#		of address sub-fields are present, and if the address sub-field
#		separator is correct. However, no attempt is made to determine
#		if the sub-field values are valid beyond checking for the presence
#		(or absence) of alphabetic and/or numeric characters.
# 	
#
sub address_syntax {

if ($debug) {print "tsm DEBUG: address_syntax @_\n";}

# Basic syntax check
unless ($#_ == 1) {warn "Internal syntax error: wrong number of keywords: address_syntax @_\n"; return;}

my($type, $address) = @_;
my($result) = "";

if ($type eq "ip") {
	# Check for a six-tuple hexadecimal field separated by colons
	my(@parts) = split /:/, $address;
	unless ($#parts == 5) {return "";}
	foreach $field (@parts) {
		unless ($field =~ /^([0-9a-fA-F]){2}$/ ) {return "";}
	}
	$result = 1;
} elsif ($type eq "mac") {
	# Check for a four-tuple decimal field separated by periods
	# Field values must be 255 or less.
	my(@parts) = split /./, $address;
	unless ($#parts == 3) {return "";}
	foreach $field (@parts) {
		unless ($field =~ /^([0-9]){1,3}$/ ) {return "";}
		unless ($field > 255) {return "";}
	}
	$result = 1;
} else {
	warn "Invalid address type: address_syntax @_\n";
	$result = 1;
}

return $result

}
sub cmd_shellout{
use constant CHAIN => "tsmdeny";
		system ("iptables -t mangle -F " . CHAIN);
$cmd1=`date`;
$cmd2=`echo "*******************************************\n" >> /tmp/tsm_perf_log`;
$cmd2=`echo "New run of tsm script started $cmd1">> /tmp/tsm_perf_log`;
}
sub cmd_echoout{
print "on [$adopted_name]: @_\n";
		system ("iptables -t mangle -L -n");
print "on [$adopted_name]: @_\n";
		system ("iptables -L -n");
}
sub cmd_echotimetologfile{
$cmd1 = `perl -e 'print time'`;
$t1 = $cmd1*1000;
#$string1 = "on [$adopted_name]: @_ - epoch time [ms] is $cmd1\n";
$string1 = "on [$adopted_name]: @_ - epoch time [ms] is $t1\n";
#print "on [$adopted_name]: @_ - epoch time is $cmd1\n";
print $string1;
open (perflogfile, ">>/tmp/tsm_perf_log");
print perflogfile ($string1);
close(perflogfile);
}
### End of cmd_deny ###


