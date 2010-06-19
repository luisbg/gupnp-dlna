# variables used for enum/marshal generation
glib_enum_headers = gstprofile.h
glib_enum_define = GST_PROFILE
glib_gen_prefix = gst_profile
glib_gen_basename = profile

built_sources = profile-enumtypes.c
built_headers = profile-enumtypes.h
BUILT_SOURCES = $(built_sources) $(built_headers)

lib_LTLIBRARIES = libgstprofile-gupnp-dlna-@GST_MAJORMINOR@.la

CLEANFILES = #$(BUILT_SOURCES)
EXTRA_DIST = $(BUILT_SOURCES)

libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES = \
	gstprofile.c
nodist_libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES = $(BUILT_SOURCES)

libgstprofile_gupnp_dlna_@GST_MAJORMINOR@includedir = $(includedir)/gupnp-dlna-1.0/gst/profile
libgstprofile_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS = \
	gstprofile.h
nodist_libgstprofile_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS = $(built_headers)

libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_CFLAGS = -I$(top_srcdir)/gst-convenience/gst-libs $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS)
libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_LIBADD = $(GST_BASE_LIBS) $(GST_LIBS)
libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_LDFLAGS = $(GST_LIB_LDFLAGS) $(GST_ALL_LDFLAGS) $(GST_LT_LDFLAGS)

#include $(top_srcdir)/common/gst-glib-gen.mak

#if HAVE_INTROSPECTION
#
#BUILT_GIRSOURCES = GstProfile-@GST_MAJORMINOR@.gir
#
#gir_headers=$(patsubst %,$(srcdir)/%, $(libgstprofile_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#gir_headers+=$(patsubst %,$(builddir)/%, $(nodist_libgstprofile_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#gir_sources=$(patsubst %,$(srcdir)/%, $(libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES))
#gir_sources+=$(patsubst %,$(builddir)/%, $(nodist_libgstprofile_gupnp_dlna_@GST_MAJORMINOR@_la_SOURCES))
#gir_cincludes=$(patsubst %,--c-include='gst/profile/%',$(libgstprofile_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#gir_cincludes+=$(patsubst %,--c-include='gst/profile/%',$(nodist_libgstprofile_gupnp_dlna_@GST_MAJORMINOR@include_HEADERS))
#
#GstProfile-@GST_MAJORMINOR@.gir: $(INTROSPECTION_SCANNER) libgstprofile-gupnp-dlna-@GST_MAJORMINOR@.la
#	$(AM_V_GEN)PKG_CONFIG_PATH="$(PKG_CONFIG_PATH):$(top_builddir)/pkgconfig" \
#		$(INTROSPECTION_SCANNER) -v --namespace GstProfile \
#		--nsversion=@GST_MAJORMINOR@ \
#		--strip-prefix=Gst \
#		$(gir_cincludes) \
#		-I$(top_srcdir)/gst-libs \
#		-I$(top_builddir)/gst-libs \
#		--add-include-path=$$($(PKG_CONFIG) --variable=girdir gstreamer-@GST_MAJORMINOR@) \
#		--add-include-path=$$($(PKG_CONFIG) --variable=girdir gstreamer-base-@GST_MAJORMINOR@) \
#		--library=libgstprofile-gupnp-dlna-@GST_MAJORMINOR@.la \
#		--include=Gst-@GST_MAJORMINOR@ \
#		--include=GstBase-@GST_MAJORMINOR@ \
#		--include=GstVideo-@GST_MAJORMINOR@ \
#		--libtool="$(top_builddir)/libtool" \
#		--pkg gstreamer-@GST_MAJORMINOR@ \
#		--pkg gstreamer-base-@GST_MAJORMINOR@ \
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
#		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)
#
#CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
#
#endif
