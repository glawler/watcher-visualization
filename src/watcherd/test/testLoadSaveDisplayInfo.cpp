#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::displayInfo test
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

#include "logger.h"
#include "edgeDisplayInfo.h"
#include "nodeDisplayInfo.h"
#include "labelDisplayInfo.h"
#include "layerDisplayInfo.h"
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
    LayerDisplayInfo lydi;  // Because you're once, twice, three times a lydi...
}

BOOST_AUTO_TEST_CASE( load_defaults_test )
{
    char *fileName="test.cfg"; 
    BOOST_CHECK( configConfig(fileName) ); 

    LayerDisplayInfo lydi;
    lydi.loadLayer("testLayer"); 

    BOOST_CHECK_EQUAL(lydi.edgeDisplayInfo.color, Color::blue); // blue is default. 
    BOOST_CHECK_EQUAL(lydi.defaultNodeDisplayInfo.flash, false); // false is default

    // remove config and kill file. 
    SingletonConfig::instance().getRoot().remove("DisplayOptions"); 
    bf::remove(fileName); 
}

BOOST_AUTO_TEST_CASE( modify_defaults_test )
{
    char *layerName="testLayer"; 
    char *defaultFileName="test.default.cfg"; 
    char *modifiedFileName="test.modified.cfg"; 

    BOOST_CHECK( configConfig(defaultFileName) ); 

    // Load defaults into new layer. 
    LayerDisplayInfo lydi;
    lydi.loadLayer(layerName); 

    // write defaults to a file.
    SingletonConfig::instance().writeFile(defaultFileName); 

    // Change some of the defaults. 
    lydi.edgeDisplayInfo.color=Color::green; 
    lydi.labelDisplayInfo.backgroundColor=Color::yellow;
    lydi.labelDisplayInfo.foregroundColor=Color::green;
    lydi.defaultNodeDisplayInfo.flash=true;
    lydi.defaultNodeDisplayInfo.label="Dharma & Greg"; 
    lydi.saveLayer();   // Commit changes to cfg store. 
    lydi.saveLayer();   // Doesn't hurt to save x times.

    // write to different file.
    SingletonConfig::instance().writeFile(modifiedFileName); 
    
    // clear config in memory
    SingletonConfig::instance().getRoot().remove("DisplayOptions"); 

    // read in from new file.
    BOOST_CHECK( configConfig(modifiedFileName) ); 

    // Load config from freshly read in file into new layer.
    LayerDisplayInfo lydi2;
    lydi2.loadLayer(layerName); 

    // New layer should have same modified values as old layer. 
    BOOST_CHECK_EQUAL(lydi.edgeDisplayInfo.color,            lydi2.edgeDisplayInfo.color); 
    BOOST_CHECK_EQUAL(lydi.labelDisplayInfo.backgroundColor, lydi2.labelDisplayInfo.backgroundColor);
    BOOST_CHECK_EQUAL(lydi.labelDisplayInfo.foregroundColor, lydi2.labelDisplayInfo.foregroundColor);
    BOOST_CHECK_EQUAL(lydi.defaultNodeDisplayInfo.flash,     lydi2.defaultNodeDisplayInfo.flash); 

    BOOST_CHECK( lydi2.edgeDisplayInfo.color != Color::blue ); // blue is default, but we've changed it.

    // remove config and kill file. 
    SingletonConfig::instance().getRoot().remove("DisplayOptions"); 
    bf::remove(defaultFileName); 
    // bf::remove(modifiedFileName); 
}


