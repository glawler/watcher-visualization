#include "singletonConfig.h"
#include "labelDisplayInfo.h"

using namespace watcher;
using namespace watcher::event;
using namespace libconfig;
using namespace std;

INIT_LOGGER(LabelDisplayInfo, "DisplayInfo.LabelDisplayInfo"); 

LabelDisplayInfo::LabelDisplayInfo() : 
    DisplayInfo("label"),
    backgroundColor(Color::white),
    foregroundColor(Color::black),
    fontName("Times New Roman"),
    pointSize(12.5), 
    labelText("")
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
LabelDisplayInfo::~LabelDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}


bool LabelDisplayInfo::loadLayer(const string &basePath)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();

    Setting &baseSetting=cfg.lookup(basePath); 

    if (!baseSetting.exists(categoryName))
        baseSetting.add(categoryName, Setting::TypeGroup);

    Setting &labelSettings=cfg.lookup(basePath + string(".") + categoryName); 

    string strVal=Color::white.toString(); 
    string key="foregroundColor"; 
    if (!labelSettings.lookupValue(key, strVal))
        labelSettings.add(key, Setting::TypeString)=strVal;
    foregroundColor.fromString(strVal); 

    strVal=Color::black.toString(); 
    key="backgroundColor"; 
    if (!labelSettings.lookupValue(key, strVal))
        labelSettings.add(key, Setting::TypeString)=strVal;
    backgroundColor.fromString(strVal); 

    strVal="Times New Roman"; 
    key="font"; 
    if (!labelSettings.lookupValue(key, strVal))
        labelSettings.add(key, Setting::TypeString)=strVal;
    fontName=strVal;

    float floatVal=12.5;
    key="pointSize"; 
    if (!labelSettings.lookupValue(key, floatVal))
        labelSettings.add(key, Setting::TypeFloat)=floatVal;
    pointSize=floatVal; 

    strVal="none"; 
    key="labelText"; 
    if (!labelSettings.lookupValue(key, strVal))
        labelSettings.add(key, Setting::TypeString)=strVal;
    labelText=strVal;

    SingletonConfig::unlock(); 

    TRACE_EXIT();
    return true; 
}

void LabelDisplayInfo::saveConfiguration(const string &basePath)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();

    Setting &baseSetting=cfg.lookup(basePath); 
    if (!baseSetting.exists(categoryName))
        baseSetting.add(categoryName, Setting::TypeGroup);

    // "DisplayOptions.layer.[LAYERNAME].edge"
    Setting &labelSetting=cfg.lookup(basePath + string(".") + categoryName);

    labelSetting["backgroundColor"]=backgroundColor.toString(); 
    labelSetting["foregroundColor"]=foregroundColor.toString(); 
    labelSetting["font"]=fontName;
    labelSetting["pointSize"]=pointSize;
    labelSetting["labelText"]=labelText;

    SingletonConfig::unlock();

    TRACE_EXIT();
}
