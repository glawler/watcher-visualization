#ifndef MARSHAL_H
#define MARSHAL_H

#include <string.h>

/* $Id: marshal.h,v 1.12 2007/03/30 15:13:38 dkindred Exp $
 *
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 * 
 * convenience macros for marshaling and unmarshaling stuff
 */

#define MARSHALLONGLONG(a,b) \
	do\
	{\
		*a++=((unsigned char)((b) >> 56)); \
		*a++=((unsigned char)((b) >> 48)); \
		*a++=((unsigned char)((b) >> 40)); \
		*a++=((unsigned char)((b) >> 32)); \
		*a++=((unsigned char)((b) >> 24)); \
		*a++=((unsigned char)((b) >> 16)); \
		*a++=((unsigned char)((b) >> 8)); \
		*a++=((unsigned char)(b)); \
	} while(0)

#define MARSHALLONG(a,b) \
	do\
	{\
		*a++=((unsigned char)((b) >> 24)); \
		*a++=((unsigned char)((b) >> 16)); \
		*a++=((unsigned char)((b) >> 8)); \
		*a++=((unsigned char)(b)); \
	} while(0)

#define MARSHALSHORT(a,b) \
	do\
	{\
		*a++=((unsigned char)((b) >> 8)); \
		*a++=((unsigned char)(b)); \
	} while(0)

#define MARSHALBYTE(a,b) \
	do\
	{\
		*a++=((unsigned char)(b)); \
	} while(0)

#define UNMARSHALLONGLONG(a,b)  {(b)=((long long int)*(a+0)<<56) | ((long long int)*(a+1)<<48) | ((long long int)*(a+2)<<40) | ((long long int)*(a+3)<<32) | ((long long int)*(a+4)<<24) | ((long long int)*(a+5)<<16) | ((long long int)*(a+6)<<8) | (long long int)*(a+7); a+=8;}

#define UNMARSHALLONG(a,b)  {b=(*a<<24) | (*(a+1)<<16) | (*(a+2)<<8) | *(a+3); a+=4;}

#define UNMARSHALSHORT(a,b)  {b=(*(a+0)<<8) | *(a+1); a+=2;}

#define UNMARSHALBYTE(a,b)  {b=(*a); a+=1;}

/* Marshal a short (length <=255) string
 */
#define MARSHALSTRINGSHORT(a,b) \
	do\
	{\
		if (b)\
		{\
			int l=strlen(b);\
			if (l>255)\
				l=255;\
			MARSHALBYTE(a,l);\
			memcpy(a,b,l);\
			a+=l;\
		}\
		else\
			MARSHALBYTE(a,0);\
	} while(0)

#define UNMARSHALSTRINGSHORT(a,b) \
	do \
	{ \
		int l; \
		UNMARSHALBYTE(a,l); \
		memcpy(b,a,l); \
		b[l]=0; \
		a+=l; \
	} while (0) 

/* marshal a buffer pointed to by b and
 * containing len bytes into a
 * NOTE: length is not automatically encoded
 */
#define MARSHALBUFFERLONG(a,b,len) \
	do\
	{\
		memcpy(a,b,len);\
		a+=len;\
	} while(0)

/* unmarshal a buffer of length "len" into
 * the pre-allocated memory pointed to by b
 */
#define UNMARSHALBUFFERLONG(a,b,len) \
	do \
	{ \
		memcpy(b,a,len); \
		a+=len; \
	} while (0)


/* Marshal a medium (length <=64K) string
 */
#define MARSHALSTRINGMEDIUM(a,b) \
	do\
	{\
		if (b)\
		{\
			int l=strlen(b);\
			if (l>65535)\
				l=65535;\
			MARSHALSHORT(a,l);\
			memcpy(a,b,l);\
			a+=l;\
		}\
		else\
			MARSHALSHORT(a,0);\
	} while(0)

#define UNMARSHALSTRINGMEDIUM(a,b) \
	do \
	{ \
		int l; \
		UNMARSHALSHORT(a,l); \
		memcpy(b,a,l); \
		b[l]=0; \
		a+=l; \
	} while (0) 

/* Return the length of the following string.  Do not move the pointer 
 */
#define UNMARSHALSTRINGMEDIUMLENGET(a,b) \
	do \
	{ \
		UNMARSHALSHORT(a,b); \
		a-=2; \
	} \

#define MARSHALNETWORK(dst,src,size) \
	do\
	{\
		memcpy(dst,&src,size);\
		dst+=size;\
	} while(0)

#define UNMARSHALNETWORK(src,dst,size) \
	do\
	{\
		memcpy(&dst,src,size);\
		src+=size;\
	} while(0)

#define MARSHALNETWORKLONG(dst,src)  MARSHALNETWORK(dst,src,4)
#define MARSHALNETWORKSHORT(dst,src) MARSHALNETWORK(dst,src,2)

#define UNMARSHALNETWORKLONG(src,dst) UNMARSHALNETWORK(src,dst,4)
#define UNMARSHALNETWORKSHORT(src,dst) UNMARSHALNETWORK(src,dst,2)

#endif
