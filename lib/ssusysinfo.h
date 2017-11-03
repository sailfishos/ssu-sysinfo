/** @file ssusysinfo.h
 *
 * ssu-sysinfo - Library API functions
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

#ifndef SSUSYSINFO_H_
# define SSUSYSINFO_H_

# ifdef __cplusplus
extern "C" {
# elif 0
}
# endif

/* ========================================================================= *
 * TYPES
 * ========================================================================= */

/** Opaque SSU configuration object structure
 *
 * Can be created/initialized with ssusysinfo_create() function and
 * should be released with ssusysinfo_delete() function.
 */
typedef struct ssusysinfo_t ssusysinfo_t;

/* ========================================================================= *
 * FUNCTIONS
 * ========================================================================= */

# pragma GCC visibility push(default)

/* -- ssusysinfo -- */

/** Create SSU configuration object
 *
 * Parses SSU configuration files and returns handle
 * that can be used for querying values.
 *
 * @return ssusysinfo object pointer
 */
ssusysinfo_t *ssusysinfo_create             (void);

/** Delete SSU configuration object
 *
 * @param self ssusysinfo object pointer, or NULL
 */
void          ssusysinfo_delete             (ssusysinfo_t *self);

/** Type agnostic callback for deleting SSU configuration objects
 *
 * @param self ssusysinfo object as void pointer, or NULL
 */
void          ssusysinfo_delete_cb          (void *self);

/** Force realoading of SSU configuration files
 *
 * @param self ssusysinfo object pointer
 */
void          ssusysinfo_reload             (ssusysinfo_t *self);

/** Query device model
 *
 * Try to find out ond what kind of system this is running.
 *
 * Uses flag file / cpuinfo content heuristics or looks it up
 * from /etc/hw-release file.
 *
 * Returns values such as:
 *   "SbJ"
 *   "tbj"
 *   "l500d"
 *   "tk7001"
 *   "SDK"
 *   "SDK Target"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::deviceModel()
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_model       (ssusysinfo_t *self);

/** Query device base model
 *
 * For variant devices #ssusysinfo_device_model() returns the
 * variant name. The name of the base model can be queried
 * with this function.
 *
 * Lookup is done from [variants] section defined in board mappings
 *
 * Should be functionally equivalent with:
 *   SsuDeviceInfo::deviceVariant(false)
 *
 * If the device is not an variant and there is no base model,
 * returns "UNKNOWN" - otherwise return values are similar as
 * what can be expected from #ssusysinfo_device_model().
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */

const char   *ssusysinfo_device_base_model     (ssusysinfo_t *self);

/** Query device designation
 *
 * Type designation, like NCC-1701.
 *
 * Uses ssusysinfo_device_model() to locate board mapping section and
 * returns value of key "deviceDesignation".
 *
 * Returns values such as:
 *   "JP-1301"
 *   "JT-1501"
 *   "Aqua Fish"
 *   "TK7001"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::displayName(Ssu::DeviceDesignation)
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_designation (ssusysinfo_t *self);

/** Query device manufacturer
 *
 * Manufacturer, like ACME Corp.
 *
 * Uses ssusysinfo_device_model() to locate board mapping section and
 * returns value of key "deviceManufacturer".
 *
 *
 * Returns values such as:
 *   "Jolla"
 *   "Intex"
 *   "Turing Robotic Industries"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::displayName(Ssu::DeviceManufacturer)
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_manufacturer(ssusysinfo_t *self);

/** Query device pretty name
 *
 * Marketed device name, like Pogoblaster 3000.
 *
 * Uses ssusysinfo_device_model() to locate board mapping section and
 * returns value of key "prettyModel".
 *
 * Returns values such as:
 *   "Jolla"
 *   "Jolla Tablet"
 *   "Intex Aqua Fish"
 *   "Turing Phone"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::displayName(Ssu::Ssu::DeviceModel)
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_pretty_name (ssusysinfo_t *self);

#pragma GCC visibility pop

# ifdef __cplusplus
};
# endif

#endif /* SSUSYSINFO_H_ */
