#define BOOST_TEST_MODULE watcher::watcherGraph test
#include <boost/test/unit_test.hpp>

#include "logger.h"
#include "../watcherGraph.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

EdgeMessagePtr createEdgeMessage()
{
    LabelMessagePtr lmm(new LabelMessage); 
    lmm->label="I am the middle label"; 
    lmm->fontSize=15;
    lmm->foreground=Color::black;
    lmm->background=Color::white;
    lmm->expiration=5000;

    LabelMessagePtr lm1(new LabelMessage); 
    lm1->label="I am node1 label"; 
    lm1->fontSize=30;
    lm1->foreground=Color::blue;
    lm1->background=Color::red;
    lm1->expiration=10000;

    // Test with a NULL MessageLabelPtr for the node2 label.
    // LabelMessagePtr lm2(new LabelMessage); 
    // lm2->label="I am node2 label"; 
    // lm2->fontSize=10;
    // lm2->foreground=Color::red;
    // lm2->background=Color::blue;
    // lm2->expiration=7500;

    EdgeMessagePtr em(new EdgeMessage); 
    em->node1=asio::ip::address::from_string("192.168.1.101");
    em->node2=asio::ip::address::from_string("192.168.1.102");
    em->edgeColor=Color::red;
    em->expiration=20000;
    em->width=15;
    em->layer=UNDEFINED_LAYER;
    em->addEdge=true;
    em->setMiddleLabel(lmm);
    em->setNode1Label(lm1);
    // em->setNode2Label(lm2);

    return em;
}

BOOST_AUTO_TEST_CASE( ctor_test )
{
    // Do this in first test so we can log.
    LOAD_LOG_PROPS("log.properties"); 

    WatcherGraph wg();
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("Checking Graph output operator,"); 

    WatcherGraph wg;

    cout << "Empty Graph:" << endl << wg;

    EdgeMessagePtr em=createEdgeMessage();
    wg.updateGraph(em);

    cout << "Graph with edge message:" << endl << wg;

    std::vector<watcher::NodeIdentifier> neighbors;
    for(unsigned long i=0xc0a80165; i<0xc0a8016a; i++) // 0xc0a80165==192.168.1.101 in host byte order
        neighbors.push_back(boost::asio::ip::address_v4(i));
        
    ConnectivityMessagePtr cm(new ConnectivityMessage);
    cm->neighbors=neighbors;
    cm->fromNodeID=asio::ip::address::from_string("192.168.1.100"); 
    wg.updateGraph(cm);

    em=EdgeMessagePtr(new EdgeMessage); 
    em->bidirectional=true;
    em->node1=asio::ip::address::from_string("192.168.1.103"); 
    em->node2=asio::ip::address::from_string("192.168.1.104"); 
    em->edgeColor=Color::green;
    wg.updateGraph(em); 

    // add the same neighbors again...
    wg.updateGraph(cm); 

    cout << "Graph with edge message and neighbors:" << endl << wg;
}


