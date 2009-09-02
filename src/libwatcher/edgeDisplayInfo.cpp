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

    string strVal="none"; 
    string key="label"; 
    if (!edgeSetting.lookupValue(key, strVal))
        edgeSetting.add(key, Setting::TypeString)=strVal;
    label=strVal;

    strVal=blue.toString(); 
    key="color"; 
    if (!edgeSetting.lookupValue(key, strVal))
        edgeSetting.add(key, Setting::TypeString)=strVal;
    color.fromString(strVal); 

    float floatVal=15;
    key="width"; 
    if (!edgeSetting.lookupValue(key, floatVal))
        edgeSetting.add(key, Setting::TypeFloat)=floatVal;
    width=floatVal;

    bool boolVal=false;
    key="flash"; 
    if (!edgeSetting.lookupValue(key, boolVal))
        edgeSetting.add(key, Setting::TypeBoolean)=boolVal;
    flash=boolVal;

    int intVal=100;
    key="flashInterval"; 
    if (!edgeSetting.lookupValue(key, intVal))
        edgeSetting.add(key, Setting::TypeInt)=intVal;
    flashInterval=intVal;

    strVal=labelFont;
    key="labelFont";
    if (!edgeSetting.lookupValue(key, strVal))
        edgeSetting.add(key, Setting::TypeString)=strVal;
    labelFont=strVal;

    floatVal=labelPointSize;
    key="labelPointSize"; 
    if (!edgeSetting.lookupValue(key, floatVal))
        edgeSetting.add(key, Setting::TypeFloat)=floatVal;
    labelPointSize=floatVal;

    strVal=blue.toString(); 
    key="labelColor"; 
    if (!edgeSetting.lookupValue(key, strVal))
        edgeSetting.add(key, Setting::TypeString)=strVal;
    labelColor.fromString(strVal); 

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

    edgeSetting["color"]=color.toString(); 
    edgeSetting["width"]=width; 
    edgeSetting["flash"]=flash; 
    edgeSetting["flashInterval"]=(int)flashInterval;
    edgeSetting["label"]=label; 

    SingletonConfig::unlock();

    TRACE_EXIT();
}
