/** @file logging.c
 *
 * ssu-sysinfo - Diagnostic logging functionality
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

#include "logging.h"

#include "xmalloc.h"
#include "util.h"

#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

/* ========================================================================= *
 * State Data
 * ========================================================================= */

/** Where to log; default to syslog */
static log_target_t log_target_cached = LOG_TO_UNDEFINED;

/** Logging level */
static int         log_level_cached   = -1;

/** Prognane prefix to use */
static const char *log_progname_cached = 0;

/* ========================================================================= *
 * Prototypes
 * ========================================================================= */

/* -- log -- */

static void        log_getmonotime  (struct timeval *tv);
static const char *log_timestamp    (void);
void               log_set_target   (log_target_t target);
static const char *log_pfix         (int lev);
static const char *log_color        (int lev);
void               log_set_progname (const char *name);
void               log_set_verbosity(int lev);
bool               log_p_           (const char *file, const char *func, int lev);
static void        log_to_stream    (int lev, const char *msg, FILE *fh);
static int         log_map_level    (int lev);
void               log_emit_va      (const char *file, const char *func, int lev, const char *fmt, va_list va);
void               log_emit_        (const char *file, const char *func, int lev, const char *fmt, ...);

#ifdef DEAD_CODE
void               log_fatal_       (const char *file, const char *func, int lev, const char *fmt, ...);
#endif

/* ========================================================================= *
 * Functions
 * ========================================================================= */

static void
log_getmonotime(struct timeval *tv)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    TIMESPEC_TO_TIMEVAL(tv, &ts);
}

static const char *
log_timestamp(void)
{
    static const struct timeval lim = { 2, 0 };

    static struct timeval t0, t1;
    struct timeval t2, d0, d1;

    log_getmonotime(&t2);

    if( !timerisset(&t0) )
        t0 = t1 = t2;

    timersub(&t2, &t0, &d0);
    timersub(&t2, &t1, &d1);

    static char buf[64];

    snprintf(buf, sizeof buf, "%6.3f %+7.3f",
             d0.tv_sec + d0.tv_usec * 1e-6,
             d1.tv_sec + d1.tv_usec * 1e-6);

    if( timercmp(&lim, &d1, <) )
        t0 = t2;

    t1 = t2;

    return buf;
}

/** Evaluate logging target
 */
static log_target_t
log_get_target(void)
{
    if( log_target_cached == LOG_TO_UNDEFINED ) {
        log_target_t use = LOG_TO_SYSLOG;

        const char *env = getenv("SSUSYSINFO_LOG_TARGET");

        if( strutil_equals(env, "stderr") )
            use = LOG_TO_STDERR;
        else if( strutil_equals(env, "stdout") )
            use = LOG_TO_STDOUT;

        log_target_cached = use;
    }
    return log_target_cached;
}

/** Get logging level prefix helper
 *
 * @param lev logging level
 *
 * @return prefix string
 */
static const char *
log_pfix(int lev)
{
    const char *res = "?";

    switch( lev ) {
    case LOG_EMERG:   res = "X"; break;
    case LOG_ALERT:   res = "A"; break;
    case LOG_CRIT:    res = "C"; break;
    case LOG_ERR:     res = "E"; break;
    case LOG_WARNING: res = "W"; break;
    case LOG_NOTICE:  res = "N"; break;
    case LOG_INFO:    res = "I"; break;
    case LOG_DEBUG:   res = "D"; break;
    case LOG_TRACE:   res = "T"; break;
    default: break;
    }

    return res;
}

static const char *
log_color(int lev)
{
    const char *res = "\33[0m";

    switch( lev ) {
    case LOG_EMERG:   res = "\33[34m"; break;
    case LOG_ALERT:   res = "\33[34m"; break;
    case LOG_CRIT:    res = "\33[34m"; break;
    case LOG_ERR:     res = "\33[31m"; break;
    case LOG_WARNING: res = "\33[33m"; break;
    case LOG_NOTICE:  res = "\33[32m"; break;
    case LOG_INFO:    res = "\33[36m"; break;
    case LOG_DEBUG:   res = "\33[90m"; break;
    case LOG_TRACE:   res = "\33[90m"; break;
    default: break;
    }

    return res;
}

/** Evaluate progname to use when logging
 */
static const char *
log_get_progname(void)
{
    if( !log_progname_cached ) {
        char tmp[128];

        /* Try cmdline 1st, as exe is likely to be booster binary */
        int fd = open("/proc/self/cmdline", O_RDONLY);
        int rc = read(fd, tmp, sizeof tmp - 1);
        close(fd);

        /* Try exe symlink */
        if( rc <= 0 )
            rc = readlink("/proc/self/exe", tmp, sizeof tmp - 1);

        /* Use whichever we got, or set to default */
        if( rc > 0 )
            tmp[rc] = 0, log_progname_cached = xstrdup(tmp);
        else
            log_progname_cached = "unknown";
    }
    return log_progname_cached;
}

/** Evaluate logging verbosity
 */
static int
log_get_verbosity(void)
{
    if( log_level_cached < 0 ) {

        int use = LOG_WARNING;

        const char *env = getenv("SSUSYSINFO_LOG_LEVEL");
        if( env )
            use = strtol(env, 0, 0);

        if( use < LOGGING_MIN_LEVEL )
            use = LOGGING_MIN_LEVEL;

        if( use > LOGGING_MAX_LEVEL )
            use = LOGGING_MAX_LEVEL;

        log_level_cached = use;
    }

    return log_level_cached;
}

/** Logging enabled for level predicate
 *
 * @param lev logging level (LOG_CRIT ... LOG_TRACE)
 *
 * return true if lev would be logged, false otherwise
 */
bool
log_p_(const char *file, const char *func, int lev)
{
    (void)file, (void)func;

    return lev <= log_get_verbosity();
}

/** Emit formatted message to stdio stream
 *
 * @param lev logging level (LOG_CRIT ... LOG_TRACE)
 * @param msg formatted and stripped message
 * @param fh  file handle for stream to write to
 */
static void
log_to_stream(int lev, const char *msg, FILE *fh)
{
    if( fh && msg && !ferror(fh) ) {
        fprintf(fh, "%s: %s%s %s: %s%s\n",
                log_get_progname(),
                log_color(lev),
                log_timestamp(),
                log_pfix(lev),
                msg,
                log_color(-1));
        fflush(fh);
    }
}

static int
log_map_level(int lev)
{
    if( lev < LOG_CRIT )
        return LOG_CRIT;
    if( lev > LOG_DEBUG )
        return LOG_DEBUG;
    return lev;
}

/** Emit logging message - va_list version
 *
 * Notes:
 * - Excess white space will be trimmed from the formatted
 *   message - similarly to what syslog does.
 * - Linefeeds are not obeyed, each message will be one line
 * - The errno content is preserved over the call
 *
 * @param lev logging level (LOG_CRIT ... LOG_TRACE)
 * @param fmt printf style format string
 * @param va  arguments needed by the format string
 */
void
log_emit_va(const char *file, const char *func, int lev, const char *fmt, va_list va)
{
    if( log_p_(file, func, lev) ) {
        int saved = errno;
        char *msg = 0;

        if( vasprintf(&msg, fmt, va) < 0 )
            msg = 0;

        switch( log_get_target() ) {
        case LOG_TO_STDERR:
            log_to_stream(lev, strutil_strip(msg) ?: fmt, stderr);
            break;

        case LOG_TO_STDOUT:
            log_to_stream(lev, strutil_strip(msg) ?: fmt, stdout);
            break;

        default:
        case LOG_TO_SYSLOG:
            syslog(log_map_level(lev), "%s", msg ?: fmt);
            break;
        }
        free(msg);
        errno = saved;
    }

    if( lev <= LOG_ALERT ) {
        const char m[] = "*** FATAL\n\n";
        if( write(2, m, sizeof m - 1) < 0 ) { }
        _exit(EXIT_FAILURE);
        //abort();
    }
}

/** Emit logging message
 *
 * Notes:
 * - Excess white space will be trimmed from the formatted
 *   message - similarly to what syslog does.
 * - Linefeeds are not obeyed, each message will be one line
 * - The errno content is preserved over the call
 *
 * @param lev logging level (LOG_CRIT ... LOG_TRACE)
 * @param fmt printf style format string
 * @param ... arguments needed by the format string
 */
void
log_emit_(const char *file, const char *func, int lev, const char *fmt, ...)
{
    // assume: va_start() & va_end() do not modify errno
    va_list va;
    va_start(va, fmt);
    log_emit_va(file, func, lev, fmt, va);
    va_end(va);
}

#ifdef DEAD_CODE
void
log_fatal_(const char *file, const char *func, int lev, const char *fmt, ...)
{
    // assume: va_start() & va_end() do not modify errno
    va_list va;
    va_start(va, fmt);
    log_emit_va(file, func, lev, fmt, va);
    va_end(va);
    abort();
}
#endif
