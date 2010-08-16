/* Example application for using GstProfile and encodebin
 * Copyright (C) 2009 Edward Hervey <edward.hervey@collabora.co.uk>
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
#include <glib.h>
#include <gst/gst.h>
#include <gst/profile/gstprofile.h>
#include <gst/pbutils/pbutils.h>
#include <libgupnp-dlna/gupnp-dlna-discoverer.h>

static gboolean silent = FALSE;

static void
pad_added_cb (GstElement * uridecodebin, GstPad * pad, GstElement * encodebin)
{
  GstPad *sinkpad;

  sinkpad = gst_element_get_compatible_pad (encodebin, pad, NULL);

  if (sinkpad == NULL) {
    GstCaps *caps;

    /* Ask encodebin for a compatible pad */
    caps = gst_pad_get_caps (pad);
    g_signal_emit_by_name (encodebin, "request-pad", caps, &sinkpad);
    if (caps)
      gst_caps_unref (caps);
  }
  if (sinkpad == NULL) {
    g_print ("Couldn't get an encoding channel for pad %s:%s\n",
        GST_DEBUG_PAD_NAME (pad));
    return;
  }

  if (G_UNLIKELY (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK)) {
    g_print ("Couldn't link pads\n");
  }

  return;
}

static gboolean
autoplug_continue_cb (GstElement * uridecodebin, GstPad * somepad,
    GstCaps * caps, GstElement * encodebin)
{
  GstPad *sinkpad;

  g_signal_emit_by_name (encodebin, "request-pad", caps, &sinkpad);

  if (sinkpad == NULL)
    return TRUE;

  return FALSE;
}

static void
bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
      g_print ("ERROR\n");
      g_main_loop_quit (mainloop);
      break;
    case GST_MESSAGE_EOS:
      g_print ("Done\n");
      g_main_loop_quit (mainloop);
      break;
    default:
      break;
  }
}

static void
transcode_file (gchar * uri, gchar * outputuri, const GstEncodingProfile * prof)
{
  GstElement *pipeline;
  GstElement *src;
  GstElement *ebin;
  GstElement *sink;
  GstBus *bus;
  GstCaps *profilecaps, *rescaps;
  GMainLoop *mainloop;

  g_print (" Input URI  : %s\n", uri);
  g_print (" Output URI : %s\n", outputuri);

  sink = gst_element_make_from_uri (GST_URI_SINK, outputuri, "sink");
  if (G_UNLIKELY (sink == NULL)) {
    g_print ("Can't create output sink, most likely invalid output URI !\n");
    return;
  }

  src = gst_element_factory_make ("uridecodebin", NULL);
  if (G_UNLIKELY (src == NULL)) {
    g_print ("Can't create uridecodebin for input URI, aborting!\n");
    return;
  }

  /* Figure out the streams that can be passed as-is to encodebin */
  g_object_get (src, "caps", &rescaps, NULL);
  rescaps = gst_caps_copy (rescaps);
  profilecaps = gst_encoding_profile_get_codec_caps ((GstEncodingProfile *) prof);
  gst_caps_append (rescaps, profilecaps);

  /* Set properties */
  g_object_set (src, "uri", uri, "caps", rescaps, NULL);

  ebin = gst_element_factory_make ("encodebin", NULL);
  g_object_set (ebin, "profile", prof, NULL);

  g_signal_connect (src, "autoplug-continue", G_CALLBACK (autoplug_continue_cb),
      ebin);
  g_signal_connect (src, "pad-added", G_CALLBACK (pad_added_cb), ebin);

  pipeline = gst_pipeline_new ("encoding-pipeline");

  gst_bin_add_many (GST_BIN (pipeline), src, ebin, sink, NULL);

  gst_element_link (ebin, sink);

  mainloop = g_main_loop_new (NULL, FALSE);

  bus = gst_pipeline_get_bus ((GstPipeline *) pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", G_CALLBACK (bus_message_cb), mainloop);

  if (gst_element_set_state (pipeline,
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
    g_print ("Failed to start the encoding\n");
    return;
  }

  g_main_loop_run (mainloop);

  gst_element_set_state (pipeline, GST_STATE_NULL);
}

static gchar *
ensure_uri (gchar * location)
{
  gchar *res;
  gchar *path;

  if (gst_uri_is_valid (location))
    return g_strdup (location);

  if (!g_path_is_absolute (location)) {
    gchar *cur_dir;
    cur_dir = g_get_current_dir ();
    path = g_build_filename (cur_dir, location, NULL);
    g_free (cur_dir);
  } else
    path = g_strdup (location);

  res = g_filename_to_uri (path, NULL, NULL);
  g_free (path);

  return res;
}

int
main (int argc, char **argv)
{
  GError *err = NULL;
  gchar *outputuri = NULL;
  gchar *format = NULL;
  GOptionEntry options[] = {
    {"silent", 's', 0, G_OPTION_ARG_NONE, &silent,
        "Don't output the information structure", NULL},
    {"outputuri", 'o', 0, G_OPTION_ARG_STRING, &outputuri,
        "URI to encode to", "URI (<protocol>://<location>)"},
    {"format", 'f', 0, G_OPTION_ARG_STRING, &format,
        "DLNA profile to use", NULL},
    {NULL}
  };
  GOptionContext *ctx;
  GUPnPDLNADiscoverer *discoverer;
  GUPnPDLNAProfile *profile;
  gchar *inputuri;

  if (!g_thread_supported ())
    g_thread_init (NULL);

  ctx = g_option_context_new ("- encode URIs with GstProfile and encodebin");
  g_option_context_add_main_entries (ctx, options, NULL);
  g_option_context_add_group (ctx, gst_init_get_option_group ());

  if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
    g_print ("Error initializing: %s\n", err->message);
    exit (1);
  }

  g_option_context_free (ctx);

  if (outputuri == NULL || argc != 2) {
    g_print ("usage: %s <inputuri> -o <outputuri> --format <profile>\n",
        argv[0]);
    exit (-1);
  }

  gst_init(&argc, &argv);

  /* Create the profile */
  discoverer = gupnp_dlna_discoverer_new ((GstClockTime) GST_SECOND,
                                          FALSE,
                                          FALSE);
  profile = gupnp_dlna_discoverer_get_profile (discoverer, format);
  if (G_UNLIKELY (profile == NULL)) {
    g_print ("Encoding arguments are not valid !\n");
    return 1;
  }

  /* Fixup outputuri to be a URI */
  inputuri = ensure_uri (argv[1]);
  outputuri = ensure_uri (outputuri);

  /* Trancode file */
  transcode_file (inputuri,
                  outputuri,
                  gupnp_dlna_profile_get_encoding_profile (profile));

  /* cleanup */
  g_object_unref (profile);
  g_object_unref (discoverer);

  return 0;
}
