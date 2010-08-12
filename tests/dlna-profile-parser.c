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

#include <libgupnp-dlna/gupnp-dlna-load.h>
#include <libgupnp-dlna/gupnp-dlna-profile.h>
#include <gst/profile/gstprofile.h>
#include <stdlib.h>

static void usage (void)
{
        g_print ("Usage: dlna-profile-parser file1 file2 ... dir1 dir2 ...\n");
}

static void print_caps (GstCaps *caps)
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
        const GstEncodingProfile *enc_profile;
        GList *tmp;
        gchar *caps_str;

        enc_profile = gupnp_dlna_profile_get_encoding_profile (profile);
        tmp = enc_profile->encodingprofiles;
        caps_str = gst_caps_to_string (enc_profile->format);

        g_print ("Loaded DLNA Profile: %s, %s - format %s\n",
                 gupnp_dlna_profile_get_name (profile),
                 gupnp_dlna_profile_get_mime (profile),
                 caps_str);

        while (tmp) {
                print_caps (((GstStreamEncodingProfile *) tmp->data)->format);
                tmp = tmp->next;
        }

        g_print ("\n");
        g_free (caps_str);
}

int
main (int argc, char **argv)
{
        GList *profiles = NULL;
        GHashTable *restrictions, *profile_ids, *files_hash;
        gint i;

        g_thread_init (NULL);
        gst_init (&argc, &argv);

        if (argc < 2) {
                usage ();
                return EXIT_FAILURE;
        }

        restrictions = g_hash_table_new_full (g_str_hash,
                                              g_str_equal,
                                              (GDestroyNotify) xmlFree,
                                              (GDestroyNotify)
                                              gst_stream_encoding_profile_free);
        profile_ids = g_hash_table_new_full (g_str_hash,
                                              g_str_equal,
                                              (GDestroyNotify) xmlFree,
                                              (GDestroyNotify)
                                              gst_encoding_profile_free);
        files_hash = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            NULL);


        for (i = 1; i < argc; i++) {
                GList *tmp;

                if (g_file_test (argv[i], G_FILE_TEST_IS_DIR))
                        tmp = gupnp_dlna_load_profiles_from_dir (argv[i],
                                                                 files_hash);
                else
                        tmp = gupnp_dlna_load_profiles_from_file (argv[i],
                                                                  restrictions,
                                                                  profile_ids,
                                                                  files_hash);

                profiles = g_list_concat (profiles, tmp);
        }

        g_list_foreach (profiles, (GFunc)print_profile, NULL);
        g_list_foreach (profiles, (GFunc)g_object_unref, NULL);

        g_hash_table_unref (restrictions);
        g_hash_table_unref (profile_ids);
        g_hash_table_unref (files_hash);
        return EXIT_SUCCESS;
}
