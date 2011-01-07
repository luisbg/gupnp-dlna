AC_PREREQ(2.53)
AC_INIT(gupnp-dlna, 0.4.2, http://www.gupnp.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libgupnp-dlna/gupnp-dlna-profile.h)
AM_CONFIG_HEADER(config.h)

dnl library versioning
dnl Increase when changing the API
GUPNP_DLNA_CURRENT=1

dnl Update when changing implementation of current API,
dnl reset to 0 when changing CURRENT.  This is the revision of
dnl current API version
GUPNP_DLNA_REVISION=0

dnl Increase if API change is ABI compatible, otherwise reset to 0
GUPNP_DLNA_AGE=0

GUPNP_DLNA_VERSION_INFO="$GUPNP_DLNA_CURRENT:$GUPNP_DLNA_REVISION:$GUPNP_DLNA_AGE"
AC_SUBST(GUPNP_DLNA_VERSION_INFO)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],)

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(LIBXML, libxml-2.0 >= 2.5.0)

GST_MAJORMINOR=0.10
GST_REQ=0.10.29.2
GSTPBU_REQ=0.10.31

PKG_CHECK_MODULES(GST, gstreamer-$GST_MAJORMINOR >= $GST_REQ)
PKG_CHECK_MODULES(GST_PBU, gstreamer-pbutils-$GST_MAJORMINOR >= $GSTPBU_REQ)
AC_SUBST(GST_MAJORMINOR)

GST_CFLAGS="$GST_CFLAGS -DGST_USE_UNSTABLE_API"
AC_SUBST(GST_CFLAGS)

# glib-genmarshal
GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

# Debugging
AC_ARG_ENABLE(debug,
	[  --enable-debug          enable debugging],,
        enable_debug=no)
if test "x$enable_debug" = "xyes"; then
        CFLAGS="$CFLAGS -g -Wall"
fi

GTK_DOC_CHECK([1.0])

AC_OUTPUT([
Makefile
gst-convenience/Makefile
gst-convenience/gst-libs/Makefile
gst-convenience/gst-libs/gst/Makefile
gst-convenience/gst-libs/gst/profile/Makefile
libgupnp-dlna/Makefile
tools/Makefile
tests/Makefile
tests/test-discoverer.sh
doc/Makefile
doc/version.xml
data/Makefile
gupnp-dlna-1.0.pc
gupnp-dlna-1.0-uninstalled.pc
])
