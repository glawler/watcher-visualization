#include <string>
#include <iostream>
#include <vector>

#include "../testMessage.h"
#include "logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

using namespace std;
using namespace watcher;

int main(int argc, char **argv)
{
    log4cxx::PropertyConfigurator::configure("log.properties"); 

    vector<int> ints;
    ints.push_back(argc);
    ints.push_back(argc+1);
    ints.push_back(argc+2);
    ints.push_back(argc+3);
    TestMessage mess(argv[0], ints);

    // cout << "TestMessage: [" << mess << "]" << endl;
    
    TestMessage other(mess);

    if (other == mess)
        cout << "copy ctor works" << endl;

    return 0;
}


    
