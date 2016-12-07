/** @file ssusysinfo.c
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

#include "ssusysinfo.h"

#include "inifile.h"
#include "xmalloc.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <glob.h>

/* ========================================================================= *
 * TYPES
 * ========================================================================= */

/** SSU configuration object structure */
struct ssusysinfo_t
{
    inifile_t *cfg_ini;
};

/* ========================================================================= *
 * DATA
 * ========================================================================= */

/** Placeholder string value returned whenever value can't be deduced */
static const char ssusysinfo_unknown[] = "UNKNOWN";

/* ========================================================================= *
 * PROTOTYPES
 * ========================================================================= */

static void        ssusysinfo_ctor                        (ssusysinfo_t *self);
static void        ssusysinfo_dtor                        (ssusysinfo_t *self);
ssusysinfo_t      *ssusysinfo_create                      (void);
void               ssusysinfo_delete                      (ssusysinfo_t *self);
void               ssusysinfo_delete_cb                   (void *self);

static void        ssusysinfo_load_board_mappings         (ssusysinfo_t *self);
static void        ssusysinfo_load_release_info           (ssusysinfo_t *self);
static void        ssusysinfo_load                        (ssusysinfo_t *self);
static void        ssusysinfo_unload                      (ssusysinfo_t *self);
void               ssusysinfo_reload                      (ssusysinfo_t *self);

static const char *ssusysinfo_device_model_from_cpuinfo   (ssusysinfo_t *self);
static const char *ssusysinfo_device_model_from_flagfiles (ssusysinfo_t *self);
static const char *ssusysinfo_device_model_from_hw_release(ssusysinfo_t *self);
const char        *ssusysinfo_device_model                (ssusysinfo_t *self);

static const char *ssusysinfo_device_attr                 (ssusysinfo_t *self, const char *key);
const char        *ssusysinfo_device_designation          (ssusysinfo_t *self);
const char        *ssusysinfo_device_manufacturer         (ssusysinfo_t *self);
const char        *ssusysinfo_device_pretty_name          (ssusysinfo_t *self);

/* ========================================================================= *
 * FUNCTIONS
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * Internal Functions
 * ------------------------------------------------------------------------- */

/** Initialize freshly allocated configuration object to safe defaults
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_ctor(ssusysinfo_t *self)
{
    self->cfg_ini = 0;
}

/** Release dynamic resources held by initialized  configuration object
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_dtor(ssusysinfo_t *self)
{
    ssusysinfo_unload(self);
}

/** Load board mapping configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_load_board_mappings(ssusysinfo_t *self)
{
    glob_t gl = {};

    if( glob("/usr/share/ssu/board-mappings.d/*.ini", 0, 0, &gl) == 0 ) {
        for( size_t i = 0; i < gl.gl_pathc; ++i )
            inifile_load(self->cfg_ini, gl.gl_pathv[i], 0);
    }

    globfree(&gl);
}

/** Load release information configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_load_release_info(ssusysinfo_t *self)
{
    inifile_load(self->cfg_ini, "/etc/hw-release", "hw-release");
    inifile_load(self->cfg_ini, "/etc/sailfish-release", "sailfish-release");
}

/** Load all SSU configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_load(ssusysinfo_t *self)
{
    if( !self )
        goto EXIT;

    if( self->cfg_ini )
        goto EXIT;

    self->cfg_ini = inifile_create();

    ssusysinfo_load_board_mappings(self);
    ssusysinfo_load_release_info(self);

EXIT:
    return;
}

/** Unload all values from SSU configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_unload(ssusysinfo_t *self)
{
    inifile_delete(self->cfg_ini),
        self->cfg_ini = 0;
}

/** Try to determine device model based on cpuinfo and config file data
 *
 * @param self ssusysinfo object pointer
 *
 * @return c-string, or NULL in case model can't be determined
 */
static const char *
ssusysinfo_device_model_from_cpuinfo(ssusysinfo_t *self)
{
    inival_t   *res  = 0;
    inisec_t   *sec  = 0;
    const char *path = "/proc/cpuinfo";
    char       *text = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( !(sec = inifile_get_section(self->cfg_ini, "cpuinfo.contains")) )
        goto EXIT;

    if( !(text = fileutil_read(path, 0)) )
        goto EXIT;

    for( size_t i = 0; ; ++i ) {
        inival_t *val = inisec_elem(sec, i);
        if( !val )
            break;

        /* Later than current choise in the ini-file */
        if( res && inival_get_ord(res) > inival_get_ord(val) )
            continue;

        /* String given in ini-file exists in cpuinfo */
        if( strstr(text, inival_get_val(val)) )
            res = val;
    }

EXIT:
    free(text);

    return res ? inival_get_key(res) : 0;
}

/** Try to determine device model based on flag file configuration
 *
 * @param self ssusysinfo object pointer
 *
 * @return c-string, or NULL in case model can't be determined
 */
static const char *
ssusysinfo_device_model_from_flagfiles(ssusysinfo_t *self)
{
    inival_t *res = 0;
    inisec_t *sec = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( !(sec = inifile_get_section(self->cfg_ini, "file.exists")) )
        goto EXIT;

    for( size_t i = 0; ; ++i ) {
        inival_t *val = inisec_elem(sec, i);
        if( !val )
            break;

        /* Later than current choise in the ini-file */
        if( res && inival_get_ord(res) > inival_get_ord(val) )
            continue;

        /* Path given in ini-file exists in file system */
        if( fileutil_exists(inival_get_val(val)) )
            res = val;
    }

EXIT:
    return res ? inival_get_key(res) : 0;
}

/** Try to determine device model based on /etc/hw-release content
 *
 * @param self ssusysinfo object pointer
 *
 * @return c-string, or NULL in case model can't be determined
 */
static const char *
ssusysinfo_device_model_from_hw_release(ssusysinfo_t *self)
{
    const char *res = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    res = inifile_get(self->cfg_ini, "hw-release", "MER_HA_DEVICE", 0);

EXIT:
    return res;
}

/** Lookup a key in device mode specific section from board mappings
 *
 * @param self ssusysinfo object pointer
 * @param key  key name used in board config ini files
 *
 * @return c-string, or NULL in case value is not specified
 */
static const char *
ssusysinfo_device_attr(ssusysinfo_t *self, const char *key)
{
    const char *res   = 0;
    const char *model = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( !(model = ssusysinfo_device_model(self)) )
        goto EXIT;

    res = inifile_get(self->cfg_ini, model, key, 0);

EXIT:
    /* Return valid c-string or NULL on failure */
    return res;
}

/* ------------------------------------------------------------------------- *
 * Public Functions
 * ------------------------------------------------------------------------- */

/* NOTE: Public functions are documented in ssusysinfo.h header file which
 *       is included in the development package.
 */

ssusysinfo_t *
ssusysinfo_create(void)
{
    ssusysinfo_t *self = xcalloc(1, sizeof *self);

    ssusysinfo_ctor(self);
    ssusysinfo_load(self);

    return self;
}

void
ssusysinfo_delete(ssusysinfo_t *self)
{
  if( self != 0 )
  {
    ssusysinfo_dtor(self);
    free(self);
  }
}

void
ssusysinfo_delete_cb(void *self)
{
  ssusysinfo_delete(self);
}

void
ssusysinfo_reload(ssusysinfo_t *self)
{
    ssusysinfo_unload(self);
    ssusysinfo_load(self);
}

const char *
ssusysinfo_device_model(ssusysinfo_t *self)
{
    const char *cached = 0;
    const char *probed = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( (cached = inifile_get(self->cfg_ini, "cached-values", "model", 0)) )
        goto EXIT;

    /* Guess by looking at flag files - this needs to be done 1st
     * so that detecting "sdk" / "sdk-target" works gegardless of
     * what product configuration files are installed in the sdk. */
    if( (probed = ssusysinfo_device_model_from_flagfiles(self)) )
        goto CACHE;

    /* Normally it should be defined in hw-config file */
    if( (probed = ssusysinfo_device_model_from_hw_release(self)) )
        goto CACHE;

    /* Attempt some /proc/cpyinfo based heuristics */
    if( (probed = ssusysinfo_device_model_from_cpuinfo(self)) )
        goto CACHE;

    /* We have data, but were unable to determine device model */
    probed = ssusysinfo_unknown;

CACHE:
    /* Update the cache so that we do not need to repeat the above
     * heuristics the next time */
    inifile_set(self->cfg_ini, "cached-values", "model", (cached = probed));

EXIT:
    /* Always return valid c-string */
    return cached ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_device_designation(ssusysinfo_t *self)
{
    /* Always return valid c-string */
    return ssusysinfo_device_attr(self,
                                  "deviceDesignation") ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_device_manufacturer(ssusysinfo_t *self)
{
    /* Always return valid c-string */
    return ssusysinfo_device_attr(self,
                                  "deviceManufacturer") ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_device_pretty_name(ssusysinfo_t *self)
{
    /* Always return valid c-string */
    return ssusysinfo_device_attr(self, "prettyModel") ?: ssusysinfo_unknown;
}
