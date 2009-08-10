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

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMMUNICATIONS_CONFIG_DIR
#define COMMUNICATIONS_CONFIG_DIR "./"
#endif

typedef struct Config Config;

/* This is a simple configuration file object.
 *
 * It reads a file with the form
 *    foo:bar
 *    #comment
 *
 * foo is the name, and bar is the value.  The values can then
 * be looked up by name using configSearch*()
 */

Config *configLoad(const char *fname);

char const *configSearchStr(Config const *conf, char const *name);
int configSearchInt(Config const *conf, char const *name);
double configSearchDouble(Config const *conf, char const *name);

/* Expand the given pathname relative to the config directory and
 * write to buf.  Returns 0 on success. */
int configGetPathname(Config const *conf, const char *path, 
                      char *buf, int buflen);

/* Search for and return a value, with a default
 * to return if its not there
 */
int configSetInt(Config const *conf, char const *var, int def);
double configSetDouble(Config const *conf, char const *var, double def);

void configFree(Config *conf);

#ifdef __cplusplus
};
#endif

#endif
