#ifdef GTYPE_DEF_H
#undef GTYPE_DEF_H

#undef gchar
#undef gshort
#undef glong
#undef gint
#undef gboolean

#undef guchar
#undef gushort
#undef gulong
#undef guint

#undef gfloat
#undef gdouble

#undef gpointer
#undef gconstpointer

#undef gint8
#undef guint8
#undef gint16
#undef guint16
#undef gint32
#undef guint32

#ifdef FRIBIDI_INTERNAL_INCLUDE

#ifdef FRIBIDI_DEFINED_TRUE
#undef TRUE
#undef FRIBIDI_DEFINED_TRUE
#endif

#ifdef FRIBIDI_DEFINED_FALSE
#undef FALSE
#undef FRIBIDI_DEFINED_FALSE
#endif

#undef USE_SIMPLE_MALLOC
#undef g_malloc
#undef g_new
#undef g_free

#endif /* fribidi internal */
#endif
