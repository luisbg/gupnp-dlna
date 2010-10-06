/* GStreamer
 * Copyright (C) 2009 Edward Hervey <edward.hervey@collabora.co.uk>
 *               2009 Nokia Corporation
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

#ifndef _GST_DISCOVERER_H_
#define _GST_DISCOVERER_H_

#ifndef GST_USE_UNSTABLE_API
#warning "GstDiscovererInternal is unstable API and may change in future."
#warning "You can define GST_USE_UNSTABLE_API to avoid this warning."
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/discoverer/gstdiscoverer-enumtypes.h>

G_BEGIN_DECLS

/**
 * GstStreamType:
 * @GST_STREAM_CONTAINER: Container media stream
 * @GST_STREAM_AUDIO: Audio media stream
 * @GST_STREAM_VIDEO: Video media stream
 * @GST_STREAM_IMAGE: Single-picture media stream
 * @GST_STREAM_UNKNOWN: Unknown media stream
 *
 * The various types of #GstStreamInformation.
 */
typedef enum {
  GST_STREAM_CONTAINER,
  GST_STREAM_AUDIO,
  GST_STREAM_VIDEO,
  GST_STREAM_IMAGE,
  GST_STREAM_UNKNOWN,
} GstStreamType;

typedef struct _GstStreamInformation GstStreamInformation;

/**
 * GstStreamInformation:
 * @streamtype: the type of stream, can be a container or a media type (audio/video/text)
 * @previous: previous #GstStreamInformation in a chain, NULL for starting points
 * @next: next #GstStreamInformation in a chain, NULL for containers
 * @caps: #GstCaps for the stream
 * @tags: #GstTagList containing tags for the stream
 * @misc: #GstStructure with additional information (for example, codec version, profile, etc.)
 *
 * Base structure for informations concerning a media stream. Depending on the @streamtype,
 * One can find more media-specific information in #GstStreamAudioInformation,
 * #GstStreamVideoInformation, #GstStreamContainerInformation.
 */
struct _GstStreamInformation {
  GstStreamType         streamtype;

  GstStreamInformation *previous;  /* NULL for starting points */
  GstStreamInformation *next; /* NULL for containers */

  GstCaps               *caps;
  GstTagList            *tags;
  GstStructure          *misc;
};

GstStreamInformation * gst_stream_information_new (void);
GstStreamInformation *
gst_stream_information_copy (GstStreamInformation * info);
void gst_stream_information_free (GstStreamInformation *topology);
GType gst_stream_information_get_type (void);

#define GST_TYPE_STREAM_INFORMATION (gst_stream_information_get_type ())
#define GST_STREAM_INFORMATION(object) ((GstStreamInformation *)(object))


/**
 * GstStreamContainerInformation:
 * @parent: See #GstStreamInformation for fields
 * @streams: List of #GstStreamInformation objects in this container
 *
 * #GstStreamInformation specific to streams of type #GST_STREAM_CONTAINER.
 */
typedef struct _GstStreamContainerInformation GstStreamContainerInformation;

struct _GstStreamContainerInformation {
  GstStreamInformation parent;

  GList               *streams;
};

GstStreamContainerInformation * gst_stream_container_information_new (void);
GstStreamContainerInformation *
gst_stream_container_information_copy (GstStreamContainerInformation * ptr);
void gst_stream_container_information_free (GstStreamContainerInformation *ptr);
GType gst_stream_container_information_get_type (void);

#define GST_TYPE_STREAM_CONTAINER_INFORMATION       \
  (gst_stream_container_information_get_type ())
#define GST_STREAM_CONTAINER_INFORMATION(object)    \
  ((GstStreamContainerInformation *)(object))


/**
 * GstStreamAudioInformation:
 * @parent: See #GstStreamInformation for fields
 * @channels: Number of channels in the stream
 * @sample_rate: Sampling rate of the stream in Hz
 * @depth: Number of bits used per sample
 * @bitrate: Bitrate of the stream in bits/second
 * @is_vbr: True if the stream has a variable bitrate
 *
 * #GstStreamInformation specific to streams of type #GST_STREAM_AUDIO.
 */
typedef struct _GstStreamAudioInformation GstStreamAudioInformation;

struct _GstStreamAudioInformation {
  GstStreamInformation parent;

  guint channels;
  guint sample_rate;
  guint depth;

  guint bitrate;
  guint max_bitrate;
  gboolean is_vbr;
};

GstStreamAudioInformation * gst_stream_audio_information_new (void);
GstStreamAudioInformation *
gst_stream_audio_information_copy (GstStreamAudioInformation * ptr);
void gst_stream_audio_information_free (GstStreamAudioInformation *ptr);
GType gst_stream_audio_information_get_type (void);

#define GST_TYPE_STREAM_AUDIO_INFORMATION       \
  (gst_stream_audio_information_get_type ())
#define GST_STREAM_AUDIO_INFORMATION(object)    \
  ((GstStreamAudioInformation *)(object))


/**
 * GstStreamVideoInformation:
 * @parent: See #GstStreamInformation for fields
 * @width: Width of the video stream
 * @height: Height of the video stream
 * @depth: Depth in bits of the video stream (only relevant for RGB streams)
 * @frame_rate: Frame rte of the video stream as a fraction
 * @pixel_aspect_ratio: PAR of the video stream as a fraction
 * @format: Colorspace and depth of the stream as a #GstVideoFormat
 * @interlaced: True if the stream is interlaced, false otherwise
 *
 * #GstStreamInformation specific to streams of type #GST_STREAM_VIDEO.
 */
typedef struct _GstStreamVideoInformation GstStreamVideoInformation;

struct _GstStreamVideoInformation {
  GstStreamInformation parent;

  guint width;
  guint height;
  guint depth;
  GValue frame_rate;
  GValue pixel_aspect_ratio;
  GstVideoFormat format;
  gboolean interlaced;
};

GstStreamVideoInformation * gst_stream_video_information_new (void);
GstStreamVideoInformation *
gst_stream_video_information_copy (GstStreamVideoInformation * ptr);
void gst_stream_video_information_free (GstStreamVideoInformation *ptr);
GType gst_stream_video_information_get_type (void);

#define GST_TYPE_STREAM_VIDEO_INFORMATION       \
  (gst_stream_video_information_get_type ())
#define GST_STREAM_VIDEO_INFORMATION(object)    \
  ((GstStreamVideoInformation *)(object))


/**
 * GstDiscovererResult:
 * @GST_DISCOVERER_OK: The discovery was successfull
 * @GST_DISCOVERER_URI_INVALID: the URI is invalid
 * @GST_DISCOVERER_ERROR: an error happend and the GError is set
 * @GST_DISCOVERER_TIMEOUT: the discovery timed-out
 * @GST_DISCOVERER_BUSY: the discoverer was already discovering a file
 * @GST_DISCOVERER_MISSING_PLUGINS: Some plugins are missing for full discovery
 *
 * Result values for the discovery process.
 */
typedef enum
  {
    GST_DISCOVERER_OK               = 0,
    GST_DISCOVERER_URI_INVALID      = (1<<0),
    GST_DISCOVERER_ERROR            = (1<<1),
    GST_DISCOVERER_TIMEOUT          = (1<<2),
    GST_DISCOVERER_BUSY             = (1<<3),
    GST_DISCOVERER_MISSING_PLUGINS  = (1<<4)
  } GstDiscovererResult;


/**
 * GstDiscovererInformation:
 * @uri: The URI for which the information was discovered
 * @result: Result of discovery as a #GstDiscovererResult 
 * @stream_info: #GstStreamInformation struct with information about the stream and its substreams, preserving the original hierarchy
 * @stream_list: #GList of streams for easy iteration
 * @duration: Duration of the stream in nanoseconds
 * @misc: Miscellaneous information stored as a #GstStructure (for example, information about missing plugins)
 *
 * Structure containing the information of a URI analyzed by #GstDiscovererInternal.
 */
typedef struct _GstDiscovererInformation GstDiscovererInformation;

struct _GstDiscovererInformation {
  gchar *uri;
  GstDiscovererResult result;

  /* Sub-streams */
  GstStreamInformation *stream_info;
  GList *stream_list;

  /* Stream global information */
  GstClockTime duration;
  GstStructure *misc;
  GstTagList *tags;
};

GstDiscovererInformation * gst_discoverer_information_new (void);
GstDiscovererInformation *
gst_discoverer_information_copy (GstDiscovererInformation * ptr);
void gst_discoverer_information_free (GstDiscovererInformation *ptr);
GType gst_discoverer_information_get_type (void);

#define GST_TYPE_DISCOVERER_INFORMATION (gst_discoverer_information_get_type ())
#define GST_DISCOVERER_INFORMATION(object) \
  ((GstDiscovererInformation *)(object))


#define GST_TYPE_DISCOVERER_INTERNAL \
  (gst_discoverer_internal_get_type())
#define GST_DISCOVERER_INTERNAL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DISCOVERER_INTERNAL,GstDiscovererInternal))
#define GST_DISCOVERER_INTERNAL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DISCOVERER_INTERNAL,GstDiscovererInternalClass))
#define GST_IS_DISCOVERER_INTERNAL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DISCOVERER_INTERNAL))
#define GST_IS_DISCOVERER_INTERNAL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DISCOVERER_INTERNAL))

typedef struct _GstDiscovererInternal GstDiscovererInternal;
typedef struct _GstDiscovererInternalClass GstDiscovererInternalClass;

/**
 * GstDiscovererInternal:
 *
 * The #GstDiscovererInternal structure.
 **/
struct _GstDiscovererInternal {
  GObject parent;

  /*< private >*/
  gboolean async;

  /* allowed time to discover each uri in nanoseconds */
  GstClockTime timeout;

  /* list of pending URI to process (current excluded) */
  GList *pending_uris;

  GMutex *lock;

  /* TRUE if processing a URI */
  gboolean running;

  /* current items */
  /* FIXME : Protect all this with a lock */
  GstDiscovererInformation *current_info;
  GError *current_error;
  GstStructure *current_topology;

  /* List of private streams */
  GList *streams;

  /* Global elements */
  GstBin *pipeline;
  GstElement *uridecodebin;
  GstBus *bus;

  GType decodebin2_type;
};

struct _GstDiscovererInternalClass {
  GObjectClass parentclass;

  /*< signals >*/
  void        (*ready)           (GstDiscovererInternal *discoverer);
  void        (*starting)        (GstDiscovererInternal *discoverer);
  void        (*discovered)      (GstDiscovererInternal *discoverer,
                                  GstDiscovererInformation *info,
				  GError *err);
};

GType gst_discoverer_internal_get_type (void);
GstDiscovererInternal *gst_discoverer_internal_new (GstClockTime timeout);

/* Asynchronous API */
void gst_discoverer_internal_start (GstDiscovererInternal *discoverer);
void gst_discoverer_internal_stop (GstDiscovererInternal *discoverer);
gboolean gst_discoverer_internal_append_uri (GstDiscovererInternal *discoverer, gchar *uri);


/* Synchronous API */
GstDiscovererInformation *
gst_discoverer_internal_discover_uri (GstDiscovererInternal * discoverer, gchar * uri, GError ** err);

G_END_DECLS

#endif /* _GST_DISCOVERER_H */
