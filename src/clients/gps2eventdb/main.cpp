#include <stdlib.h>
#include "configuration.h"

int main(int argc, char *argv[])
{
    // Load config and bail on error. 
    if (!Gps2EventDb::loadConfiguration(argc, argv))
        return EXIT_FAILURE;

    // Show loaded config on stdout
    Gps2EventDb::dumpConfiguration(std::cout);

    return(EXIT_SUCCESS);
}
