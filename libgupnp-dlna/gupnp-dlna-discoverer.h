/*
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Authors: Arun Raghavan <arun.raghavan@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _GUPNP_DLNA_DISCOVERER
#define _GUPNP_DLNA_DISCOVERER

#include <glib-object.h>
#include <gst/discoverer/gstdiscoverer.h>
#include "gupnp-dlna-information.h"
#include "gupnp-dlna-profile.h"

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

/**
 * GUPnPDLNADiscoverer:
 *
 * The top-level object used to for metadata extraction.
 */
typedef struct {
        GstDiscoverer parent;
} GUPnPDLNADiscoverer;

typedef struct {
        GstDiscovererClass parent_class;

        /*< signals >*/
        void (*done) (GUPnPDLNADiscoverer *discoverer,
                      GUPnPDLNAInformation *dlna,
                      GError *err);

        /*< private >*/
        GList *profiles_list;

} GUPnPDLNADiscovererClass;

GType gupnp_dlna_discoverer_get_type (void);

GUPnPDLNADiscoverer *
gupnp_dlna_discoverer_new (GstClockTime timeout,
                           gboolean     relaxed_mode,
                           gboolean     extended_mode);

/* Asynchronous API */
#define gupnp_dlna_discoverer_start(discoverer) \
        gst_discoverer_start(GST_DISCOVERER((discoverer)))
#define gupnp_dlna_discoverer_stop(discoverer) \
        gst_discoverer_stop(GST_DISCOVERER((discoverer)))
gboolean
gupnp_dlna_discoverer_discover_uri (GUPnPDLNADiscoverer *discoverer,
                                    gchar               *uri);

/* Synchronous API */
GUPnPDLNAInformation *
gupnp_dlna_discoverer_discover_uri_sync (GUPnPDLNADiscoverer *discoverer,
                                         gchar               *uri,
                                         GError              **err);

/* Get a GUPnPDLNAProfile by name */
GUPnPDLNAProfile *
gupnp_dlna_discoverer_get_profile (GUPnPDLNADiscoverer *self,
                                   const gchar         *name);

/* API to list all available profiles */
const GList *
gupnp_dlna_discoverer_list_profiles (GUPnPDLNADiscoverer *self);
gboolean
gupnp_dlna_discoverer_get_relaxed_mode (GUPnPDLNADiscoverer *self);
gboolean
gupnp_dlna_discoverer_get_extended_mode (GUPnPDLNADiscoverer *self);

G_END_DECLS

#endif /* _GUPNP_DLNA_DISCOVERER */
