noinst_PROGRAMS = dlna-profile-parser dlna-encoding

AM_CFLAGS = -I$(top_srcdir) $(GST_CFLAGS) $(GST_PBU_CFLAGS) $(LIBXML_CFLAGS)
LIBS = $(GST_LIBS) \
       $(LIBXML_LIBS) \
       $(GST_PBU_LIBS) \
       $(top_builddir)/libgupnp-dlna/libgupnp-dlna-1.0.la

dlna_profile_parser_SOURCES = dlna-profile-parser.c
dlna_encoding_SOURCES = dlna-encoding.c

TESTS_ENVIRONMENT = MEDIA_DIR="$(srcdir)/media" FILE_LIST="$(srcdir)/media/media-list.txt" ${SHELL}
TESTS = test-discoverer.sh
