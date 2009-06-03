#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::displayInfo test
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

#include "logger.h"
#include "libwatcher/edgeDisplayInfo.h"
#include "libwatcher/nodeDisplayInfo.h"
#include "libwatcher/labelDisplayInfo.h"
#include "libwatcher/messageTypesAndVersions.h"
#include "singletonConfig.h"
#include "initConfig.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace libconfig;
using namespace boost::unit_test_framework;
namespace bf=boost::filesystem;

bool configConfig(char *filename)
{
    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();

    if (!bf::exists(filename))
    {
        ofstream f(filename);
        f.close();
    }
    try
    {
        cfg.readFile(filename); 
    }
    catch (ParseException &e)
    {
        SingletonConfig::unlock();
        cerr << "Parse error when reading config file: " << filename << endl;
        return false; 
    }

    SingletonConfig::unlock();
    return true;
}

BOOST_AUTO_TEST_CASE( ctors_test )
{
    LOAD_LOG_PROPS("test.log.properties"); 
    EdgeDisplayInfo edi;
    NodeDisplayInfo ndi;
    LabelDisplayInfo lbdi;
}

BOOST_AUTO_TEST_CASE( load_defaults_test )
{
    char *fileName="test.cfg"; 
    BOOST_CHECK( configConfig(fileName) ); 

    EdgeDisplayInfoPtr edip(new EdgeDisplayInfo); 
    edip->loadConfiguration(PHYSICAL_LAYER); 

    BOOST_CHECK_EQUAL(edip->color, Color::blue); // blue is default edge color.

    // remove config and kill file. 
    SingletonConfig::instance().getRoot().remove("displayOptions"); 
    bf::remove(fileName); 
}

BOOST_AUTO_TEST_CASE( modify_defaults_test )
{
    char *layerName="Unfunny Sitcom Layer"; 
    char *defaultFileName="test.default.cfg"; 
    char *modifiedFileName="test.modified.cfg"; 

    BOOST_CHECK( configConfig(defaultFileName) ); 

    // Load defaults into new layer. 
    EdgeDisplayInfoPtr edip(new EdgeDisplayInfo); 
    edip->loadConfiguration(layerName); 

    // write defaults to a file.
    SingletonConfig::instance().writeFile(defaultFileName); 

    // Change some of the defaults. 
    edip->color=Color::green; 
    edip->width=42.42;
    edip->label="Dharma & Greg"; 
    edip->saveConfiguration();   // Commit changes to cfg store. 
    edip->saveConfiguration();   // Doesn't hurt to save x times.

    // write to different file.
    SingletonConfig::instance().writeFile(modifiedFileName); 
    
    // clear config in memory
    SingletonConfig::instance().getRoot().remove("displayOptions"); 

    // read in from new file.
    BOOST_CHECK( configConfig(modifiedFileName) ); 

    // Load config from freshly read in file into new layer.
    EdgeDisplayInfoPtr edip2(new EdgeDisplayInfo);
    edip2->loadConfiguration(layerName); 

    // New layer should have same modified values as old layer. 
    BOOST_CHECK_EQUAL(edip->color, edip2->color); 
    BOOST_CHECK_EQUAL(edip->label, edip2->label); 
    BOOST_CHECK_EQUAL(edip->width, edip2->width); 

    BOOST_CHECK( edip2->color != Color::blue ); // blue is default, but we've changed it.

    // remove config and kill file. 
    bf::remove(defaultFileName); 

    // Load in the modified file, but load a new layer
    edip2.reset(new EdgeDisplayInfo);
    edip2->loadConfiguration(PHYSICAL_LAYER); 
    BOOST_CHECK_EQUAL( edip2->color, Color::blue); 

    SingletonConfig::instance().writeFile(modifiedFileName); 

    SingletonConfig::instance().getRoot().remove("displayOptions"); 
    // bf::remove(modifiedFileName); 
}

BOOST_AUTO_TEST_CASE( layer_name_test )
{
    // GTL - this test was to allow layers with spaces, etc in them 
    // but, meh - just don't allow them. 
    // BOOST_CHECK(configConfig("test.cfg")); 
    // NodeDisplayInfoPtr ndip(new NodeDisplayInfo);
    // ndip->loadConfiguration("Layer with spaces in the name"); 
}

BOOST_AUTO_TEST_CASE( node_config_test )
{
    char *filename="test.node.cfg"; 
    BOOST_CHECK(configConfig(filename)); 

    NodeDisplayInfoPtr ndip1(new NodeDisplayInfo), ndip2(new NodeDisplayInfo); 
    ndip1->loadConfiguration(PHYSICAL_LAYER); 

    NodeIdentifier nid101=NodeIdentifier::from_string("192.168.1.101"); 
    ndip1->loadConfiguration(PHYSICAL_LAYER, nid101); 
    ndip1->sparkle=true; 
    ndip1->spin=true; 
    ndip1->flash=true; 

    // ndip2's nodeID is ignored.  
    // So ndip2 should not equal ndip1
    // and should be saved in the default section
    ndip2->nodeId=NodeIdentifier::from_string("192.168.1.3");
    ndip2->loadConfiguration(PHYSICAL_LAYER); 

    // Write it out
    ndip1->saveConfiguration();
    ndip2->saveConfiguration();
    SingletonConfig::instance().writeFile(filename); 

    // Forget everything and reload
    SingletonConfig::instance().getRoot().remove("displayOptions");
    BOOST_CHECK(configConfig(filename)); 

    ndip1.reset(new NodeDisplayInfo); 
    ndip2.reset(new NodeDisplayInfo); 
    ndip1->loadConfiguration(PHYSICAL_LAYER, nid101); 
    ndip2->loadConfiguration(PHYSICAL_LAYER); 

    // saved not default
    BOOST_CHECK_EQUAL( ndip1->sparkle, true ); 
    BOOST_CHECK_EQUAL( ndip1->spin, true ); 
    BOOST_CHECK_EQUAL( ndip1->flash, true ); 

    // default
    BOOST_CHECK_EQUAL( ndip2->sparkle, false ); 
}

BOOST_AUTO_TEST_CASE( bad_config_init_test )
{
    // GTL test case for a config that hasn't been initialized. 
    // Was hanging....
}
