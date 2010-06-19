/*
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Authors: Arun Raghavan <arun.raghavan@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _GUPNP_DLNA_DISCOVERER
#define _GUPNP_DLNA_DISCOVERER

#include <glib-object.h>
#include <gst/discoverer/gstdiscoverer.h>
#include "gupnp-dlna-information.h"

G_BEGIN_DECLS

#define GUPNP_TYPE_DLNA_DISCOVERER gupnp_dlna_discoverer_get_type()

#define GUPNP_DLNA_DISCOVERER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
         GUPNP_TYPE_DLNA_DISCOVERER, \
         GUPnPDLNADiscoverer))

#define GUPNP_DLNA_DISCOVERER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), \
         GUPNP_TYPE_DLNA_DISCOVERER, \
         GUPnPDLNADiscovererClass))

#define GUPNP_IS_DLNA_DISCOVERER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
         GUPNP_TYPE_DLNA_DISCOVERER))

#define GUPNP_IS_DLNA_DISCOVERER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
         GUPNP_TYPE_DLNA_DISCOVERER))

#define GUPNP_DLNA_DISCOVERER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
         GUPNP_TYPE_DLNA_DISCOVERER, \
         GUPnPDLNADiscovererClass))

/** GUPnPDLNADiscoverer
 * @parent: The parent #GstDiscoverer object
 *
 * The GUPnPDLNADiscoverer API provides a light-weight wrapper over the
 * #GstDiscoverer API. The latter provides a simple interface to discover
 * media metadata given a URI. #GUPnPDLNADiscoverer extends this API to also
 * provide a DLNA profile name and mime type for the media.
 *
 * The API provided is identical to the API provided by #GstDiscoverer.
 */
typedef struct {
        GstDiscoverer parent;
} GUPnPDLNADiscoverer;

typedef struct {
        GstDiscovererClass parent_class;

        void (*done) (GUPnPDLNADiscoverer *discoverer,
                      GUPnPDLNAInformation *dlna,
                      GError *err);
} GUPnPDLNADiscovererClass;

GType gupnp_dlna_discoverer_get_type (void);

GUPnPDLNADiscoverer* gupnp_dlna_discoverer_new (GstClockTime timeout);

/* Asynchronous API */
void gupnp_dlna_discoverer_start (GUPnPDLNADiscoverer *discoverer);
void gupnp_dlna_discoverer_stop (GUPnPDLNADiscoverer *discoverer);
gboolean
gupnp_dlna_discoverer_discover_uri (GUPnPDLNADiscoverer *discoverer,
                                    gchar               *uri);

/* Synchronous API */
GUPnPDLNAInformation *
gupnp_dlna_discoverer_discover_uri_sync (GUPnPDLNADiscoverer *discoverer,
                                         gchar               *uri,
                                         GError              **err);

G_END_DECLS

#endif /* _GUPNP_DLNA_DISCOVERER */
