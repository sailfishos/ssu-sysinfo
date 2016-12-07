/** @file logging.h
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

#ifndef  LOGGING_H_
# define LOGGING_H_

# include <stdarg.h>
# include <stdbool.h>
# include <syslog.h>

/* ========================================================================= *
 * Configuration
 * ========================================================================= */

/* How much logging functionality to compile into the binary */
# ifndef  LOGGING_LEVEL
#  define LOGGING_LEVEL LOG_DEBUG
# endif

/* Non-standard trace level for frequently occurring things */
#define LOG_TRACE 8 /* = LOG_DEBUG + 1, but must be literal */

/* Bounds for runtime logging verbosity selection */
enum
{
    LOGGING_MIN_LEVEL = LOG_EMERG,
    LOGGING_MAX_LEVEL = LOG_TRACE,
};

/* ========================================================================= *
 * Constants
 * ========================================================================= */

/* Where to log */
typedef enum {
    LOG_TO_UNDEFINED,
    LOG_TO_SYSLOG,
    LOG_TO_STDERR,
    LOG_TO_STDOUT,
} log_target_t;

/* ========================================================================= *
 * Functions
 * ========================================================================= */

bool log_p_           (const char *file, const char *func, int lev);
void log_emit_va      (const char *file, const char *func, int lev, const char *fmt, va_list va);
void log_emit_        (const char *file, const char *func, int lev, const char *fmt, ...) __attribute__((format(printf, 4, 5)));

#ifdef DEAD_CODE
void log_fatal_       (const char *file, const char *func, int lev, const char *fmt, ...) __attribute__((format(printf, 4, 5))) __attribute__ ((__noreturn__));
#endif

/* ========================================================================= *
 * Macros
 * ========================================================================= */

# define log_p(LEV) log_p_(__FILE__,__FUNCTION__,LEV)

# define log_emit(LEV,FMT,ARG...)  log_emit_(__FILE__, __FUNCTION__, LEV, FMT, ## ARG)

# define log_fatal(   FMT, ARG...) log_fatal_(__FILE__, __FUNCTION__, LOG_ALERT, "%s:" FMT, __FUNCTION__, ## ARG)

# if  LOGGING_LEVEL >= LOG_CRIT
#  define log_crit(   FMT, ARG...) do { if( log_p(LOG_CRIT) ) log_emit(LOG_CRIT, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_crit(   ...)         do {  } while(0)
# endif

# if  LOGGING_LEVEL >= LOG_ERR
#  define log_err(    FMT, ARG...) do { if( log_p(LOG_ERR) ) log_emit(LOG_ERR, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_err(    ...)         do {  } while(0)
# endif

# if  LOGGING_LEVEL >= LOG_WARNING
#  define log_warning(FMT, ARG...) do { if( log_p(LOG_WARNING) ) log_emit(LOG_WARNING, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_warning(...)         do {  } while(0)
# endif

# if  LOGGING_LEVEL >= LOG_NOTICE
#  define log_notice( FMT, ARG...) do { if( log_p(LOG_NOTICE) ) log_emit(LOG_NOTICE, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_notice( ...)         do {  } while(0)
# endif

# if  LOGGING_LEVEL >= LOG_INFO
#  define log_info(   FMT, ARG...) do { if( log_p(LOG_INFO) ) log_emit(LOG_INFO, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_info(   ...)         do {  } while(0)
# endif

# if  LOGGING_LEVEL >= LOG_DEBUG
#  define log_debug(  FMT, ARG...) do { if( log_p(LOG_DEBUG) ) log_emit(LOG_DEBUG, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_debug(  ...)         do {  } while(0)
# endif

# if  LOGGING_LEVEL >= LOG_TRACE
#  define log_trace(  FMT, ARG...) do { if( log_p(LOG_TRACE) ) log_emit(LOG_TRACE, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)
# else
#  define log_trace(  ...)         do {  } while(0)
# endif

# define  log_devel(  FMT, ARG...) do { if( log_p(LOG_CRIT) ) log_emit(LOG_CRIT, "%s: "FMT, __FUNCTION__, ## ARG); } while(0)

#endif /* LOGGING_H_ */
