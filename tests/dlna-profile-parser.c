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

#include <gst/gst.h>
#include <libgupnp-dlna/profile-loading.h>
#include <libgupnp-dlna/gupnp-dlna-profile.h>
#include <gst/pbutils/pbutils.h>
#include <libxml/xmlmemory.h>
#include <stdlib.h>

static void usage (void)
{
        g_print ("Usage: dlna-profile-parser file1 file2 ... dir1 dir2 ...\n");
}

static void print_caps (const GstCaps *caps)
{
        int i;

        for (i = 0; i < gst_caps_get_size (caps); i++) {
                GstStructure *structure = gst_caps_get_structure (caps, i);
                gchar *tmp = gst_structure_to_string (structure);

                g_print ("%s`- %s\n", i ? "    " : "", tmp);

                g_free (tmp);
        }
}

static void print_profile (GUPnPDLNAProfile *profile, gpointer unused)
{
        GstEncodingProfile *enc_profile;
        const GList *tmp;
        gchar *caps_str;

        enc_profile = gupnp_dlna_profile_get_encoding_profile (profile);
        tmp = gst_encoding_container_profile_get_profiles (GST_ENCODING_CONTAINER_PROFILE (enc_profile));
        caps_str = gst_caps_to_string ((GstCaps *)gst_encoding_profile_get_format (enc_profile));

        g_print ("Loaded DLNA Profile: %s, %s - format %s\n",
                 gupnp_dlna_profile_get_name (profile),
                 gupnp_dlna_profile_get_mime (profile),
                 caps_str);

        while (tmp) {
                print_caps (gst_encoding_profile_get_format
                                        (GST_ENCODING_PROFILE(tmp->data)));
                tmp = tmp->next;
        }

        g_print ("\n");
        g_free (caps_str);
        gst_encoding_profile_unref (enc_profile);
}

static void
free_restrictions_struct (gpointer data, gpointer user_data)
{
        GUPnPDLNARestrictions *restr = (GUPnPDLNARestrictions *)data;
        if (restr) {
                if (restr->caps)
                        gst_caps_unref (restr->caps);

                g_free (restr);
        }
}

int
main (int argc, char **argv)
{
        GList *profiles = NULL;
        GUPnPDLNALoadState *data;
        gboolean relaxed_mode = FALSE;
        gboolean extended_mode = FALSE;
        GError *err = NULL;
        gint i;

        GOptionEntry options[] = {
                {"relaxed mode", 'r', 0, G_OPTION_ARG_NONE, &relaxed_mode,
                 "Enable Relaxed mode", NULL},
                {"extended mode", 'e', 0, G_OPTION_ARG_NONE, &extended_mode,
                 "Enable extended mode", NULL},
                {NULL}
        };

        GOptionContext *ctx;

        if (!g_thread_supported ())
                g_thread_init (NULL);

        ctx = g_option_context_new (" - test to parse dlna profiles");
        g_option_context_add_main_entries (ctx, options, NULL);
        g_option_context_add_group (ctx, gst_init_get_option_group ());

        if (!g_option_context_parse (ctx, &argc, &argv, &err)) {

                g_print ("Error initializing: %s\n", err->message);
                exit (1);
        }

        g_option_context_free (ctx);

        gst_init (&argc, &argv);

        if (argc < 2) {
                usage ();
                return EXIT_FAILURE;
        }

        data = g_new (GUPnPDLNALoadState, 1);

        data->restrictions = g_hash_table_new_full (g_str_hash,
                                                    g_str_equal,
                                                    (GDestroyNotify) xmlFree,
                                                    (GDestroyNotify)
                                                    free_restrictions_struct);
        data->profile_ids = g_hash_table_new_full (g_str_hash,
                                                   g_str_equal,
                                                   (GDestroyNotify) xmlFree,
                                                   (GDestroyNotify)
                                                   g_object_unref);
        data->files_hash = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  NULL);

        data->relaxed_mode = relaxed_mode;
        data->extended_mode = extended_mode;

        for (i = 1; i < argc; i++) {
                GList *tmp;

                if (g_file_test (argv[i], G_FILE_TEST_IS_DIR))
                        tmp = gupnp_dlna_load_profiles_from_dir (argv[i],
                                                                 data);
                else
                        tmp = gupnp_dlna_load_profiles_from_file (argv[i],
                                                                  data);

                profiles = g_list_concat (profiles, tmp);
        }

        g_list_foreach (profiles, (GFunc)print_profile, NULL);
        g_list_foreach (profiles, (GFunc)g_object_unref, NULL);

        g_hash_table_unref (data->restrictions);
        g_hash_table_unref (data->profile_ids);
        g_hash_table_unref (data->files_hash);
        g_free (data);
        data = NULL;
        return EXIT_SUCCESS;
}
