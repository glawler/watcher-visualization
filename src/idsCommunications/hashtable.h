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

#ifndef HASHTABLE_H
#define HASHTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*hashtablecompare)(char *k, int keylen, int nbuck);
typedef void (*hashtableitem)(char *k, void *data);

typedef struct _hashbucket
{
	void *data;
	char *key;
	int keylen;
	struct _hashbucket *next;
} hashbucket;

typedef struct _hastable
{
	int nbuck;
	hashtablecompare hashfunc;
	hashbucket **buck;
	
} hashtable;

hashtable *hashtableinit(int nbucket,hashtablecompare hashfunc);
void hashtableinsert(hashtable *t,char *key, int keylen,void *dat);
void *hashtablesearch(hashtable *t,char *key, int keylen);
void hashtabledelete(hashtable *t,char *key, int keylen);
void hashtabletraverse(hashtable *t,hashtableitem itemfunc);
int hashtablehash(char *k, int keylen, int nbuck);
void hashtablefree(hashtable *t);

#ifdef __cplusplus
};
#endif

#endif
