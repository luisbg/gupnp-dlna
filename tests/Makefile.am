noinst_PROGRAMS = dlna-profile-parser dlna-encoding

AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/gst-convenience/gst-libs $(GST_CFLAGS) $(LIBXML_CFLAGS)
LIBS = $(GST_LIBS) \
       $(LIBXML_LIBS) \
       $(top_builddir)/gst-convenience/gst-libs/gst/profile/.libs/libgstprofile-gupnp-dlna-@GST_MAJORMINOR@.la \
       $(top_builddir)/libgupnp-dlna/libgupnp-dlna-1.0.la

dlna_profile_parser_SOURCES = dlna-profile-parser.c
dlna_encoding_SOURCES = dlna-encoding.c

TESTS_ENVIRONMENT = MEDIA_DIR="$(srcdir)/media" FILE_LIST="$(srcdir)/media/media-list.txt" ${SHELL}
TESTS = test-discoverer.sh
