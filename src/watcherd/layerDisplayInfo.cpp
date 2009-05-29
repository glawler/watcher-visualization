#include "singletonConfig.h"
#include "layerDisplayInfo.h"

using namespace watcher;
using namespace watcher::event;
using namespace libconfig; 
using namespace std; 

INIT_LOGGER(LayerDisplayInfo, "DisplayInfo.LayerDisplayInfo"); 

LayerDisplayInfo::LayerDisplayInfo() :
    DisplayInfo("layer"), 
    basePath("DisplayOptions"),
    theLayer("unknown")
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
LayerDisplayInfo::~LayerDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool LayerDisplayInfo::loadLayer(const string &layerName_)
{
    TRACE_ENTER();

    // Read in values from cfg or create/set defaults. 
    if (layerName_ != string("default"))
        loadDefault(); 

    theLayer=layerName_;

    Config &cfg=SingletonConfig::instance();
    Setting &root=cfg.getRoot();

    SingletonConfig::lock();

    string path(basePath);                                          
    if (!root.exists(path))
        root.add(path, Setting::TypeGroup); 

    // "DisplayOptions" group
    Setting &dispOptsGroupSetting=cfg.lookup(path);                 
    if (!dispOptsGroupSetting.exists(categoryName))                 
        dispOptsGroupSetting.add(categoryName, Setting::TypeGroup);

    // "DisplayOptions.layer" group
    Setting &categorySetting=cfg.lookup(dispOptsGroupSetting.getPath() + string(".") + categoryName); 
    if (!categorySetting.exists(theLayer))                            
        categorySetting.add(theLayer, Setting::TypeGroup);

    // "DisplayOptions.layer.[LAYERNAME]" group
    Setting &layerSetting=cfg.lookup(categorySetting.getPath() + string(".") + theLayer); 

    SingletonConfig::unlock();

    defaultNodeDisplayInfo.loadLayer(layerSetting.getPath());
    edgeDisplayInfo.loadLayer(layerSetting.getPath());
    labelDisplayInfo.loadLayer(layerSetting.getPath());

    TRACE_EXIT_RET(true);
    return true;
}

bool LayerDisplayInfo::loadDefault()
{
    TRACE_ENTER();
    string defStr("default"); 
    loadLayer(defStr);
    TRACE_EXIT();
    return true; 
}

void LayerDisplayInfo::saveLayerAs(const GUILayer &layer)
{
    TRACE_ENTER();
    string tmp=theLayer;
    theLayer=layer;
    saveLayer(); 
    theLayer=tmp;
    TRACE_EXIT(); 
}

void LayerDisplayInfo::saveLayer()
{
    TRACE_ENTER();
    // "DisplayOptions.layer.[LAYERNAME]"
    string path(basePath + string(".") + categoryName + string(".") + theLayer); 
    defaultNodeDisplayInfo.saveConfiguration(path); 
    edgeDisplayInfo.saveConfiguration(path); 
    labelDisplayInfo.saveConfiguration(path); 
    TRACE_EXIT(); 
}



