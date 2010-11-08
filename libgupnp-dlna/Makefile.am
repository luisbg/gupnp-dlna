# Version format current:revision:age
# If the library source code has changed at all since the last update, then
# increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
# If any interfaces have been added, removed, or changed since the last update,
# increment current, and set revision to 0.
# If any interfaces have been added since the last public release, then
# increment age.
# If any interfaces have been removed since the last public release, then set
# age to 0.
LTVERSION = 0:0:0

shareddir = $(datadir)/gupnp-dlna

AM_CFLAGS = -I$(top_srcdir) \
	    -I$(top_srcdir)/gst-convenience/gst-libs \
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

noinst_HEADERS = gupnp-dlna-load.h

libgupnp_dlna_1_0_la_SOURCES = gupnp-dlna-information.c \
			       gupnp-dlna-discoverer.c \
			       gupnp-dlna-profile.c \
			       gupnp-dlna-profiles.c \
			       gupnp-dlna-load.c \
			       $(BUILT_SOURCES)

libgupnp_dlna_1_0_la_LIBADD = $(LIBXML_LIBS) \
                              $(top_builddir)/gst-convenience/gst-libs/gst/profile/.libs/libgstprofile-gupnp-dlna-0.10.la \
			      $(GST_PBU_LIBS)

EXTRA_DIST = gupnp-dlna-marshal.list

CLEANFILES = $(BUILT_SOURCES)
