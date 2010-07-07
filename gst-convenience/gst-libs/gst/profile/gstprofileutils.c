/* GStreamer encoding profiles library utilities
 * Copyright (C) 2010 Edward Hervey <edward.hervey@collabora.co.uk>
 *           (C) 2010 Nokia Corporation
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstprofile.h"

/**
 * gst_encoding_category_list_target:
 * @category: a profile target category name. Can be NULL.
 *
 * NOT IMPLEMENTED
 *
 * Returns: the list of all available #GstEncodingTarget for the given @category.
 * If @category is #NULL, then all available #GstEncodingTarget are returned.
 */

GList *
gst_encoding_category_list_target (gchar * category)
{
  /* FIXME : IMPLEMENT */
  return NULL;
}

/**
 * gst_profile_list_target_categories:
 *
 * list available profile target categories
 *
 * NOT IMPLEMENTED
 */
GList *
gst_profile_list_target_categories (void)
{
  /* FIXME : IMPLEMENT */
  return NULL;
}

/**
 * gst_profile_target_save:
 *
 * NOT IMPLEMENTED
 */
gboolean
gst_profile_target_save (GstEncodingTarget * target)
{
  /* FIXME : IMPLEMENT */
  return FALSE;
}

/**
 * gst_pb_utils_create_encoder:
 * @caps: The #GstCaps corresponding to a codec format
 * @preset: The name of a preset, can be #NULL
 * @name: The name to give to the returned instance, can be #NULL.
 *
 * Creates an encoder which can output the given @caps. If several encoders can
 * output the given @caps, then the one with the highest rank will be picked.
 * If a @preset is specified, it will be applied to the created encoder before
 * returning it.
 * If a @preset is specified, then the highest-ranked encoder that can accept
 * the givein preset will be returned.
 *
 * Returns: The encoder instance with the preset applied if it is available.
 * #NULL if no encoder is available.
 */
GstElement *
gst_pb_utils_create_encoder (GstCaps * caps, gchar * preset, gchar * name)
{
  /* FIXME : IMPLEMENT */
  return NULL;
}


/**
 * gst_pb_utils_create_encoder_format:
 *
 * Convenience version of @gst_pb_utils_create_encoder except one does not need
 * to create a #GstCaps.
 */

GstElement *
gst_pb_utils_create_encoder_format (gchar * format, gchar * preset,
    gchar * name)
{
  /* FIXME : IMPLEMENT */
  return NULL;
}

/**
 * gst_pb_utils_create_muxer:
 * @caps: The #GstCaps corresponding to a codec format
 * @preset: The name of a preset, can be NULL
 *
 * Creates an muxer which can output the given @caps. If several muxers can
 * output the given @caps, then the one with the highest rank will be picked.
 * If a @preset is specified, it will be applied to the created muxer before
 * returning it.
 * If a @preset is specified, then the highest-ranked muxer that can accept
 * the givein preset will be returned.
 *
 * Returns: The muxer instance with the preset applied if it is available.
 * #NULL if no muxer is available.
 */
GstElement *
gst_pb_utils_create_muxer (GstCaps * caps, gchar * preset)
{
  /* FIXME : IMPLEMENT */
  return NULL;
}


/**
 * gst_pb_utils_create_muxer_format:
 *
 * Convenience version of @gst_pb_utils_create_muxer except one does not need
 * to create a #GstCaps.
 */
GstElement *
gst_pb_utils_create_muxer_format (gchar * format, gchar * preset, gchar * name)
{
  /* FIXME : IMPLEMENT */
  return NULL;
}




/*
 * GstPreset modifications
 */

/**
 * gst_preset_create:
 * @preset: The #GstPreset on which to create the preset
 * @name: A name for the preset
 * @properties: The properties
 *
 * Creates a new preset with the given properties. This preset will only
 * exist during the lifetime of the process.
 * If you wish to use it after the lifetime of the process, you must call
 * @gst_preset_save_preset.
 *
 * Returns: #TRUE if the preset could be created, else #FALSE.
 */
gboolean
gst_preset_create (GstPreset * preset, gchar * name, GstStructure * properties)
{
  /* FIXME : IMPLEMENT */
  return FALSE;
}

/**
 * gst_preset_reset:
 * @preset: a #GstPreset
 *
 * Sets all the properties of the element back to their default values.
 */
/* FIXME : This could actually be put at the GstObject level, or maybe even
 * at the GObject level. */
void
gst_preset_reset (GstPreset * preset)
{

}
