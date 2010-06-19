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

#include "gupnp-dlna-discoverer.h"
#include "gupnp-dlna-marshal.h"

enum {
        DONE,
        SIGNAL_LAST
};

static guint signals[SIGNAL_LAST];


G_DEFINE_TYPE (GUPnPDLNADiscoverer, gupnp_dlna_discoverer, GST_TYPE_DISCOVERER)

static void
gupnp_dlna_discoverer_dispose (GObject *object)
{
        G_OBJECT_CLASS (gupnp_dlna_discoverer_parent_class)->dispose (object);
}

static void
gupnp_dlna_discoverer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gupnp_dlna_discoverer_parent_class)->finalize (object);
}

static void gupnp_dlna_discovered_cb (GstDiscoverer            *discoverer,
                                      GstDiscovererInformation *info,
                                      GError                   *err)
{
        GUPnPDLNAInformation *dlna = NULL;

        if (info)
                dlna = gupnp_dlna_information_new_from_discoverer_info (info);

        g_signal_emit (GUPNP_DLNA_DISCOVERER (discoverer),
                       signals[DONE], 0, dlna, err);

        if (dlna)
                g_object_unref (dlna);
}

static void
gupnp_dlna_discoverer_class_init (GUPnPDLNADiscovererClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->dispose = gupnp_dlna_discoverer_dispose;
        object_class->finalize = gupnp_dlna_discoverer_finalize;

        /**
         * GUPnPDLNADiscoverer::done:
         * @discoverer: the #GUPnPDLNADiscoverer
         * @dlna: the results as #GUPnPDLNAInformation
         *
         * Will be emitted when all information on a URI could be discovered.
         *
         * The reciever must free @dlna with #gupnp_dlna_information_free() when
         * done using it.
         */
        signals[DONE] =
                g_signal_new ("done", G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (GUPnPDLNADiscovererClass, done),
                              NULL, NULL,
                              gupnp_dlna_marshal_VOID__OBJECT_BOXED,
                              G_TYPE_NONE, 2, GUPNP_TYPE_DLNA_INFORMATION,
                              GST_TYPE_G_ERROR);
}

static void
gupnp_dlna_discoverer_init (GUPnPDLNADiscoverer *self)
{
        g_signal_connect (&self->parent,
                          "discovered",
                          G_CALLBACK (gupnp_dlna_discovered_cb),
                          NULL);
}

GUPnPDLNADiscoverer*
gupnp_dlna_discoverer_new (GstClockTime timeout)
{
        return g_object_new (GUPNP_TYPE_DLNA_DISCOVERER,
                             "timeout", timeout,
                             NULL);
}

/* Asynchronous API */
void gupnp_dlna_discoverer_start (GUPnPDLNADiscoverer *discoverer)
{
        gst_discoverer_start (GST_DISCOVERER (discoverer));
}
void gupnp_dlna_discoverer_stop (GUPnPDLNADiscoverer *discoverer)
{
        gst_discoverer_stop (GST_DISCOVERER (discoverer));
}

gboolean
gupnp_dlna_discoverer_discover_uri (GUPnPDLNADiscoverer *discoverer, gchar *uri)
{
        return gst_discoverer_append_uri (GST_DISCOVERER (discoverer), uri);
}

/* Synchronous API */
GUPnPDLNAInformation *
gupnp_dlna_discoverer_discover_uri_sync (GUPnPDLNADiscoverer *discoverer,
                                         gchar               *uri,
                                         GError              **err)
{
        GstDiscovererInformation *info;

        info = gst_discoverer_discover_uri (GST_DISCOVERER (discoverer),
                                            uri,
                                            err);

        if (info)
                return gupnp_dlna_information_new_from_discoverer_info (info);

        return NULL;
}
