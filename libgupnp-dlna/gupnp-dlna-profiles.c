/*
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Authors: Arun Raghavan <arun.raghavan@collabora.co.uk>
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

#include <glib.h>
#include <gst/pbutils/pbutils.h>
#include "gupnp-dlna-discoverer.h"
#include "gupnp-dlna-profile.h"

/*
 * This file provides the infrastructure to load DLNA profiles and the
 * corresponding restrictions from an on-disk representation, and use them to
 * map a given stream to its DLNA profile, if possible.
 *
 * Each DLNA profile is represented as a GstEncodingProfile (there might be
 * exceptions where a single DLNA profile is represented by multiple
 * GstEncodingProfiles - right now that's only LPCM).
 *
 * For a GstEncodingProfile "profile", the following fields will be populated:
 *
 *   profile.name = "<DLNA Profile Name>"
 *   profile.format = Muxing format caps (with restrictions) if specified,
 *                    else GST_CAPS_NONE
 *   profile.encodingprofiles = GList of GstStreamEncodingProfiles
 *
 * For each stream of the given profile, "profile.encodingprofiles" will have
 * a GstEncodingProfile representing the restrictions for that stream (for a
 * video format there will be one audio and one video stream, for example).
 *
 *   enc_profile.type = GST_ENCODING_PROFILE_{AUDIO,VIDEO,...} (UNKNOWN for
 *                      container restrictions)
 *   enc_profile.format = GstCaps with all the restrictions for this format
 *   enc_profile.restriction = GST_CAPS_ANY
 *
 * We assume that all DLNA profiles have exactly one audio stream, or one audio
 * stream and one video stream.
 *
 * Things yet to account for:
 *
 *   1. Multiple audio/video streams (we need to pick the "main" one - how?
 *      Possibly get information from the demuxer.)
 *
 *   2. How do we handle discovered metadata which is in tags, but not in caps?
 *      Could potentially move it to caps in a post-discovery, pre-guessing
 *      phase
 */

/* New profile guessing API */

#define GUPNP_DLNA_DEBUG_ENV "GUPNP_DLNA_DEBUG"

#define gupnp_dlna_debug(args...)                               \
do {                                                            \
        const gchar *_e = g_getenv (GUPNP_DLNA_DEBUG_ENV);      \
        if (_e && !g_str_equal (_e, "0"))                       \
                g_debug (args);                                 \
} while (0)

static gboolean
is_video_profile (const GstEncodingProfile *profile)
{
        const GList *i;

        if (GST_IS_ENCODING_CONTAINER_PROFILE (profile)) {
                for (i = gst_encoding_container_profile_get_profiles
                             (GST_ENCODING_CONTAINER_PROFILE (profile));
                     i;
                     i = i->next)
                        if (GST_IS_ENCODING_VIDEO_PROFILE (i->data))
                                return TRUE;
        }

        return FALSE;
}

static gboolean
structure_can_intersect (const GstStructure *st1, const GstStructure *st2)
{
        /* Since there is no API to intersect GstStructures, we cheat (thanks
         * for the idea, tpm!) and make caps from the structuresa */

        GstCaps *caps1, *caps2;
        gboolean ret;

        caps1 = gst_caps_new_full (gst_structure_copy (st1), NULL);
        caps2 = gst_caps_new_full (gst_structure_copy (st2), NULL);

        ret = gst_caps_can_intersect (caps1, caps2);

        gst_caps_unref (caps1);
        gst_caps_unref (caps2);

        return ret;
}

static gboolean
structure_is_subset (const GstStructure *st1, const GstStructure *st2)
{
        int i;

        for (i = 0; i < gst_structure_n_fields (st2); i++) {
                const gchar *name = gst_structure_nth_field_name (st2, i);

                if (!gst_structure_has_field(st1, name)) {
                        gupnp_dlna_debug ("    missing field %s", name);
                        return FALSE;
                }
        }

        return TRUE;
}

/*
 * Returns TRUE if stream_caps and profile_caps can intersect, and the
 * intersecting structure from profile_caps is a subset of stream_caps. Put
 * simply, the condition being met is that stream_caps intersects with
 * profile_caps, and that intersection includes *all* fields specified by
 * profile_caps (viz. all the fields specified by the DLNA profile's
 * restrictions)
 */
static gboolean
caps_can_intersect_and_is_subset (GstCaps *stream_caps, const GstCaps *profile_caps)
{
        int i;
        GstStructure *stream_st, *profile_st;

        stream_st = gst_caps_get_structure (stream_caps, 0);

        for (i = 0; i < gst_caps_get_size (profile_caps); i++) {
                profile_st = gst_caps_get_structure (profile_caps, i);

                if (structure_can_intersect (stream_st, profile_st) &&
                    structure_is_subset (stream_st, profile_st))
                        return TRUE;
        }

        return FALSE;
}

static gboolean
match_profile (GstEncodingProfile *profile,
               GstCaps            *caps,
               GType              type)
{
        const GList *i;
        const gchar *name;

        /* Profiles with an empty name are used only for inheritance and should
         * not be matched against. */
        name = gst_encoding_profile_get_name (profile);
        if (name[0] == '\0')
                return FALSE;

        for (i = gst_encoding_container_profile_get_profiles
                                        (GST_ENCODING_CONTAINER_PROFILE (profile));
             i;
             i = i->next){
                GstEncodingProfile *enc_profile = GST_ENCODING_PROFILE
                                        (i->data);
                const GstCaps *format = gst_encoding_profile_get_format
                                        (enc_profile);

                if (type == G_TYPE_FROM_INSTANCE (enc_profile) &&
                    caps_can_intersect_and_is_subset (caps, format))
                        return TRUE;
        }

        return FALSE;
}

static gboolean
check_container (GstDiscovererInfo  *info,
                 GstEncodingProfile *profile)
{
        GstDiscovererStreamInfo *stream_info;
        GType stream_type;
        GstCaps *stream_caps;
        gboolean ret = FALSE;

        const GstCaps *profile_caps = gst_encoding_profile_get_format (profile);

        /* Top-level GstStreamInformation in the topology will be
         * the container */
        stream_info = gst_discoverer_info_get_stream_info (info);
        stream_caps = gst_discoverer_stream_info_get_caps (stream_info);
        stream_type = G_TYPE_FROM_INSTANCE (stream_info);

        if (stream_type == GST_TYPE_DISCOVERER_CONTAINER_INFO &&
            gst_caps_can_intersect (stream_caps, profile_caps))
                ret = TRUE;
        else if (stream_type != GST_TYPE_DISCOVERER_CONTAINER_INFO &&
                 gst_caps_is_empty (profile_caps))
                ret = TRUE;

        gst_discoverer_stream_info_unref (stream_info);
        gst_caps_unref (stream_caps);

        return ret;
}

static GstCaps *
caps_from_audio_stream_info (GstDiscovererStreamInfo *info)
{
        GstCaps *temp = gst_discoverer_stream_info_get_caps (info);
        GstCaps *caps = gst_caps_copy (temp);
        const GstDiscovererAudioInfo *audio_info =
                GST_DISCOVERER_AUDIO_INFO(info);
        guint data;

        gst_caps_unref (temp);

        data = gst_discoverer_audio_info_get_sample_rate (audio_info);
        if (data)
                gst_caps_set_simple (caps, "rate", G_TYPE_INT, data, NULL);

        data = gst_discoverer_audio_info_get_channels (audio_info);
        if (data)
                gst_caps_set_simple (caps, "channels", G_TYPE_INT, data, NULL);

        data = gst_discoverer_audio_info_get_bitrate (audio_info);
        if (data)
                gst_caps_set_simple (caps, "bitrate", G_TYPE_INT, data, NULL);

        data = gst_discoverer_audio_info_get_max_bitrate (audio_info);
        if (data)
                gst_caps_set_simple
                        (caps, "maximum-bitrate", G_TYPE_INT, data, NULL);

        data = gst_discoverer_audio_info_get_depth (audio_info);
        if (data)
                gst_caps_set_simple (caps, "depth", G_TYPE_INT, data, NULL);

        return caps;
}

static gboolean
check_audio_profile (GstDiscovererInfo  *info,
                     GstEncodingProfile *profile)
{
        GstCaps *caps;
        GList *i, *stream_list;
        gboolean found = FALSE;

        /* Optimisation TODO: this can be pre-computed */
        if (is_video_profile (profile))
                return FALSE;

        stream_list = gst_discoverer_info_get_stream_list (info);

        for (i = stream_list; !found && i; i = i->next) {
                GstDiscovererStreamInfo *stream =
                        GST_DISCOVERER_STREAM_INFO(i->data);
                GType stream_type = G_TYPE_FROM_INSTANCE (stream);

                if (stream_type != GST_TYPE_DISCOVERER_AUDIO_INFO)
                        continue;

                caps = caps_from_audio_stream_info (stream);

                if (match_profile (profile,
                                   caps,
                                   GST_TYPE_ENCODING_AUDIO_PROFILE)) {
                        found = TRUE;
                        break;
                }

                gst_caps_unref (caps);
        }

        gst_discoverer_stream_info_list_free (stream_list);

        return found;
}

static void
guess_audio_profile (GstDiscovererInfo *info,
                     gchar             **name,
                     gchar             **mime,
                     GList             *profiles)
{
        GList *i;
        GUPnPDLNAProfile *profile;
        GstEncodingProfile *enc_profile;

        for (i = profiles; i; i = i->next) {
                profile = (GUPnPDLNAProfile *)(i->data);
                enc_profile = gupnp_dlna_profile_get_encoding_profile (profile);

                gupnp_dlna_debug ("Checking DLNA profile %s",
                                  gupnp_dlna_profile_get_name (profile));

                if (!check_audio_profile (info, enc_profile))
                        gupnp_dlna_debug ("  Audio did not match");
                else if (!check_container (info, enc_profile))
                        gupnp_dlna_debug ("  Container did not match");
                else {
                        *name = g_strdup
                                (gupnp_dlna_profile_get_name (profile));
                        *mime = g_strdup
                                (gupnp_dlna_profile_get_mime (profile));
                        break;
                }
        }
}

static GstCaps *
caps_from_video_stream_info (GstDiscovererStreamInfo *info)
{
        GstCaps *temp = gst_discoverer_stream_info_get_caps (info);
        GstCaps *caps = gst_caps_copy (temp);
        const GstDiscovererVideoInfo *video_info =
                GST_DISCOVERER_VIDEO_INFO (info);
        const GstTagList *stream_tag_list;
        guint n, d, data;
        gboolean value;

        gst_caps_unref (temp);

        data = gst_discoverer_video_info_get_height (video_info);
        if (data)
                gst_caps_set_simple (caps, "height", G_TYPE_INT, data, NULL);

        data = gst_discoverer_video_info_get_width (video_info);
        if (data)
                gst_caps_set_simple (caps, "width", G_TYPE_INT, data, NULL);

        data = gst_discoverer_video_info_get_depth (video_info);
        if (data)
                gst_caps_set_simple (caps, "depth", G_TYPE_INT, data, NULL);

        n = gst_discoverer_video_info_get_framerate_num (video_info);
        d = gst_discoverer_video_info_get_framerate_denom (video_info);
        if (n && d)
                gst_caps_set_simple (caps,
                                     "framerate",
                                     GST_TYPE_FRACTION, n, d,
                                     NULL);

        n = gst_discoverer_video_info_get_par_num (video_info);
        d = gst_discoverer_video_info_get_par_denom (video_info);
        if (n && d)
                gst_caps_set_simple (caps,
                                     "pixel-aspect-ratio",
                                     GST_TYPE_FRACTION, n, d,
                                     NULL);

        value = gst_discoverer_video_info_is_interlaced (video_info);
        if (value)
                gst_caps_set_simple
                        (caps, "interlaced", G_TYPE_BOOLEAN, value, NULL);

        stream_tag_list = gst_discoverer_stream_info_get_tags (info);
        if (stream_tag_list) {
                guint bitrate;
                if (gst_tag_list_get_uint (stream_tag_list, "bitrate", &bitrate))
                        gst_caps_set_simple
                                (caps, "bitrate", G_TYPE_INT, (int) bitrate, NULL);

                if (gst_tag_list_get_uint (stream_tag_list,
                                           "maximum-bitrate",
                                           &bitrate))
                        gst_caps_set_simple (caps,
                                             "maximum-bitrate",
                                             G_TYPE_INT,
                                             (int) bitrate,
                                             NULL);
        }

        return caps;
}

static gboolean
check_video_profile (GstDiscovererInfo  *info,
                     GstEncodingProfile *profile)
{
        GList *i, *stream_list;
        gboolean found_video = FALSE, found_audio = FALSE;;

        stream_list = gst_discoverer_info_get_stream_list (info);

        /* Check video and audio restrictions */
        for (i = stream_list;
             i && !(found_video && found_audio);
             i = i->next) {
                GstDiscovererStreamInfo *stream;
                GType stream_type;
                GstCaps *caps = NULL;

                stream = GST_DISCOVERER_STREAM_INFO(i->data);
                stream_type = G_TYPE_FROM_INSTANCE (stream);

                if (!found_video &&
                    stream_type == GST_TYPE_DISCOVERER_VIDEO_INFO) {
                        caps = caps_from_video_stream_info (stream);
                        if (match_profile (profile,
                                           caps,
                                           GST_TYPE_ENCODING_VIDEO_PROFILE))
                                found_video = TRUE;
                        else
                                gupnp_dlna_debug ("  Video did not match");
                } else if (!found_audio &&
                           stream_type == GST_TYPE_DISCOVERER_AUDIO_INFO) {
                        caps = caps_from_audio_stream_info (stream);
                        if (match_profile (profile,
                                           caps,
                                           GST_TYPE_ENCODING_AUDIO_PROFILE))
                                found_audio = TRUE;
                        else
                                gupnp_dlna_debug ("  Audio did not match");
                }

                if (caps)
                        gst_caps_unref (caps);
        }

        gst_discoverer_stream_info_list_free (stream_list);

        if (!found_video || !found_audio)
                return FALSE;

        /* Check container restrictions */
        if (!check_container (info, profile)) {
                gupnp_dlna_debug ("  Container did not match");
                return FALSE;
        }

        return TRUE;
}

static void
guess_video_profile (GstDiscovererInfo *info,
                     gchar             **name,
                     gchar             **mime,
                     GList             *profiles)
{
        GUPnPDLNAProfile *profile = NULL;
        GstEncodingProfile *enc_profile;
        GList *i;

        for (i = profiles; i; i = i->next) {
                profile = (GUPnPDLNAProfile *)(i->data);
                enc_profile = gupnp_dlna_profile_get_encoding_profile (profile);

                gupnp_dlna_debug ("Checking DLNA profile %s",
                                  gupnp_dlna_profile_get_name (profile));
                if (check_video_profile (info, enc_profile)) {
                        *name = g_strdup (gupnp_dlna_profile_get_name (profile));
                        *mime = g_strdup (gupnp_dlna_profile_get_mime (profile));
                        break;
                }
        }
}

static void
guess_image_profile (GstDiscovererStreamInfo *info,
                     gchar                   **name,
                     gchar                   **mime,
                     GList                   *profiles)
{
        GstCaps *caps;
        GList *i;
        gboolean found = FALSE;
        GUPnPDLNAProfile *profile;
        GstEncodingProfile *enc_profile;
        const GstDiscovererVideoInfo *video_info =
                GST_DISCOVERER_VIDEO_INFO (info);

        if (!info || !gst_discoverer_video_info_is_image (video_info))
                return;

        caps = caps_from_video_stream_info (info);

        for (i = profiles; !found && i; i = i->next) {
                profile = (GUPnPDLNAProfile *)(i->data);
                enc_profile = gupnp_dlna_profile_get_encoding_profile (profile);

                /* Optimisation TODO: this can be pre-computed */
                if (is_video_profile (enc_profile))
                        continue;

                if (match_profile (enc_profile,
                                   caps,
                                   GST_TYPE_ENCODING_VIDEO_PROFILE)) {
                        /* Found a match */
                        *name = g_strdup (gupnp_dlna_profile_get_name (profile));
                        *mime = g_strdup (gupnp_dlna_profile_get_mime (profile));
                        break;
                }
        }

        gst_caps_unref (caps);
}

GUPnPDLNAInformation *
gupnp_dlna_information_new_from_discoverer_info (GstDiscovererInfo *info,
                                                 GList             *profiles)
{
        GUPnPDLNAInformation *dlna;
        GList *video_list, *audio_list;
        gchar *name = NULL, *mime = NULL;

        video_list = gst_discoverer_info_get_video_streams (info);
        audio_list = gst_discoverer_info_get_audio_streams (info);
        if (video_list) {
                if ((g_list_length (video_list) ==1 ) &&
                    gst_discoverer_video_info_is_image
                                        (GST_DISCOVERER_VIDEO_INFO
                                                  (video_list->data))) {
                        GstDiscovererStreamInfo *stream;
                        stream = (GstDiscovererStreamInfo *) video_list->data;
                        guess_image_profile (stream, &name, &mime, profiles);
                } else
                        guess_video_profile (info, &name, &mime, profiles);
        } else if (audio_list)
                guess_audio_profile (info, &name, &mime, profiles);

        gst_discoverer_stream_info_list_free (audio_list);
        gst_discoverer_stream_info_list_free (video_list);

        dlna = gupnp_dlna_information_new (name, mime, info);


        g_free (name);
        g_free (mime);

        return dlna;
}
