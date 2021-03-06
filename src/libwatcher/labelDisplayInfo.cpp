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

#include "singletonConfig.h"
#include "labelDisplayInfo.h"
#include "logger.h"

using namespace watcher;
using namespace watcher::event;
using namespace watcher::colors;
using namespace libconfig;
using namespace std;

INIT_LOGGER(LabelDisplayInfo, "DisplayInfo.LabelDisplayInfo"); 

LabelDisplayInfo::LabelDisplayInfo() : 
    DisplayInfo("label"),
    backgroundColor(black),
    foregroundColor(white),
    fontName("Times New Roman"),
    pointSize(20), 
    labelText(""),
    expiration(Infinity) 
{
    TRACE_ENTER();

    // create the "default" layer if it's not already there.
    loadConfiguration(layer); 

    TRACE_EXIT();
}

LabelDisplayInfo::LabelDisplayInfo(const std::string &label) : 
    DisplayInfo("label"),
    backgroundColor(black),
    foregroundColor(white),
    fontName("Times New Roman"),
    pointSize(20), 
    labelText(label),
    expiration(Infinity) 
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

LabelDisplayInfo::LabelDisplayInfo(const LabelDisplayInfo &copy) : DisplayInfo(copy)
{
    *this=copy;
}
LabelDisplayInfo &LabelDisplayInfo::operator=(const LabelDisplayInfo &rhs)
{
    DisplayInfo::operator=(rhs); 
    backgroundColor=rhs.backgroundColor;
    foregroundColor=rhs.foregroundColor;
    fontName=rhs.fontName; 
    pointSize=rhs.pointSize;
    labelText=rhs.labelText;
    expiration=rhs.expiration;
}
void LabelDisplayInfo::initialize(const watcher::event::LabelMessagePtr &m)
{
    // Commented out as we want to use the cfg settings fir these fields. 
    // backgroundColor=m->background;
    // foregroundColor=m->foreground;
    // pointSize=m->fontSize?m->fontSize:pointSize;
    labelText=m->label;
    expiration=m->expiration;
}
bool LabelDisplayInfo::loadConfiguration(const GUILayer &layer_)
{
    TRACE_ENTER();

    layer=layer_.size()==0 ? PHYSICAL_LAYER : layer_; 

    Config &cfg=SingletonConfig::instance();

    Setting &labelSettings=cfg.lookup(getBasePath(layer)); 

    try {

        string strVal;
        string key="foregroundColor"; 
        if (!labelSettings.lookupValue(key, strVal))
            labelSettings.add(key, Setting::TypeString)=foregroundColor.toString();
        foregroundColor.fromString(strVal); 

        key="backgroundColor"; 
        if (!labelSettings.lookupValue(key, strVal))
            labelSettings.add(key, Setting::TypeString)=backgroundColor.toString();
        else
            backgroundColor.fromString(strVal); 

        key="font"; 
        if (!labelSettings.lookupValue(key, strVal))
            labelSettings.add(key, Setting::TypeString)=fontName;
        else
            fontName=strVal;

        float floatVal;
        key="pointSize"; 
        if (!labelSettings.lookupValue(key, floatVal))
            labelSettings.add(key, Setting::TypeFloat)=pointSize;
        else
            pointSize=floatVal; 

        // strVal="none"; 
        // key="labelText"; 
        // if (!labelSettings.lookupValue(key, strVal))
        //     labelSettings.add(key, Setting::TypeString)=strVal;
        // labelText=strVal;

    }
    catch (const SettingException &e) {
        LOG_ERROR("Error in configuration setting \"" << e.getPath() << "\"");
    }

    LOG_DEBUG("loaded label display information from cfg for layer " << layer << ":"); 
    LOG_DEBUG(
        "     foregroundColor: " << foregroundColor.toString() <<
        "     backgroundColor: " << backgroundColor.toString() <<
        "     fontName: " << fontName <<
        "     pointSize: " << pointSize); 

    TRACE_EXIT();
    return true; 
}

void LabelDisplayInfo::saveConfiguration() const
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();
    Setting &labelSetting=cfg.lookup(getBasePath(layer)); 

    try {
        labelSetting["backgroundColor"]=backgroundColor.toString(); 
        labelSetting["foregroundColor"]=foregroundColor.toString(); 
        labelSetting["font"]=fontName;
        labelSetting["pointSize"]=pointSize;
    }
    catch (const SettingException &e) {
        LOG_ERROR("Error saving configuration at " << e.getPath() << ": " << e.what() << "  " << __FILE__ << ":" << __LINE__);
    }

    TRACE_EXIT();
}

// virtual 
ostream &LabelDisplayInfo::toStream(ostream &out) const
{
    TRACE_ENTER();
    out << " text: \"" << labelText << "\" point: " << pointSize
        << " bg: " << backgroundColor 
        << " fg: " << foregroundColor 
        << " exp: " << expiration;
    TRACE_EXIT();
    return out; 
}

bool LabelDisplayInfo::operator==(const LabelDisplayInfo &other)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return labelText==other.labelText && backgroundColor==other.backgroundColor && foregroundColor==other.foregroundColor; 
}

bool LabelDisplayInfo::operator<(const LabelDisplayInfo &lhs) const
{
    return labelText<lhs.labelText;
}

bool LabelDisplayInfo::loadConfiguration(const LabelMessagePtr &mess)
{
    TRACE_ENTER();

    if (mess->expiration!=Infinity) {
        expiration=mess->timestamp+mess->expiration;
        LOG_DEBUG(" New label expires at " << mess->expiration << " ms: " << *this);
    }
    else
        expiration=Infinity;

    backgroundColor=mess->background;
    foregroundColor=mess->foreground;
    // no fontName in labelMessage? 
    if (mess->fontSize)
        pointSize=mess->fontSize; 
    layer=mess->layer; 
    labelText=mess->label;

    // Overwrite given config with local file config
    loadConfiguration(mess->layer); 

    TRACE_EXIT_RET_BOOL(true); 
    return true; 
}

ostream& watcher::operator<<(ostream &out, const LabelDisplayInfo &obj)
{
    obj.operator<<(out);
    return out;
}
