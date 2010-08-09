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
            virtual bool yield(Timestamp &ts, double &x, double &y, double &z) = 0;

        protected:
            FILE *theFile;
        private:
    };
    class ManeSpecFileParser : public GpsFileParser {
        public:
            ManeSpecFileParser();
            virtual ~ManeSpecFileParser();

            /** return the next x,y,z,timestamp quad in the opened file. Return false on error. */
            virtual bool yield(Timestamp &ts, double &x, double &y, double &z);

        protected:
        private:
    };
    class ManeLogFileParser : public GpsFileParser {
        public:
            ManeLogFileParser();
            virtual ~ManeLogFileParser();

            /** return the next x,y,z,timestamp quad in the opened file. Return false on error. */
            virtual bool yield(Timestamp &ts, double &x, double &y, double &z);

        protected:
            bool getElapsedTime(int h, int m, int s, Timestamp &ts);
        private:
    };
}
#endif //  GPS2DB_FILEPARSER
