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

#ifndef __GUPNP_DLNA_PROFILE_H__
#define __GUPNP_DLNA_PROFILE_H__

#include <gst/profile/gstprofile.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GUPNP_TYPE_DLNA_PROFILE gupnp_dlna_profile_get_type()

#define GUPNP_DLNA_PROFILE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                     GUPNP_TYPE_DLNA_PROFILE, \
                                     GUPnPDLNAProfile))

#define GUPNP_DLNA_PROFILE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                  GUPNP_TYPE_DLNA_PROFILE, \
                                  GUPnPDLNAProfileClass))

#define GUPNP_IS_DLNA_PROFILE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GUPNP_TYPE_DLNA_PROFILE))

#define GUPNP_IS_DLNA_PROFILE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GUPNP_TYPE_DLNA_PROFILE))

#define GUPNP_DLNA_PROFILE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                    GUPNP_TYPE_DLNA_PROFILE, \
                                    GUPnPDLNAProfileClass))

typedef struct {
  GObject            parent;
} GUPnPDLNAProfile;

typedef struct {
  GObjectClass parent_class;
} GUPnPDLNAProfileClass;

GType gupnp_dlna_profile_get_type (void);

GUPnPDLNAProfile* gupnp_dlna_profile_new (gchar              *name,
                                          gchar              *mime,
                                          GstEncodingProfile *enc_profile);

const gchar * gupnp_dlna_profile_get_name (GUPnPDLNAProfile *self);
const gchar * gupnp_dlna_profile_get_mime (GUPnPDLNAProfile *self);
const GstEncodingProfile *
gupnp_dlna_profile_get_encoding_profile (GUPnPDLNAProfile *self);

G_END_DECLS

#endif /* __GUPNP_DLNA_PROFILE_H__ */
