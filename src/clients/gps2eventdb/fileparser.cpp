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
        return true;
    }
    void GpsFileParser::closeFile() 
    {
        if (theFile)
            fclose(theFile);
    }
    ManeSpecFileParser::ManeSpecFileParser() : GpsFileParser()
    {
    }
    ManeSpecFileParser::~ManeSpecFileParser()
    {
    }
    bool ManeSpecFileParser::yield(Timestamp &ts, double &x, double &y, double &z)
    {
        if (!theFile)
            return false;
        static time_t now=0;
        if (!now)
            now=time(NULL);
        char buf[256];
        if(!fgets(buf, sizeof(buf), theFile))
            return false;
        float fx, fy, fz;
        int fts;
        if (4!=sscanf(buf, "%d,%f,%f,%f\n", &fts, &fx, &fy, &fz))
            return false;
        x=fx;
        y=fy;
        z=fz;
        ts=fts+now;  // offset from "now" as spec file starts at time of test, not absolute time.
        return true;
    }
    ManeLogFileParser::ManeLogFileParser() : GpsFileParser()
    {
    }
    ManeLogFileParser::~ManeLogFileParser()
    {
    }
    bool ManeLogFileParser::yield(Timestamp &ts, double &x, double &y, double &z)
    {
        if (!theFile)
            return false;
        char buf[256];
        if(!fgets(buf, sizeof(buf), theFile))
            return false;
        // time>14:40:14.000000 position>0.000259,0.001400,0.000000
        unsigned int h, m;
        float s, fx, fy, fz;
        if (6!=sscanf(buf, "time>%2u:%2u:%f position>%f,%f,%f\n", &h, &m, &s, &fx, &fy, &fz)) 
            return false;
        if (!getElapsedTime(h, m, s, ts))
            return false;
        x=fx;
        y=fy;
        z=fz;
        return true;
    }
    bool ManeLogFileParser::getElapsedTime(int h, int m, int s, Timestamp &ts) 
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
        return true;
    }
}
