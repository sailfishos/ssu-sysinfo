/** @file hw_key.h
 *
 * ssu-sysinfo - HW keycode functions
 * <p>
 * Copyright (c) 2017 Jolla Ltd.
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

#ifndef  HW_KEY_H_
# define HW_KEY_H_

# include "ssusysinfo.h"

# ifdef __cplusplus
extern "C" {
# endif

hw_key_t     hw_key_from_string (const char *name);
const char  *hw_key_to_string   (hw_key_t code);
const char **hw_key_names       (void);
bool         hw_key_is_valid    (hw_key_t code);
hw_key_t    *hw_key_parse_array (const char *text);

# ifdef __cplusplus
};
# endif

#endif /* HW_KEY_H_ */
