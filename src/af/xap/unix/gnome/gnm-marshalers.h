
#ifndef __gnm__MARSHAL_H__
#define __gnm__MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:OBJECT (/dev/stdin:23) */
extern void gnm__BOOLEAN__OBJECT (GClosure     *closure,
                                  GValue       *return_value,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint,
                                  gpointer      marshal_data);

/* BOOLEAN:POINTER (/dev/stdin:24) */
extern void gnm__BOOLEAN__POINTER (GClosure     *closure,
                                   GValue       *return_value,
                                   guint         n_param_values,
                                   const GValue *param_values,
                                   gpointer      invocation_hint,
                                   gpointer      marshal_data);

/* INT:INT (/dev/stdin:25) */
extern void gnm__INT__INT (GClosure     *closure,
                           GValue       *return_value,
                           guint         n_param_values,
                           const GValue *param_values,
                           gpointer      invocation_hint,
                           gpointer      marshal_data);

/* POINTER:INT,INT (/dev/stdin:26) */
extern void gnm__POINTER__INT_INT (GClosure     *closure,
                                   GValue       *return_value,
                                   guint         n_param_values,
                                   const GValue *param_values,
                                   gpointer      invocation_hint,
                                   gpointer      marshal_data);

/* POINTER:VOID (/dev/stdin:27) */
extern void gnm__POINTER__VOID (GClosure     *closure,
                                GValue       *return_value,
                                guint         n_param_values,
                                const GValue *param_values,
                                gpointer      invocation_hint,
                                gpointer      marshal_data);

/* VOID:BOOLEAN (/dev/stdin:28) */
#define gnm__VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

/* VOID:INT (/dev/stdin:29) */
#define gnm__VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:INT,INT (/dev/stdin:30) */
extern void gnm__VOID__INT_INT (GClosure     *closure,
                                GValue       *return_value,
                                guint         n_param_values,
                                const GValue *param_values,
                                gpointer      invocation_hint,
                                gpointer      marshal_data);

/* VOID:OBJECT (/dev/stdin:31) */
#define gnm__VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT

/* VOID:POINTER (/dev/stdin:32) */
#define gnm__VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:POINTER,BOOLEAN,BOOLEAN,BOOLEAN (/dev/stdin:33) */
extern void gnm__VOID__POINTER_BOOLEAN_BOOLEAN_BOOLEAN (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:STRING (/dev/stdin:34) */
#define gnm__VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:VOID (/dev/stdin:35) */
#define gnm__VOID__VOID	g_cclosure_marshal_VOID__VOID

G_END_DECLS

#endif /* __gnm__MARSHAL_H__ */

