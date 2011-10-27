#ifndef GO_ERROR_INFO_H
#define GO_ERROR_INFO_H

#include <goffice/app/goffice-app.h>
#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	GO_WARNING = 1,
	GO_ERROR
} GOSeverity;

GOErrorInfo *go_error_info_new_str			(char const *msg);
GOErrorInfo *go_error_info_new_printf		(char const *msg_format, ...) G_GNUC_PRINTF (1, 2);
GOErrorInfo *go_error_info_new_vprintf		(GOSeverity severity,
						 char const *msg_format,
						 va_list args);
GOErrorInfo *go_error_info_new_str_with_details	(char const *msg, GOErrorInfo *details);
GOErrorInfo *go_error_info_new_str_with_details_list (char const *msg, GSList *details);
GOErrorInfo *go_error_info_new_from_error_list	(GSList *errors);
GOErrorInfo *go_error_info_new_from_errno		(void);
void	   go_error_info_add_details		(GOErrorInfo *error, GOErrorInfo *details);
void	   go_error_info_add_details_list	(GOErrorInfo *error, GSList *details);
void	   go_error_info_free			(GOErrorInfo *error);
void	   go_error_info_print			(GOErrorInfo *error);
char const*go_error_info_peek_message		(GOErrorInfo *error);
GSList	  *go_error_info_peek_details		(GOErrorInfo *error);
GOSeverity go_error_info_peek_severity		(GOErrorInfo *error);

#define GO_INIT_RET_ERROR_INFO(ret_error) \
G_STMT_START { \
	g_assert (ret_error != NULL); \
	*ret_error = NULL; \
} G_STMT_END

G_END_DECLS

#endif /* GO_ERROR_INFO_H */
