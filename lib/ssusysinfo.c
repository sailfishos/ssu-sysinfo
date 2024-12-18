/** @file ssusysinfo.c
 *
 * ssu-sysinfo - Library API functions
 * <p>
 * Copyright (c) 2016 - 2022 Jolla Ltd.
 * <p>
 * @author Simo Piiroinen <simo.piiroinen@jolla.com>
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
#include "hw_key.h"
#include "hw_feature.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <time.h>
#include <endian.h>

/* ========================================================================= *
 * CONSTANTS
 * ========================================================================= */

/** SSU Configuration version ssu-sysinfo is known to be compatible with
 *
 * SSU configuration version change involves automated edits to data
 * already present on devices in active use.
 *
 * Assumption is that version upgrades are going to be mostly harmless
 * from ssu-sysinfo point of view, but when a version bump is detected
 * a warning is emitted to journal to prompt manual check & bringing
 * ssu-sysinfo back in sync with ssu.
 */
#define EXPECTED_SSU_CONFIG_VERSION 15

/** Possible paths for OS release data */
static const char * const os_release_paths[] = {
    "/etc/os-release",
    "/usr/lib/os-release",
    NULL
};

/** Internal config data section to use for OS release data */
#define OS_RELEASE_SECTION      "os-release"

/** Possible paths for HW release data */
static const char * const hw_release_paths[] = {
    "/etc/hw-release",
    "/usr/lib/hw-release",
    NULL
};

/** Internal config data section to use for HW release data */
#define HW_RELEASE_SECTION      "hw-release"

/* ========================================================================= *
 * TYPES
 * ========================================================================= */

/** SSU configuration object structure */
struct ssusysinfo_t
{
    inifile_t *cfg_ini;
    inifile_t *ssu_ini;
};

/* ========================================================================= *
 * DATA
 * ========================================================================= */

/** Placeholder string value returned whenever value can't be deduced */
static const char ssusysinfo_unknown[] = "UNKNOWN";

/* ========================================================================= *
 * PROTOTYPES
 * ========================================================================= */

static int         qtdecoder_digit_value                    (int chr);
static int         qtdecoder_parse_char                     (const char **ppos, int base, int len);
static void       *qtdecoder_parse_blob                     (const char *txt, size_t *psize);
static char       *qtdecoder_parse_datetime                 (const char *txt);
#if SSU_INCLUDE_CREDENTIAL_ITEMS
static char       *qtdecoder_parse_bytearray                (const char *txt, size_t *psize);
#endif

static void        ssusysinfo_ctor                          (ssusysinfo_t *self);
static void        ssusysinfo_dtor                          (ssusysinfo_t *self);
ssusysinfo_t      *ssusysinfo_create                        (void);
void               ssusysinfo_delete                        (ssusysinfo_t *self);
void               ssusysinfo_delete_cb                     (void *self);

static void        ssusysinfo_load_board_mappings           (ssusysinfo_t *self);
static void        ssusysinfo_load_release_file             (ssusysinfo_t *self, const char * const *paths, const char *section);
static void        ssusysinfo_load_release_info             (ssusysinfo_t *self);
static void        ssusysinfo_load_hw_settings              (ssusysinfo_t *self);
static void        ssusysinfo_load_ssu_config               (ssusysinfo_t *self);

static void        ssusysinfo_load                          (ssusysinfo_t *self);
static void        ssusysinfo_unload                        (ssusysinfo_t *self);
void               ssusysinfo_reload                        (ssusysinfo_t *self);

static const char *ssusysinfo_device_model_from_cpuinfo     (ssusysinfo_t *self);
static const char *ssusysinfo_device_model_from_flagfiles   (ssusysinfo_t *self);
static const char *ssusysinfo_device_model_from_hw_release  (ssusysinfo_t *self);
const char        *ssusysinfo_device_model                  (ssusysinfo_t *self);

static const char *ssusysinfo_device_attr                   (ssusysinfo_t *self, const char *key);
const char        *ssusysinfo_device_designation            (ssusysinfo_t *self);
const char        *ssusysinfo_device_manufacturer           (ssusysinfo_t *self);
const char        *ssusysinfo_device_pretty_name            (ssusysinfo_t *self);

static void        ssusysinfo_load_ssu_config               (ssusysinfo_t *self);
static const char *ssusysinfo_ssu_attr_ex                   (ssusysinfo_t *self, const char *sec, const char *key);
static const char *ssusysinfo_ssu_attr                      (ssusysinfo_t *self, const char *key);
int                ssusysinfo_ssu_config_version            (ssusysinfo_t *self);
bool               ssusysinfo_ssu_registered                (ssusysinfo_t *self);
ssu_device_mode_t  ssusysinfo_ssu_device_mode               (ssusysinfo_t *self);
bool               ssusysinfo_ssu_in_rnd_mode               (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_arch                      (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_brand                     (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_flavour                   (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_domain                    (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_release                   (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_def_release               (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_rnd_release               (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_enabled_repos             (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_disabled_repos            (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_last_credentials_update   (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_credentials_scope         (ssusysinfo_t *self);
static const char *ssusysinfo_ssu_credentials_url           (ssusysinfo_t *self, const char *scope);
const char        *ssusysinfo_ssu_credentials_url_jolla     (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_credentials_url_store     (ssusysinfo_t *self);
#if SSU_INCLUDE_CREDENTIAL_ITEMS
static const char *ssusysinfo_ssu_credentials_username      (ssusysinfo_t *self, const char *scope);
const char        *ssusysinfo_ssu_credentials_username_jolla(ssusysinfo_t *self);
const char        *ssusysinfo_ssu_credentials_username_store(ssusysinfo_t *self);
static const char *ssusysinfo_ssu_credentials_password      (ssusysinfo_t *self, const char *scope);
const char        *ssusysinfo_ssu_credentials_password_jolla(ssusysinfo_t *self);
const char        *ssusysinfo_ssu_credentials_password_store(ssusysinfo_t *self);
const char        *ssusysinfo_ssu_certificate               (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_private_key               (ssusysinfo_t *self);
#endif
const char        *ssusysinfo_ssu_default_rnd_domain        (ssusysinfo_t *self);
const char        *ssusysinfo_ssu_home_url                  (ssusysinfo_t *self);

/* ========================================================================= *
 * FUNCTIONS
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * Utility functions
 * ------------------------------------------------------------------------- */

static int
qtdecoder_digit_value(int chr)
{
    switch( chr ) {
    case '0' ... '9':
        return chr - '0';
    case 'a' ... 'z':
        return chr - 'a' + 10;
    case 'A' ... 'Z':
        return chr - 'A' + 10;
    default:
        return 256;
    }
}

static int
qtdecoder_parse_char(const char **ppos, int base, int len)
{
    /* Caveat caller: On unexpected inputs this function
     * can return without advancing the parse position.
     */

    const char *pos = *ppos;
    int res = 0;

    while( len > 0 ) {
        int digit = qtdecoder_digit_value(*pos);
        if( digit >= base )
            break;
        int value = res * base + digit;
        if( value > 255 )
            break;
        res = value, ++pos, --len;
    }

    return *ppos = pos, res;
}

static void *
qtdecoder_parse_blob(const char *txt, size_t *psize)
{
    char *buf = 0;
    char *dst = 0;

    /* NB Decoding can't make the string longer */
    if( !(dst = buf = malloc(strlen(txt) + 1)) )
        goto EXIT;

    const char *pos = txt;
    while( *pos ) {
        if( *pos != '\\' ) {
            *dst++ = *pos++;
            continue;
        }
        switch( *++pos ) {
        case  0:
            // stray backslash at EOS
            break;
        case 'a':
            // bell
            *dst++ = 0x07; ++pos;
            break;
        case 'b':
            // backspace
            *dst++ = 0x08; ++pos;
            break;
        case 't':
            // horizontal tab
            *dst++ = 0x09; ++pos;
            break;
        case 'n':
            // new line
            *dst++ = 0x0a; ++pos;
            break;
        case 'v':
            // vertical tab
            *dst++ = 0x0b; ++pos;
            break;
        case 'f':
            // form feed
            *dst++ = 0x0c; ++pos;
            break;
        case 'r':
            // carriage return
            *dst++ = 0x0d; ++pos;
            break;
        case 'e':
            // escape
            *dst++ = 0x1b; ++pos;
            break;
        case '0':
            // nul-char
            *dst++ = 0; ++pos;
            break;
        case '1' ... '7':
            // octal char
            *dst++ = qtdecoder_parse_char(&pos, 8, 3);
            break;
        case 'x':
            // hexadecimal char
            ++pos;
            *dst++ = qtdecoder_parse_char(&pos, 16, 2);
            break;
        default:
            // assume: raw escaped char
            *dst++ = *pos++;
            break;
        }
    }

EXIT:
    return *psize = dst-buf, buf;
}

static char *
qtdecoder_parse_datetime(const char *txt)
{
    /* What we are dealing here with is result of:
     *
     * 1. SSU puts QDateTime::currentDateTime() data in QSettings
     *    object.
     * 2. On save to file QSettings converts QDateTime objects
     *    to strings by serializing datetime via QDataStream
     *    and then surrounding the resulting ascii escaped binary
     *    blob within "@DateTime()" quote block.
     */
    char    *res = 0;
    char    *tmp = 0;
    size_t   len = 0;
    uint8_t *dta = 0;

    /* Extract the escaped binary blob */
    static const char tag[] = "@DateTime(";
    const char *beg = strstr(txt, tag);
    if( !beg )
        goto EXIT;

    beg += sizeof tag - 1;

    const char *end = strrchr(beg, ')');
    if( !end )
        goto EXIT;

    if( !(tmp = strndup(beg, end - beg)) )
        goto EXIT;

    /* Decode the binary blob
     *
     * What we expect to see is something like:
     *
     * [0000] 00 00 00 10 = uint32: QDataStream version
     *                      -> 0x00000010 == QDataStream::Qt_5_4
     *
     * [0004] 00          = ???
     * [0005] 00          = ???
     * [0006] 00          = ???
     * [0007] 00          = ???
     * [0008] 00          = ???
     *
     * [0009] 00 25 83 e5 = uint32: Julian day
     *                      -> 0x002583e5 = 2019-04-23
     *
     * [000d] 02 57 3b 22 = uint32: ms since midnight
     *                      -> 0x02573b22 = 10:65:31.202
     *
     * [0011] 00          = uint8: Qt::TimeSpec
     *                      -> 0x00 == Qt::LocalTime
     *
     * [0012] (tz data if LocalTime were not used?)
     */
    dta = qtdecoder_parse_blob(tmp, &len);

    if( len < 0x0012 )
        goto EXIT;

    uint32_t vers = 0;
    uint32_t date = 0;
    uint32_t msec = 0;
    uint8_t  spec = 0;

    /* Qt serializer ends up using funky alignments.
     * Use memcpy() to avoid bus faults etc.
     */
    memcpy(&vers, dta + 0x0000, sizeof vers);
    memcpy(&date, dta + 0x0009, sizeof date);
    memcpy(&msec, dta + 0x000d, sizeof msec);
    memcpy(&spec, dta + 0x0011, sizeof spec);

    /* Convert from big endian to host endian
     */
    vers = be32toh(vers);
    date = be32toh(date);
    msec = be32toh(msec);

    /* While Qt might be able to be backwards / forwards
     * compatible with itself, we do not have such luxury.
     *
     * Only QDataStream::Qt_5_4 (=0x10) version is supported.
     */
    if( vers != 0x10 )
        goto EXIT;

    /* Convert Julian date to time_t at midnight that day
     * and add seconds since midnight.
     */
    time_t t = (date - 2440588) * 86400 + msec / 1000;

    /* Interpret the value as if it were in UTC
     * -> We should have correct time and date
     *    representation in broken down time.
     * -> Used as fallback if localtime heuristics fail
     */
    struct tm local_tm = {};
    struct tm utc_tm = {};
    struct tm *tm = 0;

    if( !(tm = gmtime_r(&t, &utc_tm)) )
        goto EXIT;

    switch( spec ) {
    case 0: // Qt::LocalTime
        /* Time in whatever timezone that happened to be relevant
         * whenever the timestamp was created ... sigh.
         *
         * 1. Try to interpret the value in local time
         * 2. Make a leap of faith and assume that:
         *    - Current timezone is the same as what was used for
         *      creating the time stamp
         *    - We have appropriate UTC offset for the current
         *      timezone at the time timestamp was created
         * 3. Adjust past timestamp accordingly
         * 4. Use the corrected value if it is valid and
         *    date / time values match what we expect to see.
         */
        if( localtime_r(&t, &local_tm) ) {
            time_t guess = t - local_tm.tm_gmtoff;
            if( localtime_r(&guess, &local_tm) ) {
                if( utc_tm.tm_year == local_tm.tm_year &&
                    utc_tm.tm_mon  == local_tm.tm_mon  &&
                    utc_tm.tm_mday == local_tm.tm_mday &&
                    utc_tm.tm_hour == local_tm.tm_hour &&
                    utc_tm.tm_min  == local_tm.tm_min  &&
                    utc_tm.tm_sec  == local_tm.tm_sec )
                    tm = &local_tm;
            }
        }
        break;
    case 1: // Qt::UTC
        /* Can be used as-is, but represented in local time */
        if( localtime_r(&t, &local_tm) )
            tm = &local_tm;
        break;
    default:
    case 2: // Qt::OffsetFromUTC
    case 3: // Qt::TimeZone
        /* FIXME: Support fully qualified local timestamps
         *        when/if ssu starts using them ... */
        log_warning("Unknown Qt::TimeSpec value %u - "
                    "timezone data ignored", (unsigned)spec);
        break;
    }

    /* Use ISO-8601 compatible time representation.
     */
    int sign = '+';
    int offs = tm->tm_gmtoff / 60; // [s] -> [min]
    if( offs < 0 )
        offs = -offs, sign = '-';
    if( asprintf(&res, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
                 tm->tm_year + 1900,
                 tm->tm_mon + 1,
                 tm->tm_mday,
                 tm->tm_hour,
                 tm->tm_min,
                 tm->tm_sec,
                 sign, offs/60, offs%60) == -1 )
        res = 0;

EXIT:
    free(dta);
    free(tmp);

    return res;
}

#if SSU_INCLUDE_CREDENTIAL_ITEMS
static char *
qtdecoder_parse_bytearray(const char *txt, size_t *psize)
{
    /* What we are dealing here with is result of:
     *
     * 1. SSU puts QByteArray() data in QSettings object.
     * 2. On save to file QSettings converts QByteArray objects
     *    to strings by serializing content via QDataStream
     *    and then surrounding the resulting ascii escaped binary
     *    blob within "@ByteArray()" quote block.
     */
    char    *res = 0;
    char    *tmp = 0;
    size_t   len = 0;
    uint8_t *dta = 0;

    /* Extract the escaped binary blob */
    static const char tag[] = "@ByteArray(";
    const char *beg = strstr(txt, tag);
    if( !beg )
        goto EXIT;

    beg += sizeof tag - 1;

    const char *end = strrchr(beg, ')');
    if( !end )
        goto EXIT;

    if( !(tmp = strndup(beg, end - beg)) )
        goto EXIT;

    /* Decode the binary blob */
    dta = qtdecoder_parse_blob(tmp, &len);

    /* Make it NUL terminated */
    if( (res = realloc(dta, len + 1)) )
        res[len] = 0, dta = 0;

EXIT:
    free(dta);
    free(tmp);

    return *psize = (res ? len : 0), res;
}
#endif /* SSU_INCLUDE_CREDENTIAL_ITEMS */

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
    self->ssu_ini = 0;
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

/** Load release information from list of possible file paths
 *
 * @param self     ssusysinfo object pointer
 * @param paths    array of altrernate paths to try
 * @param section  config section in which to store loaded data
 */
static void
ssusysinfo_load_release_file(ssusysinfo_t *self, const char * const *paths,
                             const char *section)
{
    for( ;; ) {
        const char *path = *paths++;
        if( !path ) {
            log_warning("%s data not found", section);
            break;
        }
        if( access(path, F_OK) == -1 )
            continue;
        /* Note: The first existing alternative is used, regardless
         *       of whether it can be successfully parsed or not. */
        inifile_load(self->cfg_ini, path, section);
        break;
    }
}

/** Load release information configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_load_release_info(ssusysinfo_t *self)
{
    ssusysinfo_load_release_file(self, hw_release_paths, HW_RELEASE_SECTION);
    ssusysinfo_load_release_file(self, os_release_paths, OS_RELEASE_SECTION);
}

/** Load CSD hw feature configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_load_hw_settings(ssusysinfo_t *self)
{
    glob_t gl = {};

    if (glob("/usr/share/csd/settings.d/*hw-settings*.ini", 0, 0, &gl) == 0) {
        for (size_t i = 0; i < gl.gl_pathc; ++i)
            inifile_load(self->cfg_ini, gl.gl_pathv[i], 0);
    }

    globfree(&gl);
}

/** Load SSU configuration files
 *
 * @param self ssusysinfo object pointer
 */
static void
ssusysinfo_load_ssu_config(ssusysinfo_t *self)
{
    inifile_load(self->ssu_ini, "/etc/ssu/ssu.ini", 0);

    int version_want = EXPECTED_SSU_CONFIG_VERSION;
    int version_have = ssusysinfo_ssu_config_version(self);
    if( version_have != version_want ) {
        log_warning("expected ssu config version %d, found %d",
                    version_want, version_have);
    }
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
    self->ssu_ini = inifile_create();

    ssusysinfo_load_ssu_config(self);
    ssusysinfo_load_board_mappings(self);
    ssusysinfo_load_release_info(self);
    ssusysinfo_load_hw_settings(self);

#if 0 /* for devel time debugging */
    inifile_dump(self->cfg_ini);
    inifile_dump(self->ssu_ini);
#endif

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
    inifile_delete(self->ssu_ini),
        self->ssu_ini = 0;

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

    res = inifile_get(self->cfg_ini, HW_RELEASE_SECTION, "MER_HA_DEVICE", 0);

EXIT:
    return res;
}

/** Lookup a key in device mode specific section from board mappings
 *
 * @param self ssusysinfo object pointer
 * @param key  key name used in board config ini files
 *
 * @return c-string
 */
static const char *
ssusysinfo_device_attr(ssusysinfo_t *self, const char *key)
{
    const char *cached = 0;
    const char *probed = 0;
    const char *model  = 0;
    const char *base   = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    /* Check if this attr has already been resolved */
    if( (cached = inifile_get(self->cfg_ini, "cached-attrs", key, 0)) )
        goto EXIT;

    /* Attempt to resolve based on model name */
    if( (model = ssusysinfo_device_model(self)) ) {
        if( (probed = inifile_get(self->cfg_ini, model, key, 0)) )
            goto CACHE;
    }

    /* In case of variant, attempt to resolve based on base model name */
    if( (base = ssusysinfo_device_base_model(self)) ) {
        if( (probed = inifile_get(self->cfg_ini, base, key, 0)) )
            goto CACHE;
    }

    /* Use model name as fallback for some attrs */
    if( !strcmp(key, "deviceDesignation") || !strcmp(key, "prettyModel") )
        probed = model;

    /* And as an ultimate fallback select unknown */
    if( !probed )
        probed = ssusysinfo_unknown;

CACHE:
    /* Update the cache so that we do not need to repeat the above
     * heuristics the next time */
    inifile_set(self->cfg_ini, "cached-attrs", key, (cached = probed));

EXIT:

    /* Always return valid c-string */
    return cached ?: ssusysinfo_unknown;
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
ssusysinfo_device_base_model(ssusysinfo_t *self)
{
    const char *cached = 0;
    const char *probed = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( (cached = inifile_get(self->cfg_ini, "cached-values", "base_model", 0)) )
        goto EXIT;

    /* Get model name, which is potentially a variant */
    const char *model = ssusysinfo_device_model(self);

    /* Lookup base model from [variants] */
    if( (probed = inifile_get(self->cfg_ini, "variants", model, 0)) )
        goto CACHE;

    /* We have data, but were unable to determine base model */
    probed = ssusysinfo_unknown;

CACHE:
    /* Update the cache so that we do not need to repeat the above
     * heuristics the next time */
    inifile_set(self->cfg_ini, "cached-values", "base_model", (cached = probed));

EXIT:
    /* Always return valid c-string */
    return cached ?: ssusysinfo_unknown;
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
    /* Always returns valid c-string */
    return ssusysinfo_device_attr(self, "deviceDesignation");
}

const char *
ssusysinfo_device_manufacturer(ssusysinfo_t *self)
{
    /* Always returns valid c-string */
    return ssusysinfo_device_attr(self, "deviceManufacturer");
}

const char *
ssusysinfo_device_pretty_name(ssusysinfo_t *self)
{
    /* Always returns valid c-string */
    return ssusysinfo_device_attr(self, "prettyModel");
}

static const char *
ssusysinfo_ssu_attr_ex(ssusysinfo_t *self, const char *sec, const char *key)
{
    const char *res = 0;
    if( self && self->ssu_ini )
        res = inifile_get(self->ssu_ini, sec, key, 0);
    /* Always return valid c-string */
    return res ?: ssusysinfo_unknown;
}

static const char *
ssusysinfo_ssu_attr(ssusysinfo_t *self, const char *key)
{
    return ssusysinfo_ssu_attr_ex(self, "General", key);
}

int
ssusysinfo_ssu_config_version(ssusysinfo_t *self)
{
    const char *val = ssusysinfo_ssu_attr(self, "configVersion");
    return (int)strtol(val, 0, 0);
}

bool
ssusysinfo_ssu_registered(ssusysinfo_t *self)
{
    const char *val = ssusysinfo_ssu_attr(self, "registered");
    return !strcmp(val, "true");
}

ssu_device_mode_t
ssusysinfo_ssu_device_mode(ssusysinfo_t *self)
{
    const char *val = ssusysinfo_ssu_attr(self, "deviceMode");
    return (ssu_device_mode_t)strtol(val, 0, 0);
}

bool
ssusysinfo_ssu_in_rnd_mode(ssusysinfo_t *self)
{
    ssu_device_mode_t mode = ssusysinfo_ssu_device_mode(self);
    return (mode & SSU_DEVICE_MODE_RND) != 0;
}

const char *
ssusysinfo_ssu_arch(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "arch");
}

const char *
ssusysinfo_ssu_brand(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "brand");
}

const char *
ssusysinfo_ssu_flavour(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "flavour");
}

const char *
ssusysinfo_ssu_domain(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "domain");
}

const char *
ssusysinfo_ssu_release(ssusysinfo_t *self)
{
    if( ssusysinfo_ssu_in_rnd_mode(self) )
        return ssusysinfo_ssu_rnd_release(self);
    return ssusysinfo_ssu_def_release(self);
}

const char *
ssusysinfo_ssu_def_release(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "release");
}

const char *
ssusysinfo_ssu_rnd_release(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "rndRelease");
}

const char *
ssusysinfo_ssu_enabled_repos(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "enabled-repos");
}

const char *
ssusysinfo_ssu_disabled_repos(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "disabled-repos");
}

const char *
ssusysinfo_ssu_last_credentials_update(ssusysinfo_t *self)
{
    const char key[] = "lastCredentialsUpdate";
    const char *cached = 0;

    /* We need to demangle DateTime formatting. While the
     * process does not take that much cpu time, caching the
     * result yields the same lifetime and constness for the
     * returned data as what the more direct lookups have.
     */

    if( (cached = inifile_get(self->cfg_ini, "cached-values", key, 0)) )
        goto EXIT;

    const char *datetime = ssusysinfo_ssu_attr(self, key);
    char *probed = qtdecoder_parse_datetime(datetime);
    inifile_set(self->cfg_ini, "cached-values", key, probed ?: datetime);
    free(probed);

    cached = inifile_get(self->cfg_ini, "cached-values", key, 0);

EXIT:
    return cached ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_ssu_credentials_scope(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "credentials-scope");
}

static const char *
ssusysinfo_ssu_credentials_url(ssusysinfo_t *self, const char *scope)
{
    /* Stored in ssu.ini as:
     *
     * [General]
     * credentials-url-<SCOPE> = <URL>
     */
    char key[256];
    snprintf(key, sizeof key, "credentials-url-%s", scope);
    return ssusysinfo_ssu_attr(self, key);
}

const char *
ssusysinfo_ssu_credentials_url_jolla(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_credentials_url(self, "jolla");
}

const char *
ssusysinfo_ssu_credentials_url_store(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_credentials_url(self, "store");
}

#if SSU_INCLUDE_CREDENTIAL_ITEMS
/* While access to the SSU values related to identity and authentication
 * is not limited, we need to take care to minimize chances of accidental
 * exposure - for example in situations where ssu-sysinfo might be used by
 * end users to provide information in context of debugging some issues.
 *
 * Code is left in for purposes of documentation and possible future use,
 * but not compiled into the binary.
 */

static const char *
ssusysinfo_ssu_credentials_username(ssusysinfo_t *self, const char *scope)
{
    /* Stored in ssu.ini as:
     *
     * [credentials-<SCOPE>]
     * username=<USERNAME>
     */
    char sec[256];
    snprintf(sec, sizeof sec, "credentials-%s", scope);
    return ssusysinfo_ssu_attr_ex(self, sec, "username");
}

/** Query ssu jolla credentials username setting
 *
 * Currently fetches "username" value from "credentials-jolla" section in ssu.ini.
 *
 * @return username / username hash
 */
const char *
ssusysinfo_ssu_credentials_username_jolla(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_credentials_username(self, "jolla");
}

/** Query ssu store credentials username setting
 *
 * Currently fetches "username" value from "credentials-jolla" section in ssu.ini.
 *
 * @return username / username hash
 */
const char *
ssusysinfo_ssu_credentials_username_store(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_credentials_username(self, "store");
}

static const char *
ssusysinfo_ssu_credentials_password(ssusysinfo_t *self, const char *scope)
{
    /* Stored in ssu.ini as:
     *
     * [credentials-<SCOPE>]
     * password=<PASSWORD>
     */
    char sec[256];
    snprintf(sec, sizeof sec, "credentials-%s", scope);
    return ssusysinfo_ssu_attr_ex(self, sec, "password");
}

/** Query ssu jolla credentials password setting
 *
 * Currently fetches "password" value from "credentials-jolla" section in ssu.ini.
 *
 * @return password hash
 */
const char *
ssusysinfo_ssu_credentials_password_jolla(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_credentials_password(self, "jolla");
}

/** Query ssu store credentials password setting
 *
 * Currently fetches "password" value from "credentials-jolla" section in ssu.ini.
 *
 * @return password hash
 */
const char *
ssusysinfo_ssu_credentials_password_store(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_credentials_password(self, "store");
}

/** Query ssu certificate setting
 *
 * Currently fetches "certificate" value from "General" section in ssu.ini.
 *
 * @return certificate blob
 */
const char *
ssusysinfo_ssu_certificate(ssusysinfo_t *self)
{
    const char key[] = "certificate";
    const char *cached = 0;

    /* We need to demangle ByteArray formatting. While the
     * process does not take that much cpu time, caching the
     * result yields the same lifetime and constness for the
     * returned data as what the more direct lookups have.
     */

    if( (cached = inifile_get(self->cfg_ini, "cached-values", key, 0)) )
        goto EXIT;

    const char *bytearray = ssusysinfo_ssu_attr(self, key);
    size_t length = 0;
    char *probed = qtdecoder_parse_bytearray(bytearray, &length);
    if( probed && strlen(probed) != length )
        log_warning("%s: has embedded NUL chars", key);
    inifile_set(self->cfg_ini, "cached-values", key, probed ?: bytearray);
    free(probed);

    cached = inifile_get(self->cfg_ini, "cached-values", key, 0);

EXIT:
    return cached ?: ssusysinfo_unknown;
}

/** Query ssu private key setting
 *
 * Currently fetches "privateKey" value from "General" section in ssu.ini.
 *
 * @return key blob
 */
const char *
ssusysinfo_ssu_private_key(ssusysinfo_t *self)
{
    const char key[] = "privateKey";
    const char *cached = 0;

    /* We need to demangle ByteArray formatting. While the
     * process does not take that much cpu time, caching the
     * result yields the same lifetime and constness for the
     * returned data as what the more direct lookups have.
     */

    if( (cached = inifile_get(self->cfg_ini, "cached-values", key, 0)) )
        goto EXIT;

    const char *bytearray = ssusysinfo_ssu_attr(self, key);
    size_t length = 0;
    char *probed = qtdecoder_parse_bytearray(bytearray, &length);
    if( probed && strlen(probed) != length )
        log_warning("%s: has embedded NUL chars", key);
    inifile_set(self->cfg_ini, "cached-values", key, probed ?: bytearray);
    free(probed);

    cached = inifile_get(self->cfg_ini, "cached-values", key, 0);

EXIT:
    return cached ?: ssusysinfo_unknown;
}
#endif /* SSU_INCLUDE_CREDENTIAL_ITEMS */

const char *
ssusysinfo_ssu_default_rnd_domain(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "default-rnd-domain");
}

const char *
ssusysinfo_ssu_home_url(ssusysinfo_t *self)
{
    return ssusysinfo_ssu_attr(self, "home-url");
}

const char *
ssusysinfo_os_name(ssusysinfo_t *self)
{
    const char *res = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    res = inifile_get(self->cfg_ini, OS_RELEASE_SECTION, "NAME", 0);

EXIT:
    return res ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_os_version(ssusysinfo_t *self)
{
    const char *res = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    res = inifile_get(self->cfg_ini, OS_RELEASE_SECTION, "VERSION_ID", 0);

EXIT:
    return res ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_os_pretty_version(ssusysinfo_t *self)
{
    const char *res = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    res = inifile_get(self->cfg_ini, OS_RELEASE_SECTION, "VERSION", 0);

EXIT:
    return res ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_hw_version(ssusysinfo_t *self)
{
    const char *res = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    res = inifile_get(self->cfg_ini, HW_RELEASE_SECTION, "VERSION_ID", 0);

EXIT:
    return res ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_hw_pretty_version(ssusysinfo_t *self)
{
    const char *res = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    res = inifile_get(self->cfg_ini, HW_RELEASE_SECTION, "VERSION", 0);

EXIT:
    return res ?: ssusysinfo_unknown;
}

const char *
ssusysinfo_board_version(ssusysinfo_t *self)
{
    static const char path[] = "/sys/firmware/devicetree/base/model";
    static const char sec[]  = "cached-values";
    static const char key[]  = "BOARD_VERSION";

    const char *cached = NULL;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( !(cached = inifile_get(self->cfg_ini, sec, key, NULL)) ) {
        char *probed = NULL;
        if( fileutil_exists(path) ) {
            if( (probed = fileutil_read(path, NULL)) )
                strutil_trim(probed);
        }
        inifile_set(self->cfg_ini, sec, key,
                    probed && *probed ? probed : ssusysinfo_unknown);
        cached = inifile_get(self->cfg_ini, sec, key, NULL);
        free(probed);
    }

EXIT:
    return cached ?: ssusysinfo_unknown;
}

#if SSU_INCLUDE_UNUSED_ITEMS
/* Accessor functions for ssu.ini items were written in mass,
 * before discovering that there are some values present in
 * existing ssu.ini files that might be vestigial remnants from
 * distant past i.e. not used by the current SSU implementation.
 *
 * The already written functions are left as a kind of documentation
 * about the unusedness of the values ...
 */

/** Query ssu initialization status
 *
 * Currently fetches "initialized" value from "General" section in ssu.ini.
 *
 * @note This setting is not used by SSU.
 *
 * @return true if ssu is initialized, false otherwise
 */
bool
ssusysinfo_ssu_initialized(ssusysinfo_t *self)
{
    /* XXX: Not used in SSU - leftover legacy fluff? */
    const char *val = ssusysinfo_ssu_attr(self, "initialized");
    return !strcmp(val, "true");
}

/** Query ssu credentials time to live setting
 *
 * Currently fetches "credentials-ttl" value from "General" section in ssu.ini.
 *
 * Expected values: 1800, ...
 *
 * @note This setting is not used by SSU.
 *
 * @return ttl value
 */
int
ssusysinfo_ssu_credentials_ttl(ssusysinfo_t *self)
{
    /* XXX: Not used in SSU - leftover legacy fluff? */
    const char *val = ssusysinfo_ssu_attr(self, "credentials-ttl");
    return strtol(val, 0, 0);
}

/** Query ssu credential scopes list
 *
 * Currently fetches "credentialScopes" value from "General" section in ssu.ini.
 *
 * Expected values: "jolla, something_else", ...
 *
 * @note SSU has logic for writing this value, but it is never read.
 *
 * @return list of scope names
 */
const char *
ssusysinfo_ssu_credential_scopes(ssusysinfo_t *self)
{
    /* XXX: Only written in SSU - leftover legacy fluff? */
    return ssusysinfo_ssu_attr(self, "credentialScopes");
}

#endif /* SSU_INCLUDE_UNUSED_ITEMS */

/* ------------------------------------------------------------------------- *
 * HW Features
 * ------------------------------------------------------------------------- */

const char *
ssusysinfo_hw_feature_to_name(hw_feature_t id)
{
    return hw_feature_is_valid(id) ? hw_feature_to_string(id) : 0;
}

hw_feature_t
ssusysinfo_hw_feature_from_name(const char *name)
{
    return hw_feature_from_string(name);
}

bool
ssusysinfo_has_hw_feature(ssusysinfo_t *self, hw_feature_t id)
{
    bool supported = false;

    if( !self || !self->cfg_ini )
        goto EXIT;

    if( !hw_feature_is_valid(id) )
        goto EXIT;

    const char *key = hw_feature_to_csd_key(id);
    const char *val = inifile_get(self->cfg_ini, "features", key, 0);

    if( val )
        supported = (strtol(val, 0, 0) != 0);
    else
        supported = hw_feature_get_fallback(id);

EXIT:
    return supported;
}

hw_feature_t *
ssusysinfo_get_hw_features(ssusysinfo_t *self)
{
    hw_feature_t *data = 0;
    size_t        used = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    data = xcalloc(Feature_Count, sizeof *data);

    for( hw_feature_t id = Feature_Invalid + 1; id < Feature_Count; ++id ) {
        if( ssusysinfo_has_hw_feature(self, id) )
            data[used++] = id;
    }
    data[used] = Feature_Invalid;

EXIT:
    return data;
}

const char **
ssusysinfo_hw_feature_names(void)
{
    return hw_feature_names();
}

/* ------------------------------------------------------------------------- *
 * HW Keys
 * ------------------------------------------------------------------------- */

const char *
ssusysinfo_hw_key_to_name(hw_key_t code)
{
    return hw_key_to_string(code);
}

hw_key_t
ssusysinfo_hw_key_from_name(const char *name)
{
    return name ? hw_key_from_string(name) : 0;
}

hw_key_t *
ssusysinfo_get_hw_keys(ssusysinfo_t *self)
{
    hw_key_t *data = 0;

    if( !self || !self->cfg_ini )
        goto EXIT;

    data = hw_key_parse_array(inifile_get(self->cfg_ini, "Keys", "Keys", 0));

EXIT:
    return data;
}

bool
ssusysinfo_has_hw_key(ssusysinfo_t *self, hw_key_t code)
{
    bool supported = false;

    hw_key_t *keys = 0;

    if( !hw_key_is_valid(code) )
        goto EXIT;

    if( !(keys = ssusysinfo_get_hw_keys(self)) )
        goto EXIT;

    for( size_t i = 0; keys[i]; ++i ) {
        if( keys[i] == code ) {
            supported = true;
            break;
        }
    }

EXIT:
    free(keys);

    return supported;
}

const char **
ssusysinfo_hw_key_names(void)
{
    return hw_key_names();
}
