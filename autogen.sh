#! /bin/sh
gtkdocize --flavour no-tmpl || exit 1
ACLOCAL="${ACLOCAL-aclocal} $ACLOCAL_FLAGS" autoreconf -v --install || exit 1
glib-gettextize --force --copy || exit 1
./configure --enable-debug "$@"
