/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/regex.hpp>
#include "singletonConfig.h"
#include "displayInfo.h"
#include "logger.h"

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
    layerCfgId("layer"), 
    layerExisted(false)
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

DisplayInfo::DisplayInfo(const DisplayInfo &copy)
{
    *this=copy;
}

DisplayInfo &DisplayInfo::operator=(const DisplayInfo &lhs)
{
    layer=lhs.layer;
    categoryName=lhs.categoryName;
}

const string DisplayInfo::getBasePath(const GUILayer &layer) const
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    Setting &root=cfg.getRoot();

    string retVal;
    try {

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

        if (!layerCfgSetting.exists(layerName)) {
            layerCfgSetting.add(layerName, Setting::TypeGroup);
            layerExisted=false;
        }
        else 
            layerExisted=true;

        // "DisplayOptions.layer.[LAYERNAME]" group
        Setting &layerSetting=cfg.lookup(layerCfgSetting.getPath() + separator + layerName); 

        // "DisplayOptions.layer.[LAYERNAME].[category] group 
        if (!layerSetting.exists(categoryName))
            layerSetting.add(categoryName, Setting::TypeGroup);
        Setting &categorySetting=cfg.lookup(layerSetting.getPath() + separator + categoryName); 
        retVal=categorySetting.getPath();
    }
    catch (const SettingException &e) {
        LOG_ERROR("Error in configuration setting \"" << e.getPath() << "\"");
    }

    TRACE_EXIT_RET(true);
    return retVal;
}
