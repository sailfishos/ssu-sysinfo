/******************************************************************************
** This file is part of profile-qt
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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

#ifndef INIFILE_H_
# define INIFILE_H_

# include <stdio.h>

# ifdef __cplusplus
extern "C" {
# elif 0
} /* fool JED indentation ... */
# endif

/* ========================================================================= *
 * Types
 * ========================================================================= */

typedef struct inifile_t inifile_t;
typedef struct inisec_t  inisec_t;
typedef struct inival_t  inival_t;

/* ========================================================================= *
 * Functions
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * inival_t
 * ------------------------------------------------------------------------- */

const char *inival_get_key   (const inival_t *self);
const char *inival_get_val   (const inival_t *self);
int         inival_get_ord   (const inival_t *self);
void        inival_set       (inival_t *self, const char *val);
inival_t   *inival_create    (const char *key, const char *val);
void        inival_delete    (inival_t *self);
int         inival_compare   (const inival_t *self, const char *key);
void        inival_delete_cb (void *self);

/* ------------------------------------------------------------------------- *
 * inisec_t
 * ------------------------------------------------------------------------- */

inival_t   *inisec_elem      (const inisec_t *self, size_t ind);
const char *inisec_get_name  (const inisec_t *self);
void        inisec_ctor      (inisec_t *self);
void        inisec_dtor      (inisec_t *self);
inisec_t   *inisec_create    (const char *name);
void        inisec_delete    (inisec_t *self);
int         inisec_compare   (const inisec_t *self, const char *name);
void        inisec_delete_cb (void *self);
void        inisec_set       (inisec_t *self, const char *key, const char *val);
const char *inisec_get       (inisec_t *self, const char *key, const char *val);
int         inisec_has       (inisec_t *self, const char *key);
void        inisec_del       (inisec_t *self, const char *key);
int         inisec_emit      (const inisec_t *self, FILE *file);

/* ------------------------------------------------------------------------- *
 * inifile_t
 * ------------------------------------------------------------------------- */

void         inifile_ctor             (inifile_t *self);
void         inifile_dtor             (inifile_t *self);
inifile_t  * inifile_create           (void);
void         inifile_delete           (inifile_t *self);
inisec_t   * inifile_get_section      (const inifile_t *self, const char *sec);
inisec_t   * inifile_add_section      (inifile_t *self, const char *sec);
void         inifile_set              (inifile_t *self, const char *sec, const char *key, const char *val);
const char * inifile_get              (inifile_t *self, const char *sec, const char *key, const char *val);
int          inifile_load             (inifile_t *self, const char *path, const char *defsec);

# ifdef __cplusplus
};
# endif

#endif /* INIFILE_H_ */
