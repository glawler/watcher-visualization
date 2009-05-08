#!/usr/bin/env python

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

def main():
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
                retCode=subprocess.call(['gpsMessageTest',
                                         '-x', str(session.fix.latitude), 
                                         '-y', str(session.fix.longitude), 
                                         '-z', str(session.fix.altitude),
                                         '-s', 'glory' ])
            except OSError:
                print 'Caught exception when trying to run gpsMessageTest, is it in your $PATH?'
                print 'If not, type \'export PATH=$PATH:/path/to/dir/with/gpsMessageTest/in/it\' in this shell'
                sys.exit(1)

        time.sleep(1)

if __name__ == '__main__': 
    main()
