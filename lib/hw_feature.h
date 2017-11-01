/** @file hw_feature.h
 *
 * ssu-sysinfo - HW feature functions
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

#ifndef  HW_FEATURE_H_
# define HW_FEATURE_H_

# include "ssusysinfo.h"

# ifdef __cplusplus
extern "C" {
# endif

bool          hw_feature_is_valid     (hw_feature_t id);
bool          hw_feature_get_fallback (hw_feature_t id);
const char   *hw_feature_to_string    (hw_feature_t id);
hw_feature_t  hw_feature_from_string  (const char *name);
const char   *hw_feature_to_csd_key   (hw_feature_t id);
const char  **hw_feature_names        (void);

# ifdef __cplusplus
};
# endif

#endif /* HW_FEATURE_H_ */
