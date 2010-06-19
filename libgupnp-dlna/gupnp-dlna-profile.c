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

#include "gupnp-dlna-profile.h"

G_DEFINE_TYPE (GUPnPDLNAProfile, gupnp_dlna_profile, G_TYPE_OBJECT)

static void
gupnp_dlna_profile_finalize (GObject *object)
{
        GUPnPDLNAProfile *self = GUPNP_DLNA_PROFILE (object);

        if (self->name)
                g_free (self->name);
        if (self->mime)
                g_free (self->mime);
        if (self->enc_profile)
                gst_encoding_profile_free (self->enc_profile);

        G_OBJECT_CLASS (gupnp_dlna_profile_parent_class)->finalize (object);
}

static void
gupnp_dlna_profile_class_init (GUPnPDLNAProfileClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gupnp_dlna_profile_finalize;
}

static void
gupnp_dlna_profile_init (GUPnPDLNAProfile *self)
{
        self->name = NULL;
        self->mime = NULL;
        self->enc_profile = NULL;
}

GUPnPDLNAProfile*
gupnp_dlna_profile_new (gchar              *name,
                        gchar              *mime,
                        GstEncodingProfile *enc_profile)
{
        GUPnPDLNAProfile *profile = g_object_new (GUPNP_TYPE_DLNA_PROFILE,
                                                  NULL);

        profile->name = g_strdup (name);
        profile->mime = g_strdup (mime);
        profile->enc_profile = gst_encoding_profile_copy (enc_profile);

        return profile;
}
