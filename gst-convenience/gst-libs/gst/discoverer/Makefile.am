lib_LTLIBRARIES = libgstdiscoverer-gupnp-dlna-@GST_MAJORMINOR@.la

glib_enum_define = GST_DISCOVERER
glib_gen_prefix = __gst_discoverer
glib_enum_headers = gstdiscoverer.h
glib_gen_basename = gstdiscoverer

#include $(top_srcdir)/common/gst-glib-gen.mak

built_sources = gstdiscoverer-marshal.c gstdiscoverer-enumtypes.c
built_headers = gstdiscoverer-marshal.h gstdiscoverer-enumtypes.h

BUILT_SOURCES = $(built_sources) $(built_headers)

nodist_libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES = $(BUILT_SOURCES)

libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES = gstdiscoverer.c \
	gstdiscoverer-types.c
libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) \
	$(GST_VIDEO_CFLAGS) -I$(top_srcdir)/gst-convenience/gst-libs
libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_LIBADD = $(GST_BASE_LIBS) $(GST_VIDEO_LIBS)
libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_LDFLAGS = $(GST_LIB_LDFLAGS) $(GST_ALL_LDFLAGS) $(GST_LT_LDFLAGS)
libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_LIBTOOLFLAGS = --tag=disable-static

libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@includedir = $(includedir)/gupnp-dlna-1.0/gst/discoverer
libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS = gstdiscoverer.h
nodist_libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS = gstdiscoverer-enumtypes.h

CLEANFILES = #$(BUILT_SOURCES)

EXTRA_DIST = gstdiscoverer-marshal.list $(BUILT_SOURCES)

#if HAVE_INTROSPECTION
#
#BUILT_GIRSOURCES = GstDiscoverer-@GST_MAJORMINOR@.gir
#
#gir_headers=$(patsubst %,$(srcdir)/%, $(libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#gir_headers+=$(patsubst %,$(builddir)/%, $(nodist_libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#gir_sources=$(patsubst %,$(srcdir)/%, $(libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES))
#gir_sources+=$(patsubst %,$(builddir)/%, $(nodist_libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES))
#gir_cincludes=$(patsubst %,--c-include='gst/discoverer/%',$(libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#gir_cincludes+=$(patsubst %,--c-include='gst/discoverer/%',$(nodist_libgstdiscoverer_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#
#GstDiscoverer-@GST_MAJORMINOR@.gir: $(INTROSPECTION_SCANNER) libgstdiscoverer-gupnp-dlna-@GST_MAJORMINOR@.la
#	$(AM_V_GEN)PKG_CONFIG_PATH="$(PKG_CONFIG_PATH):$(top_builddir)/pkgconfig" \
#		$(INTROSPECTION_SCANNER) -v --namespace GstDiscoverer \
#		--nsversion=@GST_MAJORMINOR@ \
#		--strip-prefix=Gst \
#		$(gir_cincludes) \
#		-I$(top_srcdir)/gst-libs \
#		-I$(top_builddir)/gst-libs \
#		--add-include-path=$$($(PKG_CONFIG) --variable=girdir gstreamer-@GST_MAJORMINOR@) \
#		--add-include-path=$$($(PKG_CONFIG) --variable=girdir gstreamer-base-@GST_MAJORMINOR@) \
#		--add-include-path=$$($(PKG_CONFIG) --variable=girdir gstreamer-video-@GST_MAJORMINOR@) \
#		--library=libgstdiscoverer-gupnp-dlna-@GST_MAJORMINOR@.la \
#		--include=Gst-@GST_MAJORMINOR@ \
#		--include=GstBase-@GST_MAJORMINOR@ \
#		--include=GstVideo-@GST_MAJORMINOR@ \
#		--libtool="$(top_builddir)/libtool" \
#		--pkg gstreamer-@GST_MAJORMINOR@ \
#		--pkg gstreamer-base-@GST_MAJORMINOR@ \
#		--pkg gstreamer-video-@GST_MAJORMINOR@ \
#		--output $@ \
#		$(gir_headers) \
#		$(gir_sources)
#
## INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
## install anything - we need to install inside our prefix.
#girdir = $(datadir)/gir-1.0
#gir_DATA = $(BUILT_GIRSOURCES)
#
#typelibsdir = $(libdir)/girepository-1.0/
#
#typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)
#
#%.typelib: %.gir $(INTROSPECTION_COMPILER)
#	$(AM_V_GEN)$(INTROSPECTION_COMPILER) \
#		--includedir=$(srcdir) \
#                --includedir=$(builddir) \
#		--includedir=$$($(PKG_CONFIG) --variable=girdir gstreamer-@GST_MAJORMINOR@) \
#		--includedir=$$($(PKG_CONFIG) --variable=girdir gstreamer-base-@GST_MAJORMINOR@) \
#		--includedir=$$($(PKG_CONFIG) --variable=girdir gstreamer-video-@GST_MAJORMINOR@) \
#		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)
#
#CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
#
#endif
