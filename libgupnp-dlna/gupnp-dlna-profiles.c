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

#include <glib.h>
#include <gst/profile/gstprofile.h>
#include "gupnp-dlna-discoverer.h"
#include "gupnp-dlna-load.h"
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

static GList *profiles = NULL;

static gboolean is_video_profile (GstEncodingProfile *profile)
{
        GList *i;

        for (i = profile->encodingprofiles; i; i = i->next)
                if (((GstStreamEncodingProfile *) i->data)->type ==
                                        GST_ENCODING_PROFILE_VIDEO)
                        return TRUE;

        return FALSE;
}

static gboolean structure_can_intersect (const GstStructure *st1,
                                         const GstStructure *st2)
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

static gboolean structure_is_subset (const GstStructure *st1,
                                     const GstStructure *st2)
{
        int i;

        for (i = 0; i < gst_structure_n_fields (st2); i++) {
                const gchar *name = gst_structure_nth_field_name (st2, i);

                if (!gst_structure_has_field(st1, name))
                        return FALSE;
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
static gboolean caps_can_intersect_and_is_subset (GstCaps *stream_caps,
                                                  GstCaps *profile_caps)
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
match_profile (GstEncodingProfile     *profile,
               GstCaps                *caps,
               GstEncodingProfileType type)
{
        GList *i;

        /* Profiles with an empty name are used only for inheritance and should
         * not be matched against. */
        if (profile->name[0] == '\0')
          return FALSE;

        for (i = profile->encodingprofiles; i; i = i->next) {
                GstStreamEncodingProfile *enc_profile = i->data;

                if (enc_profile->type == type &&
                    caps_can_intersect_and_is_subset (caps,
                                                      enc_profile->format))
                        return TRUE;
        }

        return FALSE;
}

static gboolean
check_container (GstDiscovererInformation *info, GstEncodingProfile *profile)
{
        /* Top-level GstStreamInformation in the topology will be
         * the container */
        if (info->stream_info->streamtype == GST_STREAM_CONTAINER &&
            gst_caps_can_intersect (info->stream_info->caps, profile->format))
                return TRUE;
        else if (info->stream_info->streamtype != GST_STREAM_CONTAINER &&
                 gst_caps_is_empty (profile->format))
                return TRUE;

        return FALSE;
}

static GstCaps *
caps_from_audio_stream_info (GstStreamAudioInformation *audio)
{
        GstCaps *caps = gst_caps_copy ((GST_STREAM_INFORMATION (audio))->caps);

        if (audio->sample_rate)
                gst_caps_set_simple (caps, "rate", G_TYPE_INT,
                                audio->sample_rate, NULL);
        if (audio->channels)
                gst_caps_set_simple (caps, "channels", G_TYPE_INT,
                                audio->channels, NULL);
        if (audio->bitrate)
                gst_caps_set_simple (caps, "bitrate", G_TYPE_INT,
                                audio->bitrate, NULL);
        if (audio->max_bitrate)
                gst_caps_set_simple (caps, "maximum-bitrate", G_TYPE_INT,
                                audio->max_bitrate, NULL);
        if (audio->depth)
                gst_caps_set_simple (caps, "depth", G_TYPE_INT,
                                audio->depth, NULL);

        return caps;
}

static gboolean
check_audio_profile (GstEncodingProfile       *profile,
                     GstDiscovererInformation *info)
{
        GstCaps *caps;
        GList *i;
        gboolean found = FALSE;

        /* Optimisation TODO: this can be pre-computed */
        if (is_video_profile (profile))
                return FALSE;

        for (i = info->stream_list; !found && i; i = i->next) {
                GstStreamInformation *stream = (GstStreamInformation *) i->data;
                GstStreamAudioInformation *audio;

                if (stream->streamtype != GST_STREAM_AUDIO)
                        continue;

                audio = GST_STREAM_AUDIO_INFORMATION (stream);
                caps = caps_from_audio_stream_info (audio);

                if (match_profile (profile, caps, GST_ENCODING_PROFILE_AUDIO)) {
                        found = TRUE;
                        break;
                }

                gst_caps_unref (caps);
        }

        return found;
}

static void
guess_audio_profile (GstDiscovererInformation *info, gchar **name, gchar **mime)
{
        GList *i;

        for (i = profiles; i; i = i->next) {
                GUPnPDLNAProfile *profile = (GUPnPDLNAProfile *)(i->data);

                if (check_audio_profile (profile->enc_profile, info) &&
                    check_container (info, profile->enc_profile)) {
                        *name = g_strdup (profile->name);
                        *mime = g_strdup (profile->mime);
                        break;
                }
        }
}

static GstCaps *
caps_from_video_stream_info (GstStreamVideoInformation *video)
{
        GstStreamInformation *stream = GST_STREAM_INFORMATION (video);
        GstCaps *caps = gst_caps_copy (stream->caps);
        int n, d;

        if (video->height)
                gst_caps_set_simple (caps, "height", G_TYPE_INT,
                                video->height, NULL);
        if (video->width)
                gst_caps_set_simple (caps, "width", G_TYPE_INT,
                                video->width, NULL);
        if (GST_VALUE_HOLDS_FRACTION (&video->frame_rate)) {
                n = gst_value_get_fraction_numerator (&video->frame_rate);
                d = gst_value_get_fraction_denominator (&video->frame_rate);
                gst_caps_set_simple (caps, "framerate", GST_TYPE_FRACTION,
                                n, d, NULL);
        }
        if (GST_VALUE_HOLDS_FRACTION (&video->pixel_aspect_ratio)) {
                n = gst_value_get_fraction_numerator (
                                        &video->pixel_aspect_ratio);
                d = gst_value_get_fraction_denominator (
                                        &video->pixel_aspect_ratio);
                gst_caps_set_simple (caps, "pixel-aspect-ratio",
                                GST_TYPE_FRACTION, n, d, NULL);
        }
        if (video->format != GST_VIDEO_FORMAT_UNKNOWN)
                gst_caps_set_simple (caps, "format", GST_TYPE_FOURCC,
                                gst_video_format_to_fourcc (video->format),
                                NULL);
        if (stream->tags) {
                guint bitrate;
                if (gst_tag_list_get_uint (stream->tags, "bitrate", &bitrate))
                        gst_caps_set_simple (caps, "bitrate", G_TYPE_INT,
                                        (int) bitrate, NULL);
                if (gst_tag_list_get_uint (stream->tags, "maximum-bitrate",
					&bitrate))
                        gst_caps_set_simple (caps, "maximum-bitrate",
					G_TYPE_INT, (int) bitrate, NULL);
        }

        return caps;
}

static gboolean
check_video_profile (GstEncodingProfile *profile,
                     GstDiscovererInformation *info)
{
        GList *i;
        gboolean found_video = FALSE, found_audio = FALSE;;

        /* Check video and audio restrictions */
        for (i = info->stream_list;
             i && !(found_video && found_audio);
             i = i->next) {
                GstStreamInformation *stream;
                GstCaps *caps = NULL;

                stream = (GstStreamInformation *) i->data;

                if (!found_video &&
                    stream->streamtype == GST_STREAM_VIDEO) {
                        caps = caps_from_video_stream_info (
                                        GST_STREAM_VIDEO_INFORMATION (stream));
                        if (match_profile (profile,
                                           caps,
                                           GST_ENCODING_PROFILE_VIDEO))
                                found_video = TRUE;
                } else if (!found_audio &&
                           stream->streamtype == GST_STREAM_AUDIO) {
                        caps = caps_from_audio_stream_info (
                                        GST_STREAM_AUDIO_INFORMATION (stream));
                        if (match_profile (profile,
                                           caps,
                                           GST_ENCODING_PROFILE_AUDIO))
                                found_audio = TRUE;
                }

                if (caps)
                        gst_caps_unref (caps);
        }

        if (!found_video || !found_audio)
                return FALSE;

        /* Check container restrictions */
        if (!check_container (info, profile))
                return FALSE;

        return TRUE;
}

static void
guess_video_profile (GstDiscovererInformation *info, gchar **name, gchar **mime)
{
        GUPnPDLNAProfile *profile = NULL;
        GList *i;

        for (i = profiles; i; i = i->next) {
                profile = (GUPnPDLNAProfile *)(i->data);

                if (check_video_profile (profile->enc_profile, info)) {
                        *name = g_strdup (profile->name);
                        *mime = g_strdup (profile->mime);
                        break;
                }
        }
}

static void
guess_image_profile (GstStreamInformation *info, gchar **name, gchar **mime)
{
        GstStreamVideoInformation *video = GST_STREAM_VIDEO_INFORMATION (info);
        GstCaps *caps;
        GList *i;
        gboolean found = FALSE;

        if (!info || info->streamtype != GST_STREAM_IMAGE)
                return;

        caps = caps_from_video_stream_info (video);

        for (i = profiles; !found && i; i = i->next) {
                GUPnPDLNAProfile *profile = (GUPnPDLNAProfile *)(i->data);

                /* Optimisation TODO: this can be pre-computed */
                if (is_video_profile (profile->enc_profile))
                        continue;

                if (match_profile (profile->enc_profile,
                                   caps,
                                   GST_ENCODING_PROFILE_IMAGE)) {
                        /* Found a match */
                        *name = g_strdup (profile->name);
                        *mime = g_strdup (profile->mime);
                        break;
                }
        }

        gst_caps_unref (caps);
}

GUPnPDLNAInformation *
gupnp_dlna_information_new_from_discoverer_info (GstDiscovererInformation * info)
{
        GUPnPDLNAInformation *dlna;
        GstStreamType type = GST_STREAM_UNKNOWN;
        GList *tmp = info->stream_list;
        gchar *name = NULL, *mime = NULL;

        if (!profiles) {
                profiles = g_list_concat (profiles,
                                        gupnp_dlna_load_profiles_from_disk ());
        }

        while (tmp) {
                GstStreamInformation *stream_info =
                        (GstStreamInformation *) tmp->data;

                if (stream_info->streamtype == GST_STREAM_VIDEO)
                        type = GST_STREAM_VIDEO;
                else if (stream_info->streamtype == GST_STREAM_IMAGE)
                        type = GST_STREAM_IMAGE;
                else if (stream_info->streamtype == GST_STREAM_AUDIO &&
                         type != GST_STREAM_VIDEO)
                        type = GST_STREAM_AUDIO;

                tmp = tmp->next;
        }

        if (type == GST_STREAM_AUDIO)
                guess_audio_profile (info, &name, &mime);
        else if (type == GST_STREAM_VIDEO)
                guess_video_profile (info, &name, &mime);
        else if (type == GST_STREAM_IMAGE)
                /* There will be only one GstStreamInformation for images */
                guess_image_profile (info->stream_info, &name, &mime);

        dlna = gupnp_dlna_information_new (name, mime, info);

        g_debug ("DLNA profile: %s -> %s, %s", info->uri, name, mime);

        g_free (name);
        g_free (mime);
        return dlna;
}

GUPnPDLNAProfile *
gupnp_dlna_profile_from_name (const gchar *name)
{
        GList *i;

        if (!profiles) {
                profiles = g_list_concat (profiles,
                                        gupnp_dlna_load_profiles_from_disk ());
        }

        for (i = profiles; i != NULL; i = i->next) {
                GUPnPDLNAProfile *profile = (GUPnPDLNAProfile *) i->data;

                if (g_str_equal (profile->name, name)) {
                        g_object_ref (profile);
                        return profile;
                }
        }

        return NULL;
}
