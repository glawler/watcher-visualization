/* Copyright 2012 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @file messageFactory.h
 * @author Geoff Lawler <geoff.lawler@sparta.com> 
 * @date 2009-07-15
 */
#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include "message.h"

namespace watcher {
    namespace event {
		// Factory - create a MessagePtr given a type.
        MessagePtr createMessage(MessageType t); 
    }
}

#endif 
