SUBDIRS = gst-libs

vapidir = $(datadir)/vala/vapi
vapifiles = gstreamer-profile-gupnp-dlna-0.10.vapi
vapideps = gstreamer-profile-gupnp-dlna-0.10.deps

EXTRA_DIST = $(vapifiles) $(vapideps)

install-data-local:
	$(INSTALL) -d ${DESTDIR}$(vapidir) -m 755
	@for i in $(vapifiles) $(vapideps); do \
		$(INSTALL_DATA) $(top_srcdir)/gst-convenience/$${i} ${DESTDIR}$(vapidir)/; \
	done

uninstall-local:
	cd ${DESTDIR}${vapidir} && rm -f ${vapifiles} ${vapideps}
