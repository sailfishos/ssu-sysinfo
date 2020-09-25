/** @file ssu-sysinfo.c
 *
 * ssu-sysinfo - Command line utility for making queries
 * <p>
 * Copyright (c) 2016-2017 Jolla Ltd.
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
#include <string.h>
#include <getopt.h>

/* ========================================================================= *
 * Types
 * ========================================================================= */

typedef struct
{
  const char *name;
  int         mask;
  int         bits;
} bitfield_t;

/* ========================================================================= *
 * Functions
 * ========================================================================= */

static char         *bitfield_repr          (const bitfield_t *lut, int bits, char *buf, size_t size);
static void          del_cfg                (void);
static ssusysinfo_t *get_cfg                (void);
static void          output_usage           (const char *name);
static void          output_ssu_info        (void);
#if SSU_INCLUDE_CREDENTIAL_ITEMS
static void          output_ssu_certificate (void);
static void          output_ssu_private_key (void);
#endif
static void          output_arch            (void);
static void          output_brand           (void);
static void          output_flavour         (void);
static void          output_domain          (void);
static void          output_release         (void);
static void          output_all             (void);
static void          output_device_info     (void);
static void          output_model           (void);
static void          output_designation     (void);
static void          output_manufacturer    (void);
static void          output_pretty_name     (void);
static void          output_list_hw_features(void);
static void          output_hw_features     (void);
static bool          require_has_hw_feature (const char *name);
static void          output_list_hw_keys    (void);
static void          output_hw_keys         (void);
static bool          require_has_hw_key     (const char *name);

/* ========================================================================= *
 * BITFIELD
 * ========================================================================= */

static char *
bitfield_repr(const bitfield_t *lut, int bits, char *buf, size_t size)
{
    char *pos = buf;
    char *end = buf + size - 1;

    auto void adds(const char *str) {
        if( pos > buf && pos < end )
            *pos++ = '|';
        while( *str && pos < end )
            *pos++ = *str++;
    }

    for( ; lut->name; ++lut ) {
        if( (bits & lut->mask) == lut->bits ) {
            bits &= ~lut->mask;
            adds(lut->name);
        }
    }

    if( bits ) {
        char num[16];
        snprintf(num, sizeof num, "0x%x", bits);
        adds(num);
    }

    *pos = 0;

    return buf;
}

static const bitfield_t bitfield_device_mode[] =
{
    {
        .name = "DISABLE_REPO_MANAGER",
        .mask = SSU_DEVICE_MODE_DISABLE_REPO_MANAGER,
        .bits = SSU_DEVICE_MODE_DISABLE_REPO_MANAGER,
    },
    {
        /* Neither RND nor RELEASE set -> RELEASE implied */
        .name = "IMPLIED_RELEASE",
        .mask = SSU_DEVICE_MODE_RND | SSU_DEVICE_MODE_RELEASE,
        .bits = 0,
    },
    {
        /* Both RND and RELEASE set -> RND plus RELEASE repos used */
        .name = "RND_AND_RELEASE",
        .mask = SSU_DEVICE_MODE_RND | SSU_DEVICE_MODE_RELEASE,
        .bits = SSU_DEVICE_MODE_RND | SSU_DEVICE_MODE_RELEASE,
    },
    {
        .name = "RND",
        .mask = SSU_DEVICE_MODE_RND,
        .bits = SSU_DEVICE_MODE_RND,
    },
    {
        .name = "RELEASE",
        .mask = SSU_DEVICE_MODE_RELEASE,
        .bits = SSU_DEVICE_MODE_RELEASE,
    },
    {
        .name = "LENIENT",
        .mask = SSU_DEVICE_MODE_LENIENT,
        .bits = SSU_DEVICE_MODE_LENIENT,
    },
    {
        .name = "UPDATE",
        .mask = SSU_DEVICE_MODE_UPDATE,
        .bits = SSU_DEVICE_MODE_UPDATE,
    },
    {
        .name = "APP_INSTALL",
        .mask = SSU_DEVICE_MODE_APP_INSTALL,
        .bits = SSU_DEVICE_MODE_APP_INSTALL,
    },
    {
        .name = 0,
        .mask = 0,
        .bits = 0,
    },
};

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

// Unused short options:
// - B - - E F G H I J - L - N O - Q R - T U V W X Y Z
// - - c - e - g - i j - - - - o - q - s t u v w x y z

/** Lookup table for long option parsing */
const struct option opt_long[] =
{
    {"help",             no_argument,       0, 'h'},
    {"usage",            no_argument,       0, 'h'},
    {"model",            no_argument,       0, 'm'},
    {"designation",      no_argument,       0, 'd'},
    {"manufacturer",     no_argument,       0, 'M'},
    {"pretty-name",      no_argument,       0, 'p'},
    {"device-info",      no_argument,       0, 'D'},
    {"arch",             no_argument,       0, 'A'},
    {"brand",            no_argument,       0, 'b'},
    {"flavour",          no_argument,       0, 'l'},
    {"domain",           no_argument,       0, 'n'},
    {"release",          no_argument,       0, 'r'},
    {"ssu-info",         no_argument,       0, 'S'},
    {"all",              no_argument,       0, 'a'},
#if SSU_INCLUDE_CREDENTIAL_ITEMS
    {"ssu-certificate",  no_argument,       0, 'C'},
    {"ssu-private-key",  no_argument,       0, 'P'},
#endif
    {"list-hw-features", no_argument,       0, 901 },
    {"hw-features",      no_argument,       0, 'f'},
    {"has-hw-feature",   required_argument, 0, 'F'},
    {"list-hw-keys",     no_argument,       0, 902 },
    {"hw-keys",          no_argument,       0, 'k'},
    {"has-hw-key",       required_argument, 0, 'K'},
    {0, 0, 0, 0}
};

/** Lookup string for short option parsing */
const char opt_short[] =
"h"  // --help
"m"  // --model
"d"  // --designation
"M"  // --manufacturer
"p"  // --pretty-name
"D"  // --device-info
"A"  // --arch
"b"  // --brand
"l"  // --flavour
"n"  // --domain
"r"  // --release
"S"  // --ssu-info
"a"  // --all
#if SSU_INCLUDE_CREDENTIAL_ITEMS
"C"  // --ssu-certificate
"P"  // --ssu-private-key
#endif
"f"  // --hw-features
"F:" // --has-hw-feature
"k"  // --hw-keys
"K:" // --has-hw-key
;

/** Freeform usage text */
const char opt_help[] =
"\n"
"  -h --help                   Print usage information\n"
"\n"
"  -m --model                  Print device model\n"
"  -d --designation            Print device designation\n"
"  -M --manufacturer           Print device manufacturer\n"
"  -p --pretty-name            Print device pretty name\n"
"  -D --device-info            Print all of the above\n"
"  -A --arch                   Print ssu arch\n"
"  -b --brand                  Print ssu brand\n"
"  -l --flavour                Print ssu flavour\n"
"  -n --domain                 Print ssu domain\n"
"  -r --release                Print ssu release\n"
"  -S --ssu-info               Print all ssu information\n"
"  -a --all                    Print all device and ssu information\n"
#if SSU_INCLUDE_CREDENTIAL_ITEMS
"  -C --ssu-certificate        Print ssu certificate\n"
"  -P --ssu-private-key        Print ssu private key\n"
#endif
"\n"
"  --list-hw-features          List all known hw features\n"
"  -f --hw-features            Print available hw-features\n"
"  -F --has-hw-feature=<NAME>  Check if hw-feature is available\n"
"\n"
"  --list-hw-keys              List all known hw keys\n"
"  -k --hw-keys                Print available hw-keys\n"
"  -K --has-hw-key=<NAME>      Check if hw-key is available\n"
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

static void
output_ssu_info(void)
{
    ssusysinfo_t *info = get_cfg();

    ssu_device_mode_t device_mode = ssusysinfo_ssu_device_mode(info);
    char device_mode_repr[256];
    bitfield_repr(bitfield_device_mode, (int)device_mode,
                  device_mode_repr, sizeof device_mode_repr);

    printf("registered: %s\n",            ssusysinfo_ssu_registered(info) ? "yes" : "no");
    printf("device_mode: %d (%s)\n",      (int)device_mode, device_mode_repr);
    printf("arch: %s\n",                  ssusysinfo_ssu_arch(info));
    printf("brand: %s\n",                 ssusysinfo_ssu_brand(info));
    printf("flavour: %s\n",               ssusysinfo_ssu_flavour(info));
    printf("domain: %s\n",                ssusysinfo_ssu_domain(info));

    printf("release: %s\n",               ssusysinfo_ssu_release(info));
    printf("def_release: %s\n",           ssusysinfo_ssu_def_release(info));
    printf("rnd_release: %s\n",           ssusysinfo_ssu_rnd_release(info));

    printf("enabled_repos: %s\n",         ssusysinfo_ssu_enabled_repos(info));
    printf("disabled_repos: %s\n",        ssusysinfo_ssu_disabled_repos(info));

    printf("credentials_updated: %s\n",   ssusysinfo_ssu_last_credentials_update(info));
    printf("credentials_scope: %s\n",     ssusysinfo_ssu_credentials_scope(info));

    printf("credentials_url_jolla: %s\n", ssusysinfo_ssu_credentials_url_jolla(info));
#if SSU_INCLUDE_CREDENTIAL_ITEMS
    printf("credentials_username_jolla: %s\n", ssusysinfo_ssu_credentials_username_jolla(info));
    printf("credentials_password_jolla: %s\n", ssusysinfo_ssu_credentials_password_jolla(info));
#endif

    printf("credentials_url_store: %s\n", ssusysinfo_ssu_credentials_url_store(info));
#if SSU_INCLUDE_CREDENTIAL_ITEMS
    printf("credentials_username_store: %s\n", ssusysinfo_ssu_credentials_username_store(info));
    printf("credentials_password_store: %s\n", ssusysinfo_ssu_credentials_password_store(info));
#endif

    printf("default_rnd_domain: %s\n",    ssusysinfo_ssu_default_rnd_domain(info));
    printf("home_url: %s\n",              ssusysinfo_ssu_home_url(info));
}

#if SSU_INCLUDE_CREDENTIAL_ITEMS
/** Handler for --ssu-certificate option
 */
static void
output_ssu_certificate(void)
{
    const char *text = ssusysinfo_ssu_certificate(get_cfg());
    const char *tail = strrchr(text, '\n');
    const char *feed = tail && tail[1] == 0 ? "" : "\n";
    printf("%s%s", text, feed);
}

/** Handler for --ssu-private-key option
 */
static void
output_ssu_private_key(void)
{
    const char *text = ssusysinfo_ssu_private_key(get_cfg());
    const char *tail = strrchr(text, '\n');
    const char *feed = tail && tail[1] == 0 ? "" : "\n";
    printf("%s%s", text, feed);
}
#endif /* SSU_INCLUDE_CREDENTIAL_ITEMS */

/** Handler for --arch option
 */
static void
output_arch(void)
{
    printf("%s\n", ssusysinfo_ssu_arch(get_cfg()));
}

/** Handler for --brand option
 */
static void
output_brand(void)
{
    printf("%s\n", ssusysinfo_ssu_brand(get_cfg()));
}

/** Handler for --flavour option
 */
static void
output_flavour(void)
{
    printf("%s\n", ssusysinfo_ssu_flavour(get_cfg()));
}

/** Handler for --domain option
 */
static void
output_domain(void)
{
    printf("%s\n", ssusysinfo_ssu_domain(get_cfg()));
}

/** Handler for --release option
 */
static void
output_release(void)
{
    printf("%s\n", ssusysinfo_ssu_release(get_cfg()));
}

/** Handler for --all option
 */
static void
output_all(void)
{
    printf("DEVICE INFO:\n");
    output_device_info();
    printf("\n");
    printf("SSU INFO\n");
    output_ssu_info();
}

/** Handler for --device-info option
 */
static void
output_device_info(void)
{
    ssusysinfo_t *info = get_cfg();

    printf("model: %s\n",        ssusysinfo_device_model(info));
    printf("designation: %s\n",  ssusysinfo_device_designation(info));
    printf("manufacturer: %s\n", ssusysinfo_device_manufacturer(info));
    printf("pretty_name: %s\n",  ssusysinfo_device_pretty_name(info));
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

/** Handler for --list-hw-features option
 */
static void
output_list_hw_features(void)
{
    const char **arr = ssusysinfo_hw_feature_names();

    for( size_t i = 0; arr && arr[i]; ++i )
        printf("%s\n", arr[i]);

    free(arr);
}

/** Handler for --hw-features option
 */
static void
output_hw_features(void)
{
    hw_feature_t *arr = ssusysinfo_get_hw_features(get_cfg());

    for( size_t i = 0; arr && arr[i]; ++i ) {
        printf("%s\n", ssusysinfo_hw_feature_to_name(arr[i]));
    }
    free(arr);
}

/** Handler for --has-hw-feature=<NAME> option
 */
static bool
require_has_hw_feature(const char *name)
{
    return ssusysinfo_has_hw_feature(get_cfg(),
                                     ssusysinfo_hw_feature_from_name(name));
}

/** Handler for --list-hw-keys option
 */
static void
output_list_hw_keys(void)
{
    const char **arr = ssusysinfo_hw_key_names();

    for( size_t i = 0; arr && arr[i]; ++i )
        printf("%s\n", arr[i]);

    free(arr);
}

/** Handler for --hw-keys option
 */
static void
output_hw_keys(void)
{
    hw_key_t *arr = ssusysinfo_get_hw_keys(get_cfg());

    for( size_t i = 0; arr && arr[i]; ++i ) {
        printf("%s\n", ssusysinfo_hw_key_to_name(arr[i]) ?: "unknown");
    }
    free(arr);
}

/** Handler for --has-hw-key=<NAME> option
 */
static bool
require_has_hw_key(const char *name)
{
    return ssusysinfo_has_hw_key(get_cfg(),
                                 ssusysinfo_hw_key_from_name(name));
}

/* ========================================================================= *
 * MAIN_ENTRY_POINT
 * ========================================================================= */

int
main(int ac, char **av)
{
    const char *progname  = av[0];
    int         exitcode  = EXIT_FAILURE;

    /* Treat no-args as if --device-info option were given */
    if( ac == 1 ) {
        output_device_info();
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

        case 'D':
            output_device_info();
            break;

        case 'A':
            output_arch();
            break;

        case 'b':
            output_brand();
            break;

        case 'l':
            output_flavour();
            break;

        case 'n':
            output_domain();
            break;

        case 'r':
            output_release();
            break;

        case 'S':
            output_ssu_info();
            break;

        case 'a':
            output_all();
            break;

#if SSU_INCLUDE_CREDENTIAL_ITEMS
        case 'C':
            output_ssu_certificate();
            break;

        case 'P':
            output_ssu_private_key();
            break;
#endif

        case 901:
            output_list_hw_features();
            break;

        case 'f':
            output_hw_features();
            break;

        case 'F':
            if( !require_has_hw_feature(optarg) )
                goto EXIT;
            break;

        case 902:
            output_list_hw_keys();
            break;

        case 'k':
            output_hw_keys();
            break;

        case 'K':
            if( !require_has_hw_key(optarg) )
                goto EXIT;
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
