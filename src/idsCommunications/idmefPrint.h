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

/*
 * idmefPrint.h
 */
#ifndef IDMEFPRINT_H_FILE
#define IDMEFPRINT_H_FILE

#include <stdio.h> /* for FILE */
#include <libxml/tree.h> /* for xmlDocPtr */

#ifdef __cplusplus
# define EXTERNC extern "C"
#else
# define EXTERNC
#endif

EXTERNC void idmefPrint(FILE *fil, xmlDocPtr messagedoc);

#endif /* IDMEFPRINT_H_FILE */
