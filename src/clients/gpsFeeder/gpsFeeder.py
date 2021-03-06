#!/usr/bin/env python

# @file gpsFeeder.py
# @author Geoff Lawler <geoff.lawler@cobham.com> 
# @date 2009-07-15

import gps, os, time, socket, subprocess

def connect():
    while 1:
        try: 
            session = gps.gps()
        except socket.error:
            print "Unable to connect to gpsd, trying again in just a sec."
            time.sleep(1)
        else:
            return session

def main(serverName, dataAddress):
    session=connect()
    while 1:
        while 1:
            try:
                session.query('admosy')
            except socket.error:
                session=connect()
            else:
                break

        # a = altitude, d = date/time, m=mode,
        # o=postion/fix, s=status, y=satellites

        print
        print ' GPS reading'
        print '----------------------------------------'
        print 'latitude    ' , session.fix.latitude
        print 'longitude   ' , session.fix.longitude
        print 'time utc    ' , session.utc, session.fix.time
        print 'altitude    ' , session.fix.altitude
        print 'eph         ' , session.fix.eph
        print 'epv         ' , session.fix.epv
        print 'ept         ' , session.fix.ept
        print 'speed       ' , session.fix.speed
        print 'climb       ' , session.fix.climb
        print
        print ' Satellites (total of', len(session.satellites) , ' in view)'

        for i in session.satellites:
            print '\t', i

        if session.fix.latitude and session.fix.longitude:
            try:
                # GTL -- need to check the return val here. 
                sendGPSArgs=['sendGPSMessage','-x', str(session.fix.longitude),'-y', str(session.fix.latitude),'-z', str(session.fix.altitude),'-s', str(serverName)];
                if dataAddress != None:
                    sendGPSArgs.append('-n')
                    sendGPSArgs.append(dataAddress)
                retCode=subprocess.call(sendGPSArgs);
            except OSError:
                print 'Caught exception when trying to run gpsMessageTest, is it in your $PATH?'
                print 'If not, type \'export PATH=$PATH:/path/to/dir/with/gpsMessageTest/in/it\' in this shell'
                sys.exit(1)

        time.sleep(1)

if __name__ == '__main__': 
    import sys
    from optparse import OptionParser
    parser=OptionParser('Usage: %prog -s watcherdServerName [-a localNodeAddress]')
    parser.add_option('-s', '--serverName', dest='serverName', help='machine name/ip address where watcherd is running')
    parser.add_option('-a', '--address', dest='dataAddress', help='local host data interface address, where the gps data "comes from"')
    (options, args)=parser.parse_args()
    if options.serverName==None:
        print 'You must give a servername on the command line: use the \'-s server\' option to do so.'
    else:
        main(options.serverName, options.dataAddress)
