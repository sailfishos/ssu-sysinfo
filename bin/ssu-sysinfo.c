/** @file ssu-sysinfo.c
 *
 * ssu-sysinfo - Command line utility for making queries
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

#include "../lib/ssusysinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/* ========================================================================= *
 * LOAD ON DEMAND
 * ========================================================================= */

/** Cached config data handle */
static ssusysinfo_t *cfg_handle = 0;

/** Atexit callback for releasing config handle
 */
static void
del_cfg(void)
{
    ssusysinfo_delete(cfg_handle),
        cfg_handle = 0;
}

/** Load & cache config data
 */
static ssusysinfo_t *
get_cfg(void)
{
    if( !cfg_handle ) {
        cfg_handle = ssusysinfo_create();
        atexit(del_cfg);
    }
    return cfg_handle;
}

/* ========================================================================= *
 * COMMAND LINE OPTIONS
 * ========================================================================= */

/** Lookup table for long option parsing */
const struct option opt_long[] =
{
    {"help",          no_argument, 0, 'h'},
    {"usage",         no_argument, 0, 'h'},
    {"model",         no_argument, 0, 'm'},
    {"designation",   no_argument, 0, 'd'},
    {"manufacturer",  no_argument, 0, 'M'},
    {"pretty-name",   no_argument, 0, 'p'},
    {"all",           no_argument, 0, 'a'},
    {0, 0, 0, 0}
};

/** Lookup string for short option parsing */
const char opt_short[] =
"h" // --help
"m" // --model
"d" // --designation
"M" // --manufacturer
"p" // --pretty-name
"a" // --all
;

/** Freeform usage text */
const char opt_help[] =
"\n"
"  -h --help         Print usage information\n"
"\n"
"  -m --model        Print device model\n"
"  -d --designation  Print device designation\n"
"  -M --manufacturer Print device manufacturer\n"
"  -p --pretty-name  Print device pretty name\n"
"  -a --all          Print all of the above\n"
"\n"
;

/** Handler for --help option
 */
static void
output_usage(const char *name)
{
    fprintf(stdout, "USAGE: %s <options>\n%s", name, opt_help);
    exit(EXIT_SUCCESS);
}

/** Handler for --all option
 */
static void
output_all(void)
{
    printf("model: %s\n",        ssusysinfo_device_model(get_cfg()));
    printf("designation: %s\n",  ssusysinfo_device_designation(get_cfg()));
    printf("manufacturer: %s\n", ssusysinfo_device_manufacturer(get_cfg()));
    printf("pretty_name: %s\n",  ssusysinfo_device_pretty_name(get_cfg()));
}

/** Handler for --model option
 */
static void
output_model(void)
{
    printf("%s\n", ssusysinfo_device_model(get_cfg()));
}

/** Handler for --designation option
 */
static void
output_designation(void)
{
    printf("%s\n", ssusysinfo_device_designation(get_cfg()));
}

/** Handler for --manufacturer option
 */
static void
output_manufacturer(void)
{
    printf("%s\n", ssusysinfo_device_manufacturer(get_cfg()));
}

/** Handler for --pretty-name option
 */
static void
output_pretty_name(void)
{
    printf("%s\n", ssusysinfo_device_pretty_name(get_cfg()));
}

/* ========================================================================= *
 * MAIN_ENTRY_POINT
 * ========================================================================= */

int
main(int ac, char **av)
{
    const char *progname  = av[0];
    int         exitcode  = EXIT_FAILURE;

    /* Treat no-args as if --all option were given */
    if( ac == 1 ) {
        output_all();
        goto DONE;
    }

    /* Handle options */
    for( ;; ) {
        int opt = getopt_long(ac, av, opt_short, opt_long, 0);

        if( opt == -1 )
            break;

        switch( opt ) {
        case 'h':
            output_usage(progname);
            break;

        case 'm':
            output_model();
            break;

        case 'd':
            output_designation();
            break;

        case 'M':
            output_manufacturer();
            break;

        case 'p':
            output_pretty_name();
            break;

        case 'a':
            output_all();
            break;

        case '?':
            fprintf(stderr, "(use --help for instructions)\n");
            goto EXIT;
        }
    }

    /* Complain about excess args */
    if( optind < ac ) {
        fprintf(stderr, "%s: unknown argument\n", av[optind]);
        fprintf(stderr, "(use --help for instructions)\n");
        goto EXIT;
    }

DONE:
    exitcode = EXIT_SUCCESS;

EXIT:

    return exitcode;
}
