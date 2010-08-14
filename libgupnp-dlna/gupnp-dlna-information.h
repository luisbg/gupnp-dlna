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

#ifndef __GUPNP_DLNA_INFORMATION_H__
#define __GUPNP_DLNA_INFORMATION_H__

#include <gst/discoverer/gstdiscoverer.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GUPNP_TYPE_DLNA_INFORMATION gupnp_dlna_information_get_type()

#define GUPNP_DLNA_INFORMATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GUPNP_TYPE_DLNA_INFORMATION, GUPnPDLNAInformation))

#define GUPNP_DLNA_INFORMATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GUPNP_TYPE_DLNA_INFORMATION, GUPnPDLNAInformationClass))

#define GUPNP_IS_DLNA_INFORMATION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GUPNP_TYPE_DLNA_INFORMATION))

#define GUPNP_IS_DLNA_INFORMATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GUPNP_TYPE_DLNA_INFORMATION))

#define GUPNP_DLNA_INFORMATION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GUPNP_TYPE_DLNA_INFORMATION, GUPnPDLNAInformationClass))

typedef struct {
        GObject parent;
} GUPnPDLNAInformation;

typedef struct {
        GObjectClass parent_class;
} GUPnPDLNAInformationClass;

GType gupnp_dlna_information_get_type (void);

GUPnPDLNAInformation*
gupnp_dlna_information_new (gchar                    *name,
                            gchar                    *mime,
                            GstDiscovererInformation *info);

const gchar * gupnp_dlna_information_get_name (GUPnPDLNAInformation *self);
const gchar * gupnp_dlna_information_get_mime (GUPnPDLNAInformation *self);
const GstDiscovererInformation *
gupnp_dlna_information_get_info (GUPnPDLNAInformation *self);

G_GNUC_INTERNAL GUPnPDLNAInformation *
gupnp_dlna_information_new_from_discoverer_info (GstDiscovererInformation *info,
                                                 GList                    *profiles);


G_END_DECLS

#endif /* __GUPNP_DLNA_INFORMATION_H__ */
