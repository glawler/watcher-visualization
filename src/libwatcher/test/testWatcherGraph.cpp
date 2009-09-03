/** 
 * @file testWatcherGraph.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::watcherGraph test
#include <boost/test/unit_test.hpp>

#include "libwatcher/watcherSerialize.h"
#include "libwatcher/watcherGraph.h"
#include "libwatcher/colors.h"

#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace watcher::colors;
using namespace boost::unit_test_framework;
using namespace boost::archive;

EdgeMessagePtr createEdgeMessage()
{
    LabelMessagePtr lmm(new LabelMessage); 
    lmm->label="I am the middle label"; 
    lmm->fontSize=15;
    lmm->foreground=colors::black;
    lmm->background=colors::white;
    lmm->expiration=5000;

    LabelMessagePtr lm1(new LabelMessage); 
    lm1->label="I am node1 label"; 
    lm1->fontSize=30;
    lm1->foreground=colors::blue;
    lm1->background=colors::red;
    lm1->expiration=10000;

    // Test with a NULL MessageLabelPtr for the node2 label.
    // LabelMessagePtr lm2(new LabelMessage); 
    // lm2->label="I am node2 label"; 
    // lm2->fontSize=10;
    // lm2->foreground=red;
    // lm2->background=blue;
    // lm2->expiration=7500;

    EdgeMessagePtr em(new EdgeMessage); 
    em->node1=asio::ip::address::from_string("192.168.1.101");
    em->node2=asio::ip::address::from_string("192.168.1.102");
    em->edgeColor=colors::red;
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
    LOAD_LOG_PROPS("test.log.properties"); 

    WatcherGraph wg();
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("Checking Graph output operator,"); 

    WatcherGraph wg;

    cout << "Empty Graph:" << endl << wg << endl;

    EdgeMessagePtr em=createEdgeMessage();
    wg.updateGraph(em);

    cout << "Graph with edge message:" << endl << wg << endl;

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
    em->edgeColor=colors::green;
    wg.updateGraph(em); 

    // add the same neighbors again...
    wg.updateGraph(cm); 

    GPSMessagePtr gm(new GPSMessage(0.1234, 0.2345, 0.3456)); 
    gm->fromNodeID=asio::ip::address::from_string("192.168.1.100"); 
    wg.updateGraph(gm); 

    // NodeStatusMessagePtr nsm(new NodeStatusMessage(NodeStatusMessage::connect));
    // nsm->fromNodeID=asio::ip::address::from_string("192.168.1.100"); 
    // wg.updateGraph(nsm); 
  
    ColorMessagePtr colm(new ColorMessage(colors::red, asio::ip::address::from_string("192.168.1.101"))); 
    wg.updateGraph(colm); 

    cout << "Graph with edge message and neighbors:" << endl << wg << endl;
}

// GTL - don't do serialize until we need it and we've added serialization to the DisplayInfo classes
// BOOST_AUTO_TEST_CASE( serialize_test )
// {
//     WatcherGraph wg;
// 
//     std::vector<watcher::NodeIdentifier> neighbors;
//     for(unsigned long i=0xc0a80165; i<0xc0a8016a; i++) // 0xc0a80165==192.168.1.101 in host byte order
//         neighbors.push_back(boost::asio::ip::address_v4(i));
//     ConnectivityMessagePtr cm(new ConnectivityMessage);
//     cm->neighbors=neighbors;
//     wg.updateGraph(cm);
// 
//     LabelMessagePtr lmp(new LabelMessage("Hello There", asio::ip::address::from_string("192.168.1.103")));
//     wg.updateGraph(lmp);
// 
//     ostringstream os;
//     text_oarchive oa(os); 
//     oa << wg; 
// 
//     WatcherGraph wgdup;
//     istringstream is(os.str());
//     text_iarchive ia(is);
//     ia >> wgdup;
// 
//     LOG_DEBUG("output: " << os.str());
//     LOG_DEBUG("input: " << is.str());
// 
//     // BOOST_CHECK_EQUAL(wg, wgdup); 
// }
// 
// BOOST_AUTO_TEST_CASE( pack_unpack_test )
// {
//     WatcherGraph wg;
// 
//     std::vector<watcher::NodeIdentifier> neighbors;
//     for(unsigned long i=0xc0a80165; i<0xc0a8016a; i++) // 0xc0a80165==192.168.1.101 in host byte order
//         neighbors.push_back(boost::asio::ip::address_v4(i));
//     ConnectivityMessagePtr cm(new ConnectivityMessage);
//     cm->neighbors=neighbors;
//     wg.updateGraph(cm);
// 
//     LabelMessagePtr lmp(new LabelMessage("Hello There", asio::ip::address::from_string("192.168.1.103")));
//     wg.updateGraph(lmp);
// 
//     ostringstream os;
//     wg.pack(os); 
// 
//     WatcherGraph wgdup;
//     istringstream is(os.str());
//     wgdup.unpack(is); 
// 
//     LOG_DEBUG("output: " << os.str());
//     LOG_DEBUG("input: " << is.str());
// 
//     // BOOST_CHECK_EQUAL(wg, wgdup); 
// }

BOOST_AUTO_TEST_CASE( graph_edge_expiration_test )
{
    WatcherGraph wg; 

    NodeIdentifier node1=asio::ip::address::from_string("192.168.1.101");
    NodeIdentifier node2=asio::ip::address::from_string("192.168.1.102");

    unsigned int numEdges=6;
    for (unsigned int i=0; i < numEdges; i++)
    {
        EdgeMessagePtr emp(new EdgeMessage);
        emp->node1=node1;
        emp->node2=node2;
        emp->expiration=i==0 ? Infinity : Timestamp(i*1000)-500;  

        // all labels expire in 2 seconds, so they should all disapear halfway though.
        // GTL - TODO: add a BOOST_CHECK to test this. 
        emp->middleLabel.reset(new LabelMessage("Hello There"));
        emp->middleLabel->expiration=2000;          // all labels expire in 2 seconds, so they should all disapear halfway though. 

        wg.updateGraph(emp);
    }

    for (unsigned int i=0; i < numEdges; i++)
    {
        LOG_INFO("Current edges in graph at " << Timestamp(time(NULL))*1000); 
        WatcherGraph::edgeIterator ei, eEnd; 
        for(tie(ei, eEnd)=edges(wg.theGraph); ei!=eEnd; ++ei)
        {
            LOG_INFO("Edge: " << wg.theGraph[*ei]); 
        }

        BOOST_CHECK_EQUAL(numEdges-i, num_edges(wg.theGraph)); 
        cout << "."; 
        cout.flush(); 
        sleep(1); 
        wg.doMaintanence(); // should remove one edge
    }

    BOOST_CHECK_EQUAL( (size_t)1, num_edges(wg.theGraph)); 
}

BOOST_AUTO_TEST_CASE( graph_node_floating_label_expiration_test )
{
    WatcherGraph wg; 

    struct 
    {
        char *str;
        Timestamp exp;
        float x, y, z;
    } labelData [] = {
        { "This floating label will never self destruct", -1, 1, 1, 1},
        { "This floating label will self destruct in .5 seconds", 500, 2, 2, 2},
        { "This floating label will self destruct in 1.5 seconds", 1500, 3, 3, 3},
        { "This floating label will self destruct in 2.5 seconds", 2500, 4, 4, 4},
        { "This floating label will self destruct in 3.5 seconds", 3500, 5, 5, 5},
        { "This floating label will self destruct in 4.5 seconds", 4500, 6, 6, 6},
    }; 

    size_t numLabels=sizeof(labelData)/sizeof(labelData[0]);
    for (unsigned int i=0; i<numLabels; i++)
    {
        LabelMessagePtr lmp(new LabelMessage(labelData[i].str, labelData[i].x, labelData[i].y, labelData[i].z)); 
        lmp->expiration=labelData[i].exp; 
        wg.updateGraph(lmp);
    }

    BOOST_CHECK_EQUAL( numLabels, wg.floatingLabels.size() );  

    for (unsigned int i=0; i<numLabels; i++)
    {
        WatcherGraph::FloatingLabelList::iterator labelIterBegin=wg.floatingLabels.begin(); 
        WatcherGraph::FloatingLabelList::iterator labelIterEnd=wg.floatingLabels.end();
        LOG_INFO("Current floating lables (" << wg.floatingLabels.size() << ") at " << Timestamp(time(NULL))*1000); 
        for( ;labelIterBegin!=labelIterEnd; ++labelIterBegin)
        {
            LOG_INFO("\t" << (*labelIterBegin)->labelText); 
        }

        BOOST_CHECK_EQUAL( numLabels-i, wg.floatingLabels.size() );

        cout << "."; 
        cout.flush(); 
        sleep(1);
        wg.doMaintanence();  // Should remove one label
    }

    BOOST_CHECK_EQUAL( (size_t)1, wg.floatingLabels.size() );
}


BOOST_AUTO_TEST_CASE( graph_node_label_expiration_test )
{
    WatcherGraph wg; 

    struct 
    {
        char *str;
        Timestamp exp;
    } labelData [] = {
        { "This message will never self destruct", -1}, 
        { "This message will self destruct in .5 seconds", 500 }, 
        { "This message will self destruct in 1.5 seconds", 1500 }, 
        { "This message will self destruct in 2.5 seconds", 2500 }, 
        { "This message will self destruct in 3.5 seconds", 3500 }, 
        { "This message will self destruct in 4.5 seconds", 4500 }
    }; 

    size_t numLabels=sizeof(labelData)/sizeof(labelData[0]);
    NodeIdentifier nodeAddr=asio::ip::address::from_string("192.168.1.100");
    for (unsigned int i=0; i<numLabels; i++)
    {
        LabelMessagePtr lmp(new LabelMessage(labelData[i].str)); 
        lmp->fromNodeID=nodeAddr;
        lmp->expiration=labelData[i].exp; 
        wg.updateGraph(lmp);
    }

    WatcherGraph::vertexIterator theNodeIter; 
    wg.findNode(nodeAddr, theNodeIter); 

    BOOST_CHECK_EQUAL( numLabels, wg.theGraph[*theNodeIter].labels.size() );  

    for (unsigned int i=0; i<numLabels; i++)
    {
        WatcherGraphNode::LabelList::iterator labelIterBegin=wg.theGraph[*theNodeIter].labels.begin(); 
        WatcherGraphNode::LabelList::iterator labelIterEnd=wg.theGraph[*theNodeIter].labels.end(); 
        LOG_INFO("Current lables on node " << wg.theGraph[*theNodeIter].nodeId << " at " << Timestamp(time(NULL))*1000); 
        for( ;labelIterBegin!=labelIterEnd; ++labelIterBegin)
        {
            LOG_INFO("\t" << (*labelIterBegin)->labelText); 
        }

        BOOST_CHECK_EQUAL( numLabels-i, wg.theGraph[*theNodeIter].labels.size() );

        cout << "."; 
        cout.flush(); 
        sleep(1);
        wg.doMaintanence();  // Should remove one label
    }

    BOOST_CHECK_EQUAL( (size_t)1, wg.theGraph[*theNodeIter].labels.size() );
}

