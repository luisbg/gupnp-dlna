


#include "profile-enumtypes.h"

#include "gstprofile.h"

/* enumerations from "gstprofile.h" */
GType
gst_encoding_profile_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const GEnumValue values[] = {
      { GST_ENCODING_PROFILE_UNKNOWN, "GST_ENCODING_PROFILE_UNKNOWN", "unknown" },
      { GST_ENCODING_PROFILE_VIDEO, "GST_ENCODING_PROFILE_VIDEO", "video" },
      { GST_ENCODING_PROFILE_AUDIO, "GST_ENCODING_PROFILE_AUDIO", "audio" },
      { GST_ENCODING_PROFILE_TEXT, "GST_ENCODING_PROFILE_TEXT", "text" },
      { GST_ENCODING_PROFILE_IMAGE, "GST_ENCODING_PROFILE_IMAGE", "image" },
      { 0, NULL, NULL }
    };
    GType g_define_type_id = g_enum_register_static ("GstEncodingProfileType", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}



