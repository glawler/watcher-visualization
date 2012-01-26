/* Copyright 2012 SPARTA, Inc.
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
 * @file marshalYAML.h
 * @author Geoff Lawler <geoff.lawer@sparta.com>
 * @date 2012-19-01
 */
#ifndef YAML_MARSHALLING_H
#define YAML_MARSHALLING_H

/* @file
 * @author
 */

// generic STL stream interface. 
#include <ostream>

// yaml-cpp library interface. 
#include <yaml.h>
#include "watcherColors.h" 
#include "watcherTypes.h"

/** Global overloaded input/output operators for some watcher types */

// NOTE: assumes the encoding happens in the 
// middle of a YAML::MapSeq. 
YAML::Emitter &operator<<(YAML::Emitter &out, const watcher::Color &c); 
void operator>>(YAML::Parser& in, watcher::Color &c); 

// Encode a NodeIdentifier. 
// NOTE: assumes the encoding happens in the 
// middle of a YAML::MapSeq. 
YAML::Emitter &operator<<(YAML::Emitter &out, const watcher::NodeIdentifier &nid); 
void operator>>(YAML::Parser& in, watcher::NodeIdentifier &nid); 

#endif //  YAML_MARSHALLING_H
