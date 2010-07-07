/* GStreamer
 * Copyright (C) <2007> Wim Taymans <wim.taymans@gmail.com>
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
#include <string.h>

#include "gstfactorylists.h"

typedef struct
{
  GstFactoryListType type;
  GstRank minrank;
} FilterData;

/* function used to sort element features. We first sort on the rank, then
 * on the element name (to get a consistent, predictable list) */
static gint
compare_ranks (GValue * v1, GValue * v2)
{
  gint diff;
  GstPluginFeature *f1, *f2;

  f1 = g_value_get_object (v1);
  f2 = g_value_get_object (v2);

  diff = f2->rank - f1->rank;
  if (diff != 0)
    return diff;

  diff = strcmp (f2->name, f1->name);

  return diff;
}


/**
 * gst_factory_list_is_type:
 * @factory: a #GstElementFactory
 * @type: a #GstFactoryListType
 *
 * Check if @factory if of the given types.
 *
 * Returns: %TRUE if @factory is of @type.
 */
gboolean
gst_factory_list_is_type (GstElementFactory * factory, GstFactoryListType type)
{
  gboolean res = FALSE;
  const gchar *klass;

  klass = gst_element_factory_get_klass (factory);

  /* Filter by element type first, as soon as it matches
   * one type, we skip all other tests */
  if (!res && (type & GST_FACTORY_LIST_SINK))
    res = (strstr (klass, "Sink") != NULL);

  if (!res && (type & GST_FACTORY_LIST_SRC))
    res = (strstr (klass, "Source") != NULL);

  if (!res && (type & GST_FACTORY_LIST_DECODER))
    res = (strstr (klass, "Decoder") != NULL);

  if (!res && (type & GST_FACTORY_LIST_ENCODER))
    res = (strstr (klass, "Encoder") != NULL);

  if (!res && (type & GST_FACTORY_LIST_MUXER))
    res = (strstr (klass, "Muxer") != NULL);

  if (!res && (type & GST_FACTORY_LIST_DEMUXER))
    res = (strstr (klass, "Demux") != NULL);

  if (!res && (type & GST_FACTORY_LIST_PARSER))
    res = ((strstr (klass, "Parse") != NULL)
        && (strstr (klass, "Codec") != NULL));

  if (!res && (type & GST_FACTORY_LIST_DEPAYLOADER))
    res = (strstr (klass, "Depayload") != NULL);


  /* Filter by media type now, we only test if it
   * matched any of the types above. */
  if (res && (type & (GST_FACTORY_LIST_AUDIO | GST_FACTORY_LIST_VIDEO)))
    res = ((type & GST_FACTORY_LIST_AUDIO) && (strstr (klass, "Audio") != NULL))
        || ((type & GST_FACTORY_LIST_VIDEO)
        && (strstr (klass, "Video") != NULL));

  return res;
}

static gboolean
element_filter (GstPluginFeature * feature, FilterData * data)
{
  gboolean res;

  /* we only care about element factories */
  if (G_UNLIKELY (!GST_IS_ELEMENT_FACTORY (feature)))
    return FALSE;

  res = (gst_plugin_feature_get_rank (feature) >= data->minrank) &&
      gst_factory_list_is_type (GST_ELEMENT_FACTORY_CAST (feature), data->type);

  return res;
}

/**
 * gst_factory_list_get_elements:
 * @type: a #GstFactoryListType
 * @minrank: Minimum rank
 *
 * Get a list of factories that match the given @type. Only elements
 * with a rank greater or equal to @minrank will be returned.
 * The list of factories is returned by decreasing rank.
 *
 * Returns: a #GValueArray of #GstElementFactory elements. Use
 * g_value_array_free() after usage.
 */
GValueArray *
gst_factory_list_get_elements (GstFactoryListType type, GstRank minrank)
{
  GValueArray *result;
  GList *walk, *list;
  FilterData data;

  /* prepare type */
  data.type = type;
  data.minrank = minrank;

  /* get the feature list using the filter */
  list = gst_default_registry_feature_filter ((GstPluginFeatureFilter)
      element_filter, FALSE, &data);

  result = g_value_array_new (g_list_length (list));

  /* convert to an array */
  for (walk = list; walk; walk = g_list_next (walk)) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY_CAST (walk->data);
    GValue val = { 0, };

    g_value_init (&val, G_TYPE_OBJECT);
    g_value_set_object (&val, factory);
    g_value_array_append (result, &val);
    g_value_unset (&val);
  }
  gst_plugin_feature_list_free (list);

  /* sort on rank and name */
  g_value_array_sort (result, (GCompareFunc) compare_ranks);

  return result;
}

/**
 * gst_factory_list_debug:
 * @array: an array of element factories
 *
 * Debug the element factory names in @array.
 */
void
gst_factory_list_debug (GValueArray * array)
{
#ifndef GST_DISABLE_GST_DEBUG
  gint i;

  for (i = 0; i < array->n_values; i++) {
    GValue *value;
    GstPluginFeature *feature;

    value = g_value_array_get_nth (array, i);
    feature = g_value_get_object (value);

    GST_DEBUG ("%s", gst_plugin_feature_get_name (feature));
  }
#endif
}

/**
 * gst_factory_list_filter:
 * @array: a #GValueArray to filter
 * @caps: a #GstCaps
 * @direction: a #GstPadDirection to filter on
 * @subsetonly: whether to filter on caps subsets or not.
 *
 * Filter out all the elementfactories in @array that can handle @caps in
 * the given direction.
 *
 * If @subsetonly is %TRUE, then only the elements whose pads templates
 * are a complete superset of @caps will be returned. Else any element
 * whose pad templates caps can intersect with @caps will be returned.
 *
 * Returns: a #GValueArray of #GstElementFactory elements that match the
 * given requisits. Use g_value_array_free() after usage.
 */
GValueArray *
gst_factory_list_filter (GValueArray * array,
    const GstCaps * caps, GstPadDirection direction, gboolean subsetonly)
{
  GValueArray *result;
  gint i;

  result = g_value_array_new (0);

  GST_DEBUG ("finding factories");

  /* loop over all the factories */
  for (i = 0; i < array->n_values; i++) {
    GValue *value;
    GstElementFactory *factory;
    const GList *templates;
    GList *walk;

    value = g_value_array_get_nth (array, i);
    factory = g_value_get_object (value);

    GST_DEBUG ("Trying %s",
        gst_plugin_feature_get_name ((GstPluginFeature *) factory));

    /* get the templates from the element factory */
    templates = gst_element_factory_get_static_pad_templates (factory);
    for (walk = (GList *) templates; walk; walk = g_list_next (walk)) {
      GstStaticPadTemplate *templ = walk->data;

      /* we only care about the sink templates */
      if (templ->direction == direction) {
        GstCaps *tmpl_caps;

        /* try to intersect the caps with the caps of the template */
        tmpl_caps = gst_static_caps_get (&templ->static_caps);

        /* FIXME, intersect is not the right method, we ideally want to check
         * for a subset here */

        /* check if the intersection is empty */
        if ((subsetonly && gst_caps_is_subset (caps, tmpl_caps)) ||
            (!subsetonly && gst_caps_can_intersect (caps, tmpl_caps))) {
          /* non empty intersection, we can use this element */
          GValue resval = { 0, };
          g_value_init (&resval, G_TYPE_OBJECT);
          g_value_set_object (&resval, factory);
          g_value_array_append (result, &resval);
          g_value_unset (&resval);
          gst_caps_unref (tmpl_caps);
          break;
        }
        gst_caps_unref (tmpl_caps);
      }
    }
  }
  return result;
}


/*
 * Caps listing convenience functions
 */

static gboolean
remove_range_foreach (GQuark field_id, const GValue * value, GstStructure * st)
{
  GType ftype = G_VALUE_TYPE (value);
  const gchar *fname;

  if (ftype == GST_TYPE_INT_RANGE || ftype == GST_TYPE_DOUBLE_RANGE ||
      ftype == GST_TYPE_FRACTION_RANGE) {
    gst_structure_remove_field (st, g_quark_to_string (field_id));
    return FALSE;
  }

  fname = g_quark_to_string (field_id);

  /* if (strstr (fname, "framerate") || strstr (fname, "pixel-aspect-ratio") || */
  /*     strstr (fname, "rate")) { */
  /*   gst_structure_remove_field (st, g_quark_to_string (field_id)); */
  /*   return FALSE; */
  /* } */

  return TRUE;
}

static void
clear_caps (GstCaps * caps, GValueArray * resarray)
{
  GstCaps *res, *res2;
  GstStructure *st;
  guint i;

  res = gst_caps_make_writable (caps);

  GST_WARNING ("incoming caps %" GST_PTR_FORMAT, res);

  /* Remove width/height/framerate/depth/width fields */
  for (i = gst_caps_get_size (res); i; i--) {
    st = gst_caps_get_structure (res, i - 1);

    /* Remove range fields */
    while (!gst_structure_foreach (st,
            (GstStructureForeachFunc) remove_range_foreach, st));
  }

  GST_WARNING ("stripped %" GST_PTR_FORMAT, res);

  /* Explode to individual structures */
  res2 = gst_caps_normalize (res);
  gst_caps_unref (res);

  GST_WARNING ("normalized %" GST_PTR_FORMAT, res2);

  /* And append to list without duplicates */
  while ((st = gst_caps_steal_structure (res2, 0))) {
    GstCaps *tmpc;
    gboolean duplicate = FALSE;
    guint i;

    /* Skip fake codecs/containers */
    if (gst_structure_has_name (st, "audio/x-raw-int") ||
        gst_structure_has_name (st, "audio/x-raw-float") ||
        gst_structure_has_name (st, "video/x-raw-yuv") ||
        gst_structure_has_name (st, "video/x-raw-rgb") ||
        gst_structure_has_name (st, "unknown/unknown")) {
      gst_structure_free (st);
      continue;
    }

    tmpc = gst_caps_new_full (st, NULL);

    /* Check if the caps are already present */
    GST_DEBUG ("resarray n_value %d", resarray->n_values);
    for (i = 0; i < resarray->n_values; i++) {
      const GstCaps *cval =
          gst_value_get_caps (g_value_array_get_nth (resarray, i));

      GST_DEBUG ("tmpc %" GST_PTR_FORMAT, tmpc);
      GST_DEBUG ("cva %" GST_PTR_FORMAT, cval);

      if (gst_caps_is_equal (tmpc, cval)) {
        duplicate = TRUE;
        break;
      }
    }

    if (!duplicate) {
      GValue resval = { 0, };
      g_value_init (&resval, GST_TYPE_CAPS);

      GST_DEBUG ("appending %" GST_PTR_FORMAT, tmpc);

      gst_value_set_caps (&resval, tmpc);
      g_value_array_append (resarray, &resval);
      g_value_unset (&resval);
    }

    gst_caps_unref (tmpc);
  }

  gst_caps_unref (res2);
}

static GValueArray *
get_all_caps (GValueArray * array, GstPadDirection direction)
{
  GValueArray *res = NULL;
  guint i;

  res = g_value_array_new (0);

  for (i = 0; i < array->n_values; i++) {
    GValue *value;
    GstElementFactory *factory;
    const GList *templates;
    GList *walk;

    value = g_value_array_get_nth (array, i);
    factory = g_value_get_object (value);

    templates = gst_element_factory_get_static_pad_templates (factory);
    for (walk = (GList *) templates; walk; walk = g_list_next (walk)) {
      GstStaticPadTemplate *templ = walk->data;
      if (templ->direction == direction)
        clear_caps (gst_static_caps_get (&templ->static_caps), res);
    }
  }

  return res;
}

/**
 * gst_caps_list_container_formats:
 * @minrank: The minimum #GstRank
 *
 * Returns an array of #GstCaps corresponding to all the container formats
 * one can mux to on this system.
 *
 * Returns: An array of #GstCaps. Free with #gst_value_array_free when done
 * with it.
 */
GValueArray *
gst_caps_list_container_formats (GstRank minrank)
{
  GValueArray *res;
  GValueArray *muxers;

  muxers = gst_factory_list_get_elements (GST_FACTORY_LIST_MUXER, minrank);
  res = get_all_caps (muxers, GST_PAD_SRC);
  g_value_array_free (muxers);

  return res;
}

static GValueArray *
gst_caps_list_encoding_formats (GstRank minrank)
{
  GValueArray *res;
  GValueArray *encoders;

  encoders = gst_factory_list_get_elements (GST_FACTORY_LIST_ENCODER, minrank);
  res = get_all_caps (encoders, GST_PAD_SRC);
  g_value_array_free (encoders);

  return res;
}

/**
 * gst_caps_list_video_encoding_formats:
 * @minrank: The minimum #GstRank
 *
 * Returns an array of #GstCaps corresponding to all the video formats one
 * can encode to on this system.
 *
 * Returns: An array of #GstCaps. Free with #gst_value_array_free when done
 * with it.
 */
GValueArray *
gst_caps_list_video_encoding_formats (GstRank minrank)
{
  GValueArray *res;
  GValueArray *encoders;

  encoders =
      gst_factory_list_get_elements (GST_FACTORY_LIST_ENCODER |
      GST_FACTORY_LIST_VIDEO, minrank);
  res = get_all_caps (encoders, GST_PAD_SRC);
  g_value_array_free (encoders);

  return res;
}


/**
 * gst_caps_list_audio_encoding_formats:
 * @minrank: The minimum #GstRank
 *
 * Returns an array of #GstCaps corresponding to all the audio formats one
 * can encode to on this system.
 *
 * Returns: An array of #GstCaps. Free with #gst_value_array_free when done
 * with it.
 */
GValueArray *
gst_caps_list_audio_encoding_formats (GstRank minrank)
{
  GValueArray *res;
  GValueArray *encoders;

  encoders =
      gst_factory_list_get_elements (GST_FACTORY_LIST_ENCODER |
      GST_FACTORY_LIST_AUDIO, minrank);
  res = get_all_caps (encoders, GST_PAD_SRC);
  g_value_array_free (encoders);

  return res;
}

/**
 * gst_caps_list_compatible_codecs:
 * @containerformat: A #GstCaps
 * @codecformats: An optional #GValueArray of #GstCaps.
 * @muxers: An optional #GValueArray of muxer #GstElementFactory.
 *
 * Returns an array of #GstCaps corresponding to the audio/video/text formats
 * one can encode to and that can be muxed in the provided @containerformat.
 *
 * If specified, only the #GstCaps contained in @codecformats will be checked
 * against, else all compatible audio/video formats will be returned.
 *
 * If specified, only the #GstElementFactory contained in @muxers will be checked,
 * else all available muxers on the system will be checked.
 *
 * Returns: An array of #GstCaps. Free with #gst_value_array_free when done
 * with it.
 */
GValueArray *
gst_caps_list_compatible_codecs (const GstCaps * containerformat,
    GValueArray * codecformats, GValueArray * muxers)
{
  guint i;
  const GList *templates;
  GstElementFactory *factory;
  GList *walk;
  GValueArray *res;
  GValueArray *tmp;
  gboolean hadmuxers = (muxers != NULL);
  gboolean hadcodecs = (codecformats != NULL);

  GST_WARNING ("containerformat %" GST_PTR_FORMAT
      ", codecformats:%p, muxers:%p", containerformat, codecformats, muxers);

  if (!hadmuxers)
    muxers =
        gst_factory_list_get_elements (GST_FACTORY_LIST_MUXER, GST_RANK_NONE);
  if (!hadcodecs)
    codecformats = gst_caps_list_encoding_formats (GST_RANK_NONE);

  res = g_value_array_new (0);

  /* Get the highest rank muxer matching containerformat */
  tmp = gst_factory_list_filter (muxers, containerformat, GST_PAD_SRC, TRUE);
  GST_WARNING ("n_values %d", tmp->n_values);
  if (G_UNLIKELY (tmp->n_values == 0))
    goto beach;
  factory = g_value_get_object (g_value_array_get_nth (tmp, 0));

  GST_WARNING ("Trying with factory %s",
      gst_element_factory_get_longname (factory));

  /* Match all muxer sink pad templates against the available codec formats */
  templates = gst_element_factory_get_static_pad_templates (factory);
  for (walk = (GList *) templates; walk; walk = walk->next) {
    GstStaticPadTemplate *templ = walk->data;

    if (templ->direction == GST_PAD_SINK) {
      GstCaps *templ_caps;

      templ_caps = gst_static_caps_get (&templ->static_caps);

      GST_WARNING ("templ_caps %" GST_PTR_FORMAT, templ_caps);
      for (i = 0; i < codecformats->n_values; i++) {
        const GstCaps *match =
            gst_value_get_caps (g_value_array_get_nth (codecformats, i));

        GST_WARNING ("Trying match %" GST_PTR_FORMAT, match);

        if (gst_caps_can_intersect (match, templ_caps)) {
          GValue resval = { 0, };
          GST_WARNING ("matches");
          g_value_init (&resval, GST_TYPE_CAPS);
          gst_value_set_caps (&resval, match);
          g_value_array_append (res, &resval);
          g_value_unset (&resval);
        }
      }
      gst_caps_unref (templ_caps);
    }
  }


beach:
  g_value_array_free (tmp);
  if (!hadmuxers)
    g_value_array_free (muxers);
  if (!hadcodecs)
    g_value_array_free (codecformats);

  return res;
}
