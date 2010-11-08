bin_PROGRAMS = gupnp-dlna-info gupnp-dlna-ls-profiles

AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/gst-convenience/gst-libs $(GST_CFLAGS)
LIBS = $(GST_LIBS) \
       $(top_builddir)/libgupnp-dlna/libgupnp-dlna-1.0.la

gupnp_dlna_ls_profiles_LDADD = \
	     $(top_builddir)/gst-convenience/gst-libs/gst/profile/.libs/libgstprofile-gupnp-dlna-@GST_MAJORMINOR@.la
