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

/**
 * @file nodeDisplayInfo.cpp
 * @author Geoff Lawler <Geoff.Lawler@cobham.com>
 * @date 2009-01-27
 */
#include <boost/foreach.hpp>
#include "singletonConfig.h"
#include "nodeDisplayInfo.h"
#include "colors.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace watcher::colors;
using namespace libconfig;
using namespace boost; 

INIT_LOGGER(NodeDisplayInfo, "DisplayInfo.NodeDisplayInfo"); 

NodeDisplayInfo::NodeDisplayInfo() : 
    DisplayInfo("node"), 
    shape(NodePropertiesMessage::CIRCLE),
    nodeProperties(),
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
    nextFlashUpdate(0),
    isFlashed(false),
    label(NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::LAST_OCTET)),
    labelFont("Helvetica"), 
    labelPointSize(12.5),
    labelColor(blue),
    color(red)
{
    TRACE_ENTER();

    // create the "default" layer if it's not already there.
    loadConfiguration(layer); 

    TRACE_EXIT();
}

// virtual
NodeDisplayInfo::~NodeDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool NodeDisplayInfo::loadConfiguration(const GUILayer &layer_, const NodeIdentifier &nid)
{
    TRACE_ENTER();

    // Convert nodeId into stupid string that libconfig can understand. 
    // libconfig path ids cannot start with numbers or contain '.'
    // which is really annoying. So an id for node 192.168.1.123 ends up 
    // looking like "node_192_168_1_123". Sigh.
    string nodeIdAsConfigId(categoryName);      // default node config is just under this. 
    if (nid!=NodeIdentifier()) 
    {
        nodeId=nid; 
        nodeIdAsConfigId+="_";
        string nodeIdAsString(nid.to_string()); 
        replace(nodeIdAsString.begin(), nodeIdAsString.end(), '.', '_'); 
        nodeIdAsConfigId+=nodeIdAsString; 
    }

    LOG_DEBUG("Looking up and loading node props for node with id: \"" << nodeIdAsConfigId << "\"");
    categoryName=nodeIdAsConfigId; 

    bool retVal=loadConfiguration(layer_); 

    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

// virtual
bool NodeDisplayInfo::loadConfiguration(const GUILayer &layer_)
{
    TRACE_ENTER();

    layer=layer_.size()==0 ? PHYSICAL_LAYER : layer_; 

    Config &cfg=SingletonConfig::instance();

    // "DisplayOptions.layer.[LAYERNAME].node_123_123_123_123"
    Setting &nodeSetting=cfg.lookup(getBasePath(layer)); 

    SingletonConfig::lock();

    string strVal, key;

    key="label"; 
    if (!nodeSetting.lookupValue(key, label))
        nodeSetting.add(key, Setting::TypeString)=label;

    strVal=color.toString();
    key="color"; 
    if (!nodeSetting.lookupValue(key, strVal))
        nodeSetting.add(key, Setting::TypeString)=strVal;
    color.fromString(strVal); 

    strVal=NodePropertiesMessage::nodeShapeToString(NodePropertiesMessage::CIRCLE);
    key="shape"; 
    if (!nodeSetting.lookupValue(key, strVal))
        nodeSetting.add(key, Setting::TypeString)=strVal;
    NodePropertiesMessage::stringToNodeShape(strVal, shape);

    key="sparkle"; 
    if (!nodeSetting.lookupValue(key, sparkle))
        nodeSetting.add(key, Setting::TypeBoolean)=sparkle;

    key="spin"; 
    if (!nodeSetting.lookupValue(key, spin))
        nodeSetting.add(key, Setting::TypeBoolean)=spin;

    key="spinTimeout"; 
    if (!nodeSetting.lookupValue(key, spinTimeout)) 
        nodeSetting.add(key, Setting::TypeInt)=spinTimeout; 

    key="spinIncrement"; 
    if (!nodeSetting.lookupValue(key, spinIncrement))
        nodeSetting.add(key, Setting::TypeFloat)=spinIncrement;

    key="flash"; 
    if (!nodeSetting.lookupValue(key, flash))
        nodeSetting.add(key, Setting::TypeBoolean)=flash;

    int intVal=(int)flashInterval;
    key="flashInterval"; 
    if (!nodeSetting.lookupValue(key, intVal))
        nodeSetting.add(key, Setting::TypeInt)=intVal;
    flashInterval=intVal;

    key="size"; 
    if (!nodeSetting.lookupValue(key, size))
        nodeSetting.add(key, Setting::TypeFloat)=size;

    key="labelFont";
    if (!nodeSetting.lookupValue(key, labelFont))
        nodeSetting.add(key, Setting::TypeString)=labelFont;

    key="labelPointSize"; 
    if (!nodeSetting.lookupValue(key, labelPointSize))
        nodeSetting.add(key, Setting::TypeFloat)=labelPointSize;

    strVal=blue.toString(); 
    key="labelColor"; 
    if (!nodeSetting.lookupValue(key, strVal))
        nodeSetting.add(key, Setting::TypeString)=strVal;
    labelColor.fromString(strVal); 

    SingletonConfig::unlock(); 

    TRACE_EXIT_RET_BOOL(true); 
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

void NodeDisplayInfo::saveConfiguration()
{
    TRACE_ENTER();

    Config &cfg=SingletonConfig::instance();

    // "DisplayOptions.layer.[LAYERNAME].node"
    string basePath=getBasePath(layer); 
    Setting &nodeSetting=cfg.lookup(basePath); 

    SingletonConfig::lock();

    nodeSetting["label"]=label;
    nodeSetting["color"]=color.toString(); 
    nodeSetting["shape"]=NodePropertiesMessage::nodeShapeToString(shape);
    nodeSetting["sparkle"]=sparkle;
    nodeSetting["spin"]=spin;
    nodeSetting["spinTimeout"]=spinTimeout;
    nodeSetting["spinIncrement"]=spinIncrement;
    nodeSetting["flash"]=flash;
    nodeSetting["flashInterval"]=(int)flashInterval; // GTL loss of precision here. 
    nodeSetting["size"]=size;

    SingletonConfig::unlock();
}

