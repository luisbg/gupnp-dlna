


#include "gstdiscoverer-enumtypes.h"

#include "gstdiscoverer.h"

/* enumerations from "gstdiscoverer.h" */
GType
gst_stream_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { GST_STREAM_CONTAINER, "GST_STREAM_CONTAINER", "container" },
      { GST_STREAM_AUDIO, "GST_STREAM_AUDIO", "audio" },
      { GST_STREAM_VIDEO, "GST_STREAM_VIDEO", "video" },
      { GST_STREAM_IMAGE, "GST_STREAM_IMAGE", "image" },
      { GST_STREAM_UNKNOWN, "GST_STREAM_UNKNOWN", "unknown" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("GstStreamType", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}
GType
gst_discoverer_result_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GFlagsValue values[] = {
      { GST_DISCOVERER_OK, "GST_DISCOVERER_OK", "ok" },
      { GST_DISCOVERER_URI_INVALID, "GST_DISCOVERER_URI_INVALID", "uri-invalid" },
      { GST_DISCOVERER_ERROR, "GST_DISCOVERER_ERROR", "error" },
      { GST_DISCOVERER_TIMEOUT, "GST_DISCOVERER_TIMEOUT", "timeout" },
      { GST_DISCOVERER_BUSY, "GST_DISCOVERER_BUSY", "busy" },
      { GST_DISCOVERER_MISSING_PLUGINS, "GST_DISCOVERER_MISSING_PLUGINS", "missing-plugins" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_flags_register_static ("GstDiscovererResult", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}



