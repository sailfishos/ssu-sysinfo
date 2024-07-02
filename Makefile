# ----------------------------------------------------------------------------
# Package version
# ----------------------------------------------------------------------------

VERSION   := 1.4.2

# ----------------------------------------------------------------------------
# Shared object version
# ----------------------------------------------------------------------------

SOMAJOR   := .1
SOMINOR   := .0
SORELEASE := .0

SOLINK    := .so
SONAME    := .so$(SOMAJOR)
SOVERS    := .so$(SOMAJOR)$(SOMINOR)$(SORELEASE)

# ----------------------------------------------------------------------------
# Files to build / install
# ----------------------------------------------------------------------------

TARGETS_DSO    += libssusysinfo$(SOVERS)
TARGETS_BIN    += ssu-sysinfo
TARGETS_ALL    += $(TARGETS_DSO) $(TARGETS_BIN) libssusysinfo$(SONAME)

INSTALL_HDR    += lib/ssusysinfo.h
INSTALL_PC     += pkg-config/ssu-sysinfo.pc

# ----------------------------------------------------------------------------
# Installation directories
# ----------------------------------------------------------------------------

# Dummy default install dir - override from packaging scripts
DESTDIR         ?= /tmp/ssu-sysinfo-test-install

# Standard install directories
_PREFIX         ?= /usr#                         # /usr
_INCLUDEDIR     ?= $(_PREFIX)/include#           # /usr/include
_EXEC_PREFIX    ?= $(_PREFIX)#                   # /usr
_BINDIR         ?= $(_EXEC_PREFIX)/bin#          # /usr/bin
_SBINDIR        ?= $(_EXEC_PREFIX)/sbin#         # /usr/sbin
_LIBEXECDIR     ?= $(_EXEC_PREFIX)/libexec#      # /usr/libexec
_LIBDIR         ?= $(_EXEC_PREFIX)/lib#          # /usr/lib
_SYSCONFDIR     ?= /etc#                         # /etc
_DATADIR        ?= $(_PREFIX)/share#             # /usr/share
_MANDIR         ?= $(_DATADIR)/man#              # /usr/share/man
_INFODIR        ?= $(_DATADIR)/info#             # /usr/share/info
_DEFAULTDOCDIR  ?= $(_DATADIR)/doc#              # /usr/share/doc
_LOCALSTATEDIR  ?= /var#                         # /var
_UNITDIR        ?= /lib/systemd/system#
_TESTSDIR       ?= /opt/tests#                   # /opt/tests

# ----------------------------------------------------------------------------
# Default flags
# ----------------------------------------------------------------------------

CPPFLAGS += -D_GNU_SOURCE
CPPFLAGS += -D_FILE_OFFSET_BITS=64

COMMON   += -Wall
COMMON   += -Wextra
COMMON   += -Werror # DEVEL TIME ONLY - DO NOT LEAVE IN
COMMON   += -Os
COMMON   += -g

CFLAGS   += $(COMMON)
CFLAGS   += -std=c99
CFLAGS   += -Wmissing-prototypes
CFLAGS   += -Wno-missing-field-initializers

CXXFLAGS += $(COMMON)

LDFLAGS  += -g

LDLIBS   += -Wl,--as-needed

# Options that are useful for weeding out unused functions
#CFLAGS += -O0 -ffunction-sections -fdata-sections
#LDLIBS += -Wl,--gc-sections,--print-gc-sections

# ----------------------------------------------------------------------------
# Flags from pkg-config
# ----------------------------------------------------------------------------

#PKG_NAMES += glib-2.0

# we do not need pkg-config for maintenance targets
maintenance  = normalize clean distclean mostlyclean
intersection = $(strip $(foreach w,$1, $(filter $w,$2)))
ifneq ($(call intersection,$(maintenance),$(MAKECMDGOALS)),)
PKG_CONFIG   ?= true
endif

# pkg-config does not grok CFLAGS vs CPPFLAGS ... deal with it
ifneq ($(strip $(PKG_NAMES)),)
PKG_CONFIG   ?= pkg-config
PKG_CFLAGS   := $(shell $(PKG_CONFIG) --cflags $(PKG_NAMES))
PKG_LDLIBS   := $(shell $(PKG_CONFIG) --libs   $(PKG_NAMES))
PKG_CPPFLAGS := $(filter -D%,$(PKG_CFLAGS)) $(filter -I%,$(PKG_CFLAGS))
PKG_CFLAGS   := $(filter-out -I%, $(filter-out -D%, $(PKG_CFLAGS)))
endif

CPPFLAGS += $(PKG_CPPFLAGS)
CFLAGS   += $(PKG_CFLAGS)
LDLIBS   += $(PKG_LDLIBS)

# ----------------------------------------------------------------------------
# Top level targets
# ----------------------------------------------------------------------------

.PHONY: build install clean distclean mostlyclean

build:: $(TARGETS_ALL)

install:: build

clean:: mostlyclean
	$(RM) $(TARGETS_ALL)

distclean:: clean

mostlyclean::
	$(RM) *.o *~ *.bak */*.o */*~ */*.bak

install ::

# ----------------------------------------------------------------------------
# Implicit rules
# ----------------------------------------------------------------------------

.SUFFIXES: %.c %.o %.pic.o %$(SOVERS) %$(SONAME)

%.o : %.c
	$(CC) -o $@ -c $< $(CPPFLAGS) $(CFLAGS)

%.pic.o : %.c
	$(CC) -o $@ -c $< -fPIC -fvisibility=hidden $(CPPFLAGS) $(CFLAGS)

%$(SOVERS) :
	$(CC) -o $@ -shared -Wl,-soname,$*$(SONAME) $^ $(LDFLAGS) $(LDLIBS)

%$(SONAME) : %$(SOVERS)
	ln -sf $< $@

% : bin/%.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# ----------------------------------------------------------------------------
# Build libssusysinfo$(SOVERS)
# ----------------------------------------------------------------------------

libssusysinfo_SRC += lib/ssusysinfo.c

libssusysinfo_SRC += lib/hw_feature.c
libssusysinfo_SRC += lib/hw_key.c
libssusysinfo_SRC += lib/inifile.c
libssusysinfo_SRC += lib/logging.c
libssusysinfo_SRC += lib/symtab.c
libssusysinfo_SRC += lib/util.c
libssusysinfo_SRC += lib/xmalloc.c

libssusysinfo_OBJ += $(patsubst %.c,%.pic.o,$(libssusysinfo_SRC))

libssusysinfo$(SOVERS) : $(libssusysinfo_OBJ)

# ----------------------------------------------------------------------------
# Build ssu-sysinfo
# ----------------------------------------------------------------------------

ssu_sysinfo_SRC += bin/ssu-sysinfo.c

ssu_sysinfo_OBJ += $(patsubst %.c,%.o,$(ssu_sysinfo_SRC))
ssu_sysinfo_OBJ += libssusysinfo$(SONAME)

ssu-sysinfo : $(ssu_sysinfo_OBJ)

# ----------------------------------------------------------------------------
# Statically linked binary for static analysis, not build normally
# ----------------------------------------------------------------------------

monolith_OBJ += $(patsubst %.c,%.o,$(ssu_sysinfo_SRC))
monolith_OBJ += $(patsubst %.c,%.o,$(libssusysinfo_SRC))

monolith : $(monolith_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
clean::
	$(RM) monolith

# ----------------------------------------------------------------------------
# Install to $(DESTDIR)
# ----------------------------------------------------------------------------

install ::
	# binaries
	install -d -m 755 $(DESTDIR)$(_BINDIR)
	install -m 755 $(TARGETS_BIN) $(DESTDIR)$(_BINDIR)/
	# dynamic libraries
	install -d -m 755 $(DESTDIR)$(_LIBDIR)
	install -m 755 $(TARGETS_DSO) $(DESTDIR)$(_LIBDIR)/
	# headers
	install -d -m 755 $(DESTDIR)$(_INCLUDEDIR)/ssusysinfo
	install -m 644 $(INSTALL_HDR) $(DESTDIR)$(_INCLUDEDIR)/ssusysinfo/
	# pkg config
	install -d -m 755 $(DESTDIR)$(_LIBDIR)/pkgconfig
	install -m 644 $(INSTALL_PC) $(DESTDIR)$(_LIBDIR)/pkgconfig
	# symlinks for dynamic linking
	for f in $(TARGETS_DSO); do \
	  ln -sf $$(basename $$f $(SOVERS))$(SONAME) \
	    $(DESTDIR)$(_LIBDIR)/$$(basename $$f $(SOVERS))$(SOLINK); \
	done

# ----------------------------------------------------------------------------
# Source code normalization
# ----------------------------------------------------------------------------

.PHONY: normalize
normalize::
	normalize_whitespace -M Makefile
	normalize_whitespace -a $(wildcard rpm/*.spec */*.pc)
	normalize_whitespace -a $(wildcard */*.py */*.sh)
	normalize_whitespace -a $(wildcard */*.[ch])

# ----------------------------------------------------------------------------
# Prototype scanning (two phases to allow preprocessing in scratchbox)
# ----------------------------------------------------------------------------

.SUFFIXES: %.q %.p %.sp

%.q  : %.c ; gcc -E -o $@ $(CPPFLAGS) $<
%.p  : %.q ; cproto < $< | prettyproto.py > $@    && cat $@
%.sp : %.q ; cproto -s < $< | prettyproto.py > $@ && cat $@

mostlyclean::
	$(RM) *.q *.p *.sp */*.q */*.p */*.sp

# ----------------------------------------------------------------------------
# Header dependecy scanning
# ----------------------------------------------------------------------------

.PHONY: depend

depend::
	$(CC) -MM -MG $(CPPFLAGS) $(wildcard */*.c) | util/depend_filter.py > .depend

ifneq ($(MAKECMDGOALS),depend) # not while: make depend
ifneq (,$(wildcard .depend))   # not if .depend does not exist
include .depend
endif
endif

# ----------------------------------------------------------------------------
# Hunt for excess include statements
# ----------------------------------------------------------------------------

.PHONY: headers
.SUFFIXES: %.checked

headers:: c_headers c_sources

%.checked : %
	find_unneeded_includes.py $(CPPFLAGS) $(CFLAGS) -- $<
	@touch $@

clean::
	$(RM) */*.checked */*.order

c_headers:: $(patsubst %,%.checked,$(wildcard */*.h))
c_sources:: $(patsubst %,%.checked,$(wildcard */*.c))

order::
	find_unneeded_includes.py -- $(wildcard */*.h) $(wildcard */*.c)
