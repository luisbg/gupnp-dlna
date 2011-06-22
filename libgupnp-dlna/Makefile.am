# Version format current:revision:age
# If the library source code has changed at all since the last update, then
# increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
# If any interfaces have been added, removed, or changed since the last update,
# increment current, and set revision to 0.
# If any interfaces have been added since the last public release, then
# increment age.
# If any interfaces have been removed since the last public release, then set
# age to 0.
LTVERSION = $(GUPNP_DLNA_VERSION_INFO)

shareddir = $(datadir)/gupnp-dlna

AM_CFLAGS = -I$(top_srcdir) \
	    $(LIBXML_CFLAGS) \
	    $(GST_CFLAGS) \
	    $(GST_PBU_CFLAGS) \
	    -DDATA_DIR='"$(shareddir)"' \
	    -DG_LOG_DOMAIN='"gupnp-dlna"'

lib_LTLIBRARIES = libgupnp-dlna-1.0.la

gupnp-dlna-marshal.c: gupnp-dlna-marshal.list
	$(AM_V_GEN) \
	$(GLIB_GENMARSHAL) --prefix=gupnp_dlna_marshal $(srcdir)/gupnp-dlna-marshal.list --header --body > gupnp-dlna-marshal.c

gupnp-dlna-marshal.h: gupnp-dlna-marshal.list
	$(AM_V_GEN) \
	$(GLIB_GENMARSHAL) --prefix=gupnp_dlna_marshal $(srcdir)/gupnp-dlna-marshal.list --header > gupnp-dlna-marshal.h

BUILT_SOURCES = gupnp-dlna-marshal.c gupnp-dlna-marshal.h

libgupnp_dlna_incdir = $(includedir)/gupnp-dlna-1.0/libgupnp-dlna

libgupnp_dlna_1_0_la_LDFLAGS = -version-info $(LTVERSION) -no-undefined

libgupnp_dlna_inc_HEADERS = gupnp-dlna-profile.h \
			    gupnp-dlna-information.h \
			    gupnp-dlna-discoverer.h

noinst_HEADERS = profile-loading.h \
                 gupnp-dlna-profile-private.h

introspection_sources = $(libgupnp_dlna_inc_HEADERS) \
			gupnp-dlna-information.c \
			gupnp-dlna-discoverer.c \
			gupnp-dlna-profile.c \
			gupnp-dlna-profiles.c \
			profile-loading.c

libgupnp_dlna_1_0_la_SOURCES = $(introspection_sources) \
			       $(BUILT_SOURCES)

libgupnp_dlna_1_0_la_LIBADD = $(LIBXML_LIBS) \
			      $(GST_PBU_LIBS)

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all				\
			     --symbol-prefix=gupnp		\
			     --identifier-prefix=GUPnP	\
			     --add-include-path=$(top_srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_srcdir)

if HAVE_INTROSPECTION
GUPnP-DLNA-1.0.gir: libgupnp-dlna-1.0.la
GUPnP_DLNA_1_0_gir_INCLUDES = libxml2-2.0 GObject-2.0 GstPbutils-0.10
GUPnP_DLNA_1_0_gir_CFLAGS =  $(INCLUDES) $(AM_CFLAGS)
GUPnP_DLNA_1_0_gir_LIBS = libgupnp-dlna-1.0.la
GUPnP_DLNA_1_0_gir_FILES = $(introspection_sources)
GUPnP_DLNA_1_0_gir_NAMESPACE = GUPnP
GUPnP_DLNA_1_0_gir_VERSION = 1.0

INTROSPECTION_GIRS += GUPnP-DLNA-1.0.gir

girdir = $(datatdir)/gir-1.0
gir_DATA = GUPnP-DLNA-1.0.gir

typelibdir = $(libdir)/girepository-1.0/
typelib_DATA = $(gir_DATA:.gir=.typelib)
endif

EXTRA_DIST = gupnp-dlna-marshal.list

CLEANFILES = $(BUILT_SOURCES) $(gir_DATA) $(typelib_DATA)
