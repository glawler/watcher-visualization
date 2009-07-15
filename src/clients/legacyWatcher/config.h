/** 
 * @file config.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef CONFIG_H
#define CONFIG_H

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* "$Id: config.h,v 1.13 2007/06/20 14:29:40 dkindred Exp $";
*/
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
