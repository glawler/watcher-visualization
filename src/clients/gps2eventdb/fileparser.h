#ifndef GPS2DB_FILEPARSER
#define GPS2DB_FILEPARSER

#include <libwatcher/watcherTypes.h>  // for Timestamp

namespace watcher {
    class GpsFileParser {
        public:
            GpsFileParser();
            virtual ~GpsFileParser(); 

            /** Open the file passed. Returns false if the file does not exist. */
            bool openFile(const std::string &filename); 
           
            /** Close an existing opened file. */
            void closeFile();

            /** return the next x,y,z,timestamp quad in the opened file. Return false on error. */
            bool yield(Timestamp &ts, double &x, double &y, double &z);

        protected:

        private:
            FILE *theFile;
            enum { SPEC, LOG } fileType;
            bool parseSpec(const char *buf, size_t size, double &x, double &y, double &z, Timestamp &ts); 
            bool parseLog(const char *buf, size_t size, double &x, double &y, double &z, Timestamp &ts); 
            bool getElapsedTime(int h, int m, int s, Timestamp &ts);

            GpsFileParser(const GpsFileParser &nocopies);
            GpsFileParser &operator=(const GpsFileParser &nocopies);
    };
}
#endif //  GPS2DB_FILEPARSER
