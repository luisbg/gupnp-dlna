/*
 * Copyright (C) 2009 Nokia Corporation.
 *
 * Authors: Zeeshan Ali <zeeshanak@gnome.org>
 *                      <zeeshan.ali@nokia.com>
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

#ifndef __GUPNP_DLNA_LOAD_H__
#define __GUPNP_DLNA_LOAD_H__

#include <glib.h>

G_BEGIN_DECLS

GList *
gupnp_dlna_load_profiles_from_file (const gchar *file_name,
                                    GHashTable *restrictions,
                                    GHashTable *profile_ids,
                                    GHashTable *files_hash);

GList *
gupnp_dlna_load_profiles_from_dir (gchar *profile_dir, GHashTable *files_hash);

GList *
gupnp_dlna_load_profiles_from_disk (void);

G_END_DECLS

#endif /* __GUPNP_DLNA_LOAD_H__ */
