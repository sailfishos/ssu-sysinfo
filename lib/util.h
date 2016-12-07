/** @file util.h
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

#ifndef  UTIL_H_
# define UTIL_H_

# include <stddef.h>
# include <stdbool.h>

/* ========================================================================= *
 * Functions
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

#endif /* UTIL_H_ */
