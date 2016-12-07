/** @file xmalloc.c
 *
 * ssu-sysinfo - Dynamic memory allocation wrappers
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

#include "xmalloc.h"

#include <stdlib.h>
#include <string.h>

/* ========================================================================= *
 * FUNCTIONS
 * ========================================================================= */

/** Succeed or die type wrapper for malloc
 *
 * Unlike real malloc() this will never return non-null result
 * for zero size allocations.
 */
void *
xmalloc(size_t size)
{
    void *data = 0;
    if( size && !(data = malloc(size)) )
        abort();
    return data;
}

/** Succeed or die type wrapper for calloc
 *
 * Unlike real calloc() this will never return non-null result
 * for zero nmemb/size allocations.
 */
void *
xcalloc(size_t nmemb, size_t size)
{
    void *data = 0;
    if( nmemb && size && !(data = calloc(nmemb, size)) )
        abort();
    return data;
}

/** Succeed or die type wrapper for realloc
 *
 * Unlike real realloc() this will never return non-null result
 * when zero new_size is requested.
 */
void *
xrealloc(void *old_data, size_t new_size)
{
    void *new_data = 0;

    if( !new_size )
        free(old_data);
    else if( !(new_data = realloc(old_data, new_size)) )
        abort();
    return new_data;
}

/** Succeed or die type wrapper for strdup
 *
 * Unlike real strdup() this tolerates null arguments.
 */
char *
xstrdup(const char *old_str)
{
    char *new_str = 0;
    if( old_str && !(new_str = strdup(old_str)) )
        abort();
    return new_str;
}
