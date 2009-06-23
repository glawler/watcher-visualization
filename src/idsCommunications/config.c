#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "config.h"

/*  Copyright (C) 2005  McAfee Inc. 
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 **  All rights reserved.
 */

typedef struct ConfigNode
{
	char *name;
	char *val;

	struct ConfigNode *next;
} ConfigNode;


struct Config
{
	ConfigNode *list;
	char configDir[PATH_MAX];
};

static const char *rcsid __attribute__ ((unused)) = "$Id: config.c,v 1.24 2007/07/18 15:35:16 mheyman Exp $";

Config *configLoad(const char *fname)
{
	Config *conf;
	ConfigNode *cn;
	char line[8192];
	FILE *fil;
	char *pos,*epos;
	int lnum = 0;

	if (fname==NULL)
		fname = COMMUNICATIONS_CONFIG_DIR "/live.conf";

	fil=fopen(fname,"r");
	if (fil==NULL)
    {
        fprintf(stderr, 
                "Failed to open config file \"%s\" for reading. "
                "%s(%d)\n",
                fname, strerror(errno), errno);
		return NULL;
    }

	conf=(Config*)malloc(sizeof(*conf));
	conf->list=NULL;

	{
		size_t len;
		snprintf(conf->configDir, sizeof(conf->configDir), ".");
		/* assumes unix directory separator */
		if ((pos = strrchr(fname, '/')) != NULL)
		{
			len = pos - fname;
			if (len < sizeof(conf->configDir))
			{
				snprintf(conf->configDir, 
					 sizeof(conf->configDir), 
					 "%.*s", len, fname);
			}
		}
	}

	while(fgets(line,sizeof(line),fil))
	{
		lnum++;
		if (line[0]=='#')
			continue;
		
		for (pos=line; isspace(*pos); pos++)
			;
		if (*pos=='\0')
		{
			/* blank line */
			continue;
		}

		pos=strchr(line,':');
		if (pos==NULL)
		{
			fprintf(stderr, "%s:%d: malformed line (no \":\"): %s\n",
				fname, lnum, line);
			configFree(conf);
			return NULL;
		}
		cn=(ConfigNode*)malloc(sizeof(*cn)+strlen(line));
		*pos=0;
		pos++;
		while(isspace(*pos))
			pos++;
		epos=pos+strlen(pos)-1;
		while((*epos=='\r') || (*epos=='\n'))
		{
			*epos=0;
			epos--;
		}

		cn->name=(char*)cn+sizeof(*cn);
		cn->val=(char*)cn+sizeof(*cn)+strlen(line)+1;
		strcpy(cn->name,line);
		strcpy(cn->val,pos);

		/* XXX if name starts with space, either strip space or fail */
		/* XXX fail/warn on duplicate name?  (as is, last instance will take precedence) */

		cn->next=conf->list;
		conf->list=cn;
	}
	fclose(fil);
	return conf;
}

static ConfigNode *confignodeSearch(ConfigNode *c, char const *name)
{
	while((c) && (strcmp(c->name, name)))
	{
		c = c->next;
	}
	return c;
}

char const *configSearchStr(Config const *conf, char const *name)
{
	ConfigNode *cn = conf ? confignodeSearch(conf->list, name) : 0;
	return cn ? cn->val : 0;
}

/* return 0 on success */
int configGetPathname(Config const *conf, const char *path, 
                      char *buf, int buflen)
{
	int rv;
	if (path == NULL) return -1;
	if (path[0] == '/')
	{
		rv = snprintf(buf, buflen, "%s", path);
	}
	else
	{
		rv = snprintf(buf, buflen, "%s/%s", conf->configDir, path);
	}
	return (rv >= 0 && rv < buflen) ? 0 : -1;
}

int configSetInt(Config const *conf, char const *var, int def)
{
	ConfigNode *cn = conf ? confignodeSearch(conf->list, var) : 0;
	int i;
	return cn ? (sscanf(cn->val, "%d", &i), i) : def;
}

double configSetDouble(Config const *conf, char const *var, double def)
{
	ConfigNode *cn = conf ? confignodeSearch(conf->list, var) : 0;
	double d;
	return cn ? (sscanf(cn->val, "%lf", &d), d) : def;
}

int configSearchInt(Config const *conf, char const *name)
{
	return configSetInt(conf, name, 0);
}

double configSearchDouble(Config const *conf, char const *name)
{
	return configSetDouble(conf, name, 0);
}

void configFree(Config *conf)
{
	ConfigNode *cn,*d;

	cn=conf->list;
	while(cn)
	{
		d=cn;
		cn=cn->next;

		free(d);
	}
	free(conf);
}
