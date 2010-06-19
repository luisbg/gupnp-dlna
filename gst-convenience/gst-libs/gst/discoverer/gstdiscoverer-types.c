/* GStreamer
 * Copyright (C) 2010 Collabora Multimedia
 *               2010 Nokia Corporation
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

#include "gstdiscoverer.h"
#include "gstdiscoverer-marshal.h"

static GstStreamContainerInformation
    * gst_stream_container_info_copy_int (GstStreamContainerInformation * ptr,
    GHashTable * stream_map);

/* Per-stream information */

/**
 * gst_stream_information_new:
 *
 * Returns a newly allocated #GstStreamInformation object. All fields are
 * initialised to zero/NULL, so callers have to allocate member structures.
 * However, these structures will automatically be freed if allocated in
 * #gst_stream_information_free().
 *
 * Returns: a newly allocated #GstStreamInformation.
 */
GstStreamInformation *
gst_stream_information_new (void)
{
  GstStreamInformation *info = g_new0 (GstStreamInformation, 1);
  info->streamtype = GST_STREAM_UNKNOWN;
  return info;
}

/* Does everything except freeing info */
static void
gst_stream_information_deinit (GstStreamInformation * info)
{
  if (info->next)
    gst_stream_information_free (info->next);

  if (info->caps)
    gst_caps_unref (info->caps);

  if (info->tags)
    gst_tag_list_free (info->tags);

  if (info->misc)
    gst_structure_free (info->misc);
}

static GstStreamInformation *
gst_stream_information_copy_int (GstStreamInformation * info,
    GHashTable * stream_map)
{
  GstStreamInformation *ret = NULL;
  g_return_val_if_fail (info != NULL, NULL);

  switch (info->streamtype) {
    case GST_STREAM_CONTAINER:
      ret = (GstStreamInformation *)
          gst_stream_container_info_copy_int (
          (GstStreamContainerInformation *) info, stream_map);
      break;

    case GST_STREAM_AUDIO:
      ret = (GstStreamInformation *)
          gst_stream_audio_information_copy (
          (GstStreamAudioInformation *) info);
      break;

    case GST_STREAM_VIDEO:
    case GST_STREAM_IMAGE:
      ret = (GstStreamInformation *)
          gst_stream_video_information_copy (
          (GstStreamVideoInformation *) info);
      break;

    default:
      ret = gst_stream_information_new ();
      break;
  }

  if (info->next) {
    ret->next = gst_stream_information_copy_int (info->next, stream_map);
    ret->next->previous = ret;
  }

  if (info->caps)
    ret->caps = gst_caps_copy (info->caps);

  if (info->tags)
    ret->tags = gst_tag_list_copy (info->tags);

  if (info->misc)
    ret->misc = gst_structure_copy (info->misc);

  if (stream_map)
    g_hash_table_insert (stream_map, info, ret);

  return ret;
}

/**
 * gst_stream_information_copy:
 * @info: the #GstStreamInformation to be copied
 *
 * Returns a deep copy of @info.
 *
 * Returns: a newly allocated #GstStreamInformation.
 */
GstStreamInformation *
gst_stream_information_copy (GstStreamInformation * info)
{
  return gst_stream_information_copy_int (info, NULL);
}

/**
 * gst_stream_information_free:
 * @info: a #GstStreamInformation
 *
 * Frees @info, along with any associated data structures. For derived types,
 * this also calls the appropriate free function.
 */
void
gst_stream_information_free (GstStreamInformation * info)
{
  g_return_if_fail (info != NULL);

  switch (info->streamtype) {
    case GST_STREAM_CONTAINER:
      gst_stream_container_information_free (
          (GstStreamContainerInformation *) info);
      break;

    case GST_STREAM_AUDIO:
      gst_stream_audio_information_free ((GstStreamAudioInformation *) info);
      break;

    case GST_STREAM_VIDEO:
    case GST_STREAM_IMAGE:
      gst_stream_video_information_free ((GstStreamVideoInformation *) info);
      break;

    default:
      gst_stream_information_deinit (info);
      g_free (info);
      break;
  }
}

GType
gst_stream_information_get_type (void)
{
  static GType gst_stream_information_type = 0;

  if (G_UNLIKELY (gst_stream_information_type == 0)) {
    gst_stream_information_type =
        g_boxed_type_register_static ("GstStreamInformation",
        (GBoxedCopyFunc) gst_stream_information_copy,
        (GBoxedFreeFunc) gst_stream_information_free);
  }

  return gst_stream_information_type;
}


/* Container information */

GstStreamContainerInformation *
gst_stream_container_information_new (void)
{
  GstStreamContainerInformation *info =
      g_new0 (GstStreamContainerInformation, 1);
  info->parent.streamtype = GST_STREAM_CONTAINER;
  return info;
}

static GstStreamContainerInformation *
gst_stream_container_info_copy_int (GstStreamContainerInformation * ptr,
    GHashTable * stream_map)
{
  GstStreamContainerInformation *ret;
  GList *tmp;

  g_return_val_if_fail (ptr != NULL, NULL);

  ret = gst_stream_container_information_new ();

  for (tmp = ((GstStreamContainerInformation *) ptr)->streams; tmp;
      tmp = tmp->next) {
    GstStreamInformation *subtop =
        gst_stream_information_copy ((GstStreamInformation *) tmp->data);
    ret->streams = g_list_append (ret->streams, subtop);
    if (stream_map)
      g_hash_table_insert (stream_map, tmp->data, subtop);
  }

  return ret;
}

GstStreamContainerInformation *
gst_stream_container_information_copy (GstStreamContainerInformation * ptr)
{
  return gst_stream_container_info_copy_int (ptr, NULL);
}

void
gst_stream_container_information_free (GstStreamContainerInformation * ptr)
{
  GList *tmp;

  g_return_if_fail (ptr != NULL);

  gst_stream_information_deinit (&ptr->parent);

  for (tmp = ((GstStreamContainerInformation *) ptr)->streams; tmp;
      tmp = tmp->next) {
    GstStreamInformation *subtop = (GstStreamInformation *) tmp->data;
    gst_stream_information_free (subtop);
  }

  g_list_free (ptr->streams);
  g_free (ptr);
}

GType
gst_stream_container_information_get_type (void)
{
  static GType gst_stream_container_information_type = 0;

  if (G_UNLIKELY (gst_stream_container_information_type == 0)) {
    gst_stream_container_information_type =
        g_boxed_type_register_static ("GstStreamContainerInformation",
        (GBoxedCopyFunc) gst_stream_container_information_copy,
        (GBoxedFreeFunc) gst_stream_container_information_free);
  }

  return gst_stream_container_information_type;
}


/* Audio information */

GstStreamAudioInformation *
gst_stream_audio_information_new (void)
{
  GstStreamAudioInformation *info = g_new0 (GstStreamAudioInformation, 1);
  info->parent.streamtype = GST_STREAM_AUDIO;
  return info;
}

GstStreamAudioInformation *
gst_stream_audio_information_copy (GstStreamAudioInformation * ptr)
{
  GstStreamAudioInformation *ret;

  g_return_val_if_fail (ptr != NULL, NULL);

  ret = gst_stream_audio_information_new ();

  ret->channels = ptr->channels;
  ret->sample_rate = ptr->sample_rate;
  ret->depth = ptr->depth;
  ret->bitrate = ptr->bitrate;
  ret->max_bitrate = ptr->max_bitrate;
  ret->is_vbr = ptr->is_vbr;

  return ret;
}

void
gst_stream_audio_information_free (GstStreamAudioInformation * ptr)
{
  g_return_if_fail (ptr != NULL);
  gst_stream_information_deinit (&ptr->parent);
  g_free (ptr);
}

GType
gst_stream_audio_information_get_type (void)
{
  static GType gst_stream_audio_information_type = 0;

  if (G_UNLIKELY (gst_stream_audio_information_type == 0)) {
    gst_stream_audio_information_type =
        g_boxed_type_register_static ("GstStreamAudioInformation",
        (GBoxedCopyFunc) gst_stream_audio_information_copy,
        (GBoxedFreeFunc) gst_stream_audio_information_free);
  }

  return gst_stream_audio_information_type;
}


/* Video information */

GstStreamVideoInformation *
gst_stream_video_information_new (void)
{
  GstStreamVideoInformation *info = g_new0 (GstStreamVideoInformation, 1);

  info->parent.streamtype = GST_STREAM_VIDEO;
  g_value_init (&info->frame_rate, GST_TYPE_FRACTION);
  g_value_init (&info->pixel_aspect_ratio, GST_TYPE_FRACTION);

  return info;
}

GstStreamVideoInformation *
gst_stream_video_information_copy (GstStreamVideoInformation * ptr)
{
  GstStreamVideoInformation *ret;

  g_return_val_if_fail (ptr != NULL, NULL);

  ret = gst_stream_video_information_new ();

  /* Because the streamtype can also be GST_STREAM_IMAGE */
  GST_STREAM_INFORMATION (ret)->streamtype =
      GST_STREAM_INFORMATION (ptr)->streamtype;
  ret->width = ptr->width;
  ret->height = ptr->height;
  ret->depth = ptr->depth;
  g_value_copy (&ptr->frame_rate, &ret->frame_rate);
  g_value_copy (&ptr->pixel_aspect_ratio, &ret->pixel_aspect_ratio);
  ret->format = ptr->format;
  ret->interlaced = ptr->interlaced;

  return ret;
}

void
gst_stream_video_information_free (GstStreamVideoInformation * ptr)
{
  g_return_if_fail (ptr != NULL);
  gst_stream_information_deinit (&ptr->parent);
  g_free (ptr);
}

GType
gst_stream_video_information_get_type (void)
{
  static GType gst_stream_video_information_type = 0;

  if (G_UNLIKELY (gst_stream_video_information_type == 0)) {
    gst_stream_video_information_type =
        g_boxed_type_register_static ("GstStreamVideoInformation",
        (GBoxedCopyFunc) gst_stream_video_information_copy,
        (GBoxedFreeFunc) gst_stream_video_information_free);
  }

  return gst_stream_video_information_type;
}


/* Global stream information */

GstDiscovererInformation *
gst_discoverer_information_new (void)
{
  GstDiscovererInformation *di = g_new0 (GstDiscovererInformation, 1);
  return di;
}

GstDiscovererInformation *
gst_discoverer_information_copy (GstDiscovererInformation * ptr)
{
  GstDiscovererInformation *ret;
  GHashTable *stream_map = g_hash_table_new (g_direct_hash, NULL);
  GList *tmp;

  g_return_val_if_fail (ptr != NULL, NULL);

  ret = gst_discoverer_information_new ();

  ret->uri = g_strdup (ptr->uri);
  if (ptr->stream_info) {
    ret->stream_info = gst_stream_information_copy_int (ptr->stream_info,
        stream_map);
  }
  ret->duration = ptr->duration;
  if (ptr->misc)
    ret->misc = gst_structure_copy (ptr->misc);

  /* We need to set up the new list of streams to correspond to old one. The
   * list just contains a set of pointers to streams in the stream_info tree,
   * so we keep a map of old stream info objects to the corresponding new
   * ones and use that to figure out correspondence in stream_list. */
  for (tmp = ptr->stream_list; tmp; tmp = tmp->next) {
    GstStreamInformation *old_stream = (GstStreamInformation *) tmp->data;
    GstStreamInformation *new_stream = g_hash_table_lookup (stream_map,
        old_stream);
    g_assert (new_stream != NULL);
    ret->stream_list = g_list_append (ret->stream_list, new_stream);
  }

  g_hash_table_destroy (stream_map);
  return ret;
}

void
gst_discoverer_information_free (GstDiscovererInformation * ptr)
{
  g_return_if_fail (ptr != NULL);

  g_free (ptr->uri);

  if (ptr->stream_info)
    gst_stream_information_free (ptr->stream_info);

  if (ptr->misc)
    gst_structure_free (ptr->misc);

  g_list_free (ptr->stream_list);

  g_free (ptr);
}

GType
gst_discoverer_information_get_type (void)
{
  static GType gst_discoverer_information_type = 0;

  if (G_UNLIKELY (gst_discoverer_information_type == 0)) {
    gst_discoverer_information_type =
        g_boxed_type_register_static ("GstDiscovererInformation",
        (GBoxedCopyFunc) gst_discoverer_information_copy,
        (GBoxedFreeFunc) gst_discoverer_information_free);
  }

  return gst_discoverer_information_type;
}
