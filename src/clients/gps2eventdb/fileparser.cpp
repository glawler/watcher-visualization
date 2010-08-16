#include "fileparser.h"

namespace watcher {
    GpsFileParser::GpsFileParser() : theFile(NULL)
    {
    }
    GpsFileParser::~GpsFileParser() 
    {
        if (theFile) {
            fclose(theFile);
            theFile=NULL;
        }
    }
    bool GpsFileParser::openFile(const std::string &filename) 
    {
        if (theFile) {
            fclose(theFile);
            theFile=NULL;
        }
        if (NULL==(theFile=fopen(filename.c_str(), "r"))) {
            fprintf(stderr, "Unable to open file %s for reading.\n", filename.c_str());
            return false;
        }
        char buf[256];
        if(!fgets(buf, sizeof(buf), theFile))
            return false;
        double x, y, z;
        Timestamp ts;
        if (parseSpec(buf, sizeof(buf), x, y, z, ts))
            fileType=SPEC;
        else if (parseLog(buf, sizeof(buf), x, y, z, ts))
            fileType=LOG;
        else {
            closeFile();
            return false;
        }
        rewind(theFile);
            
        return true;
    }
    void GpsFileParser::closeFile() 
    {
        if (theFile) {
            fclose(theFile);
            theFile=NULL;
        }
    }
    bool GpsFileParser::yield(Timestamp &ts, double &x, double &y, double &z)
    {
        if (!theFile)
            return false;
        char buf[256];
        if(!fgets(buf, sizeof(buf), theFile))
            return false;
        bool retVal;
        switch (fileType) {
            case SPEC:
                retVal=parseSpec(buf, sizeof(buf), x, y, z, ts);
                break;
            case LOG:
                retVal=parseLog(buf, sizeof(buf), x, y, z, ts);
                break;
        }
        return retVal;
    }
    bool GpsFileParser::parseSpec(const char *buf, size_t size, double &x, double &y, double &z, Timestamp &ts)
    {
        static time_t now=0;
        if (!now)
            now=time(NULL);
        unsigned long fts;
        if (4!=sscanf(buf, "%lu,%lf,%lf,%lf\n", &fts, &x, &y, &z))
            return false;
        ts=(Timestamp)fts+now;  // offset from "now" as spec file starts at time of test, not absolute time.
        ts*=1000;
        return true;
    }
    bool GpsFileParser::parseLog(const char *buf, size_t size, double &x, double &y, double &z, Timestamp &ts)
    {
        // time>14:40:14.000000 position>0.000259,0.001400,0.000000
        unsigned int h, m;
        float s, fx, fy, fz;
        if (6!=sscanf(buf, "time>%2u:%2u:%f position>%lf,%lf,%lf\n", &h, &m, &s, &x, &y, &z)) 
            return false;
        if (!getElapsedTime(h, m, s, ts))
            return false;
        return true;
    }
    bool GpsFileParser::getElapsedTime(int h, int m, int s, Timestamp &ts) 
    {
        static bool gotNow=false;
        static struct tm now;
        if (!gotNow) {
            time_t n=time(NULL);
            gmtime_r(&n, &now); 
            gotNow=true;
        }
        struct tm t;
        memcpy(&t, &now, sizeof(now));
        t.tm_sec=s;
        t.tm_min=m;
        t.tm_hour=h;
        ts=(Timestamp)timegm(&t); 
        ts*=1000;
        return true;
    }
}
