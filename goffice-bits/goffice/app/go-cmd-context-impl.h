#ifndef GO_CMD_CONTEXT_IMPL_H
#define GO_CMD_CONTEXT_IMPL_H

#include <goffice/app/go-cmd-context.h>

G_BEGIN_DECLS

typedef struct {
	GTypeInterface base;

	char *  (*get_password)		(GOCmdContext *gcc,
					 char const *filename);
	void    (*set_sensitive)	(GOCmdContext *gcc,
					 gboolean sensitive);
	struct {
		void (*error)		(GOCmdContext *gcc, GError *err);
		void (*error_info)  	(GOCmdContext *gcc, GOErrorInfo *err);
	} error;

	void    (*progress_set)		(GOCmdContext *gcc, double val);
	void    (*progress_message_set)	(GOCmdContext *gcc, gchar const *msg);

  /* This should really go into the error struct! */
        void    (*error_info_list)  	(GOCmdContext *gcc, GSList *errs);
} GOCmdContextClass;

#define GO_CMD_CONTEXT_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), GO_TYPE_CMD_CONTEXT, GOCmdContextClass))
#define GO_IS_CMD_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), GO_TYPE_CMD_CONTEXT))

/* protected, these do not really belong here, they are associated with io-context */
void  go_cmd_context_progress_set	  (GOCmdContext *gcc, double f);
void  go_cmd_context_progress_message_set (GOCmdContext *gcc, char const *msg);

G_END_DECLS

#endif /* GO_CMD_CONTEXT_IMPL_H */
