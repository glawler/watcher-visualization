#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <boost/serialization/access.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "../messageFactory.h"
#include "../testMessage.h"

#include "logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

using namespace std;
using namespace watcher;
using namespace boost;

int main(int argc, char **argv)
{
    TRACE_ENTER();

    log4cxx::PropertyConfigurator::configure("log.properties"); 

    // shared_ptr<Message> m = MessageFactory::makeMessage(MESSAGE_HEADER);
    //
    // ostringstream archiveStream;
    // archive::text_oarchive archive(archiveStream);
    // archive << *m;
    // LOG_INFO("Message m: [" << *m << "]");
    // cout << "Arch Stream: " << archiveStream.str() << endl;

    vector<int> ints;
    ints.push_back(argc);
    vector<TestMessage> tms;
    tms.push_back(TestMessage(argv[0], ints));
    ints.push_back(argc+1);
    tms.push_back(TestMessage(argv[0], ints));
    ints.push_back(argc+2);
    tms.push_back(TestMessage(argv[0], ints));
    ints.push_back(argc+3);

    // Write it out
    cout << "TestMessage Out: [";
    copy(tms.begin(), tms.end(), ostream_iterator<TestMessage>(cout, "|"));
    cout << "]" << endl;

    ofstream otfs("archived.txt");
    archive::text_oarchive ota(otfs);
    ota << tms;
    otfs.close();

    ofstream obfs("archived.dat", ios::out|ios::binary);
    archive::binary_oarchive oba(obfs);
    oba << tms;
    obfs.close();

    // Read it in.
    vector<TestMessage> fromtms;
    ifstream itfs("archived.txt");
    archive::text_iarchive ita(itfs);
    ita >> fromtms;
    itfs.close();
    cout << "TestMessage From Text: [";
    copy(tms.begin(), tms.end(), ostream_iterator<TestMessage>(cout, "|"));
    cout << "]" << endl;

    ifstream ibfs("archived.dat");
    archive::binary_iarchive iba(ibfs);
    iba >> fromtms;
    ibfs.close();
    cout << "TestMessage From Bin: [";
    copy(tms.begin(), tms.end(), ostream_iterator<TestMessage>(cout, "|"));
    cout << "]" << endl;

    TRACE_EXIT();
    return 0;
}


    
