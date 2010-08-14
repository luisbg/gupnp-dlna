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
#include "gupnp-dlna-load.h"

/**
 * SECTION:gupnp-dlna-discoverer
 * @short_description: Utility API for discovering DLNA profile/mime type and
 * other metadata for given media.
 *
 * The GUPnPDLNADiscoverer object provides a light-weight wrapper over the
 * #GstDiscoverer API. The latter provides a simple interface to discover
 * media metadata given a URI. GUPnPDLNADiscoverer extends this API to also
 * provide a DLNA profile name and mime type for the media.
 *
 * The API provided corresponds very closely to the API provided by
 * #GstDiscoverer - both synchronous and asynchronous discovery of metadata
 * are possible.
 *
 * The asynchronous mode requires a running #GMainLoop in the default
 * #GMainContext, where one connects to the various signals, appends the
 * URIs to be processed and then asks for the discovery to begin.
 */
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
        GUPnPDLNADiscovererClass *klass =
                GUPNP_DLNA_DISCOVERER_GET_CLASS (object);

        g_list_foreach (klass->profiles_list, (GFunc) g_object_unref, NULL);
        g_list_free (klass->profiles_list);
        klass->profiles_list = NULL;

        G_OBJECT_CLASS (gupnp_dlna_discoverer_parent_class)->finalize (object);
}

static void gupnp_dlna_discovered_cb (GstDiscoverer            *discoverer,
                                      GstDiscovererInformation *info,
                                      GError                   *err)
{
        GUPnPDLNAInformation *dlna = NULL;
        GUPnPDLNADiscovererClass *klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (discoverer);

        if (info)
                dlna = gupnp_dlna_information_new_from_discoverer_info (info,
                                                                        klass->profiles_list);

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
         * @err: contains details of the error if discovery fails, else is NULL
         *
         * Will be emitted when all information on a URI could be discovered.
         *
         * The reciever must unref @dlna with when done using it.
         */
        signals[DONE] =
                g_signal_new ("done", G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (GUPnPDLNADiscovererClass, done),
                              NULL, NULL,
                              gupnp_dlna_marshal_VOID__OBJECT_BOXED,
                              G_TYPE_NONE, 2, GUPNP_TYPE_DLNA_INFORMATION,
                              GST_TYPE_G_ERROR);

        /* Load DLNA profiles from disk */
        if (g_type_from_name ("GstElement"))
                klass->profiles_list = gupnp_dlna_load_profiles_from_disk ();
        else {
                klass->profiles_list = NULL;
                g_warning ("GStreamer has not yet been initialised. You need "
                           "to call gst_init()/gst_init_check() for discovery "
                           "to work.");
        }
}

static void
gupnp_dlna_discoverer_init (GUPnPDLNADiscoverer *self)
{
        g_signal_connect (&self->parent,
                          "discovered",
                          G_CALLBACK (gupnp_dlna_discovered_cb),
                          NULL);
}

/**
 * gupnp_dlna_discoverer_new:
 * @timeout: default discovery timeout, in nanoseconds
 *
 * Creates a new #GUPnPDLNADiscoverer object with the given default timeout
 * value.
 *
 * Returns: A new #GUPnPDLNADiscoverer object.
 */
GUPnPDLNADiscoverer*
gupnp_dlna_discoverer_new (GstClockTime timeout)
{
        return g_object_new (GUPNP_TYPE_DLNA_DISCOVERER,
                             "timeout", timeout,
                             NULL);
}

/* Asynchronous API */

/**
 * gupnp_dlna_discoverer_start:
 * @discoverer: #GUPnPDLNADiscoverer object to start discovery on
 *
 * Allows asynchronous discovery of URIs to begin.
 */

/**
 * gupnp_dlna_discoverer_stop:
 * @discoverer: #GUPnPDLNADiscoverer object to stop discovery on
 *
 * Stops asynchronous discovery of URIs.
 */

/**
 * gupnp_dlna_discoverer_discover_uri:
 * @discoverer: #GUPnPDLNADiscoverer object to use for discovery
 * @uri: URI to gather metadata for
 *
 * Queues @uri for metadata discovery. When discovery is completed, the
 * "discovered" signal is emitted on @discoverer.
 *
 * Returns: TRUE if @uri was successfully queued, FALSE otherwise.
 */
gboolean
gupnp_dlna_discoverer_discover_uri (GUPnPDLNADiscoverer *discoverer, gchar *uri)
{
        return gst_discoverer_append_uri (GST_DISCOVERER (discoverer), uri);
}

/* Synchronous API */

/**
 * gupnp_dlna_discoverer_discover_uri_sync:
 * @discoverer: #GUPnPDLNADiscoverer object to use for discovery
 * @uri: URI to gather metadata for
 * @err: contains details of the error if discovery fails, else is NULL
 *
 * Synchronously gathers metadata for @uri.
 *
 * Returns: a #GUPnPDLNAInformation with the metadata for @uri on success, NULL
 *          otherwise
 */
GUPnPDLNAInformation *
gupnp_dlna_discoverer_discover_uri_sync (GUPnPDLNADiscoverer *discoverer,
                                         gchar               *uri,
                                         GError              **err)
{
        GstDiscovererInformation *info;
        GUPnPDLNADiscovererClass *klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (discoverer);

        info = gst_discoverer_discover_uri (GST_DISCOVERER (discoverer),
                                            uri,
                                            err);

        if (info)
                return gupnp_dlna_information_new_from_discoverer_info (info,
                                                                        klass->profiles_list);

        return NULL;
}

/**
 * gupnp_dlna_discoverer_get_profile:
 * @self: The #GUPnPDLNADiscoverer object
 * @name: The name of the DLNA profile to be retrieved
 *
 * Given @name, this finds the corresponding DLNA profile information (stored
 * as a #GUPnPDLNAProfile).
 *
 * Returns: a #GUPnPDLNAProfile on success, NULL otherwise.
 **/
GUPnPDLNAProfile *
gupnp_dlna_discoverer_get_profile (GUPnPDLNADiscoverer *self,
                                   const gchar         *name)
{
        GList *i;
        GUPnPDLNADiscovererClass *klass;

        g_return_val_if_fail (self != NULL, NULL);
        klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (self);

        for (i = klass->profiles_list; i != NULL; i = i->next) {
                GUPnPDLNAProfile *profile = (GUPnPDLNAProfile *) i->data;

                if (g_str_equal (gupnp_dlna_profile_get_name (profile), name)) {
                        g_object_ref (profile);
                        return profile;
                }
        }

        return NULL;
}

/**
 * gupnp_dlna_discoverer_list_profiles:
 * @self: The #GUPnPDLNADiscoverer whose profile list is required
 *
 * Retuns a list of the all the DLNA profiles supported by @self.
 *
 * Returns: a #GList of #GUPnPDLNAProfile on success, NULL otherwise.
 **/
const GList *
gupnp_dlna_discoverer_list_profiles (GUPnPDLNADiscoverer *self)
{
        GUPnPDLNADiscovererClass *klass;

        g_return_val_if_fail (self != NULL, NULL);

        klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (self);

        return klass->profiles_list;
}
