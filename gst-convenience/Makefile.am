SUBDIRS = gst-libs

vapidir = $(datadir)/vala/vapi
vapifiles = gstreamer-discoverer-gupnp-dlna-0.10.vapi gstreamer-profile-gupnp-dlna-0.10.vapi
vapideps = gstreamer-discoverer-gupnp-dlna-0.10.deps gstreamer-profile-gupnp-dlna-0.10.deps

EXTRA_DIST = $(vapifiles) $(vapideps)

install-data-local:
	$(INSTALL_DATA) -d ${DESTDIR}$(vapidir) -m 755
	@for i in $(vapifiles) $(vapideps); do \
		$(INSTALL_DATA) $(top_srcdir)/gst-convenience/$${i} ${DESTDIR}$(vapidir)/; \
	done

uninstall-local:
	cd ${DESTDIR}${vapidir} && rm -f ${vapifiles} ${vapideps}
