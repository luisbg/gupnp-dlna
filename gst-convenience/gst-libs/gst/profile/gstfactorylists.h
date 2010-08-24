/* GStreamer
 * Copyright (C) <2007> Wim Taymans <wim.taymans@gmail.com>
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


#ifndef __GST_FACTORY_LISTS_H__
#define __GST_FACTORY_LISTS_H__

#include <gst/gst.h>

G_BEGIN_DECLS

/**
 * GstFactoryListType:
 * @GST_FACTORY_LIST_DECODER:
 * @GST_FACTORY_LIST_ENCODER:
 * @GST_FACTORY_LIST_SINK:
 * @GST_FACTORY_LIST_SRC:
 * @GST_FACTORY_LIST_MUXER:
 * @GST_FACTORY_LIST_PARSER:
 *
 * The type of #GstElementFactory to filter.
 */

typedef enum {
  GST_FACTORY_LIST_DECODER      = (1 << 0),
  GST_FACTORY_LIST_ENCODER      = (1 << 1),
  GST_FACTORY_LIST_SINK         = (1 << 2),
  GST_FACTORY_LIST_SRC          = (1 << 3),
  GST_FACTORY_LIST_MUXER        = (1 << 4),
  GST_FACTORY_LIST_DEMUXER      = (1 << 5),
  GST_FACTORY_LIST_PARSER       = (1 << 6),
  GST_FACTORY_LIST_DEPAYLOADER  = (1 << 7),

  GST_FACTORY_LIST_MAX_ELEMENTS = (1 << 16),
  GST_FACTORY_LIST_VIDEO        = (1 << 17),
  GST_FACTORY_LIST_AUDIO        = (1 << 18),
} GstFactoryListType;

#define GST_FACTORY_LIST_VIDEO_ENCODER (GST_FACTORY_LIST_ENCODER | GST_FACTORY_LIST_VIDEO)
#define GST_FACTORY_LIST_AUDIO_ENCODER (GST_FACTORY_LIST_ENCODER | GST_FACTORY_LIST_AUDIO)
#define GST_FACTORY_LIST_AV_SINKS (GST_FACTORY_LIST_SINK | GST_FACTORY_LIST_AUDIO | GST_FACTORY_LIST_VIDEO)

#define GST_FACTORY_LIST_DECODABLE \
  (GST_FACTORY_LIST_DECODER | GST_FACTORY_LIST_DEMUXER | GST_FACTORY_LIST_DEPAYLOADER | GST_FACTORY_LIST_PARSER)

/* Factory list functions */

gboolean      gst_factory_list_is_type      (GstElementFactory *factory, GstFactoryListType type);

GValueArray * gst_factory_list_get_elements (GstFactoryListType type, GstRank minrank);

void          gst_factory_list_debug        (GValueArray *array);

GValueArray * gst_factory_list_filter       (GValueArray *array, const GstCaps *caps,
					     GstPadDirection direction,
					     gboolean subsetonly);

/* Caps functions */

GValueArray *gst_caps_list_compatible_codecs (const GstCaps *containerformat,
					      GValueArray *codecformats,
					      GValueArray *muxers);

GValueArray *gst_caps_list_compatible_containers (GstCaps *mediaformat,
						  GValueArray *containerformats);


GValueArray *gst_caps_list_container_formats (GstRank minrank);

GValueArray *gst_caps_list_video_encoding_formats (GstRank minrank);

GValueArray *gst_caps_list_audio_encoding_formats (GstRank minrank);


#ifndef GST_DISABLE_GST_DEBUG
#define GST_FACTORY_LIST_DEBUG(array) gst_factory_list_debug(array)
#else
#define GST_FACTORY_LIST_DEBUG(array)
#endif

G_END_DECLS

#endif /* __GST_FACTORY_LISTS_H__ */
