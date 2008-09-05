/* 
 * mallocreadline.h
 */
#ifndef MALLOCREADLINE_H
#define MALLOCREADLINE_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Strip leading and trailing whitespace and comments
 */
static inline void strip(char *buf)
{
    if(buf)
    {
        char *iter1 = buf;
        char *iter2 = buf;
        char *end = isspace(*buf) ? buf : 0;
        /*
         * Skip leading spaces
         */
        while(*iter1 && isspace(*iter1))
        {
            ++iter1;
        }
        /*
         * Copy the line upto the null
         */
        while(*iter1)
        {
            *iter2 = *iter1;
            if(*iter2 == '#')
            {
                /*
                 * Strip comment - only one recognized starts with a '#'
                 */
                *iter2 = 0;
                break;
            }
            /*
             * Mark first space after last non-space
             */
            if(end)
            {
                if(!isspace(*iter2))
                {
                    end = 0;
                }
            }
            else
            {
                if(isspace(*iter2))
                {
                    end = iter2;
                }
            }
            /*
             * Move on to next character
             */
            ++iter1;
            ++iter2;
        }
        if(end)
        {
            *end = 0;
        }
	    else
	    {
            *iter2 = 0;
	    }
    }
    return;
} /* strip */

/*
 * Reads up to a newline or EOF. (Ignores carriage return.)
 *
 * Returns the line in a "malloc"ed buffer.
 *
 * Returns 0 on failure to allocate and the file pointer is left
 * at the position where the failure occured.
 */
static inline char *mallocreadline(FILE *fp)
{
    size_t len = 50;
    char *ret = (char*)malloc(len);
    if(ret)
    {
        size_t count = 0;
        for(;;)
        {
            int ch = fgetc(fp);
            if(ch == EOF || ch == '\n')
            {
                ret[count] = 0;
                break;
            }
            if(ch != '\r')
            {
                ret[count] = (unsigned char)ch;
                ++count;
                if(count == len)
                {
                    unsigned int nextlen = len << 1;
                    char *tmp = (char*)malloc(nextlen);
                    if(tmp)
                    {
                        memcpy(tmp, ret, len);
                        len = nextlen;
                        free(ret);
                        ret = tmp;
                    }
                    else
                    {
                        free(ret);
                        ret = 0;
                        break;
                    }
                }
            }
        }
    }
    return ret;
} /* mallocreadline */

/*
 * Returns non-zero if all characters in "buf" are decimal digits values
 */
static inline int isdec(char const *buf)
{
    int ret;
    if(buf && *buf) /* must have at least one character */
    {
        for(;;)
        {
            if(*buf)
            {
                if(!isdigit(*buf))
                {
                    ret = 0;
                    break;
                }
            }
            else
            {
                ret = 1;
                break;
            }
            ++buf;
        }
    }
    else
    {
        ret = 0;
    }
    return  ret;
} /* isdec */

/*
 * Returns non-zero if all characters in "buf" are hex number values
 *
 * Only deals with capital A-F
 */
static inline int ishex(char const *buf)
{
    int ret;
    if(buf && *buf) /* must have at least one character */
    {
        for(;;)
        {
            if(*buf)
            {
                if(!isxdigit(*buf))
                {
                    ret = 0;
                    break;
                }
            }
            else
            {
                ret = 1;
                break;
            }
            ++buf;
        }
    }
    else
    {
        ret = 0;
    }
    return  ret;
} /* ishex */

/*
 * Reads lines until a non-space filled line of characters are returned
 *
 * Returns 0 on failure to allocate or end of file.
 */
static inline char *mallocreadnextnonblank(FILE *fp)
{
    char *ret;
    for(;;)
    {
        char *line = mallocreadline(fp);
        if(line)
        {
            strip(line);
            if(*line)
            {
                ret = line;
                break;
            }
            free(line);
            if(feof(fp))
            {
                ret = 0;
                break;
            }
        }
        else
        {
            ret = 0;
            break;
        }
    }
    return ret;
} /* mallocreadnextnonblank */

/*
 * Reads lines until a line of hexidecimal number characters are returnd
 *
 * Returns 0 on failure to allocate or end of file.
 */
static inline char *mallocreadnexthex(FILE *fp)
{
    char *ret;
    for(;;)
    {
        char *line = mallocreadnextnonblank(fp);
        if(line)
        {
            if(ishex(line))
            {
                ret = line;
                break;
            }
            free(line);
            if(feof(fp))
            {
                ret = 0;
                break;
            }
        }
        else
        {
            ret = 0;
            break;
        }
    }
    return ret;
} /* mallocreadnexthex */

/*
 * Reads lines until a line starting with the given tag is found
 *
 * Returns 0 on failure to allocate or end of file.
 */
static inline char *mallocreadnexttag(FILE *fp, char const *tag)
{
    char *ret;
    size_t taglen = strlen(tag);
    for(;;)
    {
        char *line = mallocreadnextnonblank(fp);
        if(line)
        {
            if(strncmp(line, tag, taglen) == 0)
            {
                ret = line;
                break;
            }
            free(line);
            if(feof(fp))
            {
                ret = 0;
                break;
            }
        }
        else
        {
            ret = 0;
            break;
        }
    }
    return ret;
} /* mallocreadnexttag */

/*
 * Reads first occurence of line starting with "tag"
 *
 * Leaves file pointer at start of next line.
 */
static inline char *mallocreadfirsttag(FILE *fp, char const *tag)
{
    clearerr(fp);
    fseek(fp, 0, SEEK_SET);
    return mallocreadnexttag(fp, tag);
} /* mallocreadfirsttag */

#endif /* MALLOCREADLINE_H_FILE */
