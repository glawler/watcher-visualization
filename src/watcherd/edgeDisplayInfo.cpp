#include "edgeDisplayInfo.h"
#include "singletonConfig.h" 
#include "libwatcher/messageTypesAndVersions.h"

using namespace watcher;
using namespace watcher::event;
using namespace libconfig; 
using namespace std;

INIT_LOGGER(EdgeDisplayInfo, "DisplayInfo.EdgeDisplayInfo"); 

EdgeDisplayInfo::EdgeDisplayInfo() :
    DisplayInfo("edge"),
    color(Color::blue),
    width(30),
    flash(false), 
    flashInterval(500),
    isFlashed(false),
    label("none")
    
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
EdgeDisplayInfo::~EdgeDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool EdgeDisplayInfo::loadLayer(const string &basePath)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();

    Setting &baseSetting=cfg.lookup(basePath); 
    if (!baseSetting.exists(categoryName))
        baseSetting.add(categoryName, Setting::TypeGroup);

    // "DisplayOptions.layer.default.edge"
    Setting &edgeSetting=cfg.lookup(basePath + string(".") + categoryName);

    string strVal="none"; 
    string key="label"; 
    if (!edgeSetting.lookupValue(key, strVal))
        edgeSetting.add(key, Setting::TypeString)=strVal;
    label=strVal;

    strVal=Color::blue.toString(); 
    key="color"; 
    if (!edgeSetting.lookupValue(key, strVal))
        edgeSetting.add(key, Setting::TypeString)=strVal;
    color.fromString(strVal); 

    int intVal=15;
    key="width"; 
    if (!edgeSetting.lookupValue(key, intVal))
        edgeSetting.add(key, Setting::TypeInt)=intVal;
    width=intVal;

    bool boolVal=false;
    key="flash"; 
    if (!edgeSetting.lookupValue(key, boolVal))
        edgeSetting.add(key, Setting::TypeBoolean)=boolVal;
    flash=boolVal;

    intVal=100;
    key="flashInterval"; 
    if (!edgeSetting.lookupValue(key, intVal))
        edgeSetting.add(key, Setting::TypeInt)=intVal;
    flashInterval=intVal;

    SingletonConfig::unlock();

    TRACE_EXIT(); 
    return true; 
}
    
void EdgeDisplayInfo::saveConfiguration(const string &basePath)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();

    Setting &baseSetting=cfg.lookup(basePath); 
    if (!baseSetting.exists(categoryName))
        baseSetting.add(categoryName, Setting::TypeGroup);

    // "DisplayOptions.layer.[LAYERNAME].edge"
    Setting &edgeSetting=cfg.lookup(basePath + string(".") + categoryName);

    edgeSetting["color"]=color.toString(); 
    edgeSetting["width"]=width; 
    edgeSetting["flash"]=flash; 
    edgeSetting["flashInterval"]=(int)flashInterval;
    edgeSetting["label"]=label; 

    SingletonConfig::unlock();

    TRACE_EXIT();
}
