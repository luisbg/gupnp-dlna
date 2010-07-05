/* GUPnPDLNA
 * gupnp-dlna-info.c
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2010 Collabora Multimedia
 *
 * Authors: Parthasarathi Susarla <partha.susarla@collabora.co.uk>
 *
 * Based on 'gst-discoverer.c' by
 * Edward Hervey <edward.hervey@collabora.co.uk>
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
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <gst/gst.h>
#include <gst/profile/gstprofile.h>
#include <gst/discoverer/gstdiscoverer.h>
#include <gst/pbutils/pbutils.h>

#include <libgupnp-dlna/gupnp-dlna-load.h>
#include <libgupnp-dlna/gupnp-dlna-profile.h>
#include <libgupnp-dlna/gupnp-dlna-discoverer.h>
#include <libgupnp-dlna/gupnp-dlna-information.h>


static gboolean async = FALSE;
static gboolean verbose = FALSE;
static gint timeout = 10;

static const gchar *_gst_stream_type_to_string[] = {
        "CONTAINER",
        "AUDIO",
        "VIDEO",
        "IMAGE",
        "UNKNOWN",
};


typedef struct
{
        GUPnPDLNADiscoverer *dc;
        int argc;
        char **argv;
} PrivStruct;

/*
 * The following functions are from gst-discoverer.c (gst-convenience/tools)
 */
#define my_g_string_append_printf(str, format, ...)			\
        g_string_append_printf (str, "%*s" format, 2*depth, " ", ##__VA_ARGS__)

static gchar *
gst_stream_audio_information_to_string (GstStreamAudioInformation * info,
					gint depth)
{
        GString *s;
        gchar *tmp;
        int len = 400;

        g_return_val_if_fail (info != NULL, NULL);

        s = g_string_sized_new (len);

        my_g_string_append_printf (s, "Codec:\n");
        tmp = gst_caps_to_string (info->parent.caps);
        my_g_string_append_printf (s, "  %s\n", tmp);
        g_free (tmp);

        my_g_string_append_printf (s, "Additional info:\n");
        if (info->parent.misc) {
                tmp = gst_structure_to_string (info->parent.misc);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }

        my_g_string_append_printf (s, "Channels: %u\n", info->channels);
        my_g_string_append_printf (s, "Sample rate: %u\n", info->sample_rate);
        my_g_string_append_printf (s, "Depth: %u\n", info->depth);

        my_g_string_append_printf (s, "Bitrate: %u\n", info->bitrate);
        my_g_string_append_printf (s, "Max bitrate: %u\n", info->max_bitrate);
        my_g_string_append_printf (s, "VBR: %s\n",
                                   info->is_vbr ? "true" : "false");

        my_g_string_append_printf (s, "Tags:\n");
        if (info->parent.tags) {
                tmp = gst_structure_to_string ((GstStructure *) info->parent.tags);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }

        return g_string_free (s, FALSE);
}

static const gchar *_gst_video_format_to_string[] = {
        "UNKNOWN",
        "I420",
        "YV12",
        "YUY2",
        "UYVY",
        "AYUV",
        "RGBx",
        "BGRx",
        "xRGB",
        "xBGR",
        "RGBA",
        "BGRA",
        "ARGB",
        "ABGR",
        "RGB",
        "BGR",
        "Y41B",
        "Y42B",
        "YVYU",
        "Y444",
        "v210",
        "v216"
};

static gchar *
gst_stream_video_information_to_string (GstStreamVideoInformation * info,
					gint depth)
{
        GString *s;
        gchar *tmp;
        int len = 500, n, d;

        g_return_val_if_fail (info != NULL, NULL);

        s = g_string_sized_new (len);

        my_g_string_append_printf (s, "Codec:\n");
        tmp = gst_caps_to_string (info->parent.caps);
        my_g_string_append_printf (s, "  %s\n", tmp);
        g_free (tmp);

        my_g_string_append_printf (s, "Additional info:\n");
        if (info->parent.misc) {
                tmp = gst_structure_to_string (info->parent.misc);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }

        my_g_string_append_printf (s, "Width: %u\n", info->width);
        my_g_string_append_printf (s, "Height: %u\n", info->height);
        my_g_string_append_printf (s, "Depth: %u\n", info->depth);

        n = gst_value_get_fraction_numerator (&info->frame_rate);
        d = gst_value_get_fraction_denominator (&info->frame_rate);
        my_g_string_append_printf (s, "Frame rate: %u/%u\n", n, d);

        n = gst_value_get_fraction_numerator (&info->pixel_aspect_ratio);
        d = gst_value_get_fraction_denominator (&info->pixel_aspect_ratio);
        my_g_string_append_printf (s, "Pixel aspect ratio: %u/%u\n", n, d);

        my_g_string_append_printf (s, "Format: %s\n",
                                   _gst_video_format_to_string[info->format]);

        my_g_string_append_printf (s, "Interlaced: %s\n",
                                   info->interlaced ? "true" : "false");

        my_g_string_append_printf (s, "Tags:\n");
        if (info->parent.tags) {
                tmp = gst_structure_to_string ((GstStructure *) info->parent.tags);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }


        return g_string_free (s, FALSE);
}

static void
print_stream_info (GstStreamInformation * info, void *depth)
{
        gchar *desc = NULL;

        if (info->caps) {
                if (gst_caps_is_fixed (info->caps))
                        desc = gst_pb_utils_get_codec_description (info->caps);
                else
                        desc = gst_caps_to_string (info->caps);
        }

        g_print ("%*s%s: %s\n", 2 * GPOINTER_TO_INT (depth), " ",
                 _gst_stream_type_to_string[info->streamtype], desc);

        if (desc) {
                g_free (desc);
                desc = NULL;
        }

        switch (info->streamtype) {
        case GST_STREAM_AUDIO:
                desc = gst_stream_audio_information_to_string (
                                        (GstStreamAudioInformation *) info,
                                        GPOINTER_TO_INT (depth) + 1);
                break;

        case GST_STREAM_VIDEO:
        case GST_STREAM_IMAGE:
                desc = gst_stream_video_information_to_string (
                                         (GstStreamVideoInformation *) info,
                                         GPOINTER_TO_INT (depth) + 1);
                break;

        default:
                break;
        }

        if (desc) {
                g_print ("%s", desc);
                g_free (desc);
        }
}

static void
print_duration (const GstDiscovererInformation * info, gint tab)
{
        g_print ("%*s%" GST_TIME_FORMAT "\n", tab + 1, " ",
                 GST_TIME_ARGS (info->duration));
}

static void
print_list (const GstDiscovererInformation * info, gint tab)
{
        if (!info || !info->stream_list)
                return;

        g_list_foreach (info->stream_list, (GFunc) print_stream_info,
                        GINT_TO_POINTER (tab));
}

static void
print_gst_info (const GstDiscovererInformation *info, GError *err)
{
        /* Get done with the error parsing first */
        if (info->result & GST_DISCOVERER_URI_INVALID)
                g_print ("URI is not valid\n");
        else if (info->result & GST_DISCOVERER_TIMEOUT)
                g_print ("Timeout !\n");
        if (info->result & GST_DISCOVERER_ERROR) {
                g_print ("An error while discovering the file\n");
                g_print (" %s\n", err->message);
                if (info->result & GST_DISCOVERER_MISSING_PLUGINS) {
                        gchar *tmp = gst_structure_to_string (info->misc);
                        g_print (" (%s)\n", tmp);
                        g_free (tmp);
                }

                return;
        }

        /* Print Useful information */
        if (verbose) {
                if (!(info->result &
                      (GST_DISCOVERER_ERROR | GST_DISCOVERER_TIMEOUT))) {
                        g_print ("\nStream list:\n");
                        print_list (info, 1);
                        g_print ("\nDuration:\n");
                        print_duration (info, 1);
                }
        }
}

static void
print_dlna_info (GUPnPDLNAInformation *dlna, GError *err)
{
        g_print ("\nURI: %s\n", gupnp_dlna_information_get_info (dlna)->uri);
        g_print ("Profile Name: %s\n", gupnp_dlna_information_get_name (dlna));
        g_print ("Profile MIME: %s\n", gupnp_dlna_information_get_mime (dlna));

        print_gst_info (gupnp_dlna_information_get_info(dlna), err);

        g_print ("\n");
        return;
}

static void
discoverer_done (GUPnPDLNADiscoverer *discover,
                 GUPnPDLNAInformation *dlna,
                 GError *err)
{
        print_dlna_info (dlna, err);
        return;
}

static void
discoverer_ready (GUPnPDLNADiscoverer *dc, GMainLoop *ml)
{
        g_main_loop_quit (ml);
}

static void
process_file (GUPnPDLNADiscoverer *discover, const gchar *filename)
{
        GError *err = NULL;
        GDir *dir;
        gchar *uri, *path;
        GUPnPDLNAInformation *dlna;

        if(!gst_uri_is_valid (filename)) {
                if((dir = g_dir_open (filename, 0, NULL))) {
                        const gchar *entry;

                        while ((entry = g_dir_read_name (dir))) {
                                gchar *path;
                                path = g_strconcat (filename,
                                                    G_DIR_SEPARATOR_S,
                                                    entry,
                                                    NULL);
                                process_file (discover, path);
                                g_free(path);
                        }

                        g_dir_close (dir);
                        return;
                }

                if (!g_path_is_absolute (filename)) {
                        gchar *cur_dir;

                        cur_dir = g_get_current_dir ();
                        path = g_build_filename (cur_dir, filename, NULL);
                        g_free (cur_dir);
                } else {
                        path = g_strdup (filename);
                }

                uri = g_filename_to_uri (path, NULL, &err);
                g_free (path);
                path = NULL;

                if (err) {
                        g_warning ("Couldn't convert filename to URI: %s\n",
                                   err->message);
                        g_error_free (err);
                        err = NULL;
                        return;
                }
        } else {
                uri = g_strdup (filename);
        }

        if (async == FALSE) {
                dlna = gupnp_dlna_discoverer_discover_uri_sync (discover,
                                                                uri,
                                                                &err);
                if (err) {
                        /* Report error to user, and free error */
                        fprintf (stderr,
                                 "Unable to read file: %s\n",
                                 err->message);
                        g_error_free (err);
                        err = NULL;
                } else {
                        print_dlna_info (dlna, err);
                }
        } else {
                gupnp_dlna_discoverer_discover_uri (discover, uri);
        }

        g_free (uri);
}


static gboolean
async_idle_loop (PrivStruct * ps)
{
        gint i;

        for (i = 1; i < ps->argc; i++)
                process_file (ps->dc, ps->argv[i]);

        return FALSE;
}

/* Main */
int
main (int argc, char **argv)
{
        gint i;
        GUPnPDLNADiscoverer *discover;
        GError *err = NULL;

        GOptionEntry options[] = {
                {"timeout", 't', 0, G_OPTION_ARG_INT, &timeout,
                 "Specify timeout (in seconds, defaults to 10)", "T"},
                {"async", 'a', 0, G_OPTION_ARG_NONE, &async,
                 "Run asynchronously", NULL},
                {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
                 "Print lot more information", NULL},
                {NULL}
        };

        GOptionContext *ctx;


        g_thread_init(NULL);

        ctx = g_option_context_new (" - program to extract DLNA and related metadata");
        g_option_context_add_main_entries (ctx, options, NULL);
        g_option_context_add_group (ctx, gst_init_get_option_group ());

        if (!g_option_context_parse (ctx, &argc, &argv, &err)) {

                g_print ("Error initializing: %s\n", err->message);
                exit (1);
        }

        g_option_context_free (ctx);

        if (argc < 2) {
                g_print ("usage:%s <files>\n", argv[0]);
                return -1;
        }

        gst_init(&argc, &argv);


        discover = gupnp_dlna_discoverer_new ((GstClockTime)
                                              (timeout * GST_SECOND));

        if (async == FALSE) {
                for ( i = 1 ; i < argc ; i++ ) {
                        process_file (discover, argv[i]);
                }
        } else {
                PrivStruct *ps = g_new0 (PrivStruct, 1);
                GMainLoop *ml = g_main_loop_new (NULL, FALSE);

                ps->dc = discover;
                ps->argc = argc;
                ps->argv = argv;

                g_idle_add ((GSourceFunc) async_idle_loop, ps);

                g_signal_connect (discover, "done",
                                  (GCallback) discoverer_done, 0);
                g_signal_connect (discover, "ready",
                                  (GCallback) discoverer_ready, ml);

                gupnp_dlna_discoverer_start (discover);

                g_main_loop_run (ml);

                gupnp_dlna_discoverer_stop (discover);

        }
        g_object_unref (discover);
        return 0;
}
