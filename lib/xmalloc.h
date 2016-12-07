/** @file xmalloc.h
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

#ifndef  XMALLOC_H_
# define XMALLOC_H_

# include <stddef.h>

/* ========================================================================= *
 * Functions
 * ========================================================================= */

void *xmalloc (size_t size);
void *xcalloc (size_t nmemb, size_t size);
void *xrealloc(void *old_data, size_t new_size);
char *xstrdup (const char *old_str);

#endif /* XMALLOC_H_ */
