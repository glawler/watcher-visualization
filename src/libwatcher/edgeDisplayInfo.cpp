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

#include "edgeDisplayInfo.h"
#include "singletonConfig.h" 
#include "colors.h"
#include "messageTypesAndVersions.h"

using namespace watcher;
using namespace watcher::event;
using namespace watcher::colors;
using namespace libconfig; 
using namespace std;

INIT_LOGGER(EdgeDisplayInfo, "DisplayInfo.EdgeDisplayInfo"); 

EdgeDisplayInfo::EdgeDisplayInfo() :
    DisplayInfo("edge"),
    color(blue),
    width(30),
    flash(false), 
    flashInterval(500),
    nextFlashUpdate(0), 
    isFlashed(false),
    label("none"),
    labelFont("Helvetica"),
    labelPointSize(12.5),
    labelColor(blue)
{
    TRACE_ENTER();

    // create the "default" layer if it's not already there.
    loadConfiguration(layer); 

    TRACE_EXIT();
}

// virtual
EdgeDisplayInfo::~EdgeDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool EdgeDisplayInfo::loadConfiguration(const GUILayer &layer_)
{
    TRACE_ENTER();

    layer=layer_.size()==0 ? PHYSICAL_LAYER : layer_; 

    Config &cfg=SingletonConfig::instance();

    Setting &edgeSetting=cfg.lookup(getBasePath(layer)); 

    SingletonConfig::lock();

    try {

        string key="label"; 
        if (!edgeSetting.lookupValue(key, label))
            edgeSetting.add(key, Setting::TypeString)=label;

        string strVal=color.toString(); 
        key="color"; 
        if (!edgeSetting.lookupValue(key, strVal))
            edgeSetting.add(key, Setting::TypeString)=strVal;
        color.fromString(strVal); 

        key="width"; 
        if (!edgeSetting.lookupValue(key, width))
            edgeSetting.add(key, Setting::TypeFloat)=width;

        key="flash"; 
        if (!edgeSetting.lookupValue(key, flash))
            edgeSetting.add(key, Setting::TypeBoolean)=flash;

        int intVal=flashInterval;
        key="flashInterval"; 
        if (!edgeSetting.lookupValue(key, intVal))
            edgeSetting.add(key, Setting::TypeInt)=intVal;
        flashInterval=intVal;

        key="labelFont";
        if (!edgeSetting.lookupValue(key, labelFont))
            edgeSetting.add(key, Setting::TypeString)=labelFont;

        key="labelPointSize"; 
        if (!edgeSetting.lookupValue(key, labelPointSize))
            edgeSetting.add(key, Setting::TypeFloat)=labelPointSize;

        strVal=labelColor.toString(); 
        key="labelColor"; 
        if (!edgeSetting.lookupValue(key, strVal))
            edgeSetting.add(key, Setting::TypeString)=strVal;
        labelColor.fromString(strVal); 
    }
    catch (const SettingException &e) {
        LOG_ERROR("Error in configuration setting \"" << e.getPath() << "\"");
    }

    SingletonConfig::unlock();

    TRACE_EXIT(); 
    return true; 
}
    
void EdgeDisplayInfo::saveConfiguration()
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    Setting &edgeSetting=cfg.lookup(getBasePath(layer)); 

    SingletonConfig::lock();

    try {
        edgeSetting["color"]=color.toString(); 
        edgeSetting["width"]=width; 
        edgeSetting["flash"]=flash; 
        edgeSetting["flashInterval"]=(int)flashInterval;
        edgeSetting["label"]=label; 
    }
    catch (const SettingException &e) {
        LOG_ERROR("Error in configuration setting \"" << e.getPath() << "\"");
    }

    SingletonConfig::unlock();

    TRACE_EXIT();
}
