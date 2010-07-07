/* GStreamer encoding profiles library
 * Copyright (C) 2009 Edward Hervey <edward.hervey@collabora.co.uk>
 *           (C) 2009 Nokia Corporation
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

/**
 * SECTION:gstprofile
 * @short_description: Encoding profile library
 *
 * Since 0.10.XXXX
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstprofile.h"

/* GstEncodingProfile API */

GType
gst_encoding_profile_get_type (void)
{
  static GType gst_encoding_profile_type = 0;

  if (G_UNLIKELY (gst_encoding_profile_type == 0)) {
    gst_encoding_profile_type =
        g_boxed_type_register_static ("GstEncodingProfile",
        (GBoxedCopyFunc) gst_encoding_profile_copy,
        (GBoxedFreeFunc) gst_encoding_profile_free);
  }

  return gst_encoding_profile_type;
}

GType
gst_stream_encoding_profile_get_type (void)
{
  static GType gst_stream_encoding_profile_type = 0;

  if (G_UNLIKELY (gst_stream_encoding_profile_type == 0)) {
    gst_stream_encoding_profile_type =
        g_boxed_type_register_static ("GstStreamEncodingProfile",
        (GBoxedCopyFunc) gst_stream_encoding_profile_copy,
        (GBoxedFreeFunc) gst_stream_encoding_profile_free);
  }

  return gst_stream_encoding_profile_type;
}

GType
gst_video_encoding_profile_get_type (void)
{
  static GType gst_video_encoding_profile_type = 0;

  if (G_UNLIKELY (gst_video_encoding_profile_type == 0)) {
    gst_video_encoding_profile_type =
        g_boxed_type_register_static ("GstVideoEncodingProfile",
        (GBoxedCopyFunc) gst_stream_encoding_profile_copy,
        (GBoxedFreeFunc) gst_stream_encoding_profile_free);
  }

  return gst_video_encoding_profile_type;
}

/**
 * gst_encoding_profile_copy:
 * @profile: a #GstEncodingProfile
 *
 * Creates a copy of the given profile.
 *
 * Returns: a new #GstEncodingProfile
 */
GstEncodingProfile *
gst_encoding_profile_copy (GstEncodingProfile * prof)
{
  GstEncodingProfile *copy;
  GList *tmp;
  GstStreamEncodingProfile *sprof;

  copy = gst_encoding_profile_new (prof->name, prof->format, prof->preset,
      prof->multipass);

  for (tmp = prof->encodingprofiles; tmp; tmp = tmp->next) {
    sprof = (GstStreamEncodingProfile *) tmp->data;
    copy->encodingprofiles = g_list_append (copy->encodingprofiles,
        gst_stream_encoding_profile_copy (sprof));
  }

  return copy;
}

/**
 * gst_encoding_profile_free:
 * @profile: a #GstEncodingProfile
 *
 * Frees the profile and associated streams
 */
void
gst_encoding_profile_free (GstEncodingProfile * prof)
{
  g_return_if_fail (prof != NULL);

  if (prof->name)
    g_free (prof->name);
  if (prof->format)
    gst_caps_unref (prof->format);
  if (prof->preset)
    g_free (prof->preset);

  /* FIXME : Free stream profiles */
  g_list_foreach (prof->encodingprofiles,
      (GFunc) gst_stream_encoding_profile_free, NULL);
  g_list_free (prof->encodingprofiles);

  g_free (prof);
}

/**
 * gst_encoding_profile_new:
 * @name: the name to use for the profile
 * @format: the #GstCaps corresponding to the output container format. Can
 * be #NULL for container-less encoding profiles, in which case only a single
 * #GstStreamEncodingProfile can be added to it.
 * @preset: the #GstPreset to use on the muxer, can be #NULL
 * @multipass: Set to %TRUE if this profile uses multi-pass video encoding.
 *
 * Creates a new #GstEncodingProfile.
 *
 * All provided allocatable arguments will be internally copied, so can be
 * safely freed/unreferenced after calling this method.
 *
 * Returns: the newly created #GstEncodingProfile.
 */
GstEncodingProfile *
gst_encoding_profile_new (gchar * name, GstCaps * format, gchar * preset,
    gboolean multipass)
{
  GstEncodingProfile *prof;

  prof = g_new0 (GstEncodingProfile, 1);

  prof->name = g_strdup (name);
  if (format)
    prof->format = gst_caps_copy (format);
  if (preset)
    prof->preset = g_strdup (preset);
  prof->multipass = multipass;

  return prof;
}

/**
 * gst_encoding_profile_add_stream:
 * @profile: the #GstEncodingProfile to use
 * @stream: the #GstStreamEncodingProfile to add.
 *
 * Add a #GstStreamEncodingProfile to the list of profiles handled by @profile.
 * 
 * No copy of @stream will be made, if you wish to use it elsewhere after this
 * method you should copy it.
 *
 * Returns: %TRUE if the @stream was properly added, else %FALSE.
 */
gboolean
gst_encoding_profile_add_stream (GstEncodingProfile * profile,
    GstStreamEncodingProfile * stream)
{
  if ((profile->format == NULL) && profile->encodingprofiles) {
    GST_ERROR ("Container-less profiles can also have one stream profile");
    return FALSE;
  }
  /* FIXME : Maybe we should have a better way to detect if an exactly 
   * similar profile is already present */
  profile->encodingprofiles = g_list_append (profile->encodingprofiles, stream);

  return TRUE;
}

/**
 * gst_encoding_profile_get_codec_caps:
 * @profile: a #GstEncodingProfile
 *
 * Returns all the caps that can bypass encoding process.
 *
 * This is useful if you wish to connect (uri)decodebin to encodebin and avoid
 * as much decoding/encoding as possible. You can pass those caps to the 'caps'
 * properties of those elements.
 *
 * Returns: The caps that will bypass the encoding process.
 */
GstCaps *
gst_encoding_profile_get_codec_caps (GstEncodingProfile * profile)
{
  GstCaps *res;
  GList *tmp;

  res = gst_caps_new_empty ();

  for (tmp = profile->encodingprofiles; tmp; tmp = g_list_next (tmp)) {
    GstStreamEncodingProfile *sprof = (GstStreamEncodingProfile *) tmp->data;
    gst_caps_append (res, gst_stream_encoding_profile_get_output_caps (sprof));
  }

  return res;
}


/* GstStreamEncodingProfile API */
/**
 * gst_stream_encoding_profile_new:
 * @type: a #GstEncodingProfileType
 * @format: the #GstCaps of the encoding media.
 * @preset: the preset(s) to use on the encoder, can be #NULL
 * @restriction: the #GstCaps used to restrict the input to the encoder, can be
 * NULL.
 * @presence: the number of time this stream must be used. 0 means any number of
 *  times (including never)
 *
 * Creates a new #GstStreamEncodingProfile to be used in a #GstEncodingProfile.
 *
 * All provided allocatable arguments will be internally copied, so can be
 * safely freed/unreferenced after calling this method.
 *
 * Returns: the newly created #GstStreamEncodingProfile.
 */
GstStreamEncodingProfile *
gst_stream_encoding_profile_new (GstEncodingProfileType type,
    GstCaps * format, gchar * preset, GstCaps * restriction, guint presence)
{
  GstStreamEncodingProfile *prof;

  if (type == GST_ENCODING_PROFILE_VIDEO)
    prof = (GstStreamEncodingProfile *) g_new0 (GstVideoEncodingProfile, 1);
  else
    prof = g_new0 (GstStreamEncodingProfile, 1);
  prof->type = type;
  prof->format = gst_caps_copy (format);
  prof->preset = g_strdup (preset);
  if (restriction)
    prof->restriction = gst_caps_copy (restriction);
  else
    prof->restriction = gst_caps_new_any ();
  prof->presence = presence;

  return prof;
}

/**
 * gst_stream_encoding_profile_free:
 * @profile: a #GstStreamEncodingProfile
 *
 * Frees the profile and associated entries.
 */
void
gst_stream_encoding_profile_free (GstStreamEncodingProfile * prof)
{
  g_return_if_fail (prof != NULL);

  gst_caps_unref (prof->format);
  if (prof->preset)
    g_free (prof->preset);
  if (prof->restriction)
    gst_caps_unref (prof->restriction);
  g_free (prof);
}

/**
 * gst_stream_encoding_profile_copy:
 * @profile: a #GstStreamEncodingProfile
 *
 * Copies the stream profile and associated entries.
 *
 * Returns: A copy of the provided profile
 */
GstStreamEncodingProfile *
gst_stream_encoding_profile_copy (GstStreamEncodingProfile * prof)
{
  GstStreamEncodingProfile *copy;

  if (prof->type == GST_ENCODING_PROFILE_VIDEO)
    copy = gst_video_encoding_profile_new (prof->format, prof->preset,
        prof->restriction, prof->presence,
        ((GstVideoEncodingProfile *) prof)->pass);
  else
    copy =
        gst_stream_encoding_profile_new (prof->type, prof->format, prof->preset,
        prof->restriction, prof->presence);

  return copy;
}

/**
 * gst_video_encoding_profile_new:
 * @format: the #GstCaps
 * @preset: the preset(s) to use on the encoder, can be #NULL
 * @restriction: the #GstCaps used to restrict the input to the encoder, can be
 * NULL.
 * @presence: the number of time this stream must be used. 0 means any number of
 *  times (including never)
 * @pass: For which pass this profile should be used. Set to 0 if no multipass
 * encoding is needed.
 *
 * Creates a new #GstStreamEncodingProfile to be used in a #GstEncodingProfile.
 *
 * All provided allocatable arguments will be internally copied, so can be
 * safely freed/unreferenced after calling this method.
 *
 * Returns: the newly created #GstStreamEncodingProfile.
 */
GstStreamEncodingProfile *
gst_video_encoding_profile_new (GstCaps * format, gchar * preset,
    GstCaps * restriction, guint presence, guint pass)
{
  GstStreamEncodingProfile *prof;

  prof =
      gst_stream_encoding_profile_new (GST_ENCODING_PROFILE_VIDEO, format,
      preset, restriction, presence);
  ((GstVideoEncodingProfile *) prof)->pass = pass;

  return prof;
}

/**
 * gst_stream_encoding_profile_get_output_caps:
 * @profile: a #GstStreamEncodingProfile
 *
 * Computes the full output caps that this @profile will generate.
 *
 * Returns: The full output caps for the given @profile. Call gst_caps_unref
 * when you are done with the caps.
 */
GstCaps *
gst_stream_encoding_profile_get_output_caps (GstStreamEncodingProfile * profile)
{
  GstCaps *out, *tmp;
  GstStructure *st, *outst;
  GQuark out_name;
  guint i, len;

  /* fast-path */
  if ((profile->restriction == NULL) || gst_caps_is_any (profile->restriction))
    return gst_caps_copy (profile->format);

  /* Combine the format with the restriction caps */
  outst = gst_caps_get_structure (profile->format, 0);
  out_name = gst_structure_get_name_id (outst);
  tmp = gst_caps_new_empty ();
  len = gst_caps_get_size (profile->restriction);

  for (i = 0; i < len; i++) {
    st = gst_structure_copy (gst_caps_get_structure (profile->restriction, i));
    st->name = out_name;
    gst_caps_append_structure (tmp, st);
  }

  out = gst_caps_intersect (tmp, profile->format);
  gst_caps_unref (tmp);

  return out;
}
