#!/bin/sh

# Check that all files that should have the current version agree on it.
# Used to fail RPM builds from sw that is not properly updated before tagging.

# ----------------------------------------------------------------------------
# Assume success
# ----------------------------------------------------------------------------

RES=0

# ----------------------------------------------------------------------------
# The "master" version = whatever is in Makefile
# ----------------------------------------------------------------------------

MASTER_PATH=Makefile
MASTER_VERS=$(grep '^VERSION.*:=' $MASTER_PATH |sed -e 's/^.*:=[[:space:]]*//')

echo "$MASTER_PATH $MASTER_VERS"

# ----------------------------------------------------------------------------
# Check rpm spec file version
# ----------------------------------------------------------------------------

RPM_PATH=${RPM_SOURCE_DIR:-rpm}/${RPM_PACKAGE_NAME:-ssu-sysinfo}.spec
RPM_VERS=$(grep '^Version:' $RPM_PATH |sed -e 's/^.*:[[:space:]]*//')

echo "$RPM_PATH $RPM_VERS"

# Remove initial part of rpm version  that equals with version from Makefile
RPM_XTRA=${RPM_VERS#$MASTER_VERS}
# From that remove initial part that equals with version from spec-file
RPM_XTRA=${RPM_XTRA#$RPM_VERS}
# If the result is non-empty string, then OBS is doing
# delta-after-matching-tag kind of build, which is ok.
# But empty string means that spec and Makefile are completely
# out of sync, which is bad.

if [ "$MASTER_VERS" != "$RPM_VERS" ]; then
  if [ -z "$RPM_XTRA" ]; then
    echo >&2 "$MASTER_PATH $MASTER_VERS vs $RPM_PATH $RPM_VERS"
    RES=1
  else
    echo "(ignoring patch level in spec file: $RPM_XTRA)"
  fi
fi

# ----------------------------------------------------------------------------
# Check pkgconfig files
# ----------------------------------------------------------------------------

for PC_PATH in pkg-config/*.pc; do
  PC_VERS=$(grep '^Version:' $PC_PATH |sed -e 's/^.*:[[:space:]]*//')
  echo "$PC_PATH $PC_VERS"
  if [ "$MASTER_VERS" != "$PC_VERS" ]; then
    echo >&2 "$MASTER_PATH $MASTER_VERS vs $PC_PATH $PC_VERS"
    RES=1
  fi
done

# ----------------------------------------------------------------------------
# In case of conflicts, exit with error value
# ----------------------------------------------------------------------------

if [ $RES != 0 ]; then
  echo >&2 "Conflicting package versions"
fi

exit $RES
