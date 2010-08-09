#include "../fileparser.h"

using namespace watcher;

int main(void) 
{
    double x, y, z;
    Timestamp ts;
    unsigned int totalRead=0;
    struct {
        GpsFileParser *parser;
        const char *filename;
    } parseData [] = {
        { new ManeLogFileParser, "test.log" }, 
        { new ManeSpecFileParser, "test.spec" }
    };
   
    for (size_t i=0; i<(sizeof(parseData)/sizeof(parseData[0])); i++) {
        printf("Reading file %s\n", parseData[i].filename); 
        if (!parseData[i].parser->openFile(parseData[i].filename)) 
            return(EXIT_FAILURE);
        int thisFileRead=0;
        while (parseData[i].parser->yield(ts, x, y, z)) {
            printf("Read: %f, %f, %f @ %lu\n", x, y, z, ts);
            totalRead++;
            thisFileRead++;
        }
        parseData[i].parser->closeFile(); 
        printf("Read %u data points from file %s\n", thisFileRead, parseData[i].filename); 
        if (thisFileRead!=10)
            return(EXIT_FAILURE); 
    }
    printf("Total lines read: %d\n", totalRead);
    return(EXIT_SUCCESS); 
}
