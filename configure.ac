AC_PREREQ(2.53)
AC_INIT(gupnp-dlna, 0.1.0, http://www.gupnp.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libgupnp-dlna/gupnp-dlna-profile.h)
AM_CONFIG_HEADER(config.h)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],)

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(LIBXML, libxml-2.0 >= 2.5.0)

GST_MAJORMINOR=0.10
GST_REQ=0.10.25
GSTVID_REQ=0.10.25

PKG_CHECK_MODULES(GST, gstreamer-$GST_MAJORMINOR >= $GST_REQ)
PKG_CHECK_MODULES(GST_BASE, gstreamer-base-$GST_MAJORMINOR >= $GST_REQ)
PKG_CHECK_MODULES(GST_VIDEO, gstreamer-video-$GST_MAJORMINOR >= $GSTVID_REQ)
AC_SUBST(GST_MAJORMINOR)

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
gst-convenience/gst-libs/gst/discoverer/Makefile
gst-convenience/gst-libs/gst/profile/Makefile
libgupnp-dlna/Makefile
tests/Makefile
doc/Makefile
doc/version.xml
data/Makefile
gupnp-dlna-1.0.pc
gupnp-dlna-1.0-uninstalled.pc
])
