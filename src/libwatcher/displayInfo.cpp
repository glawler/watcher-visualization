#include <boost/regex.hpp>
#include "singletonConfig.h"
#include "displayInfo.h"

using namespace watcher;
using namespace watcher::event;
using namespace std;
using namespace libconfig; 

INIT_LOGGER(DisplayInfo, "DisplayInfo"); 

DisplayInfo::DisplayInfo(const std::string &category) :
    layer("default"), 
    categoryName(category),
    baseName("displayOptions"), 
    separator("."),
    layerCfgId("layer")
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
DisplayInfo::~DisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

const string DisplayInfo::getBasePath(const GUILayer &layer)
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    Setting &root=cfg.getRoot();

    SingletonConfig::lock();

    if (!root.exists(baseName))
        root.add(baseName, Setting::TypeGroup); 

    // "DisplayOptions" group
    Setting &dispOptsGroupSetting=cfg.lookup(baseName);
    if (!dispOptsGroupSetting.exists(layerCfgId))
        dispOptsGroupSetting.add(layerCfgId, Setting::TypeGroup);

    // "DisplayOptions.layer" group
    Setting &layerCfgSetting=cfg.lookup(dispOptsGroupSetting.getPath() + separator + layerCfgId); 

    // canonicalize the layer so libconfig won't barf. 
    boost::regex rx("[^A-Za-z0-9]"); 
    string replace("_"); 
    string layerName=regex_replace(layer, rx, replace);

    if (!layerCfgSetting.exists(layerName))
        layerCfgSetting.add(layerName, Setting::TypeGroup);

    // "DisplayOptions.layer.[LAYERNAME]" group
    Setting &layerSetting=cfg.lookup(layerCfgSetting.getPath() + separator + layerName); 

    // "DisplayOptions.layer.[LAYERNAME].[category] group 
    if (!layerSetting.exists(categoryName))
        layerSetting.add(categoryName, Setting::TypeGroup);
    Setting &categorySetting=cfg.lookup(layerSetting.getPath() + separator + categoryName); 

    SingletonConfig::unlock();

    TRACE_EXIT_RET(true);
    return categorySetting.getPath(); 
}
