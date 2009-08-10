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

#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define MALLOC malloc
#define FREE free

static const char *rcsid __attribute__ ((unused)) = "$Id: hashtable.c,v 1.6 2007/06/27 22:08:47 mheyman Exp $";

hashtable *hashtableinit(int nbucket,hashtablecompare hashfunc)
{
	int i;
	hashtable *t = (hashtable *)MALLOC(sizeof(hashtable));
	if (!t)
		return NULL;
	t->buck=(hashbucket**)MALLOC(sizeof(hashbucket*)*nbucket);
	if (!t->buck)
	{
		FREE(t);
		return NULL;
	}
	for(i=0;i<nbucket;i++)
		t->buck[i]=NULL;
	t->nbuck=nbucket;
	t->hashfunc=hashfunc;
	return t;
}

void hashtablefree(hashtable *t)
{
	int i;
	hashbucket *b,*c;

	if (t==NULL)
		return;

	for(i=0;i<t->nbuck;i++)
	{
		b=t->buck[i];
		while(b)
		{
			c=b->next;
			FREE(b->key);
			FREE(b);
			b=c;
		}
	}
	FREE(t->buck);
	FREE(t);
}

void hashtableinsert(hashtable *t,char *key, int keylen,void *dat)
{
	hashbucket *b;
	int hkey;

	if (t==NULL)
		return;

	hkey=(t->hashfunc)(key, keylen, t->nbuck);
	b=t->buck[hkey];

	while(b && (b->keylen==keylen) && (memcmp(b->key,key,keylen)))
		b=b->next;

	if (b)
		b->data=dat;      /* if key is in already, update data */
	else
	{
		b=(hashbucket*)MALLOC(sizeof(hashbucket)+keylen);  /* otherwise insert it... */
		b->key=((char *)b)+sizeof(hashbucket);
		memcpy(b->key,key,keylen);
		b->keylen=keylen;
		b->data=dat;
		b->next=t->buck[hkey];
		t->buck[hkey]=b;
	}
}

void *hashtablesearch(hashtable *t,char *key,int keylen)
{
	hashbucket *b;
	int hkey;

	if (t==NULL)
		return NULL;

	hkey=(t->hashfunc)(key,keylen,t->nbuck);
	b=t->buck[hkey];

	while(b)
	{
		if ((b->keylen==keylen) && (memcmp(b->key,key,keylen)==0))
			return b->data;
		b=b->next;
	}
	return NULL;
}

void hashtabledelete(hashtable *t,char *key, int keylen)
{
	hashbucket *b,*d;
	int hkey;

	if (t==NULL)
		return;

	hkey=(t->hashfunc)(key,keylen,t->nbuck);
	b=t->buck[hkey];
	d=NULL;

	while(b && !((b->keylen==keylen ) && (memcmp(b->key,key,keylen)==0)))
	{
		d=b;
		b=b->next;
	}
	if (!b)
		return;

	if (!d)
		t->buck[hkey]=b->next;
	else
		d->next=b->next;
	FREE(b);
}

void hashtabletraverse(hashtable *t,hashtableitem itemfunc)
{
	int i;
	hashbucket *b,*nxt;

	if (t==NULL)
		return;

	for(i=0;i<t->nbuck;i++)
	{
		b=t->buck[i];
		while(b)
		{
			nxt=b->next;
			(itemfunc)(b->key,b->data);
			b=nxt;
		}
	}
}

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

static ub4 hash(ub1 *k,ub4 length,ub4 initval)
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((ub4)k[10]<<24);
   case 10: c+=((ub4)k[9]<<16);
   case 9 : c+=((ub4)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((ub4)k[7]<<24);
   case 7 : b+=((ub4)k[6]<<16);
   case 6 : b+=((ub4)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((ub4)k[3]<<24);
   case 3 : a+=((ub4)k[2]<<16);
   case 2 : a+=((ub4)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}




int hashtablehash(char *k, int keylen, int nbuck)
{
	unsigned int t=0;

	t=hash((ub1*)k,keylen,0xDEADBEEF);

	return t % nbuck;
}
