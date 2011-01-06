/*
 * Copyright (C) 2011 Nokia Corporation.
 *
 * Authors: Parthasarathi Susarla <partha.susarla@collabora.co.uk>
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


GUPnPDLNAProfile * gupnp_dlna_profile_new (gchar     *name,
                                           gchar     *mime,
                                           GstCaps   *container_caps,
                                           GstCaps   *video_caps,
                                           GstCaps   *audio_caps,
                                           gboolean  extended);


const GstCaps * gupnp_dlna_profile_get_container_caps (GUPnPDLNAProfile *self);
const GstCaps * gupnp_dlna_profile_get_video_caps (GUPnPDLNAProfile *self);
const GstCaps * gupnp_dlna_profile_get_audio_caps (GUPnPDLNAProfile *self);

void gupnp_dlna_profile_set_container_caps (GUPnPDLNAProfile *self, GstCaps *caps);
void gupnp_dlna_profile_set_video_caps (GUPnPDLNAProfile *self, GstCaps *caps);
void gupnp_dlna_profile_set_audio_caps (GUPnPDLNAProfile *self, GstCaps *caps);
