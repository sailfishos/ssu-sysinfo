/** @file util.c
 *
 * ssu-sysinfo - Generic utility functions
 * <p>
 * Copyright (c) 2016 Jolla Ltd.
 * <p>
 * @author Simo Piiroinen <simo.piiroinen@jollamobile.com>
 *
 * ssu-sysinfo is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ssu-sysinfo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ssu-sysinfo; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "util.h"

#include "xmalloc.h"
#include "logging.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

/* ========================================================================= *
 * PROTOTYPES
 * ========================================================================= */

/* -- strutil -- */

char * strutil_trim  (char *str);
char * strutil_strip (char *str);
char * strutil_slice (char *str, char **end, int sep);

void   strutil_set   (char **pstr, const char *val);
int    strutil_equals(const char *s1, const char *s2);

/* -- fileutil -- */

bool  fileutil_exists(const char *path);
char *fileutil_read  (const char *path, size_t *psize);

/* ========================================================================= *
 * STRING UTILITIES
 * ========================================================================= */

/** At white space char predicate
 */
static inline bool
strutil_at_white(const char *str)
{
    int chr = (unsigned char)*str;
    return (chr > 0) && (chr <= 32);
}

/** At non white space char predicate
 */
static inline bool
strutil_at_black(const char *str)
{
    int chr = (unsigned char)*str;
    return (chr > 32);
}

/** At char predicate
 */
static inline bool
strutil_at_char(const char *str, int chr)
{
    return *(unsigned char *)str == chr;
}

/** Remove leading and trailing whitespace from a string
 */
char *
strutil_trim(char *str)
{
    if( str ) {
        char *src = str;
        char *dst = str;
        char *end = str;

        while( strutil_at_white(src) ) ++src;

        while( *src ) {
            while( strutil_at_black(src) ) *dst++ = *src++;
            end = dst;
            while( strutil_at_white(src) ) *dst++ = *src++;
        }
        *end = 0;
    }
    return str;
}

/** Remove all excess whitespace from a string
 */
char *
strutil_strip(char *str)
{
    if( str ) {
        char *src = str;
        char *dst = str;
        while( strutil_at_white(src) ) ++src;
        for( ;; ) {
            while( strutil_at_black(src) ) *dst++ = *src++;
            while( strutil_at_white(src) ) ++src;
            if( !*src ) break;
            *dst++ = ' ';
        }
        *dst = 0;
    }
    return str;
}

/** Parse a token from start of a string
 */
char *
strutil_slice(char *str, char **end, int sep)
{
    char *pos = str;

    if( sep <= 0 ) {
        while( strutil_at_white(pos) ) ++pos;

        for( ; *pos; ++pos ) {
            if( strutil_at_white(pos) ) {
                *pos++ = 0;
                break;
            }
        }
    }
    else {
        for( ; *pos; ++pos ) {
            if( strutil_at_char(pos, sep) ) {
                *pos++ = 0;
                break;
            }
        }
    }

    if( end ) *end = pos;

    return str;
}

void
strutil_set(char **pstr, const char *val)
{
    char *use = val ? xstrdup(val) : 0;
    free(*pstr), *pstr = use;
}

int
strutil_equals(const char *s1, const char *s2)
{
    return (!s1 || !s2) ? (s1 == s2) : !strcmp(s1,s2);
}

/* ========================================================================= *
 * FILE UTILITIES
 * ========================================================================= */

/** Check if file with given path exists
 */
bool
fileutil_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

/** Read content of any file as string
 */
char *
fileutil_read(const char *path, size_t *psize)
{
    bool    ack  = false;
    int     file = -1;
    size_t  done = 0;
    size_t  size = 0x1000;
    char   *data = xmalloc(size);

    if( (file = open(path, O_RDONLY)) == -1 )
    {
        if( errno == ENOENT )
            log_debug("%s: open: %m", path);
        else
            log_warning("%s: open: %m", path);
        goto cleanup;
    }

    for( ;; ) {
        if( done == size )
            data = xrealloc(data, (size *= 2));

        int rc = read(file, data + done, size - done);

        if( rc == -1 ) {
            log_warning("%s: read: %m", path);
            goto cleanup;
        }

        if( rc == 0 )
            break;

        done += (size_t)rc;
    }

    size = done;
    data = xrealloc(data, size + 1);
    data[size] = 0;

    ack = true;

cleanup:
    if( file != -1 )
        close(file);

    if( !ack )
        free(data), data = 0, size = 0;

    if( psize )
        *psize = size;

    return data;
}
