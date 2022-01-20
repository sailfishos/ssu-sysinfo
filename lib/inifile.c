/******************************************************************************
** This file is part of profile-qt
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Copyright (c) 2016 - 2022 Jolla Ltd.
**
** Contact: Sakari Poussa <sakari.poussa@nokia.com>
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright notice,
** this list of conditions and the following disclaimer in the documentation
** and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation nor the names of its contributors
** may be used to endorse or promote products derived from this software
** without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
** OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
** ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include "inifile.h"

#include "symtab.h"
#include "xmalloc.h"
#include "util.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

/* ========================================================================= *
 * Config
 * ========================================================================= */

enum
{
  BRA = '[',
  KET = ']',
  SEP = '=',
};

/* ========================================================================= *
 * inival_t  --  methods
 * ========================================================================= */

struct inival_t
{
  char *iv_key; // value key must not be changed
  char *iv_val;
  int   iv_ord;
};

/* ------------------------------------------------------------------------- *
 * inival_get_key
 * ------------------------------------------------------------------------- */

const char *
inival_get_key(const inival_t *self)
{
  return self->iv_key;
}

/* ------------------------------------------------------------------------- *
 * inival_get_val
 * ------------------------------------------------------------------------- */

const char *
inival_get_val(const inival_t *self)
{
  return self->iv_val;
}

/* ------------------------------------------------------------------------- *
 * inival_get_ord
 * ------------------------------------------------------------------------- */

int
inival_get_ord(const inival_t *self)
{
  return self->iv_ord;
}

/* ------------------------------------------------------------------------- *
 * inival_set
 * ------------------------------------------------------------------------- */

void
inival_set(inival_t *self, const char *val)
{
  strutil_set(&self->iv_val, val);
}

/* ------------------------------------------------------------------------- *
 * inival_create
 * ------------------------------------------------------------------------- */

inival_t *
inival_create(const char *key, const char *val)
{
  static int ord = 0;

  inival_t *self = xcalloc(1, sizeof *self);

  self->iv_key = xstrdup(key ?: "");
  self->iv_val = xstrdup(val ?: "");
  self->iv_ord = ++ord;

  return self;
}

/* ------------------------------------------------------------------------- *
 * inival_delete
 * ------------------------------------------------------------------------- */

void
inival_delete(inival_t *self)
{
  if( self != 0 )
  {
    free(self->iv_key);
    free(self->iv_val);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * inival_delete_cb
 * ------------------------------------------------------------------------- */

void
inival_delete_cb(void *self)
{
  inival_delete(self);
}

/* ------------------------------------------------------------------------- *
 * inival_create_cb
 * ------------------------------------------------------------------------- */

static
void *
inival_create_cb(const char *key)
{
  return inival_create(key, "");
}

/* ------------------------------------------------------------------------- *
 * inival_getkey_cb
 * ------------------------------------------------------------------------- */

static
const char *
inival_getkey_cb(const void *self)
{
  return inival_get_key(self);
}

/* ========================================================================= *
 * inisec_t  --  methods
 * ========================================================================= */

struct inisec_t
{
  char      *is_name; // section name must not be changed
  symtab_t   is_values;
};

/* ------------------------------------------------------------------------- *
 * inisec_get_name
 * ------------------------------------------------------------------------- */

const char *
inisec_get_name(const inisec_t *self)
{
  return self->is_name;
}

/* ------------------------------------------------------------------------- *
 * inisec_elem_count
 * ------------------------------------------------------------------------- */

size_t
inisec_elem_count(const inisec_t *self)
{
  return symtab_size(&self->is_values);
}

/* ------------------------------------------------------------------------- *
 * inisec_elem
 * ------------------------------------------------------------------------- */

inival_t *
inisec_elem(const inisec_t *self, size_t ind)
{
  return symtab_elem(&self->is_values, ind);
}

/* ------------------------------------------------------------------------- *
 * inisec_ctor
 * ------------------------------------------------------------------------- */

void
inisec_ctor(inisec_t *self)
{
  self->is_name   = 0;

  symtab_ctor(&self->is_values,
              inival_create_cb,
              inival_delete_cb,
              inival_getkey_cb);
}

/* ------------------------------------------------------------------------- *
 * inisec_dtor
 * ------------------------------------------------------------------------- */

void
inisec_dtor(inisec_t *self)
{
  symtab_dtor(&self->is_values);

  strutil_set(&self->is_name, 0);
}

/* ------------------------------------------------------------------------- *
 * inisec_create
 * ------------------------------------------------------------------------- */

inisec_t *
inisec_create(const char *name)
{
  inisec_t *self = xcalloc(1, sizeof *self);
  inisec_ctor(self);

  strutil_set(&self->is_name, name);

  return self;
}

/* ------------------------------------------------------------------------- *
 * inisec_delete
 * ------------------------------------------------------------------------- */

void
inisec_delete(inisec_t *self)
{
  if( self != 0 )
  {
    inisec_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * inisec_delete_cb
 * ------------------------------------------------------------------------- */

void
inisec_delete_cb(void *self)
{
  inisec_delete(self);
}

/* ------------------------------------------------------------------------- *
 * inisec_getkey_cb
 * ------------------------------------------------------------------------- */

static
const char *
inisec_getkey_cb(const void *self)
{
  return inisec_get_name(self);
}

/* ------------------------------------------------------------------------- *
 * inisec_create_cb
 * ------------------------------------------------------------------------- */

static
void *
inisec_create_cb(const char *name)
{
  return inisec_create(name);
}

/* ------------------------------------------------------------------------- *
 * inisec_set
 * ------------------------------------------------------------------------- */

void
inisec_set(inisec_t *self, const char *key, const char *val)
{
  inival_t *res = symtab_insert(&self->is_values, key);
  inival_set(res, val);
}

/* ------------------------------------------------------------------------- *
 * inisec_get
 * ------------------------------------------------------------------------- */

const char *
inisec_get(inisec_t *self, const char *key, const char *val)
{
  inival_t *res = symtab_lookup(&self->is_values, key);
  return res ? inival_get_val(res) : val;
}

/* ========================================================================= *
 * inifile_t  --  methods
 * ========================================================================= */

struct inifile_t
{
  symtab_t   if_sections;
};

/* ------------------------------------------------------------------------- *
 * inifile_ctor
 * ------------------------------------------------------------------------- */

void
inifile_ctor(inifile_t *self)
{
  symtab_ctor(&self->if_sections,
              inisec_create_cb,
              inisec_delete_cb,
              inisec_getkey_cb);
}

/* ------------------------------------------------------------------------- *
 * inifile_dtor
 * ------------------------------------------------------------------------- */

void
inifile_dtor(inifile_t *self)
{
  symtab_dtor(&self->if_sections);
}

/* ------------------------------------------------------------------------- *
 * inifile_create
 * ------------------------------------------------------------------------- */

inifile_t *
inifile_create(void)
{
  inifile_t *self = xcalloc(1, sizeof *self);
  inifile_ctor(self);
  return self;
}

/* ------------------------------------------------------------------------- *
 * inifile_delete
 * ------------------------------------------------------------------------- */

void
inifile_delete(inifile_t *self)
{
  if( self != 0 )
  {
    inifile_dtor(self);
    free(self);
  }
}

/* ------------------------------------------------------------------------- *
 * inifile_section_count
 * ------------------------------------------------------------------------- */

size_t
inifile_section_count(const inifile_t *self)
{
  return symtab_size(&self->if_sections);
}

/* ------------------------------------------------------------------------- *
 * inifile_get_section
 * ------------------------------------------------------------------------- */

inisec_t *
inifile_get_section(const inifile_t *self, const char *sec)
{
  return symtab_lookup(&self->if_sections, sec);
}

/* ------------------------------------------------------------------------- *
 * inifile_add_section
 * ------------------------------------------------------------------------- */

inisec_t *
inifile_add_section(inifile_t *self, const char *sec)
{
  return symtab_insert(&self->if_sections, sec);
}

/* ------------------------------------------------------------------------- *
 * inifile_set
 * ------------------------------------------------------------------------- */

void
inifile_set(inifile_t *self, const char *sec, const char *key, const char *val)
{
  inisec_set(inifile_add_section(self, sec), key, val);
}

/* ------------------------------------------------------------------------- *
 * inifile_get
 * ------------------------------------------------------------------------- */

const char *
inifile_get(inifile_t *self, const char *sec, const char *key, const char *val)
{
  inisec_t *s = symtab_lookup(&self->if_sections, sec);
  return s ? inisec_get(s, key, val) : val;
}

/* ------------------------------------------------------------------------- *
 * inifile_load
 * ------------------------------------------------------------------------- */

int
inifile_load(inifile_t *self, const char *path, const char *defsec)
{
  int     err  = -1;
  FILE   *file = 0;
  size_t  size = 0;
  char   *data = 0;

  inisec_t *sec = 0;
  char     *key = 0;
  char     *val = 0;

  log_debug("read: %s, using default section: %s", path, defsec ?: "N/A");

  if( (file = fopen(path, "r")) == 0 )
  {
    log_debug("%s: iniload/open: %m", path);
    goto cleanup;
  }

  if( defsec ) {
    sec = inifile_add_section(self, defsec);
  }

  while( getline(&data, &size, file) != -1 )
  {
    char *pos = strutil_trim(data);

    if( *pos == 0 ) continue;

    if( *pos == ';' || *pos == '#' ) continue;

    if( *pos == BRA )
    {
      char *name = strutil_slice(pos+1, 0, KET);
      sec = inifile_add_section(self, strutil_strip(name));
      continue;
    }

    key = strutil_slice(pos, &val, SEP);
    strutil_strip(key);
    strutil_trim(val);

    if( sec && *key )
    {
      if( defsec )
      {
        /* Use of default section implies files like /etc/os-release
         * that are shell scripts rather than ini files and thus can
         * contain quoted values.
         */
        if( *val == '"' || *val == '\'' )
        {
          char *end = strrchr(val + 1, *val);
          if( end )
          {
            *end = 0;
            strutil_trim(++val);
          }
        }
      }
      inisec_set(sec, key, val);
    }
  }

  err = 0;

cleanup:

  free(data);

  if( file != 0 && fclose(file) == EOF )
  {
    log_err("%s: iniload/close: %m", path);
    err = -1;
  }

  return err;
}

/* ------------------------------------------------------------------------- *
 * inifile_dump
 * ------------------------------------------------------------------------- */

void
inifile_dump(inifile_t *self)
{
  for( size_t i = 0; i < inifile_section_count(self); ++i ) {
    inisec_t *sec = symtab_elem(&self->if_sections, i);
    printf("[%s]\n", inisec_get_name(sec));

    for( size_t j = 0; j < inisec_elem_count(sec); ++j ) {
      inival_t *val = inisec_elem(sec, j);
      printf("<%s> = <%s>\n", inival_get_key(val), inival_get_val(val));
    }
  }
}
