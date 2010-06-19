shareddir = $(datadir)/gupnp-dlna

dlnaschemas = dlna-profiles.rng
dlnaprofiles = mp3.xml \
	       ac3.xml \
	       lpcm.xml \
	       aac.xml \
	       wma.xml \
	       amr.xml \
	       common.xml \
	       mpeg1.xml \
	       avc.xml \
	       mpeg-ts.xml \
	       mpeg4.xml \
	       jpeg.xml \
	       png.xml

dlnadir = $(shareddir)/dlna-profiles
dlna_DATA = $(dlnaschemas) $(dlnaprofiles)

EXTRA_DIST = $(dlnaschemas) $(dlnaprofiles)
