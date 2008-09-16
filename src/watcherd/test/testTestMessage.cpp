#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include "../messageFactory.h"
#include "../testMessage.h"
#include "../messageStatus.h"
#include "../message.h"

#include "logger.h"

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
    // cout << "Arch Stream: " << archiveStream.str() << e
    //
    
    vector<int> ints;
    ints.push_back(argc);
    vector<shared_ptr<Message> > tms;

    tms.push_back(shared_ptr<TestMessage>(new TestMessage(argv[0], ints)));
    ints.push_back(argc+1);
    tms.push_back(shared_ptr<Message>(new Message));
    ints.push_back(argc+2);
    tms.push_back(shared_ptr<TestMessage>(new TestMessage(argv[0], ints)));
    ints.push_back(argc+3);

    // Write it out
    cout << "TestMessage Out: [";
    //copy(tms.begin(), tms.end(), ostream_iterator<shared_ptr::<Message>>(cout, "|"));
    for (vector<shared_ptr<Message> >::const_iterator i = tms.begin(); i != tms.end(); ++i)
        // if ((*i)->type == TEST_MESSAGE_TYPE) 
        //    cout << static_cast<TestMessage*>(&**i) << " ";
        // else 
        cout << "\n\t " << **i;
    cout << "\n]" << endl;

    ofstream otfs("archived.txt");
    archive::polymorphic_text_oarchive ota(otfs);
    ota << tms;
    otfs.close();

    ofstream obfs("archived.dat", ios::out|ios::binary);
    archive::polymorphic_binary_oarchive oba(obfs);
    oba << tms;
    obfs.close();

    // Read it in.
    vector<shared_ptr<Message> > fromtms;
    ifstream itfs("archived.txt");
    archive::polymorphic_text_iarchive ita(itfs);
    ita >> fromtms;
    itfs.close();
    cout << "TestMessage From Text: [";
    //copy(tms.begin(), tms.end(), ostream_iterator<shared_ptr::<Message>>(cout, "|"));
    for (vector<shared_ptr<Message> >::const_iterator i = tms.begin(); i != tms.end(); ++i)
        cout << "\n\t " << **i;
    cout << "\n]" << endl;

    ifstream ibfs("archived.dat");
    archive::polymorphic_binary_iarchive iba(ibfs);
    iba >> fromtms;
    ibfs.close();
    cout << "TestMessage From Bin: [";
    // copy(tms.begin(), tms.end(), ostream_iterator<TestMessage>(cout, "|"));
    for (vector<shared_ptr<Message> >::const_iterator i = tms.begin(); i != tms.end(); ++i)
        cout << "\n\t " << **i;
    cout << "\n]" << endl;

    // MessageStatus statMess(MessageStatus::status_ok);
    // cout << statMess;

    TRACE_EXIT();
    return 0;
}


    
