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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <gst/gst.h>
#include <gst/profile/gstprofile.h>
#include <gst/pbutils/gstdiscoverer.h>

#include <libgupnp-dlna/gupnp-dlna-load.h>
#include <libgupnp-dlna/gupnp-dlna-profile.h>
#include <libgupnp-dlna/gupnp-dlna-discoverer.h>
#include <libgupnp-dlna/gupnp-dlna-information.h>


static gboolean async = FALSE;
static gboolean verbose = FALSE;
static gint timeout = 10;


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
gst_stream_audio_information_to_string (GstDiscovererStreamInfo * info,
					gint depth)
{
        GString *s;
        gchar *tmp;
        GstCaps *caps;
        const GstStructure *misc;
        const GstTagList *taglist;
        const GstDiscovererAudioInfo *audio_info;
        int len = 400;

        g_return_val_if_fail (info != NULL, NULL);

        audio_info = GST_DISCOVERER_AUDIO_INFO (info);
        s = g_string_sized_new (len);

        my_g_string_append_printf (s, "Codec:\n");
        caps = gst_discoverer_stream_info_get_caps (info);
        tmp = gst_caps_to_string (caps);
        my_g_string_append_printf (s, "  %s\n", tmp);
        gst_caps_unref (caps);
        g_free (tmp);

        my_g_string_append_printf (s, "Additional info:\n");
        misc = gst_discoverer_stream_info_get_misc (info);
        if (misc) {
                tmp = gst_structure_to_string (misc);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }

        my_g_string_append_printf (s, "Channels: %u\n",
                                   gst_discoverer_audio_info_get_channels (audio_info));
        my_g_string_append_printf (s, "Sample rate: %u\n",
                                   gst_discoverer_audio_info_get_sample_rate (audio_info));
        my_g_string_append_printf (s, "Depth: %u\n",
                                   gst_discoverer_audio_info_get_depth (audio_info));

        my_g_string_append_printf (s, "Bitrate: %u\n",
                                   gst_discoverer_audio_info_get_bitrate (audio_info));
        my_g_string_append_printf (s, "Max bitrate: %u\n",
                                   gst_discoverer_audio_info_get_max_bitrate (audio_info));

        my_g_string_append_printf (s, "Tags:\n");
        taglist = gst_discoverer_stream_info_get_tags (info);
        if (taglist) {
                tmp = gst_structure_to_string ((GstStructure *) taglist);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }

        return g_string_free (s, FALSE);
}

static gchar *
gst_stream_video_information_to_string (GstDiscovererStreamInfo * info,
					gint depth)
{
        GString *s;
        gchar *tmp;
        const GstStructure *misc;
        const GstTagList *taglist;
        const GstDiscovererVideoInfo *video_info;
        GstCaps *caps;
        int len = 500;

        g_return_val_if_fail (info != NULL, NULL);

        video_info = GST_DISCOVERER_VIDEO_INFO (info);

        s = g_string_sized_new (len);

        my_g_string_append_printf (s, "Codec:\n");
        caps = gst_discoverer_stream_info_get_caps (info);
        tmp = gst_caps_to_string (caps);
        my_g_string_append_printf (s, "  %s\n", tmp);
        gst_caps_unref (caps);
        g_free (tmp);

        my_g_string_append_printf (s, "Additional info:\n");
        misc = gst_discoverer_stream_info_get_misc (info);
        if (misc) {
                tmp = gst_structure_to_string (misc);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }

        my_g_string_append_printf (s, "Width: %u\n",
                                   gst_discoverer_video_info_get_width (video_info));
        my_g_string_append_printf (s, "Height: %u\n",
                                   gst_discoverer_video_info_get_height (video_info));
        my_g_string_append_printf (s, "Depth: %u\n",
                                   gst_discoverer_video_info_get_depth (video_info));

        my_g_string_append_printf (s, "Frame rate: %u/%u\n",
                                   gst_discoverer_video_info_get_framerate_num (video_info),
                                   gst_discoverer_video_info_get_framerate_denom (video_info));

        my_g_string_append_printf (s, "Pixel aspect ratio: %u/%u\n",
                                   gst_discoverer_video_info_get_par_num (video_info),
                                   gst_discoverer_video_info_get_par_denom (video_info));

        my_g_string_append_printf (s, "Interlaced: %s\n",
                                   gst_discoverer_video_info_is_interlaced (video_info) ? "true" : "false");

        my_g_string_append_printf (s, "Bitrate: %u\n",
                                   gst_discoverer_video_info_get_bitrate (video_info));

        my_g_string_append_printf (s, "Max bitrate: %u\n",
                                   gst_discoverer_video_info_get_max_bitrate (video_info));

        my_g_string_append_printf (s, "Tags:\n");
        taglist = gst_discoverer_stream_info_get_tags (info);
        if (taglist) {
                tmp = gst_structure_to_string ((GstStructure *) taglist);
                my_g_string_append_printf (s, "  %s\n", tmp);
                g_free (tmp);
        } else {
                my_g_string_append_printf (s, "  None\n");
        }


        return g_string_free (s, FALSE);
}

static void
print_stream_info (GstDiscovererStreamInfo * info, void *depth)
{
        gchar *desc = NULL;
        GstCaps *caps;

        caps = gst_discoverer_stream_info_get_caps (info);
        if (caps) {
                desc = gst_caps_to_string (caps);
        }

        g_print ("%*s%s: %s\n", 2 * GPOINTER_TO_INT (depth), " ",
                 gst_discoverer_stream_info_get_stream_type_nick (info),
                 desc);

        if (desc) {
                g_free (desc);
                desc = NULL;
        }

        if (GST_IS_DISCOVERER_AUDIO_INFO (info))
                desc = gst_stream_audio_information_to_string (
                                        info,
                                        GPOINTER_TO_INT (depth) + 1);
        else if (GST_IS_DISCOVERER_VIDEO_INFO (info))
                desc = gst_stream_video_information_to_string (
                                         info,
                                         GPOINTER_TO_INT (depth) + 1);

        if (desc) {
                g_print ("%s", desc);
                g_free (desc);
        }
}

static void
print_topology (GstDiscovererStreamInfo * info, gint depth)
{
        GstDiscovererStreamInfo *next;
        if (!info)
                return;

        print_stream_info (info, GINT_TO_POINTER (depth));

        next = gst_discoverer_stream_info_get_next (info);
        if (next) {
                print_topology (next, depth + 1);
                gst_discoverer_stream_info_unref (next);
        } else if (GST_IS_DISCOVERER_CONTAINER_INFO (info)) {
                GList *tmp, *streams;
                GstDiscovererContainerInfo *container =
                        GST_DISCOVERER_CONTAINER_INFO (info);

                streams = gst_discoverer_container_info_get_streams (container);
                for (tmp = streams; tmp; tmp = tmp->next) {
                        GstDiscovererStreamInfo *tmpinf =
                                GST_DISCOVERER_STREAM_INFO (tmp->data);
                        print_topology (tmpinf, depth + 1);
                }
        }
}

static void
print_duration (GstDiscovererInfo * info, gint tab)
{
        g_print ("%*s%" GST_TIME_FORMAT "\n", tab + 1, " ",
                 GST_TIME_ARGS (gst_discoverer_info_get_duration (info)));
}

static void
print_gst_info (GstDiscovererInfo *info, GError *err)
{
        GstDiscovererResult result = gst_discoverer_info_get_result (info);
        GstDiscovererStreamInfo *sinfo;

        switch (result) {
        case GST_DISCOVERER_OK:
                break;
        case GST_DISCOVERER_URI_INVALID:
                g_print ("URI is not valid\n");
                break;
        case GST_DISCOVERER_ERROR:
                g_print ("An error was encountered while discovering the file\n");
                g_print (" %s\n", err->message);
                break;
        case GST_DISCOVERER_TIMEOUT:
                g_print ("Analyzing URI timed out\n");
                break;
        case GST_DISCOVERER_BUSY:
                g_print ("Discoverer was busy\n");
                break;
        case GST_DISCOVERER_MISSING_PLUGINS:
                g_print ("Missing plugins\n");
                if (verbose) {
                        gchar *tmp =
                                gst_structure_to_string (gst_discoverer_info_get_misc (info));
                        g_print (" (%s)\n", tmp);
                        g_free (tmp);
                }
                break;
        }

        if (verbose) {
                if ((sinfo = gst_discoverer_info_get_stream_info (info))) {
                        g_print ("\nTopology:\n");
                        print_topology (sinfo, 1);
                        g_print ("\nDuration:\n");
                        print_duration (info, 1);
                        gst_discoverer_stream_info_unref (sinfo);
                }
        }

        g_print ("\n");
}

static void
print_dlna_info (GUPnPDLNAInformation *dlna, GError *err)
{
        GstDiscovererInfo *info;

        info = (GstDiscovererInfo *)gupnp_dlna_information_get_info (dlna);

        g_print ("\nURI: %s\n", gst_discoverer_info_get_uri (info));
        g_print ("Profile Name: %s\n", gupnp_dlna_information_get_name (dlna));
        g_print ("Profile MIME: %s\n", gupnp_dlna_information_get_mime (dlna));

        print_gst_info ((GstDiscovererInfo *)info, err);

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
        gboolean relaxed_mode = FALSE;
        gboolean extended_mode = FALSE;
        GError *err = NULL;

        GOptionEntry options[] = {
                {"timeout", 't', 0, G_OPTION_ARG_INT, &timeout,
                 "Specify timeout (in seconds, defaults to 10)", "T"},
                {"async", 'a', 0, G_OPTION_ARG_NONE, &async,
                 "Run asynchronously", NULL},
                {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
                 "Print lot more information", NULL},
                {"relaxed mode", 'r', 0, G_OPTION_ARG_NONE, &relaxed_mode,
                 "Enable Relaxed mode", NULL},
                {"extended mode", 'e', 0, G_OPTION_ARG_NONE, &extended_mode,
                 "Enable extended mode", NULL},
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
                                              (timeout * GST_SECOND),
                                              relaxed_mode,
                                              extended_mode);

        if (async == FALSE) {
                for ( i = 1 ; i < argc ; i++ )
                        process_file (discover, argv[i]);
        } else {
                PrivStruct *ps = g_new0 (PrivStruct, 1);
                GMainLoop *ml = g_main_loop_new (NULL, FALSE);

                ps->dc = discover;
                ps->argc = argc;
                ps->argv = argv;

                g_idle_add ((GSourceFunc) async_idle_loop, ps);

                g_signal_connect (discover, "done",
                                  (GCallback) discoverer_done, 0);
                g_signal_connect (discover, "finished",
                                  (GCallback) discoverer_ready, ml);

                gupnp_dlna_discoverer_start (discover);

                g_main_loop_run (ml);

                gupnp_dlna_discoverer_stop (discover);

        }
        g_object_unref (discover);
        return 0;
}
