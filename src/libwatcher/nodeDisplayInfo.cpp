/**
 * @file nodeDisplayInfo.cpp
 * @author Geoff Lawler <Geoff.Lawler@cobham.com>
 * @date 2009-01-27
 */
#include <boost/algorithm/string.hpp>       // for iequals();
#include "singletonConfig.h"
#include "nodeDisplayInfo.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;     // for Color
using namespace libconfig;
using namespace boost; 

INIT_LOGGER(NodeDisplayInfo, "DisplayInfo.NodeDisplayInfo"); 

NodeDisplayInfo::NodeDisplayInfo() : 
    DisplayInfo("node"), 
    shape(CIRCLE),
    sparkle(false),
    spin(false),
    flash(false),
    size(1.0),
    spinTimeout(100),
    spinIncrement(5.0),
    spinRotation_x(0.0),
    spinRotation_y(0.0),
    spinRotation_z(0.0),
    flashInterval(500),
    isFlashed(false),
    label(NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::LAST_OCTET)),
    color(Color::red)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
NodeDisplayInfo::~NodeDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// bool NodeDisplayInfo::configure()
// {
//     TRACE_ENTER();
//     TRACE_EXIT();
//     return false; 
// }

bool NodeDisplayInfo::loadLayer(const string &basePath)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();

    SingletonConfig::lock();

    bool boolVal;
    string strVal, key;
    int intVal;
    float floatVal;

    Setting &baseSetting=cfg.lookup(basePath); 
    if (!baseSetting.exists(categoryName))
        baseSetting.add(categoryName, Setting::TypeGroup);

    // "DisplayOptions.layer.[LAYERNAME].node"
    Setting &nodeSetting=cfg.lookup(baseSetting.getPath() + string(".") + categoryName); 

    strVal=NodeDisplayInfo::labelDefault2String(LAST_OCTET); 
    key="label"; 
    if (!nodeSetting.lookupValue(key, strVal))
        nodeSetting.add(key, Setting::TypeString)=strVal;
    else
        label=strVal;

    strVal=Color::red.toString(); 
    key="color"; 
    if (!nodeSetting.lookupValue(key, strVal))
        nodeSetting.add(key, Setting::TypeString)=strVal;
    else
        color.fromString(strVal); 

    strVal=NodeDisplayInfo::nodeShapeToString(CIRCLE);
    key="shape"; 
    if (!nodeSetting.lookupValue(key, strVal))
        nodeSetting.add(key, Setting::TypeString)=strVal;
    else
        shape=NodeDisplayInfo::stringToNodeShape(strVal); 

    boolVal=false;
    key="sparkle"; 
    if (!nodeSetting.lookupValue(key, boolVal))
        nodeSetting.add(key, Setting::TypeBoolean)=boolVal;
    else
        sparkle=boolVal;

    boolVal=false;
    key="spin"; 
    if (!nodeSetting.lookupValue(key, boolVal))
        nodeSetting.add(key, Setting::TypeBoolean)=boolVal;
    else
        spin=boolVal;

    intVal=100;
    key="spinTimeout"; 
    if (!nodeSetting.lookupValue(key, intVal)) 
        nodeSetting.add(key, Setting::TypeInt)=intVal;  // GTL possible 64 bit issue here, has Int64 as a type
    else
        spinTimeout=intVal;

    floatVal=5.0; 
    key="spinIncrement"; 
    if (!nodeSetting.lookupValue(key, floatVal))
        nodeSetting.add(key, Setting::TypeFloat)=floatVal;
    else
        spinIncrement=floatVal;

    boolVal=false;
    key="flash"; 
    if (!nodeSetting.lookupValue(key, boolVal))
        nodeSetting.add(key, Setting::TypeBoolean)=boolVal;
    else
        flash=boolVal;

    intVal=500; 
    key="flashInterval"; 
    if (!nodeSetting.lookupValue(key, intVal))
        nodeSetting.add(key, Setting::TypeInt)=intVal;
    else
        flashInterval=intVal;

    floatVal=1.0; 
    key="size"; 
    if (!nodeSetting.lookupValue(key, floatVal))
        nodeSetting.add(key, Setting::TypeFloat)=floatVal;
    else
        size=floatVal;

    SingletonConfig::unlock(); 

    TRACE_EXIT();
    return true; 
}

// static 
string NodeDisplayInfo::labelDefault2String(const NodeDisplayInfo::LabelDefault &labDef)
{
    TRACE_ENTER();

    string retVal;
    switch(labDef)
    {
        case FOUR_OCTETS: retVal="fourOctets"; break;
        case THREE_OCTETS: retVal="threeOctets"; break;
        case TWO_OCTETS: retVal="twoOctets"; break;
        case LAST_OCTET: retVal="lastOctet"; break;
        case HOSTNAME: retVal="hostname"; break;
        case FREE_FORM: retVal=""; break;
    }
    TRACE_EXIT_RET(retVal);
    return retVal;
}


// static 
string NodeDisplayInfo::nodeShapeToString(const NodeDisplayInfo::NodeShape &shape)
{
    TRACE_ENTER();

    string retVal;
    switch(shape) 
    {
        case CIRCLE: retVal="circle"; break;
        case SQUARE: retVal="square"; break;
        case TRIANGLE: retVal="trianle"; break;
        case TORUS: retVal="torus"; break;
        case TEAPOT: retVal="teapot"; break;
    }
    TRACE_EXIT_RET(retVal);
    return retVal;
}

// static 
NodeDisplayInfo::NodeShape NodeDisplayInfo::stringToNodeShape(const string &shape)
{
    TRACE_ENTER();
    NodeShape retVal;
    if (iequals(shape, "circle")) retVal=CIRCLE;
    else if (iequals(shape,"square")) retVal=SQUARE;
    else if (iequals(shape,"triangle")) retVal=TRIANGLE;
    else if (iequals(shape,"torue")) retVal=TORUS;
    else if (iequals(shape,"teapot")) retVal=TEAPOT;
    else
        LOG_ERROR("I don't know what shape " << shape << " represents, guessing circle"); 
    TRACE_EXIT();
    return retVal;
}

void NodeDisplayInfo::saveConfiguration(const string &basePath)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();

    Setting &baseSetting=cfg.lookup(basePath); 
    if (!baseSetting.exists(categoryName))
        baseSetting.add(categoryName, Setting::TypeGroup);

    // "DisplayOptions.layer.[LAYERNAME].node"
    Setting &nodeSetting=cfg.lookup(basePath + string(".") + categoryName);

    nodeSetting["label"]=label;
    nodeSetting["color"]=color.toString(); 
    nodeSetting["shape"]=NodeDisplayInfo::nodeShapeToString(shape);
    nodeSetting["sparkle"]=sparkle;
    nodeSetting["spin"]=spin;
    nodeSetting["spinTimeout"]=spinTimeout;
    nodeSetting["spinIncrement"]=spinIncrement;
    nodeSetting["flash"]=flash;
    nodeSetting["flashInterval"]=(int)flashInterval; // GTL loss of precision here. 
    nodeSetting["size"]=size;

    SingletonConfig::unlock();

    TRACE_EXIT();
}
