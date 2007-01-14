#ifndef _UCHAR_H
# define _UCHAR_H
#include <string.h>
#include <memory.h>
#include "basics.h"
#include "charset.h"

extern unichar_t *u_copy(const unichar_t*);
extern unichar_t *u_copyn(const unichar_t*, long);
extern unichar_t *uc_copyn(const char *, int);
extern unichar_t *uc_copy(const char*);
extern unichar_t *u_concat(const unichar_t*,const unichar_t*);
extern char      *cu_copyn(const unichar_t *pt,int len);
extern char      *cu_copy(const unichar_t*);

extern long uc_strcmp(const unichar_t *,const char *);
extern long u_strcmp(const unichar_t *, const unichar_t *);
extern long uc_strncmp(const unichar_t *,const char *,int);
extern long u_strncmp(const unichar_t *, const unichar_t *,int);
extern long uc_strmatch(const unichar_t *,const char *);
extern long uc_strnmatch(const unichar_t *,const char *,int);
extern long u_strnmatch(const unichar_t *str1, const unichar_t *str2, int len);
extern long u_strmatch(const unichar_t *, const unichar_t *);
extern void uc_strcpy(unichar_t *, const char *);
extern void cu_strcpy(char *, const unichar_t *);
extern void u_strcpy(unichar_t *, const unichar_t *);
extern void u_strncpy(unichar_t *, const unichar_t *,int);
extern void cu_strncpy(char *to, const unichar_t *from, int len);
extern void uc_strncpy(unichar_t *to, const char *from, int len);
extern void uc_strcat(unichar_t *, const char *);
extern void uc_strncat(unichar_t *, const char *,int len);
extern void cu_strcat(char *, const unichar_t *);
extern void cu_strncat(char *, const unichar_t *,int len);
extern void u_strcat(unichar_t *, const unichar_t *);
extern void u_strncat(unichar_t *, const unichar_t *, int len);
extern int  u_strlen(const unichar_t *);
extern unichar_t *u_strchr(const unichar_t *,unichar_t);
extern unichar_t *u_strrchr(const unichar_t *,unichar_t);
extern unichar_t *uc_strstr(const unichar_t *,const char *);
extern unichar_t *u_strstr(const unichar_t *,const unichar_t *);
extern unichar_t *uc_strstrmatch(const unichar_t *,const char *);
extern unichar_t *u_strstrmatch(const unichar_t *,const unichar_t *);

extern char *u_to_cb(char *,const unichar_t *, int);
extern unichar_t *c_to_ub(unichar_t *,const char *, int);
extern char *u_to_c(const unichar_t *);
extern unichar_t *c_to_u(const char *);

extern unichar_t *deftou_copy(const char *);		/* Converts def charset to unicode */
extern char *utodef_copy(const unichar_t *);		/* Converts unicode to def charset */

extern long   u_strtol(const unichar_t *,unichar_t **,int);
extern double u_strtod(const unichar_t *,unichar_t **);

extern char *strstart(const char *initial,const char *full);
extern char *strstartmatch(const char *initial,const char *full);
extern unichar_t *u_strstartmatch(const unichar_t *initial, const unichar_t *full);
extern int   strmatch(const char *,const char *);
extern char *strstrmatch(const char *,const char *);

extern unichar_t *pcumatch(const char *key,const unichar_t *str);
unichar_t *pumatch(const unichar_t *key,const unichar_t *str);

extern unichar_t *charset2u_strncpy(unichar_t *uto, const char *from, int n, enum charset cs);
extern char *u2charset_strncpy(char *to, const unichar_t *ufrom, int n, enum charset cs);
extern unichar_t *def2u_strncpy(unichar_t *uto, const char *from, int n);
extern char *u2def_strncpy(char *to, const unichar_t *ufrom, int n);
extern unichar_t *def2u_copy(const char *from);
extern char *u2def_copy(const unichar_t *ufrom);

extern char *copy(const char *);
extern char *copyn(const char *,long);

#endif
