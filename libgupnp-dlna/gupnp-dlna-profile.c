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

#include "gupnp-dlna-profile.h"
#include <gst/gstminiobject.h>

/**
 * SECTION:gupnp-dlna-profile
 * @short_description: Object representing a DLNA profile
 *
 * The #GUPnPDLNADiscoverer object provides a few APIs that return
 * #GUPnPDLNAProfile objects. These represent a single DLNA profile. Each
 * #GUPnPDLNAProfile has a name (the name of the DLNA profile), the
 * corresponding MIME type, and a #GstEncodingProfile which represents the
 * various audio/video/container restrictions specified for that DLNA profile.
 */
G_DEFINE_TYPE (GUPnPDLNAProfile, gupnp_dlna_profile, G_TYPE_OBJECT)

#define GET_PRIVATE(o)                                          \
        (G_TYPE_INSTANCE_GET_PRIVATE ((o),                      \
                                      GUPNP_TYPE_DLNA_PROFILE,  \
                                      GUPnPDLNAProfilePrivate))

typedef struct _GUPnPDLNAProfilePrivate GUPnPDLNAProfilePrivate;

struct _GUPnPDLNAProfilePrivate {
        gchar              *name;
        gchar              *mime;
        GstCaps            *container_caps;
        GstCaps            *video_caps;
        GstCaps            *audio_caps;
        gboolean           extended;
        GstEncodingProfile *enc_profile;
};

enum {
        PROP_0,
        PROP_DLNA_NAME,
        PROP_DLNA_MIME,
        PROP_DLNA_EXTENDED,
};

static void
gupnp_dlna_profile_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
        GUPnPDLNAProfile *self = GUPNP_DLNA_PROFILE (object);
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        switch (property_id) {
                case PROP_DLNA_NAME:
                        g_value_set_string (value, priv->name);
                        break;

                case PROP_DLNA_MIME:
                        g_value_set_string (value, priv->mime);
                        break;

                case PROP_DLNA_EXTENDED:
                        g_value_set_boolean (value, priv->extended);
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
                                                           property_id,
                                                           pspec);
                        break;
        }
}

static void
gupnp_dlna_profile_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
        GUPnPDLNAProfile *self = GUPNP_DLNA_PROFILE (object);
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        switch (property_id) {
                case PROP_DLNA_NAME:
                        g_free (priv->name);
                        priv->name = g_value_dup_string (value);
                        break;

                case PROP_DLNA_MIME:
                        g_free (priv->mime);
                        priv->mime = g_value_dup_string (value);
                        break;

                case PROP_DLNA_EXTENDED:
                        priv->extended = g_value_get_boolean (value);
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID
                                (object, property_id, pspec);
                        break;
        }
}

static void
gupnp_dlna_profile_finalize (GObject *object)
{
        GUPnPDLNAProfile *self = GUPNP_DLNA_PROFILE (object);
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        g_free (priv->name);
        g_free (priv->mime);

        if (priv->enc_profile)
                gst_encoding_profile_unref (priv->enc_profile);

        G_OBJECT_CLASS (gupnp_dlna_profile_parent_class)->finalize (object);
}

static void
gupnp_dlna_profile_class_init (GUPnPDLNAProfileClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GParamSpec *pspec;

        g_type_class_add_private (klass, sizeof (GUPnPDLNAProfilePrivate));

        object_class->get_property = gupnp_dlna_profile_get_property;
        object_class->set_property = gupnp_dlna_profile_set_property;
        object_class->finalize = gupnp_dlna_profile_finalize;

        pspec = g_param_spec_string ("name",
                                     "DLNA profile name",
                                     "The name of the DLNA profile ",
                                     NULL,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class, PROP_DLNA_NAME, pspec);

        pspec = g_param_spec_string ("mime",
                                     "DLNA profile MIME type",
                                     "The MIME type of the DLNA profile",
                                     NULL,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class, PROP_DLNA_MIME, pspec);

        pspec = g_param_spec_boolean ("extended",
                                      "Extended mode property",
                                      "Indicates that this profile is not "
                                      "part of the DLNA specification",
                                      FALSE,
                                      G_PARAM_READWRITE |
                                      G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class,
                                         PROP_DLNA_EXTENDED,
                                         pspec);

}

static void
gupnp_dlna_profile_init (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        priv->name = NULL;
        priv->mime = NULL;
        priv->enc_profile = NULL;
        priv->extended = FALSE;
}

GstCaps *
gupnp_dlna_profile_get_container_caps (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);
        return priv->container_caps;
}

GstCaps *
gupnp_dlna_profile_get_video_caps (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);
        return priv->video_caps;
}

GstCaps *
gupnp_dlna_profile_get_audio_caps (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);
        return priv->audio_caps;
}

void
gupnp_dlna_profile_set_container_caps (GUPnPDLNAProfile *self, GstCaps *caps)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        if (priv->container_caps)
                gst_caps_unref (priv->container_caps);
        priv->container_caps = gst_caps_copy (caps);
}

void
gupnp_dlna_profile_set_video_caps (GUPnPDLNAProfile *self, GstCaps *caps)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        if (priv->video_caps)
                gst_caps_unref (priv->video_caps);
        priv->video_caps = gst_caps_copy (caps);
}

void
gupnp_dlna_profile_set_audio_caps (GUPnPDLNAProfile *self, GstCaps *caps)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        if (priv->audio_caps)
                gst_caps_unref (priv->audio_caps);
        priv->audio_caps = gst_caps_copy (caps);
}

/**
 * gupnp_dlna_profile_new:
 *
 * Creates a new #GUPnPDLNAProfile object.
 *
 * Returns: A new #GUPnPDLNAProfile object.
 */
GUPnPDLNAProfile *
gupnp_dlna_profile_new (gchar              *name,
                        gchar              *mime,
                        GstCaps            *container_caps,
                        GstCaps            *video_caps,
                        GstCaps            *audio_caps,
                        gboolean           extended)
{
        GUPnPDLNAProfile *prof;

        prof =  g_object_new (GUPNP_TYPE_DLNA_PROFILE,
                              "name", name,
                              "mime", mime,
                              "extended", extended,
                              NULL);

        gupnp_dlna_profile_set_container_caps (prof, container_caps);
        gupnp_dlna_profile_set_video_caps (prof, video_caps);
        gupnp_dlna_profile_set_audio_caps (prof, audio_caps);

        return prof;
}

/**
 * gupnp_dlna_profile_get_name:
 * @self: The #GUPnPDLNAProfile object
 *
 * Returns: the name of the DLNA profile represented by @self
 */
const gchar *
gupnp_dlna_profile_get_name (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);
        return priv->name;
}

/**
 * gupnp_dlna_profile_get_mime:
 * @self: The #GUPnPDLNAProfile object
 *
 * Returns: the DLNA MIME type of the DLNA profile represented by @self
 */
const gchar *
gupnp_dlna_profile_get_mime (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);
        return priv->mime;
}

/**
 * gupnp_dlna_profile_get_encoding_profile:
 * @self: The #GUPnPDLNAProfile object
 *
 * Returns: a #GstEncodingProfile object that, in a future version, can be used
 *          to transcode a given stream to match the DLNA profile represented
 *          by @self.
 */
GstEncodingProfile *
gupnp_dlna_profile_get_encoding_profile (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);

        /* check if we have it cached already */
        if (priv->enc_profile) {
                gst_encoding_profile_ref (priv->enc_profile);
        } else {
                /* create an encoding-profile */
                if (GST_IS_CAPS (priv->container_caps)) {
                        priv->enc_profile =
                                (GstEncodingProfile *)gst_encoding_container_profile_new
                                        (priv->name,
                                         priv->mime,
                                         priv->container_caps,
                                         NULL);

                        if (GST_IS_CAPS (priv->video_caps) &&
                            !gst_caps_is_empty (priv->video_caps))
                                gst_encoding_container_profile_add_profile
                                        ((GstEncodingContainerProfile *)priv->enc_profile,
                                         (GstEncodingProfile *)gst_encoding_video_profile_new
                                         (priv->video_caps, NULL, NULL, 0));


                        if (GST_IS_CAPS (priv->audio_caps) &&
                            !gst_caps_is_empty (priv->audio_caps))
                                gst_encoding_container_profile_add_profile
                                        ((GstEncodingContainerProfile *)priv->enc_profile,
                                         (GstEncodingProfile *)gst_encoding_audio_profile_new
                                         (priv->audio_caps, NULL, NULL, 0));

                }
        }

        return (GstEncodingProfile *)priv->enc_profile;
}

/**
 * gupnp_dlna_profile_get_extended:
 * @self: The #GUPnPDLNAProfile object
 *
 * Returns: true if application is using extended mode and false otherwise
 */
gboolean
gupnp_dlna_profile_get_extended (GUPnPDLNAProfile *self)
{
        GUPnPDLNAProfilePrivate *priv = GET_PRIVATE (self);
        return priv->extended;
}
