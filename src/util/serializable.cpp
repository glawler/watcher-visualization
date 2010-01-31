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
 * @file serializable.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2010-01-31
 */

#include <string.h>     // memcpy
#include <netinet/in.h> // htons, &c.
#include "serializable.h"

using namespace watcher;

//static 
void write8(const uint8_t &value, void *&buffer)
{
	memcpy(buffer, &value, sizeof(uint8_t));
	buffer = reinterpret_cast<uint8_t*>(buffer) + 1; 
}
//static 
void write16(const uint16_t &value, void *&buffer)
{
	uint16_t tmp16 = htons(value); 
	memcpy(buffer, &tmp16, sizeof(uint16_t)); 
	buffer = reinterpret_cast<uint8_t*>(buffer) + 2; 
}
//static 
void write32(const uint32_t &value, void *&buffer)
{
	uint32_t tmp32 = htonl(value); 
	memcpy(buffer, &tmp32, sizeof(uint32_t)); 
	buffer = reinterpret_cast<uint8_t*>(buffer) + 4; 
}

void writeTimeT(time_t &value, void *&buffer)
{
	// machine, compiler, etc dependent representations of time_t shoule go here.
	// for now, we assume linux, long. 
#if 1
	write32(value, buffer); 
#else

#endif 
}

//static 
void read8(uint8_t &value, void *&buffer)
{
	value = *(reinterpret_cast<uint8_t*>(buffer)); 
	buffer = reinterpret_cast<uint8_t*>(buffer) + 1; 
}
//static 
void read16(uint16_t &value, void *&buffer)
{
	value = ntohs(*(reinterpret_cast<uint16_t*>(buffer))); 
	buffer = reinterpret_cast<uint8_t*>(buffer) + 2; 
}
//static 
void read32(uint32_t &value, void *&buffer)
{
	value = htonl(*(reinterpret_cast<uint32_t*>(buffer))); 
	buffer = reinterpret_cast<uint8_t*>(buffer) + 4; 
}

void readTimeT(time_t &value, void *&buffer)
{
	// machine, compiler, etc dependent representations of time_t shoule go here.
	// for now, we assume linux, long. 
#if 1
	uint32_t tmp = value; 
	read32(tmp, buffer); 
	tmp = value; 
#else

#endif 
}

