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

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                GUPNP_TYPE_DLNA_DISCOVERER, \
                                GUPnPDLNADiscovererPrivate))

typedef struct _GUPnPDLNADiscovererPrivate GUPnPDLNADiscovererPrivate;

struct _GUPnPDLNADiscovererPrivate {
        gboolean  relaxed_mode;
        gboolean  extended_mode;
};

enum {
        PROP_0,
        PROP_DLNA_RELAXED_MODE,
        PROP_DLNA_EXTENDED_MODE,
};

static void
gupnp_dlna_discoverer_set_property (GObject      *object,
                                    guint        property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
        GUPnPDLNADiscoverer *self = GUPNP_DLNA_DISCOVERER (object);
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (self);

        switch (property_id) {
                case PROP_DLNA_RELAXED_MODE:
                        priv->relaxed_mode = g_value_get_boolean (value);
                        break;

                case PROP_DLNA_EXTENDED_MODE:
                        priv->extended_mode = g_value_get_boolean (value);
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
                                                           property_id,
                                                           pspec);
                        break;
        }
}

static void
gupnp_dlna_discoverer_get_property (GObject    *object,
                                    guint      property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
        GUPnPDLNADiscoverer *self = GUPNP_DLNA_DISCOVERER (object);
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (self);

        switch (property_id) {
                case PROP_DLNA_RELAXED_MODE:
                        g_value_set_boolean (value, priv->relaxed_mode);
                        break;

                case PROP_DLNA_EXTENDED_MODE:
                        g_value_set_boolean (value, priv->extended_mode);
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
                                                           property_id,
                                                           pspec);
                        break;
        }
}

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

static void gupnp_dlna_discovered_cb (GstDiscoverer     *discoverer,
                                      GstDiscovererInfo *info,
                                      GError            *err)
{
        GUPnPDLNAInformation *dlna = NULL;
        GUPnPDLNADiscovererClass *klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (discoverer);
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (GUPNP_DLNA_DISCOVERER (discoverer));
        gboolean relaxed = priv->relaxed_mode;
        gboolean extended = priv->extended_mode;

        if (info)
                dlna = gupnp_dlna_information_new_from_discoverer_info (info,
                                                 klass->profiles_list [relaxed][extended]);

        g_signal_emit (GUPNP_DLNA_DISCOVERER (discoverer),
                       signals[DONE], 0, dlna, err);

        if (dlna)
                g_object_unref (dlna);
}

static void
gupnp_dlna_discoverer_class_init (GUPnPDLNADiscovererClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GParamSpec *pspec;

        g_type_class_add_private (klass, sizeof (GUPnPDLNADiscovererPrivate));

        object_class->get_property = gupnp_dlna_discoverer_get_property;
        object_class->set_property = gupnp_dlna_discoverer_set_property;
        object_class->dispose = gupnp_dlna_discoverer_dispose;
        object_class->finalize = gupnp_dlna_discoverer_finalize;

        /**
         * GUPnPDLNADiscoverer::relaxed-mode:
         * @relaxed_mode: setting to true will enable relaxed mode
         *
         * The current release does not support relaxed mode yet
         */
        pspec = g_param_spec_boolean ("relaxed-mode",
                                      "Relaxed mode property",
                                      "Indicates that profile matching should"
                                      "be strictly compliant with the DLNA "
                                      "specification",
                                      FALSE,
                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class,
                                         PROP_DLNA_RELAXED_MODE,
                                         pspec);

        /**
         * GUPnPDLNADiscoverer::extended-mode:
         * @extended: setting true will enable extended profile support
         *
         * The current release does not support extended mode yet
         */
        pspec = g_param_spec_boolean ("extended-mode",
                                      "Extended mode property",
                                      "Indicates support for profiles that are "
                                      "not part of the DLNA specification",
                                      FALSE,
                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class,
                                         PROP_DLNA_EXTENDED_MODE,
                                         pspec);

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
        if (g_type_from_name ("GstElement")) {
                klass->profiles_list [0][0]
                        = gupnp_dlna_load_profiles_from_disk (FALSE,
                                                              FALSE);
                klass->profiles_list [0][1]
                        = gupnp_dlna_load_profiles_from_disk (FALSE,
                                                              TRUE);
                klass->profiles_list [1][0]
                        = gupnp_dlna_load_profiles_from_disk (TRUE,
                                                              FALSE);
                klass->profiles_list [1][1]
                        = gupnp_dlna_load_profiles_from_disk (TRUE,
                                                              TRUE);
        } else {
                klass->profiles_list [0][0] = NULL;
                klass->profiles_list [0][1] = NULL;
                klass->profiles_list [1][0] = NULL;
                klass->profiles_list [1][1] = NULL;
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
gupnp_dlna_discoverer_new (GstClockTime timeout,
                           gboolean     relaxed_mode,
                           gboolean     extended_mode)
{
        return g_object_new (GUPNP_TYPE_DLNA_DISCOVERER,
                             "timeout", timeout,
                             "relaxed-mode", relaxed_mode,
                             "extended-mode", extended_mode,
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
        return gst_discoverer_discover_uri_async (GST_DISCOVERER (discoverer), uri);
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
        GstDiscovererInfo *info;
        GUPnPDLNADiscovererClass *klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (discoverer);
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (discoverer);
        gboolean relaxed = priv->relaxed_mode;
        gboolean extended = priv->extended_mode;

        info = gst_discoverer_discover_uri (GST_DISCOVERER (discoverer),
                                            uri,
                                            err);

        if (info)
                return gupnp_dlna_information_new_from_discoverer_info (info,
                                     klass->profiles_list [relaxed][extended]);

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
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (self);
        gboolean relaxed = priv->relaxed_mode;
        gboolean extended = priv->extended_mode;

        g_return_val_if_fail (self != NULL, NULL);
        klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (self);

        for (i = klass->profiles_list [relaxed][extended];
             i != NULL;
             i = i->next) {
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
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (self);
        gboolean relaxed = priv->relaxed_mode;
        gboolean extended = priv->extended_mode;

        g_return_val_if_fail (self != NULL, NULL);

        klass = GUPNP_DLNA_DISCOVERER_GET_CLASS (self);

        return klass->profiles_list [relaxed][extended];
}

/**
 * gupnp_dlna_discoverer_get_relaxed_mode:
 * @self: The #GUPnPDLNADiscoverer object
 *
 * Returns: true if relaxed mode is set and false otherwise
 */
gboolean
gupnp_dlna_discoverer_get_relaxed_mode (GUPnPDLNADiscoverer *self)
{
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (self);
        return priv->relaxed_mode;
}

/**
 * gupnp_dlna_discoverer_get_extended_mode:
 * @self: The #GUPnPDLNADiscoverer object
 *
 * Returns: true if application is using extended mode and false otherwise
 */
gboolean
gupnp_dlna_discoverer_get_extended_mode (GUPnPDLNADiscoverer *self)
{
        GUPnPDLNADiscovererPrivate *priv = GET_PRIVATE (self);
        return priv->extended_mode;
}
