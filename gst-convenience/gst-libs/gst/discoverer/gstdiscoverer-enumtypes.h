


#ifndef __GST_DISCOVERER_ENUM_TYPES_H__
#define __GST_DISCOVERER_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* enumerations from "gstdiscoverer.h" */
GType gst_stream_type_get_type (void);
#define GST_TYPE_STREAM_TYPE (gst_stream_type_get_type())
GType gst_discoverer_result_get_type (void);
#define GST_TYPE_DISCOVERER_RESULT (gst_discoverer_result_get_type())
G_END_DECLS

#endif /* __GST_DISCOVERER_ENUM_TYPES_H__ */



