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
    labelText(""),
    expiration(Infinity) 
{
    TRACE_ENTER();

    // create the "default" layer if it's not already there.
    loadConfiguration(layer); 

    TRACE_EXIT();
}

// virtual
LabelDisplayInfo::~LabelDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}


bool LabelDisplayInfo::loadConfiguration(const GUILayer &layer_)
{
    TRACE_ENTER();

    layer=layer_.size()==0 ? PHYSICAL_LAYER : layer_; 

    Config &cfg=SingletonConfig::instance();

    Setting &labelSettings=cfg.lookup(getBasePath(layer)); 

    SingletonConfig::lock();

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

    // strVal="none"; 
    // key="labelText"; 
    // if (!labelSettings.lookupValue(key, strVal))
    //     labelSettings.add(key, Setting::TypeString)=strVal;
    // labelText=strVal;

    SingletonConfig::unlock(); 

    TRACE_EXIT();
    return true; 
}

void LabelDisplayInfo::saveConfiguration()
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    Setting &labelSetting=cfg.lookup(getBasePath(layer)); 

    SingletonConfig::lock();

    labelSetting["backgroundColor"]=backgroundColor.toString(); 
    labelSetting["foregroundColor"]=foregroundColor.toString(); 
    labelSetting["font"]=fontName;
    labelSetting["pointSize"]=pointSize;

    SingletonConfig::unlock();

    TRACE_EXIT();
}

// virtual 
ostream &LabelDisplayInfo::toStream(ostream &out) const
{
    TRACE_ENTER();
    out << " text: \"" << labelText << "\" point: " << pointSize
        << " bg: " << backgroundColor 
        << " fg: " << foregroundColor; 
    TRACE_EXIT();
    return out; 
}

bool LabelDisplayInfo::loadConfiguration(const LabelMessagePtr &mess)
{
    TRACE_ENTER();

    if (mess->expiration!=Infinity)
        expiration=(Timestamp(time(NULL))*1000)+mess->expiration;
    else
        expiration=Infinity;

    // load defaults, then modify 'em 
    loadConfiguration(mess->layer); 

    backgroundColor=mess->background;
    foregroundColor=mess->foreground;
    pointSize=mess->fontSize; 
    layer=mess->layer; 
    labelText=mess->label;

    TRACE_EXIT_RET_BOOL(true); 
    return true; 
}

ostream& watcher::operator<<(ostream &out, const LabelDisplayInfo &obj)
{
    TRACE_ENTER();
    obj.operator<<(out);
    TRACE_EXIT();
    return out;
}
