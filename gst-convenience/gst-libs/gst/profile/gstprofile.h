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

#ifndef __GST_PROFILE_H__
#define __GST_PROFILE_H__

#ifndef GST_USE_UNSTABLE_API
#warning "GstProfile is unstable API and may change in future."
#warning "You can define GST_USE_UNSTABLE_API to avoid this warning."
#endif

#include <gst/gst.h>

G_BEGIN_DECLS

#include <gst/profile/profile-enumtypes.h>

#define GST_TYPE_ENCODING_PROFILE               (gst_encoding_profile_get_type())
#define GST_ENCODING_PROFILE(object)            ((GstEncodingProfile*)object)

#define GST_TYPE_STREAM_ENCODING_PROFILE        (gst_stream_encoding_profile_get_type())
#define GST_STREAM_ENCODING_PROFILE(object)     ((GstStreamEncodingProfile*)object)

#define GST_TYPE_VIDEO_ENCODING_PROFILE        (gst_video_encoding_profile_get_type())
#define GST_VIDEO_ENCODING_PROFILE(object)     ((GstVideoEncodingProfile*)object)

/**
 * GstEncodingProfileType:
 * @GST_ENCODING_PROFILE_UNKNOWN: unknown stream
 * @GST_ENCODING_PROFILE_VIDEO: video stream
 * @GST_ENCODING_PROFILE_AUDIO: audio stream
 * @GST_ENCODING_PROFILE_TEXT: text/subtitle stream
 * @GST_ENCODING_PROFILE_IMAGE: image
 *
 * Type of stream for a #GstStreamEncodingProfile
 **/
typedef enum {
  GST_ENCODING_PROFILE_UNKNOWN,
  GST_ENCODING_PROFILE_VIDEO,
  GST_ENCODING_PROFILE_AUDIO,
  GST_ENCODING_PROFILE_TEXT,
  GST_ENCODING_PROFILE_IMAGE,
  /* Room for extenstion */
} GstEncodingProfileType;

typedef struct _GstEncodingTarget GstEncodingTarget;
typedef struct _GstEncodingProfile GstEncodingProfile;
typedef struct _GstStreamEncodingProfile GstStreamEncodingProfile;
typedef struct _GstVideoEncodingProfile GstVideoEncodingProfile;

/* FIXME/UNKNOWNS
 *
 * Should encoding categories be well-known strings/quarks ?
 *
 */

/**
 * GstEncodingTarget:
 * @name: The name of the target profile.
 * @category: The target category (device, service, use-case).
 * @profiles: A list of #GstProfile this device supports.
 *
 */
struct _GstEncodingTarget {
  gchar     *name;
  gchar     *category;
  GList     *profiles;
};

/**
 * GstEncodingProfile:
 * @name: The name of the profile
 * @format: The #GstCaps corresponding to the muxing format.
 * @preset: The name of the #GstPreset(s) to be used on the muxer. This is optional.
 * @multipass: Whether this profile is a multi-pass profile or not.
 * @encodingprofiles: A list of #GstStreamEncodingProfile for the various streams.
 *
 * Profile for a specific combination of format/stream encoding.
 */

struct _GstEncodingProfile {
  gchar	        *name;
  GstCaps       *format;
  gchar         *preset;
  gboolean       multipass;
  GList         *encodingprofiles;
};

/**
 * GstStreamEncodingProfile:
 * @type: Type of profile
 * @format: The GStreamer mime type corresponding to the encoding format.
 * @preset: The name of the #GstPreset to be used on the encoder. This is optional.
 * @restriction: The #GstCaps restricting the input. This is optional.
 * @presence: The number of streams that can be created. 0 => any.
 *
 * Invididual stream profile, to be used in #GstEncodingProfile
 */
struct _GstStreamEncodingProfile {
  GstEncodingProfileType   type;
  GstCaps                 *format;
  gchar                   *preset;
  GstCaps                 *restriction;
  guint                    presence;
};

/**
 * GstVideoEncodingProfile:
 * @profile: common #GstEncodingProfile part.
 * @pass: The pass number if this is part of a multi-pass profile. Starts at 1
 * for multi-pass. Set to 0 if this is not part of a multi-pass profile.
 * @vfr: Variable Frame Rate allowed.
 * If %FALSE (default), the incoming video frames will be duplicated/dropped in
 * order to produce an exact constant framerate.
 * If %TRUE, then the incoming video stream will not be modified.
 *
 * Variant of #GstStreamEncodingProfile for video streams, allows specifying the @pass.
 */
struct _GstVideoEncodingProfile {
  GstStreamEncodingProfile      profile;
  guint                         pass;
  gboolean			vfr;
};

/* GstEncodingProfile API */
GType                 gst_encoding_profile_get_type (void);
GstEncodingProfile *  gst_encoding_profile_new (gchar *name, GstCaps *format,
						gchar *preset,
						gboolean multipass);
GstEncodingProfile *  gst_encoding_profile_copy (GstEncodingProfile *profile);

void                  gst_encoding_profile_free (GstEncodingProfile *profile);

gboolean              gst_encoding_profile_add_stream (GstEncodingProfile *profile,
						       GstStreamEncodingProfile *stream);
GstCaps * gst_encoding_profile_get_codec_caps (GstEncodingProfile *profile);


/* GstStreamEncodingProfile API */
GType                      gst_stream_encoding_profile_get_type (void);
GstStreamEncodingProfile * gst_stream_encoding_profile_new (GstEncodingProfileType type,
							    GstCaps *format,
							    gchar *preset,
							    GstCaps *restriction,
							    guint presence);

GType                      gst_video_encoding_profile_get_type (void);
GstStreamEncodingProfile * gst_video_encoding_profile_new (GstCaps *format,
							   gchar *preset,
							   GstCaps *restriction,
							   guint presence,
							   guint pass);

GstStreamEncodingProfile * gst_stream_encoding_profile_copy (GstStreamEncodingProfile *profile);

void                       gst_stream_encoding_profile_free (GstStreamEncodingProfile *profile);

GstCaps * gst_stream_encoding_profile_get_output_caps (GstStreamEncodingProfile *profile);

/* Generic helper API */
GList *gst_encoding_category_list_target (gchar *category);

GList *gst_profile_list_target_categories (void);

gboolean gst_profile_target_save (GstEncodingTarget *target);


/*
 * Application convenience methods (possibly to be added in gst-pb-utils)
 */

GstElement *gst_pb_utils_create_encoder(GstCaps *caps, gchar *preset, gchar *name);
GstElement *gst_pb_utils_create_encoder_format(gchar *format, gchar *preset,
					       gchar *name);

GstElement *gst_pb_utils_create_muxer(GstCaps *caps, gchar *preset);
GstElement *gst_pb_utils_create_muxer_format(gchar *format, gchar *preset,
					       gchar *name);

GList *gst_pb_utils_encoders_compatible_with_muxer(GstElement *muxer);

GList *gst_pb_utils_muxers_compatible_with_encoder(GstElement *encoder);


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
gboolean gst_preset_create (GstPreset *preset, gchar *name,
			    GstStructure *properties);

/**
 * gst_preset_reset:
 * @preset: a #GstPreset
 *
 * Sets all the properties of the element back to their default values.
 */
/* FIXME : This could actually be put at the GstObject level, or maybe even
 * at the GObject level. */
void gst_preset_reset (GstPreset *preset);

G_END_DECLS

#endif /* __GST_PROFILE_H__ */
